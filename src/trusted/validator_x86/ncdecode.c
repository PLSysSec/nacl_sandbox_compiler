/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * ncdecode.c - table driven decoder for Native Client
 *
 * Most x86 decoders I've looked at are big case statements. While
 * this organization is fairly transparent and obvious, it tends to
 * lead to messy control flow (gotos, etc.) that make the decoder
 * more complicated, hence harder to maintain and harder to validate.
 *
 * This decoder is table driven, which will hopefully result in
 * substantially less code. Although the code+tables may be more
 * lines of code than a decoder built around a switch statement,
 * the smaller amount of actual procedural code and the regular
 * structure of the tables should make it easier to understand,
 * debug, and easier to become confident the decoder is correct.
 *
 * As it is specialized to Native Client, this decoder can also
 * benefit from any exclusions or simplifications we decide to
 * make in the dialect of x86 machine code accepted by Native
 * Client. Any such simplifications should ultimately be easily
 * recognized by inspection of the decoder configuration tables.
 * ALSO, the decoder mostly needs to worry about accurate
 * instruction lengths and finding opcodes. It does not need
 * to completely resolve the operands of all instructions.
 */

/* To turn on debugging of instruction decoding, change value of
 * DEBUGGING to 1.
 */
#define DEBUGGING 0

#include "native_client/src/shared/utils/debugging.h"

/* Turn this flag on, so that we can print out human readable
 * names for type NaClInsttype.
 */
#define NEEDSNACLINSTTYPESTRING

#include <stdio.h>
#include <assert.h>
#include "native_client/src/trusted/validator_x86/ncdecode.h"
#include "gen/native_client/src/trusted/validator_x86/ncdecodetab.h"

/* Generates a print name for the given NCDecodeImmediateType. */
static const char* NCDecodeImmediateTypeName(NCDecodeImmediateType type) {
  switch(type) {
    case IMM_UNKNOWN: return "IMM_UNKNOWN";
    case IMM_NONE: return "IMM_NONE";
    case IMM_FIXED1: return "IMM_FIXED1";
    case IMM_FIXED2: return "IMM_FIXED2";
    case IMM_FIXED3: return "IMM_FIXED3";
    case IMM_FIXED4: return "IMM_FIXED4";
    case IMM_DATAV: return "IMM_DATAV";
    case IMM_ADDRV: return "IMM_ADDRV";
    case IMM_GROUP3_F6: return "IMM_GROUP3_F6";
    case IMM_GROUP3_F7: return "IMM_GROUP3_F7";
    case IMM_FARPTR: return "IMM_FARPTR";
    case IMM_MOV_DATAV: return "IMM_MOV_DATAV";
    default: assert(0);
  }

  /* NOTREACHED */
  return NULL;
}

/* Prints out the contents of the given OpInfo. */
static void PrintOpInfo(const struct OpInfo* info) {
  printf("opinfo(%s, hasmrm=%u, immtype=%s, opinmrm=%d)\n",
         NaClInstTypeString(info->insttype),
         info->hasmrmbyte,
         NCDecodeImmediateTypeName(info->immtype),
         info->opinmrm);
}

/* later this will make decoding x87 instructions a bit more concise. */
static const struct OpInfo* kDecodeX87Op[8] = { kDecode87D8,
                                                kDecode87D9,
                                                kDecode87DA,
                                                kDecode87DB,
                                                kDecode87DC,
                                                kDecode87DD,
                                                kDecode87DE,
                                                kDecode87DF };

static void NullDecoderAction(const struct NCDecoderState* mstate) {
  UNREFERENCED_PARAMETER(mstate);
}
static void NullDecoderStats(struct NCValidatorState* vstate) {
  UNREFERENCED_PARAMETER(vstate);
}
static void DefaultInternalError(struct NCValidatorState* vstate) {
  UNREFERENCED_PARAMETER(vstate);
}

