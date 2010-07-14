/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATE_ITER_H__
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATE_ITER_H__

/*
 * ncvalidate_iter.h
 * Type declarations of the iterator state etc.
 *
 * This is the interface to the iterator form of the NaCL validator.
 * Basic usage:
 *   -- base is initial address of ELF file.
 *   -- limit is the size of the ELF file.
 *   -- maddr is the address to the memory of a section.
 *   -- vaddr is the starting virtual address associated with a section.
 *   -- size is the number of bytes in a section.
 *
 *   NaClValidatorState* state = NaClValidatorStateCreate(base, limit, 16);
 *   if (state == NULL) fail;
 *   for each section:
 *     NaClValidateSegment(maddr, vaddr, size, state);
 *   if (!NaClValidatesOk(state)) fail;
 *   NaClValidatorStatePrintStats(stdout, state);
 *   NaClValidatorStateDestroy(state);
 */

#include <stdio.h>

#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/validator_x86/nc_inst_iter.h"
#include "native_client/src/trusted/validator_x86/nc_inst_state.h"

/* The model of a validator state. */
typedef struct NaClValidatorState NaClValidatorState;

/* Create a validator state to validate the ELF file with the given parameters.
 * Parameters.
 *   vbase - The virtual address for the contents of the ELF file.
 *   sz - The number of bytes in the ELF file.
 *   alignment: 16 or 32, specifying alignment.
 *   base_register - OperandKind defining value for base register (or
 *     RegUnknown if not defined).
 *   quit_after_first_error - Don't report multiple validator errors
 *     (if possible).
 *   log_file - The file to log messages to.
 * Returns:
 *   A pointer to an initialized validator state if everything is ok, NULL
 *  otherwise.
 */
NaClValidatorState* NaClValidatorStateCreate(const NaClPcAddress vbase,
                                             const NaClMemorySize sz,
                                             const uint8_t alignment,
                                             const NaClOpKind base_register,
                                             Bool quit_after_first_error,
                                             FILE* log_file);

/* Returns the file that messages are being logged to. */
FILE* NaClValidatorStateLogFile(NaClValidatorState* state);

/* Validate a code segment.
 * Parameters:
 *   mbase - The address of the beginning of the code segment.
 *   vbase - The virtual address associated with the beginning of the code
 *       segment.
 *   sz - The number of bytes in the code segment.
 *   state - The validator state to use while validating.
 */
void NaClValidateSegment(uint8_t* mbase,
                         NaClPcAddress vbase,
                         NaClMemorySize sz,
                         NaClValidatorState* state);

/* Returns true if the validator hasn't found any problems with the validated
 * code segments.
 * Parameters:
 *   state - The validator state used to validate code segments.
 * Returns:
 *   true only if no problems have been found.
 */
Bool NaClValidatesOk(NaClValidatorState* state);

/* Returns true if the validator quit after the first found error. */
Bool NaClValidateQuit(NaClValidatorState* state);

/* Print out statistics on the applied validation. */
void NaClValidatorStatePrintStats(FILE* file, NaClValidatorState* state);

/* Cleans up and returns the memory created by the corresponding
 * call to NaClValidatorStateCreate.
 */
void NaClValidatorStateDestroy(NaClValidatorState* state);

/* Defines a function to create local memory to be used by a validator
 * function, should it need it.
 * Parameters:
 *   state - The state of the validator.
 * Returns:
 *   Allocated space for local data associated with a validator function.
 */
typedef void* (*NaClValidatorMemoryCreate)(NaClValidatorState* state);

/* Defines a validator function to be called on each instruction.
 * Parameters:
 *   state - The state of the validator.
 *   iter - The instruction iterator's current position in the segment.
 *   local_memory - Pointer to local memory generated by the corresponding
 *          NaClValidatorMemoryCreate (or NULL if not specified).
 */
typedef void (*NaClValidator)(NaClValidatorState* state,
                              NaClInstIter* iter,
                              void* local_memory);

/* Defines a post validator function that is called after iterating through
 * a segment, but before the iterator is destroyed.
 * Parameters:
 *   state - The state of the validator,
 *   iter - The instruction iterator's current position in the segment.
 *   local_memory - Pointer to local memory generated by the corresponding
 *          NaClValidatorMemoryCreate (or NULL if not specified).
 */
