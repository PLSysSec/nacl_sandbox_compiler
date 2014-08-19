/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_PUBLIC_CHROME_MAIN_H_
#define NATIVE_CLIENT_SRC_PUBLIC_CHROME_MAIN_H_ 1

#include "native_client/src/include/nacl_base.h"
#include "native_client/src/include/portability.h"
/*
 * nacl_imc_c.h is used to define NaClHandle.  This should eventually
 * go away when Chromium's use of SRPC is removed
 * (http://crbug.com/239656).
 */
#include "native_client/src/shared/imc/nacl_imc_c.h"

EXTERN_C_BEGIN

struct NaClApp;
struct NaClValidationCache;


/*
 * This interface may be used as follows:
 *
 *   #if OS_POSIX
 *   NaClChromeMainSetUrandomFd(urandom_fd);
 *   #endif
 *   NaClChromeMainInit();
 *   // The following may be done in any order:
 *   struct NaClApp *nap = NaClAppCreate();
 *   struct NaClChromeMainArgs *args = NaClChromeMainArgsCreate();
 *   // Fill out args...
 *   NaClAppSetDesc(nap, NACL_CHROME_DESC_BASE, NaClDescMakeCustomDesc(...));
 *   int exit_status;
 *   int ok = NaClChromeMainStart(nap, args, &exit_status);
 *   if (!ok)
 *     NaClExit(1);
 *   NaClExit(exit_status);
 */

/*
 * Embedders of NaCl may use descriptor numbers of
 * NACL_CHROME_DESC_BASE and higher when setting up a NaClApp's
 * initial descriptors using NaClAppSetDesc().
 *
 * This number is chosen so as not to conflict with
 * NACL_SERVICE_PORT_DESCRIPTOR, NACL_SERVICE_ADDRESS_DESCRIPTOR and
 * export_addr_to inside NaClChromeMainStart().
 */
#define NACL_CHROME_DESC_BASE 6


struct NaClChromeMainArgs {
  /*
   * Handle for bootstrapping a NaCl IMC connection to the trusted
   * PPAPI plugin.  Required.
   */
  NaClHandle imc_bootstrap_handle;

  /*
   * File descriptor for the NaCl integrated runtime (IRT) library.
   * Note that this is a file descriptor even on Windows (where file
   * descriptors are emulated by the C runtime library).
   * Optional; may be -1.  Optional when loading nexes that don't follow
   * NaCl's stable ABI, such as the PNaCl translator.
   */
  int irt_fd;

  /* Whether to enable untrusted hardware exception handling.  Boolean. */
  int enable_exception_handling;

  /* Whether to enable NaCl's built-in GDB RSP debug stub.  Boolean. */
  int enable_debug_stub;

  /* Whether to enable NaCl's dynamic code system calls.  Boolean. */
  int enable_dyncode_syscalls;

  /* Whether or not the app is a PNaCl app. Boolean. */
  int pnacl_mode;

  /*
   * Maximum size of the initially loaded nexe's code segment, in
   * bytes.  0 for no limit, which is the default.
   *
   * This is intended for security hardening.  It reduces the
   * proportion of address space that can contain attacker-controlled
   * executable code.  It reduces the chance of a spraying attack
   * succeeding if there is a vulnerability that allows jumping into
   * the middle of an instruction.  Note that setting a limit here is
   * only useful if enable_dyncode_syscalls is false.
   */
  uint32_t initial_nexe_max_code_bytes;

#if NACL_LINUX || NACL_OSX
  /*
   * Server socket that will be used by debug stub to accept connections
   * from NaCl GDB.  This socket descriptor has already had bind() and listen()
   * called on it.  Optional; may be -1.
   */
  int debug_stub_server_bound_socket_fd;
#endif

#if NACL_WINDOWS
  /*
   * Callback called when debug stub port is known.  Optional; may be NULL.
   */
  void (*debug_stub_server_port_selected_handler_func)(uint16_t port);
#endif

  /*
   * Callback to use for creating shared memory objects.  Optional;
   * may be NULL.
   */
  NaClCreateMemoryObjectFunc create_memory_object_func;

