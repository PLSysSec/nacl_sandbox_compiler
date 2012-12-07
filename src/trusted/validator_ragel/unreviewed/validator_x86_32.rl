/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This is the core of ia32-mode validator.  Please note that this file
 * combines ragel machine description and C language actions.  Please read
 * validator_internals.html first to understand how the whole thing is built:
 * it explains how the byte sequences are constructed, what constructs like
 * “@{}” or “REX_WRX?” mean, etc.
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native_client/src/trusted/validator_ragel/unreviewed/validator_internal.h"

/* Ignore this information: it's not used by security model in IA32 mode.  */
#undef GET_VEX_PREFIX3
#define GET_VEX_PREFIX3 0
#undef SET_VEX_PREFIX3
#define SET_VEX_PREFIX3(P)

%%{
  machine x86_32_validator;
  alphtype unsigned char;
  variable p current_position;
  variable pe end_of_bundle;
  variable eof end_of_bundle;
  variable cs current_state;

  include byte_machine "byte_machines.rl";

  include prefix_actions
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";
  include prefixes_parsing_noaction
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";
  include vex_actions_ia32
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";
  include vex_parsing_ia32
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";
  include displacement_fields_actions
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";
  include displacement_fields_parsing
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";
  include modrm_parsing_ia32_noactions
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";
  include immediate_fields_actions
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";
  include immediate_fields_parsing_ia32
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";
  # rel8 actions are used in relative jumps with 8-bit offset.
  action rel8_operand {
    rel8_operand(current_position + 1, data, jump_dests, size,
                 &instruction_info_collected);
  }
  # rel16 actions are used in relative jums with 16-bit offset.
  #
  # Such instructions should not be included in the validator's DFA, but we can
  # not just exlude them because they are refenced in relative_fields_parsing
  # ragel machine.  Ensure compilations error in case of accidental usage.
  action rel16_operand {
    #error rel16_operand should never be used in nacl
  }
  # rel32 actions are used in relative calls and jumps with 32-bit offset.
  action rel32_operand {
    rel32_operand(current_position + 1, data, jump_dests, size,
                  &instruction_info_collected);
  }
  include relative_fields_parsing
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";
  include cpuid_actions
    "native_client/src/trusted/validator_ragel/unreviewed/parse_instruction.rl";

  # Action which marks last byte as not immediate.  Most 3DNow! instructions,
  # some AVX and XOP instructions have this property.   It's referenced by
  # decode_x86_32 machine in [autogenerated] “validator_x86_32_instruction.rl”
  # file.
  action last_byte_is_not_immediate {
    instruction_info_collected |= LAST_BYTE_IS_NOT_IMMEDIATE;
  }

  include decode_x86_32 "validator_x86_32_instruction.rl";

  special_instruction =
   # and $~0x1f, %eXX  call  %eXX
   #                   ↓↓↓↓↓↓↓↓↓↓
    (0x83 0xe0 0xe0    0xff (0xd0|0xe0)  | # naclcall/jmp %eax
     0x83 0xe1 0xe0    0xff (0xd1|0xe1)  | # naclcall/jmp %ecx
     0x83 0xe2 0xe0    0xff (0xd2|0xe2)  | # naclcall/jmp %edx
     0x83 0xe3 0xe0    0xff (0xd3|0xe3)  | # naclcall/jmp %ebx
     0x83 0xe4 0xe0    0xff (0xd4|0xe4)  | # naclcall/jmp %esp
     0x83 0xe5 0xe0    0xff (0xd5|0xe5)  | # naclcall/jmp %ebp
     0x83 0xe6 0xe0    0xff (0xd6|0xe6)  | # naclcall/jmp %esi
     0x83 0xe7 0xe0    0xff (0xd7|0xe7))   # naclcall/jmp %edi
   #                   ↑↑↑↑       ↑↑↑↑
   # and $~0x1f, %eXX     jmp %eXX
    @{
      UnmarkValidJumpTarget((current_position - data) - 1, valid_targets);
      instruction_start -= 3;
      instruction_info_collected |= SPECIAL_INSTRUCTION;
    } |
    (0x65 0xa1 (0x00|0x04) 0x00 0x00 0x00      | # mov %gs:0x0/0x4,%eax
     0x65 0x8b (0x05|0x0d|0x015|0x1d|0x25|0x2d|0x35|0x3d)
          (0x00|0x04) 0x00 0x00 0x00);           # mov %gs:0x0/0x4,%reg

  # Check if call is properly aligned
  #
  # For direct call we explicitly encode all variations.  For indirect call
  # we accept all the special instructions which ends with indirect call.
  call_alignment =
    ((one_instruction &
      # Direct call
      ((data16 0xe8 rel16) |
       (0xe8 rel32))) |
     (special_instruction &
      # Indirect call
      (any* data16? 0xff ((opcode_2 | opcode_3) any* &
                          (modrm_memory | modrm_registers)))))
    # Call instruction must aligned to the end of bundle.  Previously this was
    # strict requirement, today it's just warning to aid with debugging.
    @{
      if (((current_position - data) & kBundleMask) != kBundleMask)
        instruction_info_collected |= BAD_CALL_ALIGNMENT;
    };

  # This is main ragel machine: it does 99% of validation work. There are only
  # one thing to do if this machine accepts the bundles — check that direct
  # jumps are correct. This is done in the following way:
  #  • DFA fills two arrays: valid_targets and jump_dests.
  #  • ProcessInvalidJumpTargets checks that jump_dests ⊂ valid_targets.
  # All other checks are done here.
  main := ((call_alignment | one_instruction | special_instruction)
     # Here we call the user callback if there are validation errors or if the
     # CALL_USER_CALLBACK_ON_EACH_INSTRUCTION option is used.
     #
     # After that we move instruction_start and clean all the variables which
     # only used in the processing of a single instruction (prefixes, operand
     # states and instruction_info_collected).
     @{
       if ((instruction_info_collected & VALIDATION_ERRORS_MASK) ||
           (options & CALL_USER_CALLBACK_ON_EACH_INSTRUCTION)) {
         result &= user_callback(instruction_start, current_position,
                                 instruction_info_collected, callback_data);
       }
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       instruction_start = current_position + 1;
       /* Mark this position as a valid target for jump.  */
       MarkValidJumpTarget(current_position + 1 - data, valid_targets);
       instruction_info_collected = 0;
     })*
    $err{
        result &= user_callback(instruction_start, current_position,
                                UNRECOGNIZED_INSTRUCTION, callback_data);
        /*
         * Process the next bundle: “continue” here is for the “for” cycle in
         * the ValidateChunkIA32 function.
         *
         * It does not affect the case which we really care about (when code
         * is validatable), but makes it possible to detect more errors in one
         * run in tools like ncval.
         */
        continue;
    };

}%%

