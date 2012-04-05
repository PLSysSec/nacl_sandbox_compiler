/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Implement the ApplyValidator API for the x86-64 architecture. */
#include <assert.h>
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/validator/ncvalidate.h"
#include "native_client/src/trusted/validator/validation_cache.h"
#include "native_client/src/trusted/validator/x86/ncval_reg_sfi/ncvalidate_iter.h"

/* Be sure the correct compile flags are defined for this. */
#if NACL_ARCH(NACL_TARGET_ARCH) != NACL_x86
# error("Can't compile, target is for x86-64")
#else
# if NACL_TARGET_SUBARCH != 64
#  error("Can't compile, target is for x86-64")
# endif
#endif

NaClValidationStatus NaClValidatorSetup_x86_64(
    intptr_t guest_addr,
    size_t size,
    int bundle_size,
    int readonly_text,
    const NaClCPUFeaturesX86 *cpu_features,
    struct NaClValidatorState** vstate_ptr) {
  *vstate_ptr = NaClValidatorStateCreate(guest_addr, size, bundle_size, RegR15,
                                         readonly_text, cpu_features);
  return (*vstate_ptr == NULL)
      ? NaClValidationFailedOutOfMemory
      : NaClValidationSucceeded;     /* or at least to this point! */
}

static NaClValidationStatus NaClApplyValidatorSilently_x86_64(
    uintptr_t guest_addr,
    uint8_t *data,
    size_t size,
    int bundle_size,
    int readonly_text,
    const NaClCPUFeaturesX86 *cpu_features,
    struct NaClValidationCache *cache) {
  struct NaClValidatorState *vstate;
  NaClValidationStatus status;
  void *query = NULL;

  if (cache != NULL)
    query = cache->CreateQuery(cache->handle);

  if (query != NULL) {
    const char validator_id[] = "x86-64";
    cache->AddData(query, (uint8_t *) validator_id, sizeof(validator_id));
    cache->AddData(query, (uint8_t *) cpu_features, sizeof(*cpu_features));
    cache->AddData(query, data, size);
    if (cache->QueryKnownToValidate(query)) {
      cache->DestroyQuery(query);
      return NaClValidationSucceeded;
    }
  }

  status =
    NaClValidatorSetup_x86_64(guest_addr, size, bundle_size, readonly_text,
                              cpu_features, &vstate);

  if (status != NaClValidationSucceeded) {
    if (query != NULL)
      cache->DestroyQuery(query);
    return status;
  }
  NaClValidatorStateSetLogVerbosity(vstate, LOG_ERROR);
  NaClValidateSegment(data, guest_addr, size, vstate);
  status =
      NaClValidatesOk(vstate) ? NaClValidationSucceeded : NaClValidationFailed;

  if (query != NULL) {
    /* Don't cache the result if the code is modified. */
    if (status == NaClValidationSucceeded && !NaClValidatorDidStubOut(vstate))
      cache->SetKnownToValidate(query);
    cache->DestroyQuery(query);
  }
  NaClValidatorStateDestroy(vstate);
  return status;
}

NaClValidationStatus NaClApplyValidatorStubout_x86_64(
    uintptr_t guest_addr,
    uint8_t *data,
    size_t size,
    int bundle_size,
    const NaClCPUFeaturesX86 *cpu_features) {
  struct NaClValidatorState *vstate;
  NaClValidationStatus status =
      NaClValidatorSetup_x86_64(guest_addr, size, bundle_size, FALSE,
                                cpu_features, &vstate);
  if (status != NaClValidationSucceeded) return status;
  NaClValidatorStateSetDoStubOut(vstate, TRUE);
  NaClValidateSegment(data, guest_addr, size, vstate);
  NaClValidatorStateDestroy(vstate);
  return NaClValidationSucceeded;
}

NaClValidationStatus NACL_SUBARCH_NAME(ApplyValidator, x86, 64) (
    enum NaClSBKind sb_kind,
    NaClApplyValidationKind kind,
    uintptr_t guest_addr,
    uint8_t *data,
    size_t size,
    int bundle_size,
    int readonly_text,
    const NaClCPUFeaturesX86 *cpu_features,
    struct NaClValidationCache *cache) {
  NaClValidationStatus status = NaClValidationFailedNotImplemented;
  assert(NACL_SB_DEFAULT == sb_kind);
  if (bundle_size == 16 || bundle_size == 32) {
    if (!NaClArchSupported(cpu_features))
      return NaClValidationFailedCpuNotSupported;
    switch (kind) {
      case NaClApplyCodeValidation:
        status = NaClApplyValidatorSilently_x86_64(
            guest_addr, data, size, bundle_size,
            readonly_text, cpu_features, cache);
        break;
      case NaClApplyValidationDoStubout:
        status = NaClApplyValidatorStubout_x86_64(
            guest_addr, data, size, bundle_size, cpu_features);
        break;
      default:
        break;
    }
  }
  return status;
}

static NaClValidationStatus NaClApplyValidatorPair(
    uintptr_t guest_addr,
    uint8_t *data_old,
    uint8_t *data_new,
    size_t size,
    int bundle_size,
    const NaClCPUFeaturesX86 *cpu_features) {
  int is_ok;
  struct NaClValidatorState *vstate;
  NaClValidationStatus status =
    NaClValidatorSetup_x86_64(guest_addr, size, bundle_size, FALSE,
                              cpu_features, &vstate);
  if (status != NaClValidationSucceeded) return status;
  NaClValidatorStateSetLogVerbosity(vstate, LOG_ERROR);
  NaClValidateSegmentPair(data_old, data_new, guest_addr, size, vstate);
  is_ok = NaClValidatesOk(vstate);
  NaClValidatorStateDestroy(vstate);
  return is_ok ? NaClValidationSucceeded : NaClValidationFailed;
}

NaClValidationStatus NACL_SUBARCH_NAME(ApplyValidatorCodeReplacement, x86, 64)
    (enum NaClSBKind sb_kind,
     uintptr_t guest_addr,
     uint8_t *data_old,
     uint8_t *data_new,
     size_t size,
     int bundle_size,
     const NaClCPUFeaturesX86 *cpu_features) {
  NaClValidationStatus status = NaClValidationFailedNotImplemented;
  assert(NACL_SB_DEFAULT == sb_kind);
  if (bundle_size == 16 || bundle_size == 32) {
    if (!NaClArchSupported(cpu_features)) {
      status = NaClValidationFailedCpuNotSupported;
    } else {
      status = NaClApplyValidatorPair(guest_addr, data_old, data_new,
                                      size, bundle_size, cpu_features);
    }
  }
  return status;
}
