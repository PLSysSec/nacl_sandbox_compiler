/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * NaCl Simple/secure ELF loader (NaCl SEL).
 */
#include "native_client/src/include/portability.h"
#include "native_client/src/include/portability_io.h"

#if NACL_LINUX
#include <getopt.h>
#endif

#if !NACL_WINDOWS
#include <signal.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native_client/src/shared/gio/gio.h"
#include "native_client/src/shared/imc/nacl_imc_c.h"
#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/shared/platform/nacl_exit.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_sync.h"
#include "native_client/src/shared/platform/nacl_sync_checked.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"

#include "native_client/src/trusted/desc/nacl_desc_base.h"
#include "native_client/src/trusted/desc/nacl_desc_io.h"
#include "native_client/src/trusted/fault_injection/fault_injection.h"
#include "native_client/src/trusted/fault_injection/test_injection.h"
#include "native_client/src/trusted/perf_counter/nacl_perf_counter.h"
#include "native_client/src/trusted/service_runtime/env_cleanser.h"
#include "native_client/src/trusted/service_runtime/include/sys/fcntl.h"
#include "native_client/src/trusted/service_runtime/load_file.h"
#include "native_client/src/trusted/service_runtime/nacl_app.h"
#include "native_client/src/trusted/service_runtime/nacl_all_modules.h"
#include "native_client/src/trusted/service_runtime/nacl_bootstrap_channel_error_reporter.h"
#include "native_client/src/trusted/service_runtime/nacl_debug_init.h"
#include "native_client/src/trusted/service_runtime/nacl_error_log_hook.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_runtime_host_interface.h"
#include "native_client/src/trusted/service_runtime/nacl_signal.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_common.h"
#include "native_client/src/trusted/service_runtime/nacl_valgrind_hooks.h"
#include "native_client/src/trusted/service_runtime/osx/mach_exception_handler.h"
#include "native_client/src/trusted/service_runtime/outer_sandbox.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_main_common.h"
#include "native_client/src/trusted/service_runtime/sel_qualify.h"
#include "native_client/src/trusted/service_runtime/win/exception_patch/ntdll_patch.h"
#include "native_client/src/trusted/service_runtime/win/debug_exception_handler.h"


static void (*g_enable_outer_sandbox_func)(void) =
#if NACL_OSX
    NaClEnableOuterSandbox;
#else
    NULL;
#endif

void NaClSetEnableOuterSandboxFunc(void (*func)(void)) {
  g_enable_outer_sandbox_func = func;
}

static void VmentryPrinter(void           *state,
                    struct NaClVmmapEntry *vmep) {
  UNREFERENCED_PARAMETER(state);
  printf("page num 0x%06x\n", (uint32_t)vmep->page_num);
  printf("num pages %d\n", (uint32_t)vmep->npages);
  printf("prot bits %x\n", vmep->prot);
  fflush(stdout);
}

static void PrintVmmap(struct NaClApp  *nap) {
  printf("In PrintVmmap\n");
  fflush(stdout);
  NaClXMutexLock(&nap->mu);
  NaClVmmapVisit(&nap->mem_map, VmentryPrinter, (void *) 0);

  NaClXMutexUnlock(&nap->mu);
}


struct redir {
  struct redir  *next;
  int           nacl_desc;
  enum {
    HOST_DESC,
    IMC_DESC
  }             tag;
  union {
    struct {
      int d;
      int mode;
    }                         host;
    NaClHandle                handle;
    struct NaClSocketAddress  addr;
  } u;
};

int ImportModeMap(char opt) {
  switch (opt) {
    case 'h':
      return O_RDWR;
    case 'r':
      return O_RDONLY;
    case 'w':
      return O_WRONLY;
  }
  fprintf(stderr, ("option %c not understood as a host descriptor"
                   " import mode\n"),
          opt);
  exit(1);
  /* NOTREACHED */
}

