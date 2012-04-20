/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_ARM_NAMED_CLASS_DECODER_H_
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_ARM_NAMED_CLASS_DECODER_H_

#ifndef NACL_TRUSTED_BUT_NOT_TCB
#error("This file is not meant for use in the TCB")
#endif

#include "native_client/src/trusted/validator_arm/inst_classes.h"

namespace nacl_arm_test {

// Defines a wrapper class used for testing, that adds a name to
// an existing class decoder, so that we can give better error messages
// if testing finds a consistency issue in modeling instructions.
class NamedClassDecoder : public nacl_arm_dec::ClassDecoder {
 public:
  explicit NamedClassDecoder(const nacl_arm_dec::ClassDecoder& wrapped_decoder,
                             const char* name)
      : wrapped_decoder_(wrapped_decoder),
        name_(name)
  {}
  virtual ~NamedClassDecoder() {}

  // Returns the class decoder that is being named.
  const ClassDecoder& named_decoder() const {
    return wrapped_decoder_;
  }

  // Returns the name of the wrapped class decoder.
  const char* name() const {
    return name_;
  }

  // Virtual dispatching.
  virtual nacl_arm_dec::SafetyLevel safety(nacl_arm_dec::Instruction i) const {
    return wrapped_decoder_.safety(i);
  }

  virtual nacl_arm_dec::RegisterList defs(nacl_arm_dec::Instruction i) const {
    return wrapped_decoder_.defs(i);
  }

  virtual nacl_arm_dec::RegisterList
  immediate_addressing_defs(nacl_arm_dec::Instruction i) const {
    return wrapped_decoder_.immediate_addressing_defs(i);
  }

  virtual bool writes_memory(nacl_arm_dec::Instruction i) const {
    return wrapped_decoder_.writes_memory(i);
  }

  virtual nacl_arm_dec::Register
  base_address_register(nacl_arm_dec::Instruction i) const {
    return wrapped_decoder_.base_address_register(i);
  }

  virtual bool offset_is_immediate(nacl_arm_dec::Instruction i) const {
    return wrapped_decoder_.offset_is_immediate(i);
  }

  virtual nacl_arm_dec::Register
  branch_target_register(nacl_arm_dec::Instruction i) const {
    return wrapped_decoder_.branch_target_register(i);
  }

  virtual bool is_relative_branch(nacl_arm_dec::Instruction i) const {
    return wrapped_decoder_.is_relative_branch(i);
  }

  virtual int32_t branch_target_offset(nacl_arm_dec::Instruction i) const {
    return wrapped_decoder_.branch_target_offset(i);
  }

  virtual bool is_literal_pool_head(nacl_arm_dec::Instruction i) const {
    return wrapped_decoder_.is_literal_pool_head(i);
  }

  virtual bool clears_bits(nacl_arm_dec::Instruction i, uint32_t mask) const {
    return wrapped_decoder_.clears_bits(i, mask);
  }

  virtual bool sets_Z_if_bits_clear(nacl_arm_dec::Instruction i,
                                    nacl_arm_dec::Register r,
                                    uint32_t mask) const {
    return wrapped_decoder_.sets_Z_if_bits_clear(i, r, mask);
  }

 private:
  const nacl_arm_dec::ClassDecoder& wrapped_decoder_;
  const char* name_;
};

}  // namespace

#endif  // NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_ARM_NAMED_CLASS_DECODER_H_