/* TODO(bradchen): Thread safety? */
/* TODO(bradchen): More comments */
NCDecoderAction g_DecoderAction = NullDecoderAction;
NCDecoderStats g_NewSegment = NullDecoderStats;
NCDecoderStats g_InternalError = DefaultInternalError;
NCDecoderStats g_SegFault = NullDecoderStats;

/* Error Condition Handling */
static void ErrorSegmentation(struct NCValidatorState* vstate) {
  fprintf(stdout, "ErrorSegmentation\n");
  /* When the decoder is used by the NaCl validator    */
  /* the validator provides an error handler that does */
  /* the necessary bookeeping to track these errors.   */
  g_SegFault(vstate);
}

static void ErrorInternal(struct NCValidatorState* vstate) {
  fprintf(stdout, "ErrorInternal\n");
  /* When the decoder is used by the NaCl validator    */
  /* the validator provides an error handler that does */
  /* the necessary bookeeping to track these errors.   */
  g_InternalError(vstate);
}

/* Defines how to handle errors found while parsing the memory segment. */
static void NCRemainingMemoryInternalError(NCRemainingMemoryError error,
                                           struct NCRemainingMemory* memory) {
  /* Don't do anything for memory overflow! Let NCDecodeSegment generate
   * the corresponding segmentation error. This allows us to back out overflow
   * if a predefined nop is matched.
   */
  if (NCRemainingMemoryOverflow != error) {
    NCRemainingMemoryReportError(error, memory);
    ErrorInternal(memory->vstate);
  }
}

void InitDecoder(struct NCDecoderState* mstate) {
  NCInstBytesInit(&mstate->inst.bytes);
  mstate->inst.vaddr = mstate->vpc;
  mstate->inst.prefixbytes = 0;
  mstate->inst.prefixmask = 0;
  mstate->inst.num_opbytes = 1;  /* unless proven otherwise. */
  mstate->inst.hassibbyte = 0;
  mstate->inst.mrm = 0;
  mstate->inst.immtype = IMM_UNKNOWN;
  mstate->inst.immbytes = 0;
  mstate->inst.dispbytes = 0;
  mstate->inst.rexprefix = 0;
  mstate->opinfo = NULL;
}

/* Returns the number of bytes defined for the operand of the instruction. */
static int ExtractOperandSize(NCDecoderState* mstate) {
  if (NACL_TARGET_SUBARCH == 64 &&
      mstate->inst.rexprefix && mstate->inst.rexprefix & 0x8) {
    return 8;
  }
  if (mstate->inst.prefixmask & kPrefixDATA16) {
    return 2;
  }
  return 4;
}

/* at most four prefix bytes are allowed */
void ConsumePrefixBytes(struct NCDecoderState* mstate) {
  uint8_t nb;
  int ii;
  uint32_t prefix_form;

  for (ii = 0; ii < kMaxPrefixBytes; ++ii) {
    nb = NCRemainingMemoryGetNext(mstate->memory);
    prefix_form = kPrefixTable[nb];
    if (prefix_form == 0) return;
    DEBUG( printf("Consume prefix[%d]: %02x => %x\n", ii, nb, prefix_form) );
    mstate->inst.prefixmask |= prefix_form;
    mstate->inst.prefixmask |= kPrefixTable[nb];
    mstate->inst.prefixbytes += 1;
    NCInstBytesRead(&mstate->inst.bytes);
    DEBUG( printf("  prefix mask: %08x\n", mstate->inst.prefixmask) );
    if (NACL_TARGET_SUBARCH == 64 && prefix_form == kPrefixREX) {
      mstate->inst.rexprefix = nb;
      /* REX prefix must be last prefix. */
      return;
    }
  }
}