static void PrintUsage(void) {
  /* NOTE: this is broken up into multiple statements to work around
           the constant string size limit */
  fprintf(stderr,
          "Usage: sel_ldr [-h d:D] [-r d:D] [-w d:D] [-i d:D]\n"
          "               [-f nacl_file]\n"
          "               [-l log_file]\n"
          "               [-X d] [-acFglQRsSQv]\n"
          "               -- [nacl_file] [args]\n"
          "\n");
  fprintf(stderr,
          " -h\n"
          " -r\n"
          " -w associate a host POSIX descriptor D with app desc d\n"
          "    that was opened in O_RDWR, O_RDONLY, and O_WRONLY modes\n"
          "    respectively\n"
          " -i associates an IMC handle D with app desc d\n"
          " -f file to load; if omitted, 1st arg after \"--\" is loaded\n"
          " -B additional ELF file to load as a blob library\n"
          " -v increases verbosity\n"
          " -X create a bound socket and export the address via an\n"
          "    IMC message to a corresponding inherited IMC app descriptor\n"
          "    (use -1 to create the bound socket / address descriptor\n"
          "    pair, but that no export via IMC should occur)\n");
  fprintf(stderr,
          " -R an RPC supplies the NaCl module.\n"
          "    No nacl_file argument is expected, and the -f flag cannot be\n"
          "    used with this flag.\n"
          "\n"
          " (testing flags)\n"
          " -a allow file access plus some other syscalls! dangerous!\n"
          " -c ignore validator! dangerous! Repeating this option twice skips\n"
          "    validation completely.\n"
          " -F fuzz testing; quit after loading NaCl app\n"
          " -g enable gdb debug stub.  Not secure on x86-64 Windows.\n"
          " -l <file>  write log output to the given file\n"
          " -q quiet; suppress diagnostic/warning messages at startup\n"
          " -Q disable platform qualification (dangerous!)\n"
          " -s safely stub out non-validating instructions\n"
          " -S enable signal handling.  Not supported on Windows.\n"
          " -E <name=value>|<name> set an environment variable\n"
          " -Z use fixed feature x86 CPU mode\n"
          "\n"
          " (For full effect, put -l and -q at the beginning.)\n"
          );  /* easier to add new flags/lines */
}

#if NACL_LINUX
static const struct option longopts[] = {
  { "r_debug", required_argument, NULL, 'D' },
  { "reserved_at_zero", required_argument, NULL, 'z' },
  { NULL, 0, NULL, 0 }
};

static int my_getopt(int argc, char *const *argv, const char *shortopts) {
  return getopt_long(argc, argv, shortopts, longopts, NULL);
}
#else
#define my_getopt getopt
#endif

struct SelLdrOptions {
  char *nacl_file;
  char *blob_library_file;
  int app_argc;
  char **app_argv;

  int quiet;
  int verbosity;
  int fuzzing_quit_after_load;
  int skip_qualification;
  int handle_signals;
  int enable_exception_handling;
  int enable_debug_stub;
  int rpc_supplies_nexe;
  int export_addr_to;
  int debug_mode_bypass_acl_checks;
  int debug_mode_ignore_validator;
  int debug_mode_startup_signal;
  struct redir *redir_queue;
  struct redir **redir_qend;
};

static void SelLdrOptionsCtor(struct SelLdrOptions *options) {
  /* Just to be safe. */
  memset(options, 0, sizeof(*options));

  options->nacl_file = NULL;
  options->blob_library_file = NULL;
  options->app_argc = 0;
  options->app_argv = NULL;

  options->quiet = 0;
  options->verbosity = 0;
  options->fuzzing_quit_after_load = 0;
  options->skip_qualification = 0;
  options->handle_signals = 0;
  options->enable_exception_handling = 0;
  options->enable_debug_stub = 0;
  options->rpc_supplies_nexe = 0;
  options->export_addr_to = -1;
  options->debug_mode_bypass_acl_checks = 0;
  options->debug_mode_ignore_validator = 0;
  options->debug_mode_startup_signal = 0;
  options->redir_queue = NULL;
  options->redir_qend = &(options->redir_queue);
}

