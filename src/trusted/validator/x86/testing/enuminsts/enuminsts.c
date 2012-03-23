/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* enuminsts.c
 * exhaustive instruction enumeration test for x86 Native Client decoder.
 */

/* TODO(karl) - Fix the calls to the decoder for x86-32 to use the same decoder
 * as the x86-32 validator, and document how to properly test the
 * x86-32 decoder.
 */

#include "native_client/src/trusted/validator/x86/testing/enuminsts/enuminsts.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/validator/x86/testing/enuminsts/str_utils.h"

/* When non-zero, prints out additional debugging messages. */
#define kDebug 0

/* Defines the maximum buffer size used to hold text generated for the
 * disassembly of instructions, and corresponding error messages.
 */
#define kBufferSize 1024

/* When true, print more messages (i.e. verbosely). */
static Bool gVerbose = FALSE;

/* When true, don't print out messages. That is, only print instructions
 * defined by --print directives.
 */
static Bool gSilent = FALSE;

/* When true, don't report consecutive errors for consecutive instructions
 * with the same instruction mnemonic.
 */
static Bool gSkipRepeatReports = FALSE;

/* When true, check for operand differences when processing each instruction. */
static Bool gCheckOperands = FALSE;

/* Count of errors that have a high certainty of being exploitable. */
static int gSawLethalError = 0;

/* Defines the assumed text address for the test instruction */
const int kTextAddress = 0x1000;

/* If non-negative, defines the prefix to test. */
static int gPrefix = -1;

/* If non-negative, defines the opcode to test. */
static int gOpcode = -1;

/* If true, check if nacl instruction is NACL legal. */
static Bool gNaClLegal = FALSE;

/* If true, check if nacl instruction is also implemented in xed. */
static Bool gXedImplemented = FALSE;

/* If true, don't bother to do operand compares for nop's. */
static Bool gNoCompareNops = FALSE;

/* If true, only skip contiguous errors. */
static Bool gSkipContiguous = FALSE;

static const char* target_machine = "x86-"
#if NACL_TARGET_SUBARCH == 64
    "64"
#else
    "32"
#endif
    ;

/* Defines maximum number of available decoders (See comments on
 * struct NaClEnumeratorDecoder in enuminst.h for details on what
 * a decoder is.
 */
#define NACL_MAX_AVAILABLE_DECODERS 10

/* Holds the set of available decoders. */
NaClEnumeratorDecoder* kAvailableDecoders[NACL_MAX_AVAILABLE_DECODERS];

/* Holds the number of (pre)registered available decoders. */
size_t kNumAvailableDecoders;

/* This struct holds state concerning an instruction, both from the
 * various available decoders. Some of the state information is
 * redundant, preserved to avoid having to recompute it.
 */
typedef struct {
  /* Holds the set of decoders to enumerate over. */
  NaClEnumerator _enumerator;
  /* True if a decoder or comparison failed due to an error with
   * the way the instruction was decoded.
   */
  Bool _decoder_error;
} ComparedInstruction;

/* The state to use to compare instructions. */
static ComparedInstruction gCinst;

/* The name of the executable (i.e. argv[0] from the command line). */
static char *gArgv0 = "argv0";

/* Prints out summary of how to use this executable. and then exits. */
static void Usage() {
  size_t i;
  fprintf(stderr, "usage: %s [decoders] [options] [hexbytes ...]\n",
          gArgv0);
  fprintf(stderr, "\n");
  fprintf(stderr, "    Compare %s instruction decoders\n",
          target_machine);
  fprintf(stderr, "\n");
  fprintf(stderr, "    With no arguments, enumerate all %s instructions.\n",
          target_machine);
  fprintf(stderr, "    With arguments, decode each sequence of "
          "opcode bytes.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Available decoders are (select using --name):\n");
  fprintf(stderr, "\n");
  for (i = 0; i < kNumAvailableDecoders; ++i) {
    fprintf(stderr, "    %s: %s\n",
            kAvailableDecoders[i]->_id_name,
            kAvailableDecoders[i]->_usage_message);
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "Options are:\n");
  fprintf(stderr, "    --checkoperands: enables operand comparison (slow)\n");
  fprintf(stderr, "    --illegal=XX: Filter instructions to only consider "
          "those instructions\n");
  fprintf(stderr, "         that are illegal instructions, as defined by "
          "decoder XX\n");
  fprintf(stderr, "    --legal=XX: Filter instructions to only consider "
          "those instructions\n");
  fprintf(stderr, "         that are legal instructions, as defined by "
          "decoder XX\n");
  fprintf(stderr, "    --nacllegal: only compare NaCl legal instructions\n");
  fprintf(stderr, "        that are legal for nacl decoder(s).\n");
  fprintf(stderr, "    --print=XX: Prints out set of enumerated "
          "instructions,\n");
  fprintf(stderr, "                for the specified decoder XX (may be "
          "repeated).\n");
  fprintf(stderr, "                Also registers decoder XX if needed.\n");
  fprintf(stderr,
          "    --printenum: Print out enumeration of filtered instruction \n"
          "      opcode sequences\n");
  fprintf(stderr, "    --prefix=XX: only process given prefix\n");
  fprintf(stderr, "    --opcode=XX: only process given opcode for "
          "each prefix\n");
  fprintf(stderr, "    --opcode_bytes: Enumerate opcode bytes found by "
          "NaCl disassembler\n");
  fprintf(stderr, "    --nops: Don't operand compare nops.\n");
  fprintf(stderr, "    --skipcontiguous: Only skip contiguous errors\n");
  fprintf(stderr, "    --verbose: add verbose comments to output\n");
  fprintf(stderr, "    --xedimplemented: only compare NaCl instruction that "
          "are also implemented in xed\n");
  exit(gSawLethalError);
}

