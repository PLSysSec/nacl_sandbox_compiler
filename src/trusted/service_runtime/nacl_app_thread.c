/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * NaCl Server Runtime user thread state.
 */
#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/shared/platform/nacl_sync_checked.h"
#include "native_client/src/trusted/service_runtime/nacl_desc_effector_ldr.h"

#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_oop_debugger_hooks.h"
#include "native_client/src/trusted/service_runtime/nacl_tls.h"
#include "native_client/src/trusted/service_runtime/nacl_switch_to_app.h"
#include "native_client/src/trusted/service_runtime/nacl_stack_safety.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_common.h"


void WINAPI NaClThreadLauncher(void *state) {
  struct NaClAppThread *natp = (struct NaClAppThread *) state;
  NaClLog(4, "NaClThreadLauncher: entered\n");

  NaClSignalStackRegister(natp->signal_stack);

  NaClLog(4, "      natp = 0x%016"NACL_PRIxPTR"\n", (uintptr_t) natp);
  NaClLog(4, " prog_ctr  = 0x%016"NACL_PRIxNACL_REG"\n", natp->user.prog_ctr);
  NaClLog(4, "stack_ptr  = 0x%016"NACL_PRIxPTR"\n",
          NaClGetThreadCtxSp(&natp->user));

  NaClTlsSetIdx(NaClGetThreadIdx(natp));

  /*
   * We have to hold the threads_mu lock until after thread_num field
   * in this thread has been initialized.  All other threads can only
   * find and examine this natp through the threads table, so the fact
   * that natp is not consistent (no thread_num) will not be visible.
   */
  NaClXMutexLock(&natp->nap->threads_mu);
  natp->thread_num = NaClAddThreadMu(natp->nap, natp);
  NaClXMutexUnlock(&natp->nap->threads_mu);

  NaClVmHoleThreadStackIsSafe(natp->nap);

  NaClStackSafetyNowOnUntrustedStack();  /* real soon now! */

  /*
   * Notify the debug stub, that a new thread is availible.
   */
  if (NULL != natp->nap->debug_stub_callbacks) {
    natp->nap->debug_stub_callbacks->thread_create_hook(natp);
  }

  NaClOopDebuggerThreadCreateHook(natp);

  NaClStartThreadInApp(natp, natp->user.prog_ctr);
}

int NaClAppThreadCtor(struct NaClAppThread  *natp,
                      struct NaClApp        *nap,
                      uintptr_t             usr_entry,
                      uintptr_t             usr_stack_ptr,
                      uint32_t              tls_idx,
                      uintptr_t             sys_tls) {
  int                         rv;
  uint64_t                    thread_idx;

  NaClLog(4, "         natp = 0x%016"NACL_PRIxPTR"\n", (uintptr_t) natp);
  NaClLog(4, "          nap = 0x%016"NACL_PRIxPTR"\n", (uintptr_t) nap);
  NaClLog(4, "usr_stack_ptr = 0x%016"NACL_PRIxPTR"\n", usr_stack_ptr);

  NaClThreadContextCtor(&natp->user, nap, usr_entry, usr_stack_ptr, tls_idx);

  natp->signal_stack = NULL;

  if (!NaClMutexCtor(&natp->mu)) {
    return 0;
  }
  if (!NaClCondVarCtor(&natp->cv)) {
    goto cleanup_mutex;
  }

  if (!NaClClosureResultCtor(&natp->result)) {
    goto cleanup_cv;
  }
  natp->sysret = 0;
  natp->nap = nap;

  if (!NaClSignalStackAllocate(&natp->signal_stack)) {
    goto cleanup_cv;
  }

  natp->holding_sr_locks = 0;
  natp->state = NACL_APP_THREAD_ALIVE;

  natp->thread_num = -1;  /* illegal index */
  natp->sys_tls = sys_tls;
  natp->tls2 = 0;

  natp->dynamic_delete_generation = 0;

  thread_idx = NaClGetThreadIdx(natp);

  nacl_thread[thread_idx] = natp;
  nacl_user[thread_idx] = &natp->user;
  nacl_sys[thread_idx] = &natp->sys;

  rv = NaClThreadCtor(&natp->thread,
                      NaClThreadLauncher,
                      (void *) natp,
                      NACL_KERN_STACK_SIZE);
  if (rv != 0) {
    return rv; /* Success */
  }
  NaClClosureResultDtor(&natp->result);
 cleanup_cv:
  NaClCondVarDtor(&natp->cv);
 cleanup_mutex:
  NaClMutexDtor(&natp->mu);
  if (NULL != natp->signal_stack) {
    NaClSignalStackFree(&natp->signal_stack);
    natp->signal_stack = NULL;
  }
  return 0;
}


void NaClAppThreadDtor(struct NaClAppThread *natp) {
  /*
   * the thread must not be still running, else this crashes the system
   */

  /*
   * Notify the debug stub that we are done with this thread
   */
  if (NULL != natp->nap->debug_stub_callbacks) {
    natp->nap->debug_stub_callbacks->thread_exit_hook(natp);
  }

  NaClThreadDtor(&natp->thread);
  NaClSignalStackUnregister();
  NaClSignalStackFree(natp->signal_stack);
  natp->signal_stack = NULL;
  NaClClosureResultDtor(&natp->result);
  NaClTlsFree(natp);
  NaClCondVarDtor(&natp->cv);
  NaClMutexDtor(&natp->mu);
}


int NaClAppThreadAllocSegCtor(struct NaClAppThread  *natp,
                              struct NaClApp        *nap,
                              uintptr_t             usr_entry,
                              uintptr_t             usr_stack_ptr,
                              uintptr_t             sys_tls) {
  uint32_t  tls_idx;

  /*
   * Set this early, in case NaClTlsAllocate wants to examine it.
   */
  natp->nap = nap;

  /*
   * Even though we don't know what segment base/range should gs/r9/nacl_tls_idx
   * select, we still need one, since it identifies the thread when we context
   * switch back.  This use of a dummy tls is only needed for the main thread,
   * which is expected to invoke the tls_init syscall from its crt code (before
   * main or much of libc can run).  Other threads are spawned with the thread
   * pointer address as a parameter.
   */
  tls_idx = NaClTlsAllocate(natp, (void *) sys_tls);

  NaClLog(4,
        "NaClAppThreadAllocSegCtor: stack_ptr 0x%08"NACL_PRIxPTR", "
        "tls_idx 0x%02"NACL_PRIx32"\n",
         usr_stack_ptr, tls_idx);

  if (NACL_TLS_INDEX_INVALID == tls_idx) {
    NaClLog(LOG_ERROR, "No tls for thread, num_thread %d\n", nap->num_threads);
    return 0;
  }

  return NaClAppThreadCtor(natp,
                           nap,
                           usr_entry,
                           usr_stack_ptr,
                           tls_idx,
                           sys_tls);
}
