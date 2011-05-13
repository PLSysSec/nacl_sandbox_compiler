/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * NaCl Simple/secure ELF loader (NaCl SEL).
 */

#include "native_client/src/include/portability.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native_client/src/include/elf_constants.h"
#include "native_client/src/include/nacl_elf.h"
#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/include/win/mman.h"
#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_sync_checked.h"
#include "native_client/src/shared/platform/nacl_time.h"
#include "native_client/src/trusted/perf_counter/nacl_perf_counter.h"

#include "native_client/src/trusted/service_runtime/include/sys/errno.h"

#include "native_client/src/trusted/service_runtime/arch/sel_ldr_arch.h"
#include "native_client/src/trusted/service_runtime/elf_util.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/nacl_closure.h"
#include "native_client/src/trusted/service_runtime/nacl_debug_init.h"
#include "native_client/src/trusted/service_runtime/nacl_sync_queue.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_common.h"
#include "native_client/src/trusted/service_runtime/nacl_text.h"
#include "native_client/src/trusted/service_runtime/outer_sandbox.h"
#include "native_client/src/trusted/service_runtime/sel_memory.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"
#include "native_client/src/trusted/service_runtime/sel_addrspace.h"

#if !defined(SIZE_T_MAX)
# define SIZE_T_MAX     (~(size_t) 0)
#endif


/*
 * Fill from static_text_end to end of that page with halt
 * instruction, which is at least NACL_HALT_LEN in size when no
 * dynamic text is present.  Does not touch dynamic text region, which
 * should be pre-filled with HLTs.
 *
 * By adding NACL_HALT_SLED_SIZE, we ensure that the code region ends
 * with HLTs, just in case the CPU has a bug in which it fails to
 * check for running off the end of the x86 code segment.
 */
void NaClFillEndOfTextRegion(struct NaClApp *nap) {
  size_t page_pad;

  /*
   * NOTE: make sure we are not silently overwriting data.  It is the
   * toolchain's responsibility to ensure that a NACL_HALT_SLED_SIZE
   * gap exists.
   */
  if (0 != nap->data_start &&
      nap->static_text_end + NACL_HALT_SLED_SIZE > nap->data_start) {
    NaClLog(LOG_FATAL, "Missing gap between text and data for halt_sled\n");
  }
  if (0 != nap->rodata_start &&
      nap->static_text_end + NACL_HALT_SLED_SIZE > nap->rodata_start) {
    NaClLog(LOG_FATAL, "Missing gap between text and rodata for halt_sled\n");
  }

  if (NULL == nap->text_shm) {
    /*
     * No dynamic text exists.  Space for NACL_HALT_SLED_SIZE must
     * exist.
     */
    page_pad = (NaClRoundAllocPage(nap->static_text_end + NACL_HALT_SLED_SIZE)
                - nap->static_text_end);
    CHECK(page_pad >= NACL_HALT_SLED_SIZE);
    CHECK(page_pad < NACL_MAP_PAGESIZE + NACL_HALT_SLED_SIZE);
  } else {
    /*
     * Dynamic text exists; the halt sled resides in the dynamic text
     * region, so all we need to do here is to round out the last
     * static text page with HLT instructions.  It doesn't matter if
     * the size of this region is smaller than NACL_HALT_SLED_SIZE --
     * this is just to fully initialize the page, rather than (later)
     * decoding/validating zero-filled memory as instructions.
     */
    page_pad = NaClRoundAllocPage(nap->static_text_end) - nap->static_text_end;
  }

  NaClLog(4,
          "Filling with halts: %08"NACL_PRIxPTR", %08"NACL_PRIxS" bytes\n",
          nap->mem_start + nap->static_text_end,
          page_pad);

  NaClFillMemoryRegionWithHalt((void *)(nap->mem_start + nap->static_text_end),
                               page_pad);

  nap->static_text_end += page_pad;
}