/* Records that unexpected internal error occurred. */
void InternalError(const char *why) {
  fprintf(stderr, "%s: Internal Error: %s\n", gArgv0, why);
  gSawLethalError = 1;
}

/* Takes the given text and sends it to each of the instruction
 * decoders to decode the first instruction in the given text.
 * Commonly the decoded instruction will be shorter than nbytes.
 */
static void ParseFirstInstruction(ComparedInstruction *cinst,
                                  uint8_t *itext, size_t nbytes) {
  size_t i;
  memcpy(cinst->_enumerator._itext, itext, nbytes);
  cinst->_enumerator._num_bytes = nbytes;
  cinst->_decoder_error = FALSE;
  for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
    cinst->_enumerator._decoder[i]->
        _parse_inst_fn(&cinst->_enumerator, kTextAddress);
  }
}

/* Prints out the instruction each decoder disassembled */
static void PrintDisassembledInstructionVariants(ComparedInstruction* cinst) {
  size_t i;
  for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
    cinst->_enumerator._decoder[i]->_print_inst_fn(&cinst->_enumerator);
  }
}

/* Print error message "why", and show instructions that failed for
 * that reason.
 */
static void CinstInternalError(ComparedInstruction* cinst, const char* why) {
  PrintDisassembledInstructionVariants(cinst);
  InternalError(why);
}

/* Initial value for last bad opcode. */
#define NOT_AN_OPCODE "not an opcode"

/* Holds the name of the last (bad) mnemonic name matched.
 * Uses the mnemonic name associated with instructions decoded
 * by the first decoder.
 */
static char last_bad_mnemonic[kBufferSize] = NOT_AN_OPCODE;

/* Returns how many decoder errors were skipped for an opcode. */
static int nSkipped = 0;

/* Changes the last bad mnemonic name to the new value. */
static void ChangeLastBadMnemonic(const char* new_value) {
  cstrncpy(last_bad_mnemonic, new_value, kBufferSize);
}

/* Reset the counters for skipping decoder errors, assuming
 * the last_bad_opcode (if non-null) was the given value.
 */
static void ResetSkipCounts(const char* last_opcode) {
  nSkipped = 0;
  ChangeLastBadMnemonic((last_opcode == NULL) ? "not an opcode" : last_opcode);
}

/* Report number of instructions with errors that were skipped, and
 * then reset the skip counts.
 */
static void ReportOnSkippedErrors(ComparedInstruction* cinst,
                                  const char* mnemonic) {
  if (nSkipped > 0 && gSilent) {
    printf("...skipped %d errors for %s\n", nSkipped, last_bad_mnemonic);
  }
  ResetSkipCounts(mnemonic);
}

/* Report a disagreement between decoders. To reduce
 * noice from uninteresting related errors, gSkipRepeatReports will
 * avoid printing consecutive errors for the same opcode.
 */