/* TODO(ncbray): do not directly set fields on NaClApp. */
static void NaClSelLdrParseArgs(int argc, char **argv,
                                struct SelLdrOptions *options,
                                struct DynArray *env_vars,
                                struct NaClApp *nap) {
  int opt;
  char *rest;
  struct redir *entry;

  options->verbosity = NaClLogGetVerbosity();

  /*
   * On platforms with glibc getopt, require POSIXLY_CORRECT behavior,
   * viz, no reordering of the arglist -- stop argument processing as
   * soon as an unrecognized argument is encountered, so that, for
   * example, in the invocation
   *
   *   sel_ldr foo.nexe -vvv
   *
   * the -vvv flags are made available to the nexe, rather than being
   * consumed by getopt.  This makes the behavior of the Linux build
   * of sel_ldr consistent with the Windows and OSX builds.
   */
  while ((opt = my_getopt(argc, argv,
#if NACL_LINUX
                       "+D:z:"
#endif
                       "aB:cdeE:f:Fgh:i:l:qQr:RsSvw:X:Z")) != -1) {
    switch (opt) {
      case 'a':
        if (!options->quiet)
          fprintf(stderr, "DEBUG MODE ENABLED (bypass acl)\n");
        options->debug_mode_bypass_acl_checks = 1;
        break;
      case 'B':
        options->blob_library_file = optarg;
        break;
      case 'c':
        ++(options->debug_mode_ignore_validator);
        break;
      case 'd':
        options->debug_mode_startup_signal = 1;
        break;
#if NACL_LINUX
      case 'D':
        NaClHandleRDebug(optarg, argv[0]);
        break;
#endif
      case 'e':
        options->enable_exception_handling = 1;
        break;
      case 'E':
        /*
         * For simplicity, we treat the environment variables as a
         * list of strings rather than a key/value mapping.  We do not
         * try to prevent duplicate keys or require the strings to be
         * of the form "KEY=VALUE".  This is in line with how execve()
         * works in Unix.
         *
         * We expect that most callers passing "-E" will either pass
         * in a fixed list or will construct the list using a
         * high-level language, in which case de-duplicating keys
         * outside of sel_ldr is easier.  However, we could do
         * de-duplication here if it proves to be worthwhile.
         */
        if (!DynArraySet(env_vars, env_vars->num_entries, optarg)) {
          NaClLog(LOG_FATAL, "Adding item to env_vars failed\n");
        }
        break;
      case 'f':
        options->nacl_file = optarg;
        break;
      case 'F':
        options->fuzzing_quit_after_load = 1;
        break;

      case 'g':
        options->enable_debug_stub = 1;
        break;

      case 'h':
      case 'r':
      case 'w':
        /* import host descriptor */
        entry = malloc(sizeof *entry);
        if (NULL == entry) {
          fprintf(stderr, "No memory for redirection queue\n");
          exit(1);
        }
        entry->next = NULL;
        entry->nacl_desc = strtol(optarg, &rest, 0);
        entry->tag = HOST_DESC;
        entry->u.host.d = strtol(rest+1, (char **) 0, 0);
        entry->u.host.mode = ImportModeMap(opt);
        *(options->redir_qend) = entry;
        options->redir_qend = &entry->next;
        break;
      case 'i':
        /* import IMC handle */
        entry = malloc(sizeof *entry);
        if (NULL == entry) {
          fprintf(stderr, "No memory for redirection queue\n");
          exit(1);
        }
        entry->next = NULL;
        entry->nacl_desc = strtol(optarg, &rest, 0);
        entry->tag = IMC_DESC;
        entry->u.handle = (NaClHandle) strtol(rest+1, (char **) 0, 0);
        *(options->redir_qend) = entry;
        options->redir_qend = &entry->next;
        break;
      case 'l':
        if (NULL != optarg) {
          /*
           * change stdout/stderr to log file now, so that subsequent error
           * messages will go there.  unfortunately, error messages that
           * result from getopt processing -- usually out-of-memory, which
           * shouldn't happen -- won't show up.
           */
          NaClLogSetFile(optarg);
        }
        break;
      case 'q':
        options->quiet = 1;
        break;
      case 'Q':
        if (!options->quiet)
          fprintf(stderr, "PLATFORM QUALIFICATION DISABLED BY -Q - "
                  "Native Client's sandbox will be unreliable!\n");
        options->skip_qualification = 1;
        break;
      case 'R':
        options->rpc_supplies_nexe = 1;
        break;
      /* case 'r':  with 'h' and 'w' above */
      case 's':
        if (nap->validator->stubout_mode_implemented) {
          nap->validator_stub_out_mode = 1;
        } else {
           NaClLog(LOG_WARNING, "stub_out_mode is not supported, disabled\n");
        }
        break;
      case 'S':
        options->handle_signals = 1;
        break;
      case 'v':
        ++(options->verbosity);
        NaClLogIncrVerbosity();
        break;
      /* case 'w':  with 'h' and 'r' above */
      case 'X':
        options->export_addr_to = strtol(optarg, (char **) 0, 0);
        break;
#if NACL_LINUX
      case 'z':
        NaClHandleReservedAtZero(optarg);
        break;
#endif
      case 'Z':
        if (nap->validator->readonly_text_implemented) {
          NaClLog(LOG_WARNING, "Enabling Fixed-Feature CPU Mode\n");
          nap->fixed_feature_cpu_mode = 1;
          if (!nap->validator->FixCPUFeatures(nap->cpu_features)) {
            NaClLog(LOG_ERROR,
                    "This CPU lacks features required by "
                    "fixed-function CPU mode.\n");
            exit(1);
          }
        } else {
           NaClLog(LOG_ERROR, "fixed_feature_cpu_mode is not supported\n");
           exit(1);
        }
        break;
      default:
        fprintf(stderr, "ERROR: unknown option: [%c]\n\n", opt);
        PrintUsage();
        exit(-1);
    }
  }

  /* Post process the options. */

  if (options->debug_mode_ignore_validator == 1) {
    if (!options->quiet)
      fprintf(stderr, "DEBUG MODE ENABLED (ignore validator)\n");
  } else if (options->debug_mode_ignore_validator > 1) {
    if (!options->quiet)
      fprintf(stderr, "DEBUG MODE ENABLED (skip validator)\n");
  }

  if (options->verbosity) {
    int         ix;
    char const  *separator = "";

    fprintf(stderr, "sel_ldr argument list:\n");
    for (ix = 0; ix < argc; ++ix) {
      fprintf(stderr, "%s%s", separator, argv[ix]);
      separator = " ";
    }
    putc('\n', stderr);
  }

  if (options->rpc_supplies_nexe) {
    if (NULL != options->nacl_file) {
      fprintf(stderr,
              "sel_ldr: mutually exclusive flags -f and -R both used\n");
      exit(1);
    }
    /* post: NULL == nacl_file */
    if (options->export_addr_to < 0) {
      fprintf(stderr,
              "sel_ldr: -R requires -X to set up secure command channel\n");
      exit(1);
    }
  } else {
    if (NULL == options->nacl_file && optind < argc) {
      options->nacl_file = argv[optind];
      ++optind;
    }
    if (NULL == options->nacl_file) {
      fprintf(stderr, "No nacl file specified\n");
      exit(1);
    }
    /* post: NULL != nacl_file */
  }
  /*
   * post condition established by the above code (in Hoare logic
   * terminology):
   *
   * NULL == nacl_file iff rpc_supplies_nexe
   *
   * so hence forth, testing !rpc_supplies_nexe suffices for
   * establishing NULL != nacl_file.
   */
  CHECK((NULL == options->nacl_file) == options->rpc_supplies_nexe);

  /* to be passed to NaClMain, eventually... */
  if (NULL != options->nacl_file && options->debug_mode_bypass_acl_checks) {
    argv[--optind] = options->nacl_file;
  } else {
    argv[--optind] = (char *) "NaClMain";
  }

  options->app_argc = argc - optind;
  options->app_argv = argv + optind;

  /*
   * NACL_DANGEROUS_SKIP_QUALIFICATION_TEST is used by tsan / memcheck
   * (see src/third_party/valgrind/).
   */
  if (!options->skip_qualification &&
      getenv("NACL_DANGEROUS_SKIP_QUALIFICATION_TEST") != NULL) {
    if (!options->quiet)
      fprintf(stderr, "PLATFORM QUALIFICATION DISABLED BY ENVIRONMENT - "
              "Native Client's sandbox will be unreliable!\n");
    options->skip_qualification = 1;
  }

  if (getenv("NACL_UNTRUSTED_EXCEPTION_HANDLING") != NULL) {
    options->enable_exception_handling = 1;
  }
}