static const struct OpInfo *GetExtendedOpInfo(struct NCDecoderState* mstate,
                                              uint8_t opbyte2) {
  uint32_t pm;
  pm = mstate->inst.prefixmask;
  if ((pm & (kPrefixDATA16 | kPrefixREPNE | kPrefixREP)) == 0) {
    return &kDecode0FXXOp[opbyte2];
  } else if (pm & kPrefixDATA16) {
    return &kDecode660FXXOp[opbyte2];
  } else if (pm & kPrefixREPNE) {
    return &kDecodeF20FXXOp[opbyte2];
  } else if (pm & kPrefixREP) {
    return &kDecodeF30FXXOp[opbyte2];
  }
  ErrorInternal(mstate->vstate);
  return mstate->opinfo;
}

static void GetX87OpInfo(struct NCDecoderState* mstate) {
  /* WAIT is an x87 instruction but not in the coproc opcode space. */
  uint8_t op1 = NCInstBytesByte(&mstate->inst_bytes, mstate->inst.prefixbytes);
  if (op1 < kFirstX87Opcode || op1 > kLastX87Opcode) {
    if (op1 != kWAITOp) ErrorInternal(mstate->vstate);
    return;
  }
  mstate->opinfo = &kDecodeX87Op[op1 - kFirstX87Opcode][mstate->inst.mrm];
  DEBUG( printf("NACL_X87 op1 = %02x, ", op1);
         PrintOpInfo(mstate->opinfo) );
}

void ConsumeOpcodeBytes(struct NCDecoderState* mstate) {
  uint8_t opcode = NCInstBytesRead(&mstate->inst.bytes);
  mstate->opinfo = &kDecode1ByteOp[opcode];
  DEBUG( printf("NACLi_1BYTE: opcode = %02x, ", opcode);
         PrintOpInfo(mstate->opinfo) );
  if (opcode == kTwoByteOpcodeByte1) {
    uint8_t opcode2 = NCInstBytesRead(&mstate->inst.bytes);
    mstate->opinfo = GetExtendedOpInfo(mstate, opcode2);
    DEBUG( printf("NACLi_2BYTE: opcode2 = %02x, ", opcode2);
           PrintOpInfo(mstate->opinfo) );
    mstate->inst.num_opbytes = 2;
    if (mstate->opinfo->insttype == NACLi_3BYTE) {
      uint8_t opcode3 = NCInstBytesRead(&mstate->inst.bytes);
      uint32_t pm;
      pm = mstate->inst.prefixmask;
      mstate->inst.num_opbytes = 3;

      DEBUG( printf("NACLi_3BYTE: opcode3 = %02x, ", opcode3) );
      switch (opcode2) {
      case 0x38:        /* SSSE3, SSE4 */
        if (pm & kPrefixDATA16) {
          mstate->opinfo = &kDecode660F38Op[opcode3];
        } else if (pm & kPrefixREPNE) {
          mstate->opinfo = &kDecodeF20F38Op[opcode3];
        } else if (pm == 0) {
          mstate->opinfo = &kDecode0F38Op[opcode3];
        } else {
          /* Other prefixes like F3 cause an undefined instruction error. */
          /* Note from decoder table that NACLi_3BYTE is only used with   */
          /* data16 and repne prefixes.                                   */
          ErrorInternal(mstate->vstate);
        }
        break;
      case 0x3A:        /* SSSE3, SSE4 */
        if (pm & kPrefixDATA16) {
          mstate->opinfo = &kDecode660F3AOp[opcode3];
        } else if (pm == 0) {
          mstate->opinfo = &kDecode0F3AOp[opcode3];
        } else {
          /* Other prefixes like F3 cause an undefined instruction error. */
          /* Note from decoder table that NACLi_3BYTE is only used with   */
          /* data16 and repne prefixes.                                   */
          ErrorInternal(mstate->vstate);
        }
        break;
      default:
        /* if this happens there is a decoding table bug */
        ErrorInternal(mstate->vstate);
        break;
      }
      DEBUG( PrintOpInfo(mstate->opinfo) );
    }
  }
  mstate->inst.immtype = mstate->opinfo->immtype;
}