NaClErrorCode NaClAppLoadFile(struct Gio       *gp,
                              struct NaClApp   *nap,
                              enum NaClAbiCheckOption check_abi) {
  NaClErrorCode       ret = LOAD_INTERNAL;
  NaClErrorCode       subret;
  uintptr_t           rodata_end;
  uintptr_t           data_end;
  uintptr_t           max_vaddr;
  struct NaClElfImage *image = NULL;
  struct NaClPerfCounter  time_load_file;

  NaClPerfCounterCtor(&time_load_file, "NaClAppLoadFile");

  /* NACL_MAX_ADDR_BITS < 32 */
  if (nap->addr_bits > NACL_MAX_ADDR_BITS) {
    ret = LOAD_ADDR_SPACE_TOO_BIG;
    goto done;
  }

  nap->stack_size = NaClRoundAllocPage(nap->stack_size);

  /* temporay object will be deleted at end of function */
  image = NaClElfImageNew(gp, &subret);
  if (NULL == image) {
    ret = subret;
    goto done;
  }

#if 0 == NACL_DANGEROUS_DEBUG_MODE_DISABLE_INNER_SANDBOX
  check_abi = NACL_ABI_CHECK_OPTION_CHECK;
#endif

  if (NACL_ABI_CHECK_OPTION_CHECK == check_abi) {
    subret = NaClElfImageValidateAbi(image);
    if (subret != LOAD_OK) {
      ret = subret;
      goto done;
    }
  }

  subret = NaClElfImageValidateElfHeader(image);
  if (LOAD_OK != subret) {
    ret = subret;
    goto done;
  }

  subret = NaClElfImageValidateProgramHeaders(image,
                                              nap->addr_bits,
                                              &nap->static_text_end,
                                              &nap->rodata_start,
                                              &rodata_end,
                                              &nap->data_start,
                                              &data_end,
                                              &max_vaddr);
  if (LOAD_OK != subret) {
    ret = subret;
    goto done;
  }

  if (0 == nap->data_start) {
    if (0 == nap->rodata_start) {
      if (NaClRoundAllocPage(max_vaddr) - max_vaddr < NACL_HALT_SLED_SIZE) {
        /*
         * if no rodata and no data, we make sure that there is space for
         * the halt sled.
         */
        max_vaddr += NACL_MAP_PAGESIZE;
      }
    } else {
      /*
       * no data, but there is rodata.  this means max_vaddr is just
       * where rodata ends.  this might not be at an allocation
       * boundary, and in this the page would not be writable.  round
       * max_vaddr up to the next allocation boundary so that bss will
       * be at the next writable region.
       */
      ;
    }
    max_vaddr = NaClRoundAllocPage(max_vaddr);
  }
  /*
   * max_vaddr -- the break or the boundary between data (initialized
   * and bss) and the address space hole -- does not have to be at a
   * page boundary.
   */
  nap->break_addr = max_vaddr;
  nap->data_end = max_vaddr;

  NaClLog(4, "Values from NaClElfImageValidateProgramHeaders:\n");
  NaClLog(4, "rodata_start = 0x%08"NACL_PRIxPTR"\n", nap->rodata_start);
  NaClLog(4, "rodata_end   = 0x%08"NACL_PRIxPTR"\n", rodata_end);
  NaClLog(4, "data_start   = 0x%08"NACL_PRIxPTR"\n", nap->data_start);
  NaClLog(4, "data_end     = 0x%08"NACL_PRIxPTR"\n", data_end);
  NaClLog(4, "max_vaddr    = 0x%08"NACL_PRIxPTR"\n", max_vaddr);

#if 0 == NACL_DANGEROUS_DEBUG_MODE_DISABLE_INNER_SANDBOX
  nap->bundle_size = NaClElfImageGetBundleSize(image);
  if (nap->bundle_size == 0) {
    ret = LOAD_BAD_ABI;
    goto done;
  }
#else
  /* pick some reasonable default for an un-sandboxed nexe */
  nap->bundle_size = 32;
#endif
  nap->initial_entry_pt = NaClElfImageGetEntryPoint(image);

  NaClLog(2,
          "NaClApp addr space layout:\n");
  NaClLog(2,
          "nap->static_text_end    = 0x%016"NACL_PRIxPTR"\n",
          nap->static_text_end);
  NaClLog(2,
          "nap->dynamic_text_start = 0x%016"NACL_PRIxPTR"\n",
          nap->dynamic_text_start);
  NaClLog(2,
          "nap->dynamic_text_end   = 0x%016"NACL_PRIxPTR"\n",
          nap->dynamic_text_end);
  NaClLog(2,
          "nap->rodata_start       = 0x%016"NACL_PRIxPTR"\n",
          nap->rodata_start);
  NaClLog(2,
          "nap->data_start         = 0x%016"NACL_PRIxPTR"\n",
          nap->data_start);
  NaClLog(2,
          "nap->data_end           = 0x%016"NACL_PRIxPTR"\n",
          nap->data_end);
  NaClLog(2,
          "nap->break_addr         = 0x%016"NACL_PRIxPTR"\n",
          nap->break_addr);
  NaClLog(2,
          "nap->initial_entry_pt   = 0x%016"NACL_PRIxPTR"\n",
          nap->initial_entry_pt);
  NaClLog(2,
          "nap->user_entry_pt      = 0x%016"NACL_PRIxPTR"\n",
          nap->user_entry_pt);
  NaClLog(2,
          "nap->bundle_size        = 0x%x\n",
          nap->bundle_size);

  if (!NaClAddrIsValidEntryPt(nap, nap->initial_entry_pt)) {
    ret = LOAD_BAD_ENTRY;
    goto done;
  }

  /*
   * Basic address space layout sanity check.
   */
  if (0 != nap->data_start) {
    if (data_end != max_vaddr) {
      NaClLog(LOG_INFO, "data segment is not last\n");
      ret = LOAD_DATA_NOT_LAST_SEGMENT;
      goto done;
    }
  } else if (0 != nap->rodata_start) {
    if (NaClRoundAllocPage(rodata_end) != max_vaddr) {
      /*
       * This should be unreachable, but we include it just for
       * completeness.
       *
       * Here is why it is unreachable:
       *
       * NaClPhdrChecks checks the test segment starting address.  The
       * only allowed loaded segments are text, data, and rodata.
       * Thus unless the rodata is in the trampoline region, it must
       * be after the text.  And NaClElfImageValidateProgramHeaders
       * ensures that all segments start after the trampoline region.
       */
      NaClLog(LOG_INFO, "no data segment, but rodata segment is not last\n");
      ret = LOAD_NO_DATA_BUT_RODATA_NOT_LAST_SEGMENT;
      goto done;
    }
  }
  if (0 != nap->rodata_start && 0 != nap->data_start) {
    if (rodata_end > nap->data_start) {
      NaClLog(LOG_INFO, "rodata_overlaps data.\n");
      ret = LOAD_RODATA_OVERLAPS_DATA;
      goto done;
    }
  }
  if (0 != nap->rodata_start) {
    if (NaClRoundAllocPage(NaClEndOfStaticText(nap)) > nap->rodata_start) {
      ret = LOAD_TEXT_OVERLAPS_RODATA;
      goto done;
    }
  } else if (0 != nap->data_start) {
    if (NaClRoundAllocPage(NaClEndOfStaticText(nap)) > nap->data_start) {
      ret = LOAD_TEXT_OVERLAPS_DATA;
      goto done;
    }
  }

  if (0 != nap->rodata_start &&
      NaClRoundAllocPage(nap->rodata_start) != nap->rodata_start) {
    NaClLog(LOG_INFO, "rodata_start not a multiple of allocation size\n");
    ret = LOAD_BAD_RODATA_ALIGNMENT;
    goto done;
  }
  if (0 != nap->data_start &&
      NaClRoundAllocPage(nap->data_start) != nap->data_start) {
    NaClLog(LOG_INFO, "data_start not a multiple of allocation size\n");
    ret = LOAD_BAD_DATA_ALIGNMENT;
    goto done;
  }

  NaClLog(2, "Allocating address space\n");
  subret = NaClAllocAddrSpace(nap);
  if (LOAD_OK != subret) {
    ret = subret;
    goto done;
  }

  /*
   * NB: mem_map object has been initialized, but is empty.
   * NaClMakeDynamicTextShared does not touch it.
   *
   * NaClMakeDynamicTextShared also fills the dynamic memory region
   * with the architecture-specific halt instruction.  If/when we use
   * memory mapping to save paging space for the dynamic region and
   * lazily halt fill the memory as the pages become
   * readable/executable, we must make sure that the *last*
   * NACL_MAP_PAGESIZE chunk is nonetheless mapped and written with
   * halts.
   */
  NaClLog(2,
          ("Replacing gap between static text and"
           " (ro)data with shareable memory\n"));
  subret = NaClMakeDynamicTextShared(nap);
  NaClPerfCounterMark(&time_load_file,
                      NACL_PERF_IMPORTANT_PREFIX "MakeDynText");
  NaClPerfCounterIntervalLast(&time_load_file);
  if (LOAD_OK != subret) {
    ret = subret;
    goto done;
  }
#if NACL_OSX && defined(NACL_STANDALONE)
  /*
   * Enable the outer sandbox on Mac.  Do this as soon as possible.
   *
   * This is needed only when built for "Standalone" mode (which
   * currently means firefox plugin, as opposed to as an built-in
   * plugin in Chrome), since in the Chrome built-in/integrated build
   * the sandbox is enabled earlier, in chrome's zygote process
   * initialization / specialization when the zygote forks a copy of
   * itself to become sel_ldr (via sel_main_chrome.c's
   * NaClMainForChromium).
   *
   * It would be good to do this as soon as we have opened the
   * executable file and log file, but nacl_text.c currently does not
   * work in the sandbox.  See
   * http://code.google.com/p/nativeclient/issues/detail?id=583
   *
   * This is needed for the test version of the ppapi plugin and post
   * saucer separation; the integrated plugin/sel_ldr implementation
   * that spawns from a chrome zygote image would have already enabled
   * the sandbox.
   *
   * We cannot enable the sandbox if file access is enabled.
   *
   * OSX's sandbox has a race condition where mprotect in the address
   * space set up code would sometimes fail, even before nacl_text.c.
   * Ideally we would enable the OSX sandbox before examining any
   * untrusted data (the nexe), but we have to wait until we address
   * space setup is done, which requires reading the ELF headers.
   */
  if (!NaClAclBypassChecks) {
    NaClEnableOuterSandbox();
  }
#endif

  /*
   * Make sure the static image pages are marked writable before we try
   * to write them.
   * TODO(ilewis): See if this can be enabled for Win32 as well. (issue 40077)
   */
  NaClLog(2, "Loading into memory\n");
#if NACL_WINDOWS && NACL_ARCH_CPU_X86_64
  ret = NaCl_mprotect((void*) nap->mem_start,
    NaClRoundAllocPage(nap->data_end),
    PROT_READ|PROT_WRITE);
  if (ret) {
      NaClLog(LOG_FATAL, "Couldn't get writeable pages for image. "
                         "Error code 0x%X\n", ret);
  }
#endif
  subret = NaClElfImageLoad(image, gp, nap->addr_bits, nap->mem_start);
  if (LOAD_OK != subret) {
    ret = subret;
    goto done;
  }

  /*
   * NaClFillEndOfTextRegion will fill with halt instructions the
   * padding space after the static text region.
   *
   * Shm-backed dynamic text space was filled with halt instructions
   * in NaClMakeDynamicTextShared.  This extends to the rodata.  For
   * non-shm-backed text space, this extend to the next page (and not
   * allocation page).  static_text_end is updated to include the
   * padding.
   */
  NaClFillEndOfTextRegion(nap);

#if 0 == NACL_DANGEROUS_DEBUG_MODE_DISABLE_INNER_SANDBOX
  NaClLog(2, "Validating image\n");
  subret = NaClValidateImage(nap);
  NaClPerfCounterMark(&time_load_file,
                      NACL_PERF_IMPORTANT_PREFIX "ValidateImg");
  NaClPerfCounterIntervalLast(&time_load_file);
  if (LOAD_OK != subret) {
    ret = subret;
    goto done;
  }
#endif

  NaClLog(2, "Installing trampoline\n");

  NaClLoadTrampoline(nap);

  NaClLog(2, "Installing springboard\n");

  NaClLoadSpringboard(nap);

  NaClLog(2, "Applying memory protection\n");

  /*
   * NaClMemoryProtect also initializes the mem_map w/ information
   * about the memory pages and their current protection value.
   *
   * The contents of the dynamic text region will get remapped as
   * non-writable.
   */
  subret = NaClMemoryProtection(nap);
  if (LOAD_OK != subret) {
    ret = subret;
    goto done;
  }
  NaClLog(2,
          "NaClAppLoadFile done; addr space layout:\n");
  NaClLog(2,
          "nap->static_text_end    = 0x%016"NACL_PRIxPTR"\n",
          nap->static_text_end);
  NaClLog(2,
          "nap->dynamic_text_start = 0x%016"NACL_PRIxPTR"\n",
          nap->dynamic_text_start);
  NaClLog(2,
          "nap->dynamic_text_end   = 0x%016"NACL_PRIxPTR"\n",
          nap->dynamic_text_end);
  NaClLog(2,
          "nap->rodata_start       = 0x%016"NACL_PRIxPTR"\n",
          nap->rodata_start);
  NaClLog(2,
          "nap->data_start         = 0x%016"NACL_PRIxPTR"\n",
          nap->data_start);
  NaClLog(2,
          "nap->data_end           = 0x%016"NACL_PRIxPTR"\n",
          nap->data_end);
  NaClLog(2,
          "nap->break_addr         = 0x%016"NACL_PRIxPTR"\n",
          nap->break_addr);
  NaClLog(2,
          "nap->initial_entry_pt   = 0x%016"NACL_PRIxPTR"\n",
          nap->initial_entry_pt);
  NaClLog(2,
          "nap->user_entry_pt      = 0x%016"NACL_PRIxPTR"\n",
          nap->user_entry_pt);
  NaClLog(2,
          "nap->bundle_size        = 0x%x\n",
          nap->bundle_size);

  ret = LOAD_OK;
done:
  NaClElfImageDelete(image);

  NaClPerfCounterMark(&time_load_file, "EndLoadFile");
  NaClPerfCounterIntervalTotal(&time_load_file);
  return ret;
}

