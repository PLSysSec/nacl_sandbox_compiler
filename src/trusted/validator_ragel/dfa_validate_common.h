/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This file contains common parts of x86-32 and x86-64 internals (inline
 * and static functions and defines).
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_RAGEL_DFA_VALIDATE_COMMON_H_
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_RAGEL_DFA_VALIDATE_COMMON_H_

#include <stddef.h>

#include "native_client/src/shared/utils/types.h"
#include "native_client/src/trusted/validator/ncvalidate.h"

/*
 * ia32 or x86-64 instructions can not be longer than 15 bytes, but we have some
 * superinstructions which are 17 bytes long, e.g.:
 *   0:   40 89 f6                rex mov    %esi,%esi
 *   3:   49 8d 34 37             lea    (%r15,%rsi,1),%rsi
 *   7:   40 89 ff                rex mov    %edi,%edi
 *   a:   49 8d 3c 3f             lea    (%r15,%rdi,1),%rdi
 *   e:   f2 48 a7                repnz cmpsq %es:(%rdi),%ds:(%rsi)
 */
#define MAX_INSTRUCTION_LENGTH 17

Bool NaClDfaProcessValidationError(const uint8_t *begin, const uint8_t *end,
                                   uint32_t info, void *callback_data);

struct StubOutCallbackData {
  uint32_t flags;
  int did_rewrite;
};

Bool NaClDfaStubOutUnsupportedInstruction(const uint8_t *begin,
                                          const uint8_t *end,
                                          uint32_t info,
                                          void *callback_data);

struct CodeCopyCallbackData {
  NaClCopyInstructionFunc copy_func;
  /* Difference between addresses: dest - src.  */
  ptrdiff_t existing_minus_new;
};

Bool NaClDfaProcessCodeCopyInstruction(const uint8_t *begin_new,
                                       const uint8_t *end_new,
                                       uint32_t info,
                                       void *callback_data);

/* Check whether instruction is stubouted because it is not supported by current
   CPU.  */
Bool NaClDfaCodeReplacementIsStubouted(const uint8_t *begin_existing,
                                       size_t instruction_length);

#endif /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_RAGEL_DFA_VALIDATE_COMMON_H_ */