static void DecoderError(const char *why,
                         ComparedInstruction *cinst,
                         const char *details) {
  size_t i;
  cinst->_decoder_error = TRUE;

  /* Don't print errors when running silently. In such cases we
   * are generating valid opcode sequences for a decoder, and problems
   * should be ignored.
   */
  if (gSilent) return;

  /* Check if we have already reported for instruction mnemonic,
   * based on the mnemonic used by the first decoder.
   */
  if (gSkipRepeatReports) {
    /* Look for first possible name for instruction. */
    for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
      NaClEnumeratorDecoder *decoder = cinst->_enumerator._decoder[i];
      if (NULL == decoder->_get_inst_mnemonic_fn) continue;
      const char* mnemonic = cinst->_enumerator._decoder[0]->
          _get_inst_mnemonic_fn(&cinst->_enumerator);
      if (strcmp(mnemonic, last_bad_mnemonic) == 0) {
        nSkipped += 1;
        return;
      }
    }
  }

  /* If reached, did not skip, so report the error. */
  printf("**** ERROR: %s: %s\n", why, (details == NULL ? "" : details));
  PrintDisassembledInstructionVariants(cinst);
}

/* Returns the integer corresponding to a hex value in ASCII. This would
 * be faster as an array lookup, however since it's only used for command
 * line input it doesn't matter.
 */
static unsigned int A2INibble(char nibble) {
  switch (nibble) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a':
    case 'A': return 0xA;
    case 'b':
    case 'B': return 0xB;
    case 'c':
    case 'C': return 0xC;
    case 'd':
    case 'D': return 0xD;
    case 'e':
    case 'E': return 0xE;
    case 'f':
    case 'F': return 0xF;
    default: break;
  }
  fprintf(stderr, "bad hex value %c", nibble);
  exit(1);
}

/* Convert a two-character string representing an byte value in hex
 * into the corresponding byte value.
 */
static unsigned int A2IByte(char nibble1, char nibble2) {
  return A2INibble(nibble2) + A2INibble(nibble1) * 0x10;
}

#if NACL_TARGET_SUBARCH == 64
/* The instructions:
 *    48 89 e5  mov  rbp, rsp
 *    4a 89 e5  mov  rbp, rsp
 * look bad based on the simple rule, but are safe because they are
 * moving a safe address between two protected registers.
 */
static int IsSpecialSafeRegWrite(ComparedInstruction *cinst) {
  uint8_t byte0 = cinst->_enumerator._itext[0];
  uint8_t byte1 = cinst->_enumerator._itext[1];
  uint8_t byte2 = cinst->_enumerator._itext[2];

  return ((byte0 == 0x48 && byte1 == 0x89 && byte2 == 0xe5) ||
          (byte0 == 0x48 && byte1 == 0x89 && byte2 == 0xec) ||
          (byte0 == 0x48 && byte1 == 0x8b && byte2 == 0xe5) ||
          (byte0 == 0x48 && byte1 == 0x8b && byte2 == 0xec) ||
          (byte0 == 0x4a && byte1 == 0x89 && byte2 == 0xe5) ||
          (byte0 == 0x4a && byte1 == 0x89 && byte2 == 0xec) ||
          (byte0 == 0x4a && byte1 == 0x8b && byte2 == 0xe5) ||
          (byte0 == 0x4a && byte1 == 0x8b && byte2 == 0xec));
}

/* If we get this far, and Xed says it writes a NaCl reserved
 * register, this is a lethal error.
 */