void ConsumeModRM(struct NCDecoderState* mstate) {
  if (mstate->opinfo->hasmrmbyte != 0) {
    const uint8_t mrm = NCInstBytesRead(&mstate->inst.bytes);
    DEBUG( printf("Mod/RM byte: %02x\n", mrm) );
    mstate->inst.mrm = mrm;
    if (mstate->opinfo->insttype == NACLi_X87) {
      GetX87OpInfo(mstate);
    }
    if (mstate->opinfo->opinmrm) {
      const struct OpInfo *mopinfo =
        &kDecodeModRMOp[mstate->opinfo->opinmrm][modrm_opcode(mrm)];
      mstate->opinfo = mopinfo;
      DEBUG( printf("NACLi_opinmrm: modrm.opcode = %x, ", modrm_opcode(mrm));
             PrintOpInfo(mstate->opinfo) );
      if (mstate->inst.immtype == IMM_UNKNOWN) {
        assert(0);
        mstate->inst.immtype = mopinfo->immtype;
      }
      /* handle weird case for 0xff TEST Ib/Iv */
      if (modrm_opcode(mrm) == 0) {
        if (mstate->inst.immtype == IMM_GROUP3_F6) {
          mstate->inst.immtype = IMM_FIXED1;
        }
        if (mstate->inst.immtype == IMM_GROUP3_F7) {
          mstate->inst.immtype = IMM_DATAV;
        }
      }
      DEBUG( printf("  immtype = %s\n",
                    NCDecodeImmediateTypeName(mstate->inst.immtype)) );
    }
    if (mstate->inst.prefixmask & kPrefixADDR16) {
      switch (modrm_mod(mrm)) {
        case 0:
          if (modrm_rm(mrm) == 0x06) mstate->inst.dispbytes = 2;  /* disp16 */
          else mstate->inst.dispbytes = 0;
          break;
        case 1:
          mstate->inst.dispbytes = 1;           /* disp8 */
          break;
        case 2:
          mstate->inst.dispbytes = 2;           /* disp16 */
          break;
        case 3:
          mstate->inst.dispbytes = 0;           /* no disp */
          break;
        default:
          ErrorInternal(mstate->vstate);
      }
      mstate->inst.hassibbyte = 0;
    } else {
      switch (modrm_mod(mrm)) {
        case 0:
          if (modrm_rm(mrm) == 0x05) mstate->inst.dispbytes = 4;  /* disp32 */
          else mstate->inst.dispbytes = 0;
          break;
        case 1:
          mstate->inst.dispbytes = 1;           /* disp8 */
          break;
        case 2:
          mstate->inst.dispbytes = 4;           /* disp32 */
          break;
        case 3:
          mstate->inst.dispbytes = 0;           /* no disp */
          break;
        default:
          ErrorInternal(mstate->vstate);
      }
      mstate->inst.hassibbyte = ((modrm_rm(mrm) == 0x04) &&
                                 (modrm_mod(mrm) != 3));
    }
  }
  DEBUG( printf("  dispbytes = %d, hasibbyte = %d\n",
                mstate->inst.dispbytes, mstate->inst.hassibbyte) );
}

void ConsumeSIB(struct NCDecoderState* mstate) {
  if (mstate->inst.hassibbyte != 0) {
    const uint8_t sib = NCInstBytesRead(&mstate->inst.bytes);
    if (sib_base(sib) == 0x05) {
      switch (modrm_mod(mstate->inst.mrm)) {
      case 0: mstate->inst.dispbytes = 4; break;
      case 1: mstate->inst.dispbytes = 1; break;
      case 2: mstate->inst.dispbytes = 4; break;
      case 3:
      default:
        ErrorInternal(mstate->vstate);
      }
    }
    DEBUG( printf("sib byte: %02x, dispbytes = %d\n",
                  sib, mstate->inst.dispbytes) );
  }
}