NaClErrorCode NaClAppLoadFileDynamically(struct NaClApp *nap,
                                         struct Gio     *gio_file) {
  struct NaClElfImage *image = NULL;
  NaClErrorCode ret = LOAD_INTERNAL;

  image = NaClElfImageNew((struct Gio *) gio_file, &ret);
  if (NULL == image) {
    goto done;
  }
  ret = NaClElfImageValidateElfHeader(image);
  if (LOAD_OK != ret) {
    goto done;
  }
  ret = NaClElfImageLoadDynamically(image, nap, gio_file);
  if (LOAD_OK != ret) {
    goto done;
  }
  nap->user_entry_pt = nap->initial_entry_pt;
  nap->initial_entry_pt = NaClElfImageGetEntryPoint(image);

 done:
  NaClElfImageDelete(image);
  return ret;
}

int NaClAddrIsValidEntryPt(struct NaClApp *nap,
                           uintptr_t      addr) {
  if (0 != (addr & (nap->bundle_size - 1))) {
    return 0;
  }

  return addr < nap->static_text_end;
}

/*
 * preconditions:
 * argc > 0, argc and argv table is consistent
 * envv may be NULL (this happens on MacOS/Cocoa
 * if envv is non-NULL it is 'consistent', null terminated etc.
 */