static Bool BadRegWrite(ComparedInstruction *cinst) {
  size_t i;
  NaClEnumeratorDecoder* xed_decoder = NULL;
  size_t noperands;
  size_t ilength;

  /* First see if there is a xed decoder to compare against. */
  for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
    if (0 == strcmp("xed", cinst->_enumerator._decoder[i]->_id_name)) {
      xed_decoder = cinst->_enumerator._decoder[i];
      break;
    }
  }

  /* Quit if we don't have the right support for this function. */
  if ((NULL == xed_decoder) ||
      !xed_decoder->_is_inst_legal_fn(&cinst->_enumerator) ||
      (NULL == xed_decoder->_get_inst_num_operands_fn) ||
      (NULL == xed_decoder->_writes_to_reserved_reg_fn)) return 0;

  /* If reached, we found the xed decoder. */
  noperands = xed_decoder->_get_inst_num_operands_fn(&cinst->_enumerator);
  ilength = xed_decoder->_inst_length_fn(&cinst->_enumerator);
  for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
    size_t j;
    NaClEnumeratorDecoder* other_decoder = cinst->_enumerator._decoder[i];

    /* don't compare against self. */
    if (xed_decoder == other_decoder) break;

    /* don't compare if the other decoder doesn't know how to validate. */
    if (NULL == other_decoder->_segment_validates_fn) break;
    if (!other_decoder->_is_inst_legal_fn(&cinst->_enumerator)) break;

    for (j = 0; j < noperands ; j++) {
      if (xed_decoder->_writes_to_reserved_reg_fn(&cinst->_enumerator, j)) {
        if (other_decoder->_segment_validates_fn(&cinst->_enumerator,
                                                cinst->_enumerator._itext,
                                                ilength,
                                                kTextAddress)) {
          char sbuf[kBufferSize];
          /* Report problem if other decoder accepted instruction, but is
           * not one of the special safe writes.
           */
          if (!IsSpecialSafeRegWrite(cinst)) continue;
          gSawLethalError = 1;
          snprintf(sbuf, kBufferSize, "(%s) xed operand %d\n",
                   other_decoder->_id_name, (int) j);
          DecoderError("ILLEGAL REGISTER WRITE", cinst, sbuf);
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}
#endif

/* Compare operands of each decoder's disassembled instruction, reporting
 * an error if decoders disagree.
 */
static Bool AreInstOperandsEqual(ComparedInstruction *cinst) {
  size_t i;
  size_t num_decoders = 0;
  const char* operands[NACL_MAX_ENUM_DECODERS];
  NaClEnumeratorDecoder* operands_decoder[NACL_MAX_ENUM_DECODERS];
  NaClEnumerator* enumerator = &cinst->_enumerator;

  /* If no decoders, vacuously true. */
  if (0 == enumerator->_num_decoders) return FALSE;

  /* Collect operand lists and corresponding decoders. */
  for (i = 0; i < enumerator->_num_decoders; ++i) {
    /* Start by verifying that we can find operands. */
    NaClEnumeratorDecoder *decoder = enumerator->_decoder[i];
    if (NULL == decoder->_get_inst_operands_text_fn) continue;
    if (!decoder->_is_inst_legal_fn(enumerator)) continue;

    /* HACK ALERT! Special case "nops" by removing operands. We do this
     * assuming it doesn't matter what the argumnents are, the effect is
     * the same.
     */
    if (gNoCompareNops &&
        (NULL != decoder->_get_inst_mnemonic_fn) &&
        (0 == strcmp("nop", decoder->_get_inst_mnemonic_fn(enumerator)))) {
      operands[num_decoders] = "";
      operands_decoder[num_decoders++] = decoder;
      continue;
    }

    /* Not special case, record operand and decoder. */
    operands[num_decoders] = decoder->_get_inst_operands_text_fn(enumerator);
    if (NULL == operands[num_decoders]) operands[num_decoders] = "";
    operands_decoder[num_decoders++] = decoder;
  }

  /* Now test if operands compare between decoders. */
  for (i = 1; i < num_decoders; ++i) {
    if (0 != strncmp(operands[i-1], operands[i], kBufferSize)) {
      char sbuf[kBufferSize];
      snprintf(sbuf, kBufferSize, "(%s) '%s' != (%s)'%s'",
               operands_decoder[i-1]->_id_name, operands[i-1],
               operands_decoder[i]->_id_name, operands[i]);
      DecoderError("OPERAND MISMATCH", cinst, sbuf);
      return FALSE;
    }
  }

  if (kDebug && (num_decoders > 0)) {
    printf("operands match: %s\n", operands[0]);
  }
  return TRUE;
}

/* Compares mnemonic names between decoder's disassembled instructions,
 * returning true if they agree on the mnemonic name.
 */
static Bool AreInstMnemonicsEqual(ComparedInstruction *cinst) {
  size_t i;
  size_t num_decoders = 0;
  const char* name[NACL_MAX_ENUM_DECODERS];
  NaClEnumeratorDecoder* name_decoder[NACL_MAX_ENUM_DECODERS];

  /* If no decoders, vacuously true. */
  if (0 == cinst->_enumerator._num_decoders) return 0;

  /* Collect mnemonics of corresponding decoders (if defined). */
  for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
    /* Start by verifying that we can get name name. */
    NaClEnumeratorDecoder *decoder = cinst->_enumerator._decoder[i];
    if (NULL == decoder->_get_inst_mnemonic_fn) continue;
    if (!decoder->_is_inst_legal_fn(&cinst->_enumerator)) continue;

    /* Record mnemonic name and decoder for comparisons below. */
    name[num_decoders] = decoder->_get_inst_mnemonic_fn(&cinst->_enumerator);
    name_decoder[num_decoders++] = decoder;
  }

  /* Now compare mnemonics that were defined. */
  for (i = 1; i < num_decoders; ++i) {
    if (strncmp(name[i-1], name[i], kBufferSize) != 0) {
      char sbuf[kBufferSize];
      snprintf(sbuf, kBufferSize, "(%s) %s != (%s) %s",
               name_decoder[i-1]->_id_name, name[i-1],
               name_decoder[i]->_id_name, name[i]);
      DecoderError("MNEMONIC MISMATCH", cinst, sbuf);
      return FALSE;
    }
  }

  if (kDebug && (num_decoders > 0)) {
    printf("names match: %s\n", name[0]);
  }
  return TRUE;
}

/* Returns true if the decoder decodes the instruction correctly,
 * and also (to the best it can determine) validates when
 * specified to do so on the command line.
 */
static Bool ConsiderInstLegal(NaClEnumerator* enumerator,
                              NaClEnumeratorDecoder* decoder) {
  if (!decoder->_is_inst_legal_fn(enumerator)) return FALSE;
  if (!gNaClLegal) return TRUE;
  if (NULL == decoder->_maybe_inst_validates_fn) return TRUE;
  return decoder->_maybe_inst_validates_fn(enumerator);
}

/* Returns true only if the legal filters allow the instruction to
 * be processed.
 */
static Bool RemovedByInstLegalFilters(ComparedInstruction* cinst) {
  size_t i;
  for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
    if (ConsiderInstLegal(&cinst->_enumerator,
                          cinst->_enumerator._decoder[i])
        != cinst->_enumerator._decoder[i]->_legal_only) {
      return TRUE;
    }
  }
  return FALSE;
}

