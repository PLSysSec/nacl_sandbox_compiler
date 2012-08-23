/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_RAGEL_VALIDATOR_H_
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_RAGEL_VALIDATOR_H_

#include "native_client/src/trusted/validator_ragel/unreviewed/decoder.h"

EXTERN_C_BEGIN

enum validation_callback_info {
  /* validation_error & IMMEDIATES_INFO will give you immediates size.  */
  IMMEDIATES_SIZE               = 0x0000000f,
  /* Immediate sizes (immediates always come at the end of instruction).  */
  IMMEDIATE_8BIT                = 0x00000001,
  IMMEDIATE_16BIT               = 0x00000002,
  IMMEDIATE_32BIT               = 0x00000004,
  IMMEDIATE_64BIT               = 0x00000008,
  /* Second 8bit immediate is only used in "extrq/insertq" instructions.  */
  SECOND_IMMEDIATE_8BIT         = 0x00000011,
  /* Second 16bit immediate is only used in "enter" instruction.  */
  SECOND_IMMEDIATE_16BIT        = 0x00000012,
  /* Displacement sizes (displacements come at the end - before immediates).  */
  DISPLACEMENT_8BIT             = 0x00000021,
  DISPLACEMENT_16BIT            = 0x00000042,
  DISPLACEMENT_32BIT            = 0x00000064,
  /* Relative size (relative fields always come at the end if instriction).  */
  RELATIVE_8BIT                 = 0x00000081,
  RELATIVE_16BIT                = 0x00000082,
  RELATIVE_32BIT                = 0x00000084,
  /* Not a normal immediate: only two bits can be changed.  */
  IMMEDIATE_2BIT                = 0x20000010,
  /* Last restricted register.  */
  RESTRICTED_REGISTER_MASK      = 0x00001f00,
  RESTRICTED_REGISTER_SHIFT     =          8,
  /* Was restricted from previous instruction used in the current one?  */
  RESTRICTED_REGISTER_USED      = 0x00002000,
  /* Mask to select all validation errors.  */
  VALIDATION_ERRORS             = 0x03ffc000,
  /* Unrecognized instruction: fatal error, processing stops here.  */
  UNRECOGNIZED_INSTRUCTION      = 0x00004000,
  /* Direct jump to unaligned address outside of given region.  */
  DIRECT_JUMP_OUT_OF_RANGE      = 0x00008000,
  /* Instruction is not allowed on current CPU.  */
  CPUID_UNSUPPORTED_INSTRUCTION = 0x00010000,
  /* Base register can be one of: %r15, %rbp, %rip, %rsp.  */
  FORBIDDEN_BASE_REGISTER       = 0x00020000,
  /* Index must be restricted if present.  */
  UNRESTRICTED_INDEX_REGISTER   = 0x00040000,
  /* Operations with %ebp must be followed with sandboxing immediately.  */
  RESTRICTED_RBP_UNPROCESSED    = 0x00080000,
  /* Attemp to "sandbox" %rbp without restricting it first.  */
  UNRESTRICTED_RBP_PROCESSED    = 0x00100000,
  /* Operations with %esp must be followed with sandboxing immediately.  */
  RESTRICTED_RSP_UNPROCESSED    = 0x00200000,
  /* Attemp to "sandbox" %rsp without restricting it first.  */
  UNRESTRICTED_RSP_PROCESSED    = 0x00400000,
  /* Operations with %r15 are forbidden.  */
  R15_MODIFIED                  = 0x00800000,
  /* Operations with SPL are forbidden for compatibility with old validator.  */
  BPL_MODIFIED                  = 0x01000000,
  /* Operations with SPL are forbidden for compatibility with old validator.  */
  SPL_MODIFIED                  = 0x02000000,
  /* Bad call alignment: "call" must end at the end of the bundle.  */
  BAD_CALL_ALIGNMENT            = 0x04000000,
  /* Instruction is modifiable by nacl_dyncode_modify.  */
  MODIFIABLE_INSTRUCTION        = 0x08000000,
  /* Special instruction.  Uses different, non-standard validation rules.  */
  SPECIAL_INSTRUCTION           = 0x10000000,
  /* Some 3DNow! instructions use immediate byte as opcode extensions.  */
  LAST_BYTE_IS_NOT_IMMEDIATE    = 0x20000000,
  /* Bad jump target.  Note: in this case ptr points to jump target!  */
  BAD_JUMP_TARGET               = 0x40000000
};

#define kBundleSize 32
#define kBundleMask 31

enum validation_options {
  /* Call process_error function on instruction.  */
  CALL_USER_CALLBACK_ON_EACH_INSTRUCTION = 0x00000001,
  /* Process all instruction as a contiguous stream.  */
  PROCESS_CHUNK_AS_A_CONTIGUOUS_STREAM   = 0x00000002
};

/*
 * If CALL_USER_CALLBACK_ON_EACH_INSTRUCTION option is used then callback
 * is called on each instruction, otherwise only errorneous instructions
 * are inspected via callback.  If callback returns FALSE at least once
 * then ValidateChunkXXX returns false, if callback marks all the error
 * as unimportant by always returning TRUE then ValidateChunkXXX returns
 * TRUE as well.
 */
typedef Bool (*validation_callback_func) (const uint8_t *instruction_start,
                                          const uint8_t *instruction_end,
                                          uint32_t validation_info,
                                          void *callback_data);

Bool ValidateChunkAMD64(const uint8_t *data, size_t size,
                        enum validation_options options,
                        const NaClCPUFeaturesX86 *cpu_features,
                        validation_callback_func user_callback,
                        void *callback_data);

Bool ValidateChunkIA32(const uint8_t *data, size_t size,
                       enum validation_options options,
                       const NaClCPUFeaturesX86 *cpu_features,
                       validation_callback_func user_callback,
                       void *callback_data);

EXTERN_C_END

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_RAGEL_VALIDATOR_H_ */
