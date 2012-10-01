/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "native_client/tests/common/register_set.h"

#include "native_client/src/include/nacl_assert.h"

#if defined(__native_client__)
# include "native_client/src/untrusted/nacl/nacl_thread.h"
#endif


/* Flags readable and writable by untrusted code. */
const uint8_t kX86FlagBits[5] = { 0, 2, 6, 7, 11 };
/*
 * kX86KnownFlagsMask contains kX86FlagBits plus the trap flag (which
 * is not readable or writable by untrusted code) so that trusted-code
 * tests will check that the trap flag is still set.
 */
static const uint32_t kX86KnownFlagsMask =
    (1<<0) | (1<<2) | (1<<6) | (1<<7) | (1<<11) |
    (1<<8); /* Trap flag */

void RegsFillTestValues(struct NaClSignalContext *regs) {
  unsigned int index;
  for (index = 0; index < sizeof(*regs); index++) {
    ((char *) regs)[index] = index + 1;
  }
#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86
  /* Set x86 flags to a value that we know will work. */
  regs->flags = RESET_X86_FLAGS_VALUE;
#endif
}

#if defined(__native_client__)
void RegsApplySandboxConstraints(struct NaClSignalContext *regs) {
#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86 && NACL_BUILD_SUBARCH == 64
  uint64_t r15;
  __asm__("mov %%r15, %0" : "=r"(r15));
  regs->r15 = r15;
  regs->prog_ctr = r15 + (uint32_t) regs->prog_ctr;
  regs->stack_ptr = r15 + (uint32_t) regs->stack_ptr;
  regs->rbp = r15 + (uint32_t) regs->rbp;
#elif NACL_ARCH(NACL_BUILD_ARCH) == NACL_arm
  regs->r9 = (uintptr_t) nacl_tls_get();
#else
  UNREFERENCED_PARAMETER(regs);
#endif
}
#endif

void RegsAssertEqual(const struct NaClSignalContext *actual,
                     const struct NaClSignalContext *expected) {
#define CHECK_REG(regname) ASSERT_EQ(actual->regname, expected->regname)

  /*
   * We do not compare x86 segment registers because they are not
   * saved by REGS_SAVER_FUNC.
   */
#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86 && NACL_BUILD_SUBARCH == 32
  CHECK_REG(eax);
  CHECK_REG(ecx);
  CHECK_REG(edx);
  CHECK_REG(ebx);
  CHECK_REG(stack_ptr);
  CHECK_REG(ebp);
  CHECK_REG(esi);
  CHECK_REG(edi);
  CHECK_REG(prog_ctr);
  ASSERT_EQ(actual->flags & kX86KnownFlagsMask,
            expected->flags & kX86KnownFlagsMask);
#elif NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86 && NACL_BUILD_SUBARCH == 64
  CHECK_REG(rax);
  CHECK_REG(rbx);
  CHECK_REG(rcx);
  CHECK_REG(rdx);
  CHECK_REG(rsi);
  CHECK_REG(rdi);
  CHECK_REG(rbp);
  CHECK_REG(stack_ptr);
  CHECK_REG(r8);
  CHECK_REG(r9);
  CHECK_REG(r10);
  CHECK_REG(r11);
  CHECK_REG(r12);
  CHECK_REG(r13);
  CHECK_REG(r14);
  CHECK_REG(r15);
  CHECK_REG(prog_ctr);
  ASSERT_EQ(actual->flags & kX86KnownFlagsMask,
            expected->flags & kX86KnownFlagsMask);
#elif NACL_ARCH(NACL_BUILD_ARCH) == NACL_arm
  CHECK_REG(r0);
  CHECK_REG(r1);
  CHECK_REG(r2);
  CHECK_REG(r3);
  CHECK_REG(r4);
  CHECK_REG(r5);
  CHECK_REG(r6);
  CHECK_REG(r7);
  CHECK_REG(r8);
  CHECK_REG(r9);
  CHECK_REG(r10);
  CHECK_REG(r11);
  CHECK_REG(r12);
  CHECK_REG(stack_ptr);
  CHECK_REG(lr);
  CHECK_REG(prog_ctr);
  ASSERT_EQ(actual->cpsr & REGS_ARM_USER_CPSR_FLAGS_MASK,
            expected->cpsr & REGS_ARM_USER_CPSR_FLAGS_MASK);
#else
# error Unsupported architecture
#endif

#undef CHECK_REG
}

void RegsUnsetNonCalleeSavedRegisters(struct NaClSignalContext *regs) {
#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86 && NACL_BUILD_SUBARCH == 32
  regs->eax = 0;
  regs->ecx = 0;
  regs->edx = 0;
  regs->flags = 0;
#elif NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86 && NACL_BUILD_SUBARCH == 64
  regs->rax = 0;
  regs->rcx = 0;
  regs->rdx = 0;
  regs->rsi = 0;
  regs->rdi = 0;
  regs->r8 = 0;
  regs->r9 = 0;
  regs->r10 = 0;
  regs->r11 = 0;
  regs->flags = 0;
#elif NACL_ARCH(NACL_BUILD_ARCH) == NACL_arm
  regs->r0 = 0;
  regs->r1 = 0;
  regs->r2 = 0;
  regs->r3 = 0;
  regs->r12 = 0;
  regs->lr = 0;
  regs->cpsr = 0;
#else
# error Unsupported architecture
#endif
}