/* Returns true if the instruction has the same length for all decoders.
 * Reports length differences if found.
 */
static Bool AreInstructionLengthsEqual(ComparedInstruction *cinst) {
  size_t i;
  size_t num_decoders = 0;
  size_t length[NACL_MAX_ENUM_DECODERS];
  NaClEnumeratorDecoder* length_decoder[NACL_MAX_ENUM_DECODERS];

  /* If no decoders, vacuously true. */
  if (0 == cinst->_enumerator._num_decoders) return TRUE;

  /* Collect the instruction length for each decoder. */
  for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
    if (cinst->_enumerator._decoder[i]->
        _is_inst_legal_fn(&cinst->_enumerator)) {
      length[num_decoders] = cinst->_enumerator._decoder[i]->
          _inst_length_fn(&cinst->_enumerator);
      length_decoder[num_decoders] = cinst->_enumerator._decoder[i];
      ++num_decoders;
    }
  }

  /* Print out where lengths differ, if they differ. */
  for (i = 1; i < num_decoders; ++i) {
    if (length[i-1] != length[i]) {
      char sbuf[kBufferSize];
      snprintf(sbuf, kBufferSize, "(%s) %"NACL_PRIuS" != (%s) %"NACL_PRIuS,
               length_decoder[i-1]->_id_name, length[i-1],
               length_decoder[i]->_id_name, length[i]);
      DecoderError("LENGTH MISMATCH", cinst, sbuf);
      gSawLethalError = 1;
      return FALSE;
    }
  }

  if (kDebug && (num_decoders > 0)) {
    printf("length match: %"NACL_PRIuS"\n", length[0]);
  }
  return TRUE;
}

/* Print out decodings for print only directives. Returns true
 * if print only directives followed. This function is used
 * to short circuit instruction comparison, when printing of
 * filtered instructions on the command line is specified.
 */
static Bool PrintInstOnly(ComparedInstruction *cinst) {
  Bool result = FALSE;
  size_t i;
  for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
    if (cinst->_enumerator._decoder[i]->_print_only) {
      cinst->_enumerator._decoder[i]->_print_inst_fn(&cinst->_enumerator);
      result = TRUE;
    }
  }
  return result;
}

/* Test comparison for a single instruction.
 */
static void TryOneInstruction(ComparedInstruction *cinst,
                              uint8_t *itext, size_t nbytes) {
  do {
    if (gVerbose) {
      size_t i;
      printf("================");
      for (i = 0; i < nbytes; ++i) {
        printf("%02x", itext[i]);
      }
      printf("\n");
    }

    /* Try to parse the sequence of test bytes. */
    ParseFirstInstruction(cinst, itext, nbytes);

    /* Apply filters */
    if (RemovedByInstLegalFilters(cinst)) break;

    /* Print the instruction if print only option chosen, and then quit. */
    if (PrintInstOnly(cinst)) break;

    /* Apply comparison checks to the decoded instructions. */
    if (!AreInstructionLengthsEqual(cinst)) break;
    if (!AreInstMnemonicsEqual(cinst)) break;
    if (gCheckOperands && !AreInstOperandsEqual(cinst)) break;
#if NACL_TARGET_SUBARCH == 64
    if (BadRegWrite(cinst)) break;
#endif

    /* no error */
    if (gVerbose) {
      PrintDisassembledInstructionVariants(cinst);
    }
  } while (0);

  /* saw error; should have already printed stuff. Only
   * report on skipped errors if we found an error-free
   * instruction.
   */
  if (gSkipContiguous && (!cinst->_decoder_error)) {
    ReportOnSkippedErrors(cinst, NULL);
  }
}

