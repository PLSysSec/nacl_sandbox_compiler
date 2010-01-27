/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATE_ITER_INTERNAL_H__
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATE_ITER_INTERNAL_H__

/* Defines the internal data structure for the validator state. */

#include "native_client/src/shared/utils/types.h"
#include "native_client/src/trusted/gio/gio.h"
#include "native_client/src/trusted/validator_x86/nacl_cpuid.h"

/* Defines the maximum number of validators that can be registered. */
#define MAX_NCVALIDATORS 20

struct NcValidatorState {
  /* Holds the vbase value passed to NcValidatorStateCreate. */
  PcAddress vbase;
  /* Holds the sz value passed to NcValidatorStateCreate. */
  MemorySize sz;
  /* Holds the alignment value passed to NcValidatorStateCreate. */
  uint8_t alignment;
  /* Holds the upper limit of all possible addresses */
  PcAddress vlimit;
  /* Holds the alignment mask, which when applied, catches any lower
   * bits in an address that violate alignment.
   */
  PcAddress alignment_mask;
  /* Holds the value for the base register, or RegUnknown if undefined. */
  OperandKind base_register;
  /* Holds if the validation is still valid. */
  Bool validates_ok;
  /* Holds if we should quit validation quickly if an error occurs
   * (preferably after first error).
   */
  Bool quit_after_first_error;
  /* Holds the local memory associated with validators to be applied to this
   * state.
   */
  void* local_memory[MAX_NCVALIDATORS];
  /* Defines how many validators are defined for this state. */
  int number_validators;
  /* Holds the cpu features of the machine it is running on. */
  CPUFeatures cpu_features;
  /* Holds the log file to use. */
  FILE* log_file;
  /* Holds the corresponding Gio handle for logging. */
  struct GioFile log_stream;
  /* Holds the log file before building the validator state. */
  struct Gio* old_log_stream;
};

#endif
  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATE_ITER_INTERNAL_H__ */
