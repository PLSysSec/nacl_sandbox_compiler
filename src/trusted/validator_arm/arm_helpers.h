/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_ARM_V2_ARM_HELPERS_H
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_ARM_V2_ARM_HELPERS_H

/* This file defines arm helper functions that are used by the
   method definitions in armv7.table. They exists because either
   (a) ARM specifications use them, or (b) the limitations of
   (dgen) bit expressions (defined in dgen_input.py) do not allow us
   to specify the function explicitly.
*/

#include "native_client/src/trusted/validator_arm/model.h"

namespace nacl_arm_dec {

// Function to get the number of general purpose registers in
// a register list.
inline uint32_t NumGPRs(RegisterList registers) {
  return registers.numGPRs();
}

// Function to return the smallest general purpose register in
// a register list.
inline uint32_t SmallestGPR(RegisterList registers) {
  return registers.SmallestGPR();
}

// Function that returns true if the general purpose register
// in in the register list.
inline bool Contains(RegisterList registers, Register r) {
  return registers.Contains(r);
}

// Function that unions together to register lists.
inline RegisterList Union(RegisterList r1, RegisterList r2) {
  return r1.Union(r2);
}

}  // namespace nacl_arm_dec

#endif  // NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_ARM_V2_ARM_HELPERS_H