int NaClCreateMainThread(struct NaClApp     *nap,
                         int                argc,
                         char               **argv,
                         char const *const  *envv) {
  /*
   * Compute size of string tables for argv and envv
   */
  int                   retval;
  int                   envc;
  size_t                size;
  int                   auxv_entries;
  size_t                ptr_tbl_size;
  int                   i;
  char                  *p;
  char                  *strp;
  size_t                *argv_len;
  size_t                *envv_len;
  struct NaClAppThread  *natp;
  uintptr_t             stack_ptr;

  /*
   * Set an exception handler so Windows won't show a message box if
   * an exception occurs
   */
  WINDOWS_EXCEPTION_TRY;

  retval = 0;  /* fail */
  CHECK(argc > 0);
  CHECK(NULL != argv);

  envc = 0;
  if (NULL != envv) {
    char const *const *pp;
    for (pp = envv; NULL != *pp; ++pp) {
      ++envc;
    }
  }
  envv_len = 0;
  argv_len = malloc(argc * sizeof argv_len[0]);
  envv_len = malloc(envc * sizeof envv_len[0]);
  if (NULL == argv_len) {
    goto cleanup;
  }
  if (NULL == envv_len && 0 != envc) {
    goto cleanup;
  }

  if (nap->enable_debug_stub) {
    /*
     * Enable the debug stub.
     */
    if (!NaClDebugInit(nap,
                       argc, (char const * const *) argv,
                       envc, (char const * const *) envv)) {
      goto cleanup;
    }
  }

  size = 0;

  /*
   * The following two loops cannot overflow.  The reason for this is
   * that they are counting the number of bytes used to hold the
   * NUL-terminated strings that comprise the argv and envv tables.
   * If the entire address space consisted of just those strings, then
   * the size variable would overflow; however, since there's the code
   * space required to hold the code below (and we are not targetting
   * Harvard architecture machines), at least one page holds code, not
   * data.  We are assuming that the caller is non-adversarial and the
   * code does not look like string data....
   */
  for (i = 0; i < argc; ++i) {
    argv_len[i] = strlen(argv[i]) + 1;
    size += argv_len[i];
  }
  for (i = 0; i < envc; ++i) {
    envv_len[i] = strlen(envv[i]) + 1;
    size += envv_len[i];
  }

  /*
   * NaCl modules are ILP32, so the argv, envv pointers, as well as
   * the terminating NULL pointers at the end of the argv/envv tables,
   * are 32-bit values.  We also have the auxv to take into account.
   * Note that on nacl64, argc is popped, and is an 8-byte value!
   *
   * The argv and envv pointer tables came from trusted code and is
   * part of memory.  Thus, by the same argument above, adding in
   * "ptr_tbl_size" cannot possibly overflow the "size" variable since
   * it is a size_t object.  However, the extra pointers for auxv and
   * the space for argv could cause an overflow.  The fact that we
   * used stack to get here etc means that ptr_tbl_size could not have
   * overflowed.
   *
   * NB: the underlying OS would have limited the amount of space used
   * for argv and envv -- on linux, it is ARG_MAX, or 128KB -- and
   * hence the overflow check is for obvious auditability rather than
   * for correctness.
   */
  auxv_entries = 1;
  if (0 != nap->user_entry_pt) {
    auxv_entries++;
  }
  ptr_tbl_size = (argc + envc + 2 + auxv_entries * 2) * sizeof(uint32_t)
                 + sizeof(nacl_reg_t);

  if (SIZE_T_MAX - size < ptr_tbl_size) {
    NaClLog(LOG_WARNING,
            "NaClCreateMainThread: ptr_tbl_size cause size of"
            " argv / environment copy to overflow!?!\n");
    retval = 0;
    goto cleanup;
  }
  size += ptr_tbl_size;

  size = (size + NACL_STACK_ALIGN_MASK) & ~NACL_STACK_ALIGN_MASK;

  if (size > nap->stack_size) {
    retval = 0;
    goto cleanup;
  }

  /* write strings and char * arrays to stack */
  stack_ptr = (nap->mem_start + ((uintptr_t) 1U << nap->addr_bits) - size);

  NaClLog(2, "setting stack to : %016"NACL_PRIxPTR"\n", stack_ptr);

  VCHECK(0 == (stack_ptr & NACL_STACK_ALIGN_MASK),
         ("stack_ptr not aligned: %016"NACL_PRIxPTR"\n", stack_ptr));

  p = (char *) stack_ptr;
  strp = p + ptr_tbl_size;

#define BLAT(t, v) do { \
    *(t *) p = (t) v; p += sizeof(t);           \
  } while (0);

  BLAT(nacl_reg_t, argc);

  for (i = 0; i < argc; ++i) {
    BLAT(uint32_t, NaClSysToUser(nap, (uintptr_t) strp));
    NaClLog(2, "copying arg %d  %p -> %p\n",
            i, argv[i], strp);
    strcpy(strp, argv[i]);
    strp += argv_len[i];
  }
  BLAT(uint32_t, 0);

  for (i = 0; i < envc; ++i) {
    BLAT(uint32_t, NaClSysToUser(nap, (uintptr_t) strp));
    NaClLog(2, "copying env %d  %p -> %p\n",
            i, envv[i], strp);
    strcpy(strp, envv[i]);
    strp += envv_len[i];
  }
  BLAT(uint32_t, 0);
  /* Push an auxv */
  if (0 != nap->user_entry_pt) {
    BLAT(uint32_t, AT_ENTRY);
    BLAT(uint32_t, nap->user_entry_pt);
  }
  BLAT(uint32_t, AT_NULL);
  BLAT(uint32_t, 0);
