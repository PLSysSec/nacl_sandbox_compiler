/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "native_client/src/trusted/validator_arm/inst_classes.h"

#include <assert.h>
#include <string.h>

#include "native_client/src/trusted/validator_arm/validator.h"

// Implementations of instruction classes, for those not completely defined in
// in the header.

namespace nacl_arm_dec {

// Checks that both instructions can be in the same bundle,
// add updates the critical set to include the second instruction,
// since it can't be safely jumped to. If the instruction crosses
// a bundle, a set with the given violation will be returned.
static inline ViolationSet validate_instruction_pair_allowed(
    const nacl_arm_val::DecodedInstruction& first,
    const nacl_arm_val::DecodedInstruction& second,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::AddressSet* critical,
    Violation violation) {
  if (!sfi.in_same_bundle(first, second)) return ViolationBit(violation);
  critical->add(second.addr());
  return kNoViolations;
}

// Reports unsafe loads/stores of a base address by the given instruction
// pair. If the instruction pair defines a safe load/store of a base address,
// it updates the Critical set with the address of the second instruction,
// so that later code can check that the instruction pair is atomic.
//
// See comment associated with Violation::LOADSTORE_VIOLATION for details
// on when a base address is considered safe.
static inline ViolationSet get_loadstore_violations(
    const nacl_arm_val::DecodedInstruction& first,
    const nacl_arm_val::DecodedInstruction& second,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::AddressSet* critical) {
  Register base = second.base_address_register();

  if (base.Equals(Register::None()) /* not a load/store */
      || sfi.is_data_address_register(base)) {
    return kNoViolations;
  }

  // PC + immediate addressing is always safe.
  if (second.is_literal_load()) return kNoViolations;

  // The following checks if this represents a thread address pointer access,
  // which means the instruction must be one of the following forms:
  //    ldr Rn, [r9]     ; load use thread pointer.
  //    ldr Rn, [r9, #4] ; load IRT thread pointer.
  if (second.is_load_thread_address_pointer()) return kNoViolations;

  if (first.defines(base)
      && first.clears_bits(sfi.data_address_mask())
      && first.always_dominates(second)) {
    return validate_instruction_pair_allowed(
        first, second, sfi, critical, LOADSTORE_CROSSES_BUNDLE_VIOLATION);
  }

  if (sfi.conditional_memory_access_allowed_for_sfi() &&
      first.sets_Z_if_bits_clear(base, sfi.data_address_mask()) &&
      second.is_eq_conditional_on(first)
      ) {
    return validate_instruction_pair_allowed(
        first, second, sfi, critical,
        LOADSTORE_CROSSES_BUNDLE_VIOLATION);
  }

  return ViolationBit(LOADSTORE_VIOLATION);
}

// The following generates the diagnostics that corresponds to the violations
// collected by method get_loadstore_violations method (above).
static inline void generate_loadstore_diagnostics(
    ViolationSet violations,
    const nacl_arm_val::DecodedInstruction& first,
    const nacl_arm_val::DecodedInstruction& second,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::ProblemSink* out) {
  if (ContainsViolation(violations, LOADSTORE_CROSSES_BUNDLE_VIOLATION)) {
    out->ReportProblemInstructionPair(
        second.addr(),
        nacl_arm_val::kProblemUnsafeLoadStore,
        nacl_arm_val::kPairCrossesBundle,
        first,
        second);
  }
  if (ContainsViolation(violations, LOADSTORE_VIOLATION)) {
    Register base = second.base_address_register();

    if (first.defines(base)) {
      if (first.clears_bits(sfi.data_address_mask())) {
        out->ReportProblemRegisterInstructionPair(
            second.addr(),
            nacl_arm_val::kProblemUnsafeLoadStore,
            GetPairConditionProblem(first, second),
            base, first, second);
      } else {
        out->ReportProblemRegister(
            second.addr(),
            nacl_arm_val::kProblemUnsafeLoadStore,
            base);
      }
    } else if (first.sets_Z_if_bits_clear(base, sfi.data_address_mask())) {
      out->ReportProblemRegisterInstructionPair(
          second.addr(),
          nacl_arm_val::kProblemUnsafeLoadStore,
          sfi.conditional_memory_access_allowed_for_sfi() ?
          nacl_arm_val::kEqConditionalOn : nacl_arm_val::kTstMemDisallowed,
          base, first, second);
    } else if (base.Equals(Register::Pc())) {
      out->ReportProblem(
          second.addr(),
          nacl_arm_val::kProblemIllegalPcLoadStore);
    } else {
      out->ReportProblemRegister(
          second.addr(),
          nacl_arm_val::kProblemUnsafeLoadStore,
          base);
    }
  }
}

// Reports any unsafe indirect branches. If the instruction pair defines
// a safe indirect branch, it updates the Critical set with the address
// of the branch, so that later code can check that the instruction pair
// is atomic.
//
// A destination address is safe if it has specific bits masked off by its
// immediate predecessor.
static inline ViolationSet get_branch_mask_violations(
    const nacl_arm_val::DecodedInstruction& first,
    const nacl_arm_val::DecodedInstruction& second,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::AddressSet* critical) {
  Register target(second.branch_target_register());
  if (target.Equals(Register::None())) return kNoViolations;

  if (first.defines(target) &&
      first.clears_bits(sfi.code_address_mask()) &&
      first.always_dominates(second)) {
    return validate_instruction_pair_allowed(
        first, second, sfi, critical, BRANCH_MASK_CROSSES_BUNDLE_VIOLATION);
  }

  return ViolationBit(BRANCH_MASK_VIOLATION);
}

// The following generates the diagnostics that corresponds to the violations
// collected by method get_branch_mask_violations (above).
static inline void generate_branch_mask_diagnostics(
    ViolationSet violations,
    const nacl_arm_val::DecodedInstruction& first,
    const nacl_arm_val::DecodedInstruction& second,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::ProblemSink* out) {
  if (ContainsViolation(violations, BRANCH_MASK_CROSSES_BUNDLE_VIOLATION)) {
    out->ReportProblemInstructionPair(
        second.addr(),
        nacl_arm_val::kProblemUnsafeBranch,
        nacl_arm_val::kPairCrossesBundle,
        first,
        second);
  }
  if (ContainsViolation(violations, BRANCH_MASK_VIOLATION)) {
    Register target(second.branch_target_register());
    if (first.defines(target)) {
      if (first.clears_bits(sfi.code_address_mask())) {
        out->ReportProblemRegisterInstructionPair(
            second.addr(),
            nacl_arm_val::kProblemUnsafeBranch,
            GetPairConditionProblem(first, second),
            target, first, second);
      } else {
        out->ReportProblemRegister(
            second.addr(), nacl_arm_val::kProblemUnsafeBranch, target);
      }
    } else {
      out->ReportProblemRegister(
          second.addr(), nacl_arm_val::kProblemUnsafeBranch, target);
    }
  }
}

// Reports any instructions that update a data-address register without
// a valid mask. If the instruction pair safely updates the data-address
// register, it updates the Critical set with the address of the the
// second instruction, so that later code can check that the instruction
// pair is atomic.
static inline ViolationSet get_data_register_update_violations(
    const nacl_arm_val::DecodedInstruction& first,
    const nacl_arm_val::DecodedInstruction& second,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::AddressSet* critical) {

  RegisterList data_registers(sfi.data_address_registers());

  // Don't need to check if no data address register updates.
  if (!first.defines_any(data_registers)) return kNoViolations;

  // A single safe data register update doesn't affect control flow.
  if (first.clears_bits(sfi.data_address_mask())) return kNoViolations;

  // Small immediate base register writeback to data address registers
  // (e.g. SP) doesn't need to be an instruction pair.
  if (first.base_address_register_writeback_small_immediate() &&
      sfi.data_address_registers().Contains(first.base_address_register())) {
    return kNoViolations;
  }

  // Data address register modification followed by bit clear.
  RegisterList data_addr_defs(first.defs().Intersect(data_registers));
  if (second.defines_all(data_addr_defs)
      && second.clears_bits(sfi.data_address_mask())
      && second.always_postdominates(first)) {
    return validate_instruction_pair_allowed(
        first, second, sfi, critical,
        DATA_REGISTER_UPDATE_CROSSES_BUNDLE_VIOLATION);
  }

  return ViolationBit(DATA_REGISTER_UPDATE_VIOLATION);
}

// The following generates the diagnostics that corresponds to the violations
// collected by method get_data_register_violations (above).
static inline void generate_data_register_update_diagnostics(
    ViolationSet violations,
    const nacl_arm_val::DecodedInstruction& first,
    const nacl_arm_val::DecodedInstruction& second,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::ProblemSink* out) {
  if (ContainsViolation(violations,
                        DATA_REGISTER_UPDATE_CROSSES_BUNDLE_VIOLATION)) {
    out->ReportProblemInstructionPair(
        first.addr(),
        nacl_arm_val::kProblemUnsafeDataWrite,
        nacl_arm_val::kPairCrossesBundle,
        first,
        second);
  }
  if (ContainsViolation(violations, DATA_REGISTER_UPDATE_VIOLATION)) {
    RegisterList data_registers(sfi.data_address_registers());
    RegisterList data_addr_defs(first.defs().Intersect(data_registers));
    if (second.defines_all(data_addr_defs) &&
        second.clears_bits(sfi.data_address_mask())) {
      out->ReportProblemRegisterListInstructionPair(
          first.addr(),
          nacl_arm_val::kProblemUnsafeDataWrite,
          GetPairConditionProblem(first, second),
          data_addr_defs, first, second);
    } else {
      out->ReportProblemRegisterList(
          first.addr(), nacl_arm_val::kProblemUnsafeDataWrite, data_addr_defs);
    }
  }
}

// Checks if the call instruction isn't the last instruction in the
// bundle.
//
// This is not a security check per se. Rather, it is a check to prevent
// imbalancing the CPU's return stack, thereby decreasing performance.
static inline ViolationSet get_call_position_violations(
    const nacl_arm_val::DecodedInstruction& inst,
    const nacl_arm_val::SfiValidator& sfi) {
  // Identify linking branches through their definitions:
  if (inst.defines_all(RegisterList(Register::Pc()).Add(Register::Lr()))) {
    uint32_t last_slot = sfi.bundle_for_address(inst.addr()).end_addr() - 4;
    if (inst.addr() != last_slot) {
      return ViolationBit(CALL_POSITION_VIOLATION);
    }
  }
  return kNoViolations;
}

// The following generates the diagnostics that corresponds to the violations
// collected by method get_call_position_violations (above).
static inline void generate_call_position_diagnostics(
      ViolationSet violations,
      const nacl_arm_val::DecodedInstruction& inst,
      const nacl_arm_val::SfiValidator& sfi,
      nacl_arm_val::ProblemSink* out) {
  UNREFERENCED_PARAMETER(sfi);
  if (ContainsViolation(violations, CALL_POSITION_VIOLATION)) {
    out->ReportProblem(inst.addr(), nacl_arm_val::kProblemMisalignedCall);
  }
}

// Checks that the instruction doesn't set a read-only register
static inline ViolationSet get_read_only_violations(
    const nacl_arm_val::DecodedInstruction& inst,
    const nacl_arm_val::SfiValidator& sfi) {
  return inst.defines_any(sfi.read_only_registers())
      ? ViolationBit(READ_ONLY_VIOLATION)
      : kNoViolations;
}

// The following generates the diagnostics that corresponds to the violations
// collected by method get_read_only_violations (above).
static inline void generate_read_only_diagnostics(
    ViolationSet violations,
    const nacl_arm_val::DecodedInstruction& inst,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::ProblemSink* out) {
  UNREFERENCED_PARAMETER(sfi);
  if (ContainsViolation(violations, READ_ONLY_VIOLATION)) {
    out->ReportProblemRegisterList(
        inst.addr(), nacl_arm_val::kProblemReadOnlyRegister,
        inst.defs().Intersect(sfi.read_only_registers()));
  }
}

// Checks that instruction doesn't read the thread local pointer.
static inline ViolationSet get_read_thread_local_pointer_violations(
    const nacl_arm_val::DecodedInstruction& inst) {
  return (inst.uses(Register::Tp()) && !inst.is_load_thread_address_pointer())
      ? ViolationBit(READ_THREAD_LOCAL_POINTER_VIOLATION)
      : kNoViolations;
}

// The following generates the diagnostics that corresponds to the violations
// collected by method get_read_thread_local_pointer_violations (above).
static inline void generate_read_thread_local_pointer_diagnostics(
    ViolationSet violations,
    const nacl_arm_val::DecodedInstruction& inst,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::ProblemSink* out) {
  UNREFERENCED_PARAMETER(sfi);
  if (ContainsViolation(violations, READ_THREAD_LOCAL_POINTER_VIOLATION)) {
    out->ReportProblemRegister(inst.addr(),
                               nacl_arm_val::kProblemIllegalUseOfThreadPointer,
                               Register::Tp());
  }
}

// Checks that writes to the program counter are only from branches. Updates
// branches to contain the target of safe branches.
static inline ViolationSet get_pc_writes_violations(
    const nacl_arm_val::DecodedInstruction& inst,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::AddressSet* branches) {

  // Safe if a relative branch.
  if (inst.is_relative_branch()) {
    branches->add(inst.addr());
    return kNoViolations;
  }

  // If branch to register, it is checked by get_branch_mask_violation.
  if (!inst.branch_target_register().Equals(Register::None()))
    return kNoViolations;

  if (!inst.defines(nacl_arm_dec::Register::Pc())) return kNoViolations;

  if (inst.clears_bits(sfi.code_address_mask())) return kNoViolations;

  return ViolationBit(PC_WRITES_VIOLATION);
}


// The following generates the diagnostics that corresponds to the violations
// collected by method get_pc_writes_violations (above).
static inline void generate_pc_writes_diagnostics(
    ViolationSet violations,
    const nacl_arm_val::DecodedInstruction& inst,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::ProblemSink* out) {
  UNREFERENCED_PARAMETER(sfi);
  if (ContainsViolation(violations, PC_WRITES_VIOLATION)) {
    out->ReportProblemRegister(inst.addr(), nacl_arm_val::kProblemUnsafeBranch,
                               Register::Pc());
  }
}

// ClassDecoder
RegisterList ClassDecoder::defs(Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return RegisterList::Everything();
}

RegisterList ClassDecoder::uses(Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return RegisterList();
}

bool ClassDecoder::base_address_register_writeback_small_immediate(
    Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return false;
}

Register ClassDecoder::base_address_register(Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return Register::None();
}

bool ClassDecoder::is_literal_load(Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return false;
}

Register ClassDecoder::branch_target_register(Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return Register::None();
}

bool ClassDecoder::is_relative_branch(Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return false;
}

int32_t ClassDecoder::branch_target_offset(Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return 0;
}

bool ClassDecoder::is_literal_pool_head(Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return false;
}

bool ClassDecoder::clears_bits(Instruction i, uint32_t mask) const {
  UNREFERENCED_PARAMETER(i);
  UNREFERENCED_PARAMETER(mask);
  return false;
}

bool ClassDecoder::sets_Z_if_bits_clear(Instruction i,
                                        Register r,
                                        uint32_t mask) const {
  UNREFERENCED_PARAMETER(i);
  UNREFERENCED_PARAMETER(r);
  UNREFERENCED_PARAMETER(mask);
  return false;
}

bool ClassDecoder::is_load_thread_address_pointer(Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return false;
}

Instruction ClassDecoder::
dynamic_code_replacement_sentinel(Instruction i) const {
  return i;
}

ViolationSet ClassDecoder::get_violations(
    const nacl_arm_val::DecodedInstruction& first,
    const nacl_arm_val::DecodedInstruction& second,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::AddressSet* branches,
    nacl_arm_val::AddressSet* critical,
    uint32_t* next_inst_addr) const {
  // TODO(kschimpf): Move these tests into automatically generated code,
  // when tied to specific virtuals that can be checked at code
  // generation time.

  ViolationSet violations = kNoViolations;

  // Start by checking safety.
  if (second.safety() != nacl_arm_dec::MAY_BE_SAFE)
    violations = SafetyViolationBit(second.safety());

  violations = ViolationUnion(
      violations, get_loadstore_violations(first, second, sfi, critical));
  violations = ViolationUnion(
      violations, get_branch_mask_violations(first, second, sfi, critical));
  violations = ViolationUnion(
      violations,
      get_data_register_update_violations(first, second, sfi, critical));
  violations = ViolationUnion(
      violations, get_call_position_violations(second, sfi));
  violations = ViolationUnion(
      violations, get_read_only_violations(second, sfi));
  violations = ViolationUnion(
      violations, get_read_thread_local_pointer_violations(second));
  violations = ViolationUnion(
      violations, get_pc_writes_violations(second, sfi, branches));

  // If a pool head, mark address appropriately and then skip over
  // the constant bundle.
  if (second.is_literal_pool_head() && sfi.is_bundle_head(second.addr())) {
    // Add each instruction in this bundle to the critical set.
    // Skip over the literal pool head (which is also the bundle head):
    // indirect branches to it are legal, direct branches should therefore
    // also be legal.
    uint32_t last_data_addr = sfi.bundle_for_address(second.addr()).end_addr();
    for (; *next_inst_addr < last_data_addr; *next_inst_addr += 4) {
      critical->add(*next_inst_addr);
    }
  }

  return violations;
}

void ClassDecoder::generate_diagnostics(
    ViolationSet violations,
    const nacl_arm_val::DecodedInstruction& first,
    const nacl_arm_val::DecodedInstruction& second,
    const nacl_arm_val::SfiValidator& sfi,
    nacl_arm_val::ProblemSink* out) const {
  // TODO(kschimpf): Move these tests into automatically generated code,
  // when tied to specific virtuals that can be checked at code
  // generation time.

  if (ContainsSafetyViolations(violations)) {
    // Note: We assume that safety levels end with MAY_BE_SAFE, as stated
    // for enum type SafetyLevel.
    for (uint32_t safety = 0; safety != MAY_BE_SAFE; ++safety) {
      Violation violation = static_cast<Violation>(safety);
      if (ContainsViolation(violations, violation)) {
        out->ReportProblemSafety(violation, second);
      }
    }
  }

  generate_loadstore_diagnostics(violations, first, second, sfi, out);
  generate_branch_mask_diagnostics(violations, first, second, sfi, out);
  generate_data_register_update_diagnostics(violations, first, second,
                                            sfi, out);
  generate_call_position_diagnostics(violations, second, sfi, out);
  generate_read_only_diagnostics(violations, second, sfi, out);
  generate_read_thread_local_pointer_diagnostics(violations, second, sfi, out);
  generate_pc_writes_diagnostics(violations, second, sfi, out);
}

}  // namespace nacl_arm_dec