/* Returns true if for all decoders recognize legal instructions, they use
 * less than len bytes.
 */
static int IsLegalInstShorterThan(ComparedInstruction *cinst, int len) {
  size_t i;
  Bool found_legal = FALSE;
  for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
    if (cinst->_enumerator._decoder[i]->
        _is_inst_legal_fn(&cinst->_enumerator)) {
      found_legal = TRUE;
      if (cinst->_enumerator._decoder[i]->_inst_length_fn(&cinst->_enumerator)
          >= len) return FALSE;
    }
  }
  return found_legal;
}

/* Enumerate and test all 24-bit opcode+modrm+sib patterns for a
 * particular prefix.
 */
static void TestAllWithPrefix(ComparedInstruction *cinst,
                              int prefix, int prefix_length) {
  const int kFillerByteCount = 15;
  const int kInstByteCount = 15;
  const int kIterByteCount = 3;
  uint8_t itext[kBufferSize];
  int i, op, modrm, sib;
  int min_op;
  int max_op;

  if ((gPrefix >= 0) && (gPrefix != prefix)) return;

  if (!gSilent) printf("TestAllWithPrefix(%x)\n", prefix);
  /* set up prefix */
  memcpy(itext, &prefix, prefix_length);
  /* set up filler bytes */
  for (i = 0; i < kFillerByteCount; i++) {
    itext[i + prefix_length + kIterByteCount] = (uint8_t)i;
  }
  if (gOpcode < 0) {
    min_op = 0;
    max_op = 256;
  } else {
    min_op = gOpcode;
    max_op = gOpcode + 1;
  }
  for (op = min_op; op < max_op; op++) {
    itext[prefix_length] = op;
    ResetSkipCounts(NULL);
    if (!gSilent) printf("%02x 00 00\n", op);
    for (modrm = 0; modrm < 256; modrm++) {
      itext[prefix_length + 1] = modrm;
      for (sib = 0; sib < 256; sib++) {
        itext[prefix_length + 2] = sib;
        TryOneInstruction(cinst, itext, kInstByteCount);
        /* If all decoders decode without using the sib byte, don't
         * bother to try more variants.
         */
        if (IsLegalInstShorterThan(cinst, prefix_length + 3)) break;
      }
      /* If all decoders decode without using the modrm byte, don't
       * bother to try more variants.
       */
      if (IsLegalInstShorterThan(cinst, prefix_length + 2)) break;
    }
    /* Force flushing of skipped errors, since we are now moving on
     * to the next opcode.
     */
    ReportOnSkippedErrors(cinst, NULL);
  }
}

/* For all prefixes, call TestAllWithPrefix() to enumrate and test
 * all instructions.
 */
static void TestAllInstructions(ComparedInstruction *cinst) {
  gSkipRepeatReports = TRUE;
  /* NOTE: Prefix byte order needs to be reversed when written as
   * an integer. For example, for integer prefix 0x3a0f, 0f will
   * go in instruction byte 0, and 3a in byte 1.
   */
  TestAllWithPrefix(cinst, 0, 0);
  TestAllWithPrefix(cinst, 0x0f, 1);
  TestAllWithPrefix(cinst, 0x0ff2, 2);
  TestAllWithPrefix(cinst, 0x0ff3, 2);
  TestAllWithPrefix(cinst, 0x0f66, 2);
  TestAllWithPrefix(cinst, 0x0f0f, 2);
  TestAllWithPrefix(cinst, 0x380f, 2);
  TestAllWithPrefix(cinst, 0x3a0f, 2);
  TestAllWithPrefix(cinst, 0x380f66, 3);
  TestAllWithPrefix(cinst, 0x380ff2, 3);
  TestAllWithPrefix(cinst, 0x3a0f66, 3);
}

/* Used to test one instruction at a time, for example, in regression
 * testing, or for instruction arguments from the command line.
 */