#undef BLAT
  CHECK(p == (char *) stack_ptr + ptr_tbl_size);

  /* now actually spawn the thread */
  natp = malloc(sizeof *natp);
  if (!natp) {
    goto cleanup;
  }

  /* We are ready to distinguish crashes in trusted and untrusted code. */
  NaClSignalRegisterApp(nap);

  nap->running = 1;
  nap->threads_launching = 1;

  NaClLog(2, "system stack ptr : %016"NACL_PRIxPTR"\n", stack_ptr);
  NaClLog(2, "  user stack ptr : %016"NACL_PRIxPTR"\n",
          NaClSysToUserStackAddr(nap, stack_ptr));

  /* e_entry is user addr */
  if (!NaClAppThreadAllocSegCtor(natp,
                                 nap,
                                 nap->initial_entry_pt,
                                 NaClSysToUserStackAddr(nap, stack_ptr),
                                 NaClUserToSys(nap, nap->break_addr),
                                 1)) {
    retval = 0;
    goto cleanup;
  }

  /*
   * NB: Hereafter locking is required to access nap.
   */
  retval = 1;
cleanup:
  free(argv_len);
  free(envv_len);

  WINDOWS_EXCEPTION_CATCH;
  return retval;
}

int NaClWaitForMainThreadToExit(struct NaClApp  *nap) {
  struct NaClClosure        *work;

  while (NULL != (work = NaClSyncQueueDequeue(&nap->work_queue))) {
    NaClLog(3, "NaClWaitForMainThreadToExit: got work %08"NACL_PRIxPTR"\n",
            (uintptr_t) work);
    NaClLog(3, " invoking Run fn %08"NACL_PRIxPTR"\n",
            (uintptr_t) work->vtbl->Run);

    (*work->vtbl->Run)(work);
    NaClLog(3, "... done\n");
  }

  NaClLog(3, " taking NaClApp lock\n");
  NaClXMutexLock(&nap->mu);
  NaClLog(3, " waiting for exit status\n");
  while (nap->running) {
    NaClXCondVarWait(&nap->cv, &nap->mu);
    NaClLog(3, " wakeup, nap->running %d, nap->exit_status %d\n",
            nap->running, nap->exit_status);
  }
  NaClXMutexUnlock(&nap->mu);
  /*
   * Some thread invoked the exit (exit_group) syscall.
   */

  if (NULL != nap->debug_stub_callbacks) {
    nap->debug_stub_callbacks->process_exit_hook(nap->exit_status);
  }

  return (nap->exit_status);
}