void ConsumeID(struct NCDecoderState* mstate) {
  if (mstate->inst.immtype == IMM_UNKNOWN) {
    ErrorInternal(mstate->vstate);
  }
  /* NOTE: NaCl allows at most one prefix byte (for 32-bit mode) */
  if (mstate->inst.immtype == IMM_MOV_DATAV) {
    mstate->inst.immbytes = ExtractOperandSize(mstate);
  } else if (mstate->inst.prefixmask & kPrefixDATA16) {
    mstate->inst.immbytes = kImmTypeToSize66[mstate->inst.immtype];
  } else if (mstate->inst.prefixmask & kPrefixADDR16) {
    mstate->inst.immbytes = kImmTypeToSize67[mstate->inst.immtype];
  } else {
    mstate->inst.immbytes = kImmTypeToSize[mstate->inst.immtype];
  }
  NCInstBytesReadBytes((ssize_t) mstate->inst.immbytes,
                       &mstate->inst.bytes);
  NCInstBytesReadBytes((ssize_t) mstate->inst.dispbytes,
                       &mstate->inst.bytes);
  DEBUG(printf("ID: %d disp bytes, %d imm bytes\n",
               mstate->inst.dispbytes, mstate->inst.immbytes));
}

/* Actually this routine is special for 3DNow instructions */
void MaybeGet3ByteOpInfo(struct NCDecoderState* mstate) {
  if (mstate->opinfo->insttype == NACLi_3DNOW) {
    uint8_t opbyte1 = NCInstBytesByte(&mstate->inst_bytes,
                                      mstate->inst.prefixbytes);
    uint8_t opbyte2 = NCInstBytesByte(&mstate->inst_bytes,
                                      mstate->inst.prefixbytes + 1);
    if (opbyte1 == kTwoByteOpcodeByte1 &&
        opbyte2 == k3DNowOpcodeByte2) {
      uint8_t immbyte =
          NCInstBytesByte(&mstate->inst_bytes, mstate->inst.bytes.length - 1);
      mstate->opinfo = &kDecode0F0FOp[immbyte];
      DEBUG( printf(
                 "NACLi_3DNOW: byte1 = %02x, byte2 = %02x, immbyte = %02x,\n  ",
                 opbyte1, opbyte2, immbyte);
             PrintOpInfo(mstate->opinfo) );
    }
  }
}

void NCDecodeRegisterCallbacks(NCDecoderAction decoderaction,
                               NCDecoderStats newsegment,
                               NCDecoderStats segfault,
                               NCDecoderStats internalerror) {
  /* Clear old definitions before continuing. */
  g_DecoderAction = NullDecoderAction;
  g_NewSegment = NullDecoderStats;
  g_InternalError = DefaultInternalError;
  g_SegFault = NullDecoderStats;
  if (decoderaction != NULL) g_DecoderAction = decoderaction;
  if (newsegment != NULL) g_NewSegment = newsegment;
  if (segfault != NULL) g_SegFault = segfault;
  if (internalerror != NULL) g_InternalError = internalerror;
}

struct NCDecoderState *PreviousInst(const struct NCDecoderState* mstate,
                                    int nindex) {
  int index = (mstate->dbindex + nindex + kDecodeBufferSize)
      & (kDecodeBufferSize - 1);
  return &mstate->decodebuffer[index];
}

/* Initialize the ring buffer used to store decoded instructions
 *
 * mbase: The actual address in memory of the instructions being iterated.
 * vbase: The virtual address instructions will be executed from.
 * decodebuffer: Ring buffer containing kDecodeBufferSize elements (output)
 * mstate: Current instruction pointer into the ring buffer (output)
 */
