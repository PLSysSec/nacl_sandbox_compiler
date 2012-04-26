/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "native_client/src/trusted/validator_arm/baseline_classes.h"

#include <assert.h>
#include <string.h>

// Implementations of instruction classes, for those not completely defined in
// in the header.

namespace nacl_arm_dec {

// Unary1RegisterImmediateOp
SafetyLevel Unary1RegisterImmediateOp::safety(const Instruction i) const {
  if (d.reg(i) == kRegisterPc) return UNPREDICTABLE;

  // Note: We would restrict out PC as well for Rd in NaCl, but no need
  // since the ARM restriction doesn't allow it anyway.
  return MAY_BE_SAFE;
}

RegisterList Unary1RegisterImmediateOp::defs(const Instruction i) const {
  return d.reg(i) + flags.reg_if_updated(i);
}

// Unary2RegisterOp
SafetyLevel Unary2RegisterOp::safety(const Instruction i) const {
  // NaCl Constraint.
  if (d.reg(i) == kRegisterPc) return FORBIDDEN_OPERANDS;
  return MAY_BE_SAFE;
}

RegisterList Unary2RegisterOp::defs(const Instruction i) const {
  return d.reg(i) + flags.reg_if_updated(i);
}

// Binary3RegisterOp
SafetyLevel Binary3RegisterOp::safety(const Instruction i) const {
  // Unsafe if any register contains PC (ARM restriction).
  if ((d.reg(i) + m.reg(i) + n.reg(i))[kRegisterPc]) return UNPREDICTABLE;


  // Note: We would restrict out PC as well for Rd in NaCl, but no need
  // since the ARM restriction doesn't allow it anyway.
  return MAY_BE_SAFE;
}

RegisterList Binary3RegisterOp::defs(const Instruction i) const {
  return d.reg(i) + flags.reg_if_updated(i);
}

// Unary2RegisterImmedShiftedOp
SafetyLevel Unary2RegisterImmedShiftedOp::safety(const Instruction i) const {
  // NaCl Constraint.
  if (d.reg(i) == kRegisterPc) return FORBIDDEN_OPERANDS;
  return MAY_BE_SAFE;
}

RegisterList Unary2RegisterImmedShiftedOp::defs(const Instruction i) const {
  return d.reg(i) + flags.reg_if_updated(i);
}

// Unary3RegisterShiftedOp
SafetyLevel Unary3RegisterShiftedOp::safety(Instruction i) const {
  // Unsafe if any register contains PC (ARM restriction).
  if ((d.reg(i) + s.reg(i) + m.reg(i))[kRegisterPc]) return UNPREDICTABLE;

  // Note: We would restrict out PC as well for Rd in NaCl, but no need
  // since the ARM restriction doesn't allow it anyway.
  return MAY_BE_SAFE;
}

RegisterList Unary3RegisterShiftedOp::defs(const Instruction i) const {
  return d.reg(i) + flags.reg_if_updated(i);
}

// Binary3RegisterImmedShiftedOp
SafetyLevel Binary3RegisterImmedShiftedOp::safety(const Instruction i) const {
  // NaCl Constraint.
  if (d.reg(i) == kRegisterPc) return FORBIDDEN_OPERANDS;
  return MAY_BE_SAFE;
}

RegisterList Binary3RegisterImmedShiftedOp::defs(const Instruction i) const {
  return d.reg(i) + flags.reg_if_updated(i);
}

// Binary4RegisterShiftedOp
SafetyLevel Binary4RegisterShiftedOp::safety(Instruction i) const {
  // Unsafe if any register contains PC (ARM restriction).
  if ((d.reg(i) + n.reg(i) + s.reg(i) + m.reg(i))[kRegisterPc])
    return UNPREDICTABLE;

  // Note: We would restrict out PC as well for Rd in NaCl, but no need
  // since the ARM restriction doesn't allow it anyway.
  return MAY_BE_SAFE;
}

RegisterList Binary4RegisterShiftedOp::defs(const Instruction i) const {
  return d.reg(i) + flags.reg_if_updated(i);
}

// Binary2RegisterImmedShiftedTest
SafetyLevel Binary2RegisterImmedShiftedTest::safety(const Instruction i) const {
  UNREFERENCED_PARAMETER(i);
  return MAY_BE_SAFE;
}

RegisterList Binary2RegisterImmedShiftedTest::defs(const Instruction i) const {
  return flags.reg_if_updated(i);
}

// Binary3RegisterShiftedTest
SafetyLevel Binary3RegisterShiftedTest::safety(Instruction i) const {
  // Unsafe if any register contains PC (ARM restriction).
  if ((n.reg(i) + s.reg(i) + m.reg(i))[kRegisterPc]) return UNPREDICTABLE;
  return MAY_BE_SAFE;
}

RegisterList Binary3RegisterShiftedTest::defs(const Instruction i) const {
  return flags.reg_if_updated(i);
}

}  // namespace