/*
 * stack_ptr is from syscall, so a 32-bit address.
 */
int32_t NaClCreateAdditionalThread(struct NaClApp *nap,
                                   uintptr_t      prog_ctr,
                                   uintptr_t      sys_stack_ptr,
                                   uintptr_t      sys_tdb,
                                   size_t         tdb_size) {
  struct NaClAppThread  *natp;
  uintptr_t             stack_ptr;

  natp = malloc(sizeof *natp);
  if (NULL == natp) {
    NaClLog(LOG_WARNING,
            ("NaClCreateAdditionalThread: no memory for new thread context."
             "  Returning EAGAIN per POSIX specs.\n"));
    return -NACL_ABI_EAGAIN;
  }

  stack_ptr = NaClSysToUserStackAddr(nap, sys_stack_ptr);

  if (0 != ((stack_ptr + sizeof(nacl_reg_t)) & NACL_STACK_ALIGN_MASK)) {
    NaClLog(3,
            ("NaClCreateAdditionalThread:  user thread library provided "
             "an unaligned user stack pointer: 0x%"NACL_PRIxPTR"\n"),
            stack_ptr);
  }

  if (!NaClAppThreadAllocSegCtor(natp,
                                 nap,
                                 prog_ctr,
                                 stack_ptr,
                                 sys_tdb,
                                 tdb_size)) {
    NaClLog(LOG_WARNING,
            ("NaClCreateAdditionalThread: could not allocate thread index."
             "  Returning EAGAIN per POSIX specs.\n"));
    free(natp);
    return -NACL_ABI_EAGAIN;
  }
  return 0;
}