static void InitDecodeBuffer(uint8_t *mbase, NaClPcAddress vbase,
                             NaClMemorySize size,
                             struct NCValidatorState* vstate,
                             NCRemainingMemory* memory,
                             struct NCDecoderState* decodebuffer,
                             struct NCDecoderState** mstate) {
  int dbindex;
  NCRemainingMemoryInit(mbase, size, memory);
  memory->error_fn = NCRemainingMemoryInternalError;
  memory->vstate = vstate;
  for (dbindex = 0; dbindex < kDecodeBufferSize; ++dbindex) {
    decodebuffer[dbindex].memory       = memory;
    decodebuffer[dbindex].vstate       = vstate;
    decodebuffer[dbindex].decodebuffer = decodebuffer;
    decodebuffer[dbindex].dbindex      = dbindex;
    decodebuffer[dbindex].vpc          = 0;
    NCInstBytesInitMemory(&decodebuffer[dbindex].inst.bytes, memory);
    NCInstBytesPtrInit((NCInstBytesPtr*) &decodebuffer[dbindex].inst_bytes,
                       &decodebuffer[dbindex].inst.bytes);

  }
  (*mstate)           = &decodebuffer[0];
  (*mstate)->vpc      = vbase;
}

/* Modify the current instruction pointer to point to the next instruction
 * in the ring buffer.  Reset the state of that next instruction.
 */
static void IncrementDecodeBuffer(struct NCDecoderState** mstate) {
  /* giving PreviousInst a positive number will get NextInst
   * better to keep the buffer switching logic in one place
   */
  struct NCDecoderState* mnextstate = PreviousInst(*mstate, 1);
  mnextstate->vpc = (*mstate)->vpc + (*mstate)->inst.bytes.length;
  *mstate = mnextstate;
}

/* Get the i-th byte of the current instruction being parsed. */
static uint8_t GetInstByte(struct NCDecoderState* mstate, ssize_t i) {
  if (i < mstate->inst.bytes.length) {
    return mstate->inst.bytes.byte[i];
  } else {
    return NCRemainingMemoryLookahead(mstate->memory,
                                      i - mstate->inst.bytes.length);
  }
}

/* Consume a predefined nop byte sequence, if a match can be found.
 * Further, if found, replace the currently matched instruction with
 * the consumed predefined nop.
 */
static void ConsumePredefinedNop(struct NCDecoderState* mstate) {
  /* Do maximal match of possible nops */
  uint8_t pos = 0;
  struct OpInfo* matching_opinfo = NULL;
  ssize_t matching_length = 0;
  NCNopTrieNode* next = (NCNopTrieNode*) (kNcNopTrieNode + 0);
  uint8_t byte = GetInstByte(mstate, pos);
  while (NULL != next) {
    if (byte == next->matching_byte) {
      DEBUG(printf("NOP match byte: 0x%02x\n", (int) byte));
      byte = GetInstByte(mstate, ++pos);
      if (NULL != next->matching_opinfo) {
        DEBUG(printf("NOP matched rule! %d\n", pos));
        matching_opinfo = next->matching_opinfo;
        matching_length = pos;
      }
      next = next->success;
    } else {
      next = next->fail;
    }
  }
  if (NULL == matching_opinfo) {
    DEBUG(printf("NOP match failed!\n"));
  } else {
    DEBUG(printf("NOP match succeeds! Using last matched rule.\n"));
    NCRemainingMemoryReset(mstate->memory);
    InitDecoder(mstate);
    NCInstBytesReadBytes(matching_length, &mstate->inst.bytes);
    mstate->opinfo = matching_opinfo;
  }
}

/* If we didn't find a good instruction, try to consume one of the
 * predefined NOP's.
 */
static void MaybeConsumePredefinedNop(struct NCDecoderState* mstate) {
  switch (mstate->opinfo->insttype) {
    case NACLi_UNDEFINED:
    case NACLi_INVALID:
    case NACLi_ILLEGAL:
      ConsumePredefinedNop(mstate);
      break;
    default:
      break;
  }
}

/* All of the actions needed to read one additional instruction into mstate.
 */
static void ConsumeNextInstruction(struct NCDecoderState* mstate) {
  DEBUG( printf("Decoding instruction at %"NACL_PRIxNaClPcAddress":\n",
                mstate->vpc) );
  InitDecoder(mstate);
  ConsumePrefixBytes(mstate);
  ConsumeOpcodeBytes(mstate);
  ConsumeModRM(mstate);
  ConsumeSIB(mstate);
  ConsumeID(mstate);
  MaybeGet3ByteOpInfo(mstate);
  MaybeConsumePredefinedNop(mstate);
}

