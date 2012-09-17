/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * NaCl Secure Runtime
 */
#include "native_client/src/include/portability_string.h"
#include "native_client/src/trusted/service_runtime/nacl_signal.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_rt.h"
#include "native_client/src/trusted/service_runtime/arch/arm/sel_ldr_arm.h"

void NaClInitGlobals() {
   NaClLog(2, "NaClInitGlobals\n");
  /* intentionally left empty */
}


int NaClThreadContextCtor(struct NaClThreadContext  *ntcp,
                          struct NaClApp            *nap,
                          nacl_reg_t                prog_ctr,
                          nacl_reg_t                stack_ptr,
                          nacl_reg_t                tls_idx) {
  /*
   * This is set by NaClTlsAllocate before we get here, so don't wipe it.
   */
  uint32_t r9 = ntcp->r9;

  UNREFERENCED_PARAMETER(nap);

  /*
   * We call this function so that it does not appear to be dead code,
   * although it only contains compile-time assertions.
   */
  NaClThreadContextOffsetCheck();

  memset((void *)ntcp, 0, sizeof(*ntcp));
  ntcp->stack_ptr = stack_ptr;
  ntcp->prog_ctr = prog_ctr;
  ntcp->tls_idx = tls_idx;
  ntcp->r9 = r9;

  /*
   * Save the system's state of the FPSCR so we can restore
   * the same state when returning to trusted code.
   */
  __asm__ __volatile__("vmrs %0, fpscr" : "=r" (ntcp->sys_fpscr));

  NaClLog(4, "user.tls_idx: 0x%08"NACL_PRIxNACL_REG"\n", tls_idx);
  NaClLog(4, "user.stack_ptr: 0x%08"NACL_PRIxNACL_REG"\n", ntcp->stack_ptr);
  NaClLog(4, "user.prog_ctr: 0x%08"NACL_PRIxNACL_REG"\n", ntcp->prog_ctr);

  return 1;
}


uintptr_t NaClGetThreadCtxSp(struct NaClThreadContext  *th_ctx) {
  return (uintptr_t) th_ctx->stack_ptr;
}


void NaClSetThreadCtxSp(struct NaClThreadContext  *th_ctx, uintptr_t sp) {
  th_ctx->stack_ptr = (uint32_t) sp;
}


void NaClThreadContextToSignalContext(const struct NaClThreadContext *th_ctx,
                                      struct NaClSignalContext *sig_ctx) {
  sig_ctx->r0        = 0;
  sig_ctx->r1        = 0;
  sig_ctx->r2        = 0;
  sig_ctx->r3        = 0;
  sig_ctx->r4        = th_ctx->r4;
  sig_ctx->r5        = th_ctx->r5;
  sig_ctx->r6        = th_ctx->r6;
  sig_ctx->r7        = th_ctx->r7;
  sig_ctx->r8        = th_ctx->r8;
  sig_ctx->r9        = th_ctx->r9;
  sig_ctx->r10       = th_ctx->r10;
  sig_ctx->r11       = th_ctx->fp;
  sig_ctx->r12       = 0;
  sig_ctx->stack_ptr = th_ctx->stack_ptr;
  sig_ctx->lr        = 0;
  sig_ctx->prog_ctr  = th_ctx->new_prog_ctr;
}