  /* Cache for NaCl validation judgements.  Optional; may be NULL. */
  struct NaClValidationCache *validation_cache;

#if NACL_WINDOWS
  /*
   * Callback to use instead of DuplicateHandle() for copying a
   * Windows handle to another process.  Optional; may be NULL.
   */
  NaClBrokerDuplicateHandleFunc broker_duplicate_handle_func;

  /*
   * Callback to use for requesting that a debug exception handler be
   * attached to this process for handling hardware exceptions via the
   * Windows debug API.  The data in info/info_size must be passed to
   * NaClDebugExceptionHandlerRun().  Optional; may be NULL.
   */
  int (*attach_debug_exception_handler_func)(const void *info,
                                             size_t info_size);
#endif

#if NACL_LINUX || NACL_OSX
  /*
   * The result of sysconf(_SC_NPROCESSORS_ONLN).  The Chrome
   * outer-sandbox prevents the glibc implementation of sysconf from
   * working -- which just reads /proc/cpuinfo or similar file -- so
   * instead, the launcher should fill this in.  In principle this is
   * optional and may be -1, but this will make
   * sysconf(_SC_NPROCESSORS_ONLN) fail and result in some NaCl
   * modules failing.
   *
   * NB: sysconf(_SC_NPROCESSORS_ONLN) is the number of processors
   * on-line and not the same as sysconf(_SC_NPROCESSORS_CONF) -- the
   * former is possibly dynamic on systems with hotpluggable CPUs,
   * whereas the configured number of processors -- what the kernel is
   * configured to be able to handle or the number of processors
   * potentially available.  Setting number_of_cores below would
   * result in reporting a static value, rather than a potentially
   * changing, dynamic value.
   *
   * We are unlikely to ever run on hotpluggable multiprocessor
   * systems.
   */
  int number_of_cores;
#endif

#if NACL_LINUX
  /*
   * Size of address space reserved at address zero onwards for the
   * sandbox.  This is optional and may be 0 if no address space has
   * been reserved, though some sandboxes (such as ARM) might fail in
   * that case.
   */
  size_t prereserved_sandbox_size;
#endif

  /*
   * Descriptor for the user nexe module to load and run. This is optional and
   * may be NULL if SRPC is used for module loading.
   */
  struct NaClDesc *nexe_desc;
};

#if NACL_LINUX || NACL_OSX
/*
 * Sets a file descriptor for /dev/urandom for reading random data.
 * This takes ownership of the file descriptor.  This is intended for
 * use inside an outer sandbox where NaCl may not be able to open()
 * /dev/urandom.
 *
 * If this is called, it must be called before NaClChromeMainInit(),
 * otherwise NaClChromeMainInit() will try to open() /dev/urandom.
 */
void NaClChromeMainSetUrandomFd(int urandom_fd);
#endif

/* Initialize NaCl.  This must be called before NaClAppCreate(). */
void NaClChromeMainInit(void);

/*
 * Sets a function to be called when a fatal error is logged. When the passed
 * function is invoked, recent log messages will be passed in the data
 * parameter, and its length in the bytes parameter.
 * This function is only safe to call after NaClChromeMainInit().
 *
 * If NaClSetFatalErrorCallback() is not called, recent log messages will be
 * written to the IMC bootstrap channel on a fatal error.
 */
void NaClSetFatalErrorCallback(void (*func)(const char *data, size_t bytes));

/* Create a new args struct containing default values. */
struct NaClChromeMainArgs *NaClChromeMainArgsCreate(void);

/*
 * Start NaCl. This does not return.
 * TODO(teravest): Remove this.
 */
void NaClChromeMainStartApp(struct NaClApp *nap,
                            struct NaClChromeMainArgs *args);

/*
 * Start NaCl.
 * On success, returns 1 and sets exit_status to the value that the application
 * passed to _exit().
 * Returns 0 if the application fails to start.
 */
int NaClChromeMainStart(struct NaClApp *nap,
                        struct NaClChromeMainArgs *args,
                        int *exit_status);

/*
 * NaClExit() is for doing a graceful exit, when no internal errors
 * have been detected, when the caller wants to return a well-defined
 * exit status.
 *
 * This is safer than exit(), which does some teardown that can cause running
 * threads to crash.
 */
void NaClExit(int code);

EXTERN_C_END

#endif