/* The actual decoder */
void NCDecodeSegment(uint8_t *mbase, NaClPcAddress vbase,
                     NaClMemorySize size,
                     struct NCValidatorState* vstate) {
  const NaClPcAddress vlimit = vbase + size;
  struct NCDecoderState decodebuffer[kDecodeBufferSize];
  struct NCDecoderState *mstate;
  NCRemainingMemory memory;
  InitDecodeBuffer(mbase, vbase, size, vstate, &memory, decodebuffer, &mstate);

  DEBUG( printf("DecodeSegment(%p[%"NACL_PRIxNaClPcAddress
                "-%"NACL_PRIxNaClPcAddress"])\n",
                (void*) mbase, vbase, vlimit) );
  g_NewSegment(mstate->vstate);
  while (mstate->vpc < vlimit) {
    ConsumeNextInstruction(mstate);
    if (memory.overflow_count) {
      NaClPcAddress newpc = mstate->vpc + mstate->inst.bytes.length;
      fprintf(stdout, "%"NACL_PRIxNaClPcAddress" > %"NACL_PRIxNaClPcAddress
              " (read overflow of %d bytes)\n",
              newpc, vlimit, memory.overflow_count);
      ErrorSegmentation(vstate);
      break;
    }
    g_DecoderAction(mstate);
    /* get ready for next round */
    IncrementDecodeBuffer(&mstate);
  }
}

/* The actual decoder -- decodes two instruction segments in parallel */
void NCDecodeSegmentPair(uint8_t *mbase_old, uint8_t *mbase_new,
                         NaClPcAddress vbase, NaClMemorySize size,
                         struct NCValidatorState* vstate,
                         NCDecoderPairAction action) {
  const NaClPcAddress vlimit = vbase + size;
  struct NCDecoderState decodebuffer_old[kDecodeBufferSize];
  struct NCDecoderState decodebuffer_new[kDecodeBufferSize];
  struct NCDecoderState *mstate_old;
  struct NCDecoderState *mstate_new;
  NCRemainingMemory memory_old;
  NCRemainingMemory memory_new;
  NaClPcAddress newpc_old;
  NaClPcAddress newpc_new;

  InitDecodeBuffer(mbase_old, vbase, size, vstate, &memory_old,
                   decodebuffer_old, &mstate_old);
  InitDecodeBuffer(mbase_new, vbase, size, vstate, &memory_new,
                   decodebuffer_new, &mstate_new);

  DEBUG( printf("DecodeSegmentPair(%"NACL_PRIxNaClPcAddress
                "-%"NACL_PRIxNaClPcAddress")\n",
                vbase, vlimit) );
  g_NewSegment(mstate_new->vstate);
  while (mstate_old->vpc < vlimit && mstate_new->vpc < vlimit) {
    ConsumeNextInstruction(mstate_old);
    ConsumeNextInstruction(mstate_new);
    newpc_old = mstate_old->vpc + mstate_old->inst.bytes.length;
    newpc_new = mstate_new->vpc + mstate_old->inst.bytes.length;

    if (newpc_old != newpc_new) {
      fprintf(stdout, "misaligned replacement code "
              "%"NACL_PRIxNaClPcAddress" != %"NACL_PRIxNaClPcAddress"\n",
              newpc_old, newpc_new);
      ErrorSegmentation(vstate);
      break;
    }

    if (newpc_new > vlimit) {
      fprintf(stdout, "%"NACL_PRIxNaClPcAddress" > %"NACL_PRIxNaClPcAddress"\n",
              newpc_new, vlimit);
      ErrorSegmentation(vstate);
      break;
    }

    action(mstate_old, mstate_new);

    /* get ready for next round */
    IncrementDecodeBuffer(&mstate_old);
    IncrementDecodeBuffer(&mstate_new);
  }
}