static void TestOneInstruction(ComparedInstruction *cinst, char *asciihex) {
  unsigned char ibytes[NACL_ENUM_MAX_INSTRUCTION_BYTES];
  unsigned char *ibp;
  unsigned int i, len, nbytes;

  len = strlen(asciihex);
  nbytes = len / 2;
  if (nbytes * 2 != len) {
    fprintf(stderr, "bad instruction %s\nMust be an even number of chars.\n",
            asciihex);
    exit(1);
  }
  if (nbytes > NACL_ENUM_MAX_INSTRUCTION_BYTES) {
    fprintf(stderr, "bad instruction %s\nMust be less than %d bytes\n",
            asciihex, NACL_ENUM_MAX_INSTRUCTION_BYTES);
    exit(1);
  }
  for (i = 0; i < len; i += 2) {
    ibytes[i/2] = A2IByte(asciihex[i], asciihex[i+1]);
  }
  if (gVerbose) {
    printf("trying %s (%02x%02x)\n", asciihex, ibytes[0], ibytes[1]);
  }
  TryOneInstruction(cinst, ibytes, nbytes);
}

/* A set of test cases that have caused problems in the past.
 * This is a bit stale; most of the test cases came from xed_compare.py.
 * Mostly this program has been tested by TestAllInstructions(),
 * possible since this program is much faster than xed_compare.py
 */
static void RunRegressionTests(ComparedInstruction *cinst) {
  TestOneInstruction(cinst, "0024c2");
  TestOneInstruction(cinst, "017967");
  TestOneInstruction(cinst, "0f12c0");
  TestOneInstruction(cinst, "0f13c0");
  TestOneInstruction(cinst, "0f17c0");
  TestOneInstruction(cinst, "0f01c1");
  TestOneInstruction(cinst, "0f00300000112233445566778899aa");
  TestOneInstruction(cinst, "cc");
  TestOneInstruction(cinst, "C3");
  TestOneInstruction(cinst, "0f00290000112233445566778899aa");
  TestOneInstruction(cinst, "80e4f7");
  TestOneInstruction(cinst, "e9a0ffffff");
  TestOneInstruction(cinst, "4883ec08");
  TestOneInstruction(cinst, "0f00040500112233445566778899aa");
  /* Below are newly discovered mistakes in call instructions, where the wrong
   * byte length was required by x86-64 nacl validator.
   */
  TestOneInstruction(cinst, "262e7e00");
  TestOneInstruction(cinst, "2e3e7900");
}

/* Register the decoder with the given name, returning the corresponding
 * decoder.
 */
static NaClEnumeratorDecoder*
NaClRegisterEnumeratorDecoder(ComparedInstruction* cinst,
                             const char* decoder_name) {
  /* First check if already registered. If so, simply return it. */
  size_t i;
  for (i = 0; i < cinst->_enumerator._num_decoders; ++i) {
    if (0 == strcmp(cinst->_enumerator._decoder[i]->_id_name, decoder_name)) {
      return cinst->_enumerator._decoder[i];
    }
  }

  /* If reached, not registered yet. See if one with the given name
   * is available (i.e. preregistered).
   */
  for (i = 0; i < kNumAvailableDecoders; ++i) {
    NaClEnumeratorDecoder* decoder = kAvailableDecoders[i];
    if (0 == strcmp(decoder->_id_name, decoder_name)) {
      if (cinst->_enumerator._num_decoders < NACL_MAX_ENUM_DECODERS) {
        cinst->_enumerator._decoder[cinst->_enumerator._num_decoders++] =
            decoder;
        return decoder;
      }
    }
  }

  /* If reached, can't find a decoder with the given name, abort. */
  printf("Can't find decoder '%s', aborting!\n", decoder_name);
  exit(1);
}

/* Install legal filter values for corresponding available decoders. */
static void NaClInstallLegalFilter(ComparedInstruction* cinst,
                                   const char* decoder_name,
                                   Bool new_value) {
  NaClRegisterEnumeratorDecoder(cinst, decoder_name)->_legal_only = new_value;
}

/* Very simple command line arg parsing. Returns index to the first
 * arg that doesn't begin with '--', or argc if there are none.
 */