static void RedirectIO(struct NaClApp *nap, struct redir *redir_queue){
  struct redir *entry;
  /*
   * Execute additional I/O redirections.  NB: since the NaClApp
   * takes ownership of host / IMC socket descriptors, all but
   * the first run will not get access if the NaClApp closes
   * them.  Currently a normal NaClApp process exit does not
   * close descriptors, since the underlying host OS will do so
   * as part of service runtime exit.
   */
  NaClLog(4, "Processing I/O redirection/inheritance from command line\n");
  for (entry = redir_queue; NULL != entry; entry = entry->next) {
    switch (entry->tag) {
      case HOST_DESC:
        NaClAddHostDescriptor(nap, entry->u.host.d,
                              entry->u.host.mode, entry->nacl_desc);
        break;
      case IMC_DESC:
        NaClAddImcHandle(nap, entry->u.handle, entry->nacl_desc);
        break;
    }
  }
}

int NaClSelLdrMain(int argc, char **argv) {
  struct NaClApp                *nap = NULL;
  struct SelLdrOptions          optionsImpl;
  struct SelLdrOptions          *options = &optionsImpl;

  NaClErrorCode                 errcode = LOAD_INTERNAL;
  struct NaClDesc               *blob_file = NULL;

  int                           ret_code;

  struct DynArray               env_vars;
  struct NaClEnvCleanser        env_cleanser;
  char const *const             *envp;

  struct NaClPerfCounter        time_all_main;


  ret_code = 1;

  NaClAllModulesInit();

  /*
   * If this is a secondary process spun up to assist windows exception
   * handling, the following function will not return.  If this is a normal
   * sel_ldr process, the following function does nothing.
   */
  NaClDebugExceptionHandlerStandaloneHandleArgs(argc, argv);

  nap = NaClAppCreate();
  if (nap == NULL) {
    NaClLog(LOG_FATAL, "NaClAppCreate() failed\n");
  }

  NaClBootstrapChannelErrorReporterInit();
  NaClErrorLogHookInit(NaClBootstrapChannelErrorReporter, nap);

  NaClPerfCounterCtor(&time_all_main, "SelMain");

  fflush((FILE *) NULL);

  SelLdrOptionsCtor(options);
  if (!DynArrayCtor(&env_vars, 0)) {
    NaClLog(LOG_FATAL, "Failed to allocate env var array\n");
  }
  NaClSelLdrParseArgs(argc, argv, options, &env_vars, nap);

  /*
   * Define the environment variables for untrusted code.
   */
  if (!DynArraySet(&env_vars, env_vars.num_entries, NULL)) {
    NaClLog(LOG_FATAL, "Adding env_vars NULL terminator failed\n");
  }
  NaClEnvCleanserCtor(&env_cleanser, 0);
  if (!NaClEnvCleanserInit(&env_cleanser, NaClGetEnviron(),
          (char const *const *)env_vars.ptr_array)) {
    NaClLog(LOG_FATAL, "Failed to initialise env cleanser\n");
  }
  envp = NaClEnvCleanserEnvironment(&env_cleanser);

  if (options->debug_mode_startup_signal) {
#if NACL_WINDOWS
    NaClLog(LOG_FATAL, "DEBUG startup signal not supported on Windows\n");
#else
    /*
     * SIGCONT is ignored by default, so this doesn't actually do anything
     * by itself.  The purpose of raising the signal is to get a debugger
     * to stop and inspect the process before it does anything else.  When
     * sel_ldr is started via nacl_helper_bootstrap, it needs to run as far
     * as doing its option processing and calling NaClHandleRDebug before
     * the debugger will understand the association between the address
     * space and the sel_ldr binary and its dependent shared libraries.
     * When the debugger stops for the signal, the hacker can run the
     * "sharedlibrary" command (if the debugger is GDB) and thereafter
     * it becomes possible to set symbolic breakpoints and so forth.
     */
    NaClLog(LOG_ERROR, "DEBUG taking startup signal (SIGCONT) now\n");
    raise(SIGCONT);
#endif
  }

  if (options->debug_mode_bypass_acl_checks) {
    NaClInsecurelyBypassAllAclChecks();
  }

  nap->ignore_validator_result = (options->debug_mode_ignore_validator > 0);
  nap->skip_validator = (options->debug_mode_ignore_validator > 1);
  nap->enable_exception_handling = options->enable_exception_handling;

  /*
   * TODO(mseaborn): Always enable the Mach exception handler on Mac
   * OS X, and remove handle_signals and sel_ldr's "-S" option.
   */
  if (nap->enable_exception_handling || options->enable_debug_stub ||
      (options->handle_signals && NACL_OSX)) {
#if NACL_WINDOWS
    nap->attach_debug_exception_handler_func =
        NaClDebugExceptionHandlerStandaloneAttach;
#elif NACL_LINUX
    /* NaCl's signal handler is always enabled on Linux. */
#elif NACL_OSX
    if (!NaClInterceptMachExceptions()) {
      NaClLog(LOG_ERROR, "ERROR setting up Mach exception interception.\n");
      return -1;
    }
#else
# error Unknown host OS
#endif
  }

  errcode = LOAD_OK;

  /*
   * in order to report load error to the browser plugin through the
   * secure command channel, we do not immediate jump to cleanup code
   * on error.  rather, we continue processing (assuming earlier
   * errors do not make it inappropriate) until the secure command
   * channel is set up, and then bail out.
   */

  /*
   * Ensure the platform qualification checks pass.
   */
  if (!options->skip_qualification) {
    NaClErrorCode pq_error = NACL_FI_VAL("pq", NaClErrorCode,
                                         NaClRunSelQualificationTests());
    if (LOAD_OK != pq_error) {
      errcode = pq_error;
      nap->module_load_status = pq_error;
      if (!options->quiet)
        NaClLog(LOG_ERROR, "Error while loading \"%s\": %s\n",
                NULL != options->nacl_file ? options->nacl_file
                                  : "(no file, to-be-supplied-via-RPC)",
                NaClErrorString(errcode));
    }
  }

#if NACL_LINUX
  NaClSignalHandlerInit();
#endif
  /*
   * Patch the Windows exception dispatcher to be safe in the case of
   * faults inside x86-64 sandboxed code.  The sandbox is not secure
   * on 64-bit Windows without this.
   */
#if (NACL_WINDOWS && NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86 && \
     NACL_BUILD_SUBARCH == 64)
  NaClPatchWindowsExceptionDispatcher();