typedef void (*NaClValidatorPostValidate)(NaClValidatorState* state,
                                          NaClInstIter* iter,
                                          void* local_memory);

/* Defines a statistics print routine for a validator function.
 * Parameters:
 *   file - The file to print statistics to.
 *   state - The state of the validator,
 *   local_memory - Pointer to local memory generated by the corresponding
 *          NaClValidatorMemoryCreate (or NULL if not specified).
 */
typedef void (*NaClValidatorPrintStats)(FILE* file,
                                        NaClValidatorState* state,
                                        void* local_memory);

/* Defines a function to destroy local memory used by a validator function,
 * should it need to do so.
 * Parameters:
 *   state - The state of the validator.
 *   local_memory - Pointer to local memory generated by the corresponding
 *         NaClValidatorMemoryCreate (or NULL if not specified).
 */
typedef void (*NaClValidatorMemoryDestroy)(NaClValidatorState* state,
                                           void* local_memory);

/* Clear the set of registered validator functions. Allows multiple runs
 * of the validator, with different validator functions for each run.
 */
void NaClRegisterValidatorClear();

/* Registers a validator function to be called during validation.
 * Parameters are:
 *   validator - The validator function to register.
 *   post_validate - Validate global context after iterator run.
 *   print_stats - The print function to print statistics about the applied
 *     validator.
 *   memory_create - The function to call to generate local memory for
 *     the validator function (or NULL if no local memory is needed).
 *   memory_destroy - The function to call to reclaim local memory when
 *     the validator state is destroyed (or NULL if reclamation is not needed).
 */
void NaClRegisterValidator(NaClValidator validator,
                           NaClValidatorPostValidate post_validate,
                           NaClValidatorPrintStats print_stats,
                           NaClValidatorMemoryCreate memory_create,
                           NaClValidatorMemoryDestroy memory_destroy);

/* Returns the local memory associated with the given validator function,
 * or NULL if no such memory exists. Allows validators to communicate
 * shared collected information.
 * Parameters:
 *   validator - The validator function's memory you want access to.
 *   state - The current state of the validator.
 * Returns:
 *   The local memory associated with the validator (or NULL  if no such
 *   validator is known).
 */
void* NaClGetValidatorLocalMemory(NaClValidator validator,
                                  const NaClValidatorState* state);

/* Prints out a validator message for the given level.
 * Parameters:
 *   level - The level of the message, as defined in nacl_log.h
 *   state - The validator state that detected the error.
 *   format - The format string of the message to print.
 *   ... - arguments to the format string.
 */
void NaClValidatorMessage(int level,
                          NaClValidatorState* state,
                          const char* format,
                          ...) ATTRIBUTE_FORMAT_PRINTF(3, 4);

/* Prints out a validator message for the given level using
 * a variable argument list.
 * Parameters:
 *   level - The level of the message, as defined in nacl_log.h
 *   state - The validator state that detected the error.
 *   format - The format string of the message to print.
 *   ap - variable argument list for the format.
 */
void NaClValidatorVarargMessage(int level,
                                NaClValidatorState* state,
                                const char* format,
                                va_list ap);

/* Prints out a validator message for the given address.
 * Parameters:
 *   level - The level of the message, as defined in nacl_log.h
 *   state - The validator state that detected the error.
 *   addr - The address where the error occurred.
 *   format - The format string of the message to print.
 *   ... - arguments to the format string.
 */
void NaClValidatorPcAddressMessage(int level,
                                   NaClValidatorState* state,
                                   NaClPcAddress addr,
                                   const char* format,
                                   ...) ATTRIBUTE_FORMAT_PRINTF(4, 5);

/* Prints out a validator message for the given instruction.
 * Parameters:
 *   level - The level of the message, as defined in nacl_log.h
 *   state - The validator state that detected the error.
 *   inst - The instruction that caused the vaidator error.
 *   format - The format string of the message to print.
 *   ... - arguments to the format string.
 */
void NaClValidatorInstMessage(int level,
                              NaClValidatorState* state,
                              NaClInstState* inst,
                              const char* format,
                              ...) ATTRIBUTE_FORMAT_PRINTF(4, 5);

/* Returns true if the validator should quit due to previous errors. */
Bool NaClValidatorQuit(NaClValidatorState* state);

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCVALIDATE_ITER_H__ */