static int ParseArgs(ComparedInstruction* cinst, int argc, char *argv[]) {
  int i;
  uint32_t prefix;
  uint32_t opcode;
  char* cstr_value;

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      do {
        if (strcmp(argv[i], "--help") == 0) {
          Usage();
        } else if (GrokBoolFlag("--checkoperands", argv[i], &gCheckOperands) ||
                   GrokBoolFlag("--nacllegal", argv[i],
                                &gNaClLegal) ||
                   GrokBoolFlag("--xedimplemented", argv[i],
                                &gXedImplemented) ||
                   GrokBoolFlag("--nops", argv[i], &gNoCompareNops)||
                   GrokBoolFlag("--opcode_bytes", argv[i],
                                &cinst->_enumerator._print_opcode_bytes_only) ||
                   GrokBoolFlag("--printenum", argv[i],
                                &cinst->_enumerator.
                                _print_enumerated_instruction) ||
                   GrokBoolFlag("--skipcontiguous", argv[i],
                                &gSkipContiguous) ||
                   GrokBoolFlag("--verbose", argv[i], &gVerbose)) {
        } else if (GrokCstringFlag("--print", argv[i], &cstr_value)) {
          NaClRegisterEnumeratorDecoder(cinst, cstr_value)->_print_only = TRUE;
        } else if (GrokUint32HexFlag("--prefix", argv[i], &prefix)) {
          gPrefix = (int) prefix;
          printf("using prefix %x\n", gPrefix);
        } else if (GrokUint32HexFlag("--opcode", argv[i], &opcode) &&
                   opcode < 256) {
          gOpcode = (int) opcode;
          printf("using opcode %x\n", gOpcode);
        } else if (GrokCstringFlag("--legal", argv[i], &cstr_value)) {
          NaClInstallLegalFilter(cinst, cstr_value, TRUE);
        } else if (GrokCstringFlag("--illegal", argv[i], &cstr_value)) {
          NaClInstallLegalFilter(cinst, cstr_value, FALSE);
        } else if (argv[i] == strfind(argv[i], "--")) {
          NaClRegisterEnumeratorDecoder(cinst, argv[i] + 2);
        } else {
          fprintf(stderr, "Can't recognize option %s\n", argv[i]);
          exit(1);
        }
      } while (0);
    } else return i;
  }
  return argc;
}

/* Define set of available enumerator decoders. */
static void NaClPreregisterEnumeratorDecoder(ComparedInstruction* cinst,
                                            NaClEnumeratorDecoder* decoder) {
  if (kNumAvailableDecoders >= NACL_MAX_AVAILABLE_DECODERS) {
    printf("Too many preregistered enumerator decoders\n");
    exit(1);
  }
  kAvailableDecoders[kNumAvailableDecoders++] = decoder;
}

/* Define decoders that can be registered. */
extern NaClEnumeratorDecoder* RegisterXedDecoder();
extern NaClEnumeratorDecoder* RegisterNaClDecoder();

/* Initialize the set of available decoders. */
static void NaClInitializeAvailableDecoders() {
  kNumAvailableDecoders = 0;
#ifdef NACL_XED_DECODER
  NaClPreregisterEnumeratorDecoder(&gCinst, RegisterXedDecoder());
#endif
  NaClPreregisterEnumeratorDecoder(&gCinst, RegisterNaClDecoder());
}

/* Initialize the ComparedInstruction data structure. */
static void NaClInitializeComparedInstruction(ComparedInstruction* cinst) {
  cinst->_enumerator._num_decoders = 0;
  cinst->_enumerator._print_opcode_bytes_only = FALSE;
  cinst->_enumerator._print_enumerated_instruction = FALSE;
}


/* Install parsed globals, as appropriate into the corresponding
 * decoders.
 */
static void InstallFlags(NaClEnumerator* enumerator) {
  size_t i;
  for (i = 0; i < enumerator->_num_decoders; ++i) {
    enumerator->_decoder[i]->_install_flag_fn(enumerator,
                                             "--nops",
                                             &gNoCompareNops);
    enumerator->_decoder[i]->_install_flag_fn(enumerator,
                                             "--xedimplemented",
                                             &gXedImplemented);
  }
  if (enumerator->_print_opcode_bytes_only ||
      enumerator->_print_enumerated_instruction) {
    gSilent = TRUE;
  }
}

int main(int argc, char *argv[]) {
  int testargs;

  NaClLogModuleInit();
  NaClLogSetVerbosity(LOG_FATAL);
  NaClInitializeAvailableDecoders();
  NaClInitializeComparedInstruction(&gCinst);


  if (argc == 1) Usage();

  gArgv0 = argv[0];
  testargs = ParseArgs(&gCinst, argc, argv);
  InstallFlags(&gCinst._enumerator);


  if (gCinst._enumerator._num_decoders == 0) {
    fprintf(stderr, "No decoder specified, can't continue\n");
    exit(1);
  }

  if (testargs == argc) {
    if (gPrefix < 0) RunRegressionTests(&gCinst);
    TestAllInstructions(&gCinst);
  } else {
    int i;
    gSilent = FALSE;
    gVerbose = TRUE;
    for (i = testargs; i < argc; ++i) {
      TestOneInstruction(&gCinst, argv[i]);
    }
  }
  exit(gSawLethalError);
}