#endif
  NaClSignalTestCrashOnStartup();

  /*
   * Open both files first because (on Mac OS X at least)
   * NaClAppLoadFile() enables an outer sandbox.
   */
  if (NULL != options->blob_library_file) {
    NaClFileNameForValgrind(options->blob_library_file);
    blob_file = (struct NaClDesc *) NaClDescIoDescOpen(
        options->blob_library_file, NACL_ABI_O_RDONLY, 0);
    if (NULL == blob_file) {
      perror("sel_main");
      NaClLog(LOG_FATAL, "Cannot open \"%s\".\n", options->blob_library_file);
    }
    NaClPerfCounterMark(&time_all_main, "SnapshotBlob");
    NaClPerfCounterIntervalLast(&time_all_main);
  }

  NaClAppInitialDescriptorHookup(nap);

  if (!options->rpc_supplies_nexe) {
    if (LOAD_OK == errcode) {
      NaClLog(2, "Loading nacl file %s (non-RPC)\n", options->nacl_file);
      errcode = NaClAppLoadFileFromFilename(nap, options->nacl_file);
      if (LOAD_OK != errcode && !options->quiet) {
        NaClLog(LOG_ERROR, "Error while loading \"%s\": %s\n"
                "Using the wrong type of nexe (nacl-x86-32"
                " on an x86-64 or vice versa)\n"
                "or a corrupt nexe file may be"
                " responsible for this error.\n",
                options->nacl_file,
                NaClErrorString(errcode));
      }
      NaClPerfCounterMark(&time_all_main, "AppLoadEnd");
      NaClPerfCounterIntervalLast(&time_all_main);
    }

    if (options->fuzzing_quit_after_load) {
      exit(0);
    }
  }

  RedirectIO(nap, options->redir_queue);

  /*
   * If export_addr_to is set to a non-negative integer, we create a
   * bound socket and socket address pair and bind the former to
   * descriptor NACL_SERVICE_PORT_DESCRIPTOR (3 [see sel_ldr.h]) and
   * the latter to descriptor NACL_SERVICE_ADDRESS_DESCRIPTOR (4).
   * The socket address is sent to the export_addr_to descriptor.
   *
   * The service runtime also accepts a connection on the bound socket
   * and spawns a secure command channel thread to service it.
   */
  if (0 <= options->export_addr_to) {
    NaClCreateServiceSocket(nap);
    /*
     * LOG_FATAL errors that occur before NaClSetUpBootstrapChannel will
     * not be reported via the crash log mechanism (for Chromium
     * embedding of NaCl, shown in the JavaScript console).
     *
     * Some errors, such as due to NaClRunSelQualificationTests, do not
     * trigger a LOG_FATAL but instead set module_load_status to be sent
     * in the start_module RPC reply.  Log messages associated with such
     * errors would be seen, since NaClSetUpBootstrapChannel will get
     * called.
     */
    NaClSetUpBootstrapChannel(nap, (NaClHandle) options->export_addr_to);
    /*
     * NB: spawns a thread that uses the command channel.  we do
     * this after NaClAppLoadFile so that NaClApp object is more
     * fully populated.  Hereafter any changes to nap should be done
     * while holding locks.
     */
    NaClSecureCommandChannel(nap);
  }

  /*
   * May have created a thread, so need to synchronize uses of nap
   * contents henceforth.
   */

  if (options->rpc_supplies_nexe) {
    NaClErrorCode load_error = NaClWaitForLoadModuleCommand(nap);
    if (load_error != LOAD_OK) {
      errcode = load_error;
    }
    NaClPerfCounterMark(&time_all_main, "WaitForLoad");
    NaClPerfCounterIntervalLast(&time_all_main);
  }

  /*
   * Tell the debug stub to bind a TCP port before enabling the outer
   * sandbox.  This is only needed on Mac OS X since that is the only
   * platform where we have an outer sandbox in standalone sel_ldr.
   * In principle this call should work on all platforms, but Windows
   * XP seems to have some problems when we do bind()/listen() on a
   * separate thread from accept().
   */
  if (options->enable_debug_stub && NACL_OSX) {
    if (!NaClDebugBindSocket()) {
      exit(1);
    }
  }

  /*
   * Enable the outer sandbox, if one is defined.  Do this as soon as
   * possible.
   *
   * This must come after NaClWaitForLoadModuleCommand(), which waits
   * for another thread to have called NaClAppLoadFile().
   * NaClAppLoadFile() does not work inside the Mac outer sandbox in
   * standalone sel_ldr when using a dynamic code area because it uses
   * NaClCreateMemoryObject() which opens a file in /tmp.
   *
   * We cannot enable the sandbox if file access is enabled.
   */
  if (!NaClAclBypassChecks && g_enable_outer_sandbox_func != NULL) {
    g_enable_outer_sandbox_func();
  }

  if (NULL != options->blob_library_file) {
    if (LOAD_OK == errcode) {
      errcode = NaClMainLoadIrt(nap, blob_file, NULL);
      if (LOAD_OK != errcode) {
        NaClLog(LOG_ERROR, "Error while loading \"%s\": %s\n",
                options->blob_library_file,
                NaClErrorString(errcode));
      }
      NaClPerfCounterMark(&time_all_main, "BlobLoaded");
      NaClPerfCounterIntervalLast(&time_all_main);
    }

    NaClDescUnref(blob_file);
  }

  /*
   * Print out a marker for scripts to use to mark the start of app
   * output.
   */
  NaClLog(1, "NACL: Application output follows\n");

  /*
   * Make sure all the file buffers are flushed before entering
   * the application code.
   */
  fflush((FILE *) NULL);

  if (NULL != nap->secure_service) {
    NaClErrorCode start_result;
    /*
     * wait for start_module RPC call on secure channel thread.
     */
    start_result = NaClWaitForStartModuleCommand(nap);
    NaClPerfCounterMark(&time_all_main, "WaitedForStartModuleCommand");
    NaClPerfCounterIntervalLast(&time_all_main);
    if (LOAD_OK == errcode) {
      errcode = start_result;
    }
  } else {
    NaClAppStartModule(nap, NULL, NULL);
  }

  /*
   * error reporting done; can quit now if there was an error earlier.
   */
  if (LOAD_OK != errcode) {
    NaClLog(4,
            "Not running app code since errcode is %s (%d)\n",
            NaClErrorString(errcode),
            errcode);
    goto done;
  }

  if (!NaClAppLaunchServiceThreads(nap)) {
    goto done;
  }
  if (options->enable_debug_stub) {
    if (!NaClDebugInit(nap)) {
      goto done;
    }
  }
  NACL_TEST_INJECTION(BeforeMainThreadLaunches, ());
  if (!NaClCreateMainThread(nap,
                            options->app_argc,
                            options->app_argv,
                            envp)) {
    NaClLog(LOG_FATAL, "creating main thread failed\n");
  }

  /*
   * Clean up temp storage for env vars.
   */
  NaClEnvCleanserDtor(&env_cleanser);
  DynArrayDtor(&env_vars);

  NaClPerfCounterMark(&time_all_main, "CreateMainThread");
  NaClPerfCounterIntervalLast(&time_all_main);

  ret_code = NaClWaitForMainThreadToExit(nap);
  NaClPerfCounterMark(&time_all_main, "WaitForMainThread");
  NaClPerfCounterIntervalLast(&time_all_main);

  NaClPerfCounterMark(&time_all_main, "SelMainEnd");
  NaClPerfCounterIntervalTotal(&time_all_main);

  /*
   * exit_group or equiv kills any still running threads while module
   * addr space is still valid.  otherwise we'd have to kill threads
   * before we clean up the address space.
   */
  NaClExit(ret_code);

 done:
  fflush(stdout);

  if (options->verbosity) {
    printf("Dumping vmmap.\n"); fflush(stdout);
    PrintVmmap(nap);
    fflush(stdout);
  }
  /*
   * If there is a secure command channel, we sent an RPC reply with
   * the reason that the nexe was rejected.  If we exit now, that
   * reply may still be in-flight and the various channel closure (esp
   * reverse channel) may be detected first.  This would result in a
   * crash being reported, rather than the error in the RPC reply.
   * Instead, we wait for the hard-shutdown on the command channel.
   */
  if (LOAD_OK != errcode) {
    NaClBlockIfCommandChannelExists(nap);
  }

  if (options->verbosity > 0) {
    printf("Done.\n");
  }
  fflush(stdout);

#if NACL_LINUX
  NaClSignalHandlerFini();
#endif
  NaClAllModulesFini();

  NaClExit(ret_code);

  /* Unreachable, but having the return prevents a compiler error. */
  return ret_code;
}