%% write data;


Bool ValidateChunkIA32(const uint8_t *data, size_t size,
                       uint32_t options,
                       const NaClCPUFeaturesX86 *cpu_features,
                       ValidationCallbackFunc user_callback,
                       void *callback_data) {
  bitmap_word valid_targets_small[2];
  bitmap_word jump_dests_small[2];
  bitmap_word *valid_targets;
  bitmap_word *jump_dests;
  const uint8_t *current_position;
  const uint8_t *end_of_bundle;
  int result = TRUE;

  CHECK(sizeof valid_targets_small == sizeof jump_dests_small);
  CHECK(size % kBundleSize == 0);

  /*
   * For a very small sequences (one bundle) malloc is too expensive.
   *
   * Note1: we allocate one extra bit, because we set valid jump target bits
   * _after_ instructions, so there will be one at the end of the chunk.
   *
   * Note2: we don't ever mark first bit as a valid jump target but this is
   * not a problem because any aligned address is valid jump target.
   */
  if ((size + 1) <= (sizeof valid_targets_small * 8)) {
    memset(valid_targets_small, 0, sizeof valid_targets_small);
    valid_targets = valid_targets_small;
    memset(jump_dests_small, 0, sizeof jump_dests_small);
    jump_dests = jump_dests_small;
  } else {
    valid_targets = BitmapAllocate(size + 1);
    jump_dests = BitmapAllocate(size + 1);
    if (!valid_targets || !jump_dests) {
      free(jump_dests);
      free(valid_targets);
      errno = ENOMEM;
      return FALSE;
    }
  }

  /*
   * This option is usually used in tests: we will process the whole chunk
   * in one pass. Usually each bundle is processed separately which means
   * instructions (and super-instructions) can not cross borders of the bundle.
   */
  if (options & PROCESS_CHUNK_AS_A_CONTIGUOUS_STREAM)
    end_of_bundle = data + size;
  else
    end_of_bundle = data + kBundleSize;

  /*
   * Main loop.  Here we process the data array bundle-after-bundle.
   * Ragel-produced DFA does all the checks with one exception: direct jumps.
   * It collects the two arrays: valid_targets and jump_dests which are used
   * to test direct jumps later.
   */
  for (current_position = data;
       current_position < data + size;
       current_position = end_of_bundle,
       end_of_bundle = current_position + kBundleSize) {
    /* Start of the instruction being processed.  */
    const uint8_t *instruction_start = current_position;
    uint32_t instruction_info_collected = 0;
    int current_state;

    %% write init;
    %% write exec;
  }

  /*
   * Check the direct jumps.  All the targets from jump_dests must be in
   * valid_targets.
   */
  result &= ProcessInvalidJumpTargets(data, size, valid_targets, jump_dests,
                                      user_callback, callback_data);

  /* We only use malloc for a large code sequences  */
  if (jump_dests != jump_dests_small) free(jump_dests);
  if (valid_targets != valid_targets_small) free(valid_targets);
  if (!result) errno = EINVAL;
  return result;
}
