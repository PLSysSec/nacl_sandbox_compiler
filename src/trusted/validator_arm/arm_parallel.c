/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * Defines ARM parallel operations.
 */

#include <string.h>

#include "native_client/src/trusted/validator_arm/arm_parallel.h"
#include "native_client/src/trusted/validator_arm/arm_insts.h"
#include "native_client/src/trusted/validator_arm/arm_inst_modeling.h"

/*
 * Model parallel add/subtract operation specific data that needs to
 * be provided, in addition to the kParallelAddSubPattern default choices.
 */
typedef struct MetaParallelAddSubOp {
  const char* name;         /* The name of the parallel operation. */
  ArmInstKind kind;         /* The kind of operation. */
  int32_t shift;            /* The expected shift value of the pattern. */
  int32_t immediate;        /* The expected immediate value of the pattern. */
} MetaParallelAddSubOp;

static const MetaParallelAddSubOp kMetaParallelAddSubPattern[] = {
  { "add16",     ARM_QADD16,   0x0, 0x1 },
  { "add8",      ARM_QADD8,    0x0, 0x9 },
  { "addsubx",   ARM_QADDSUBX, 0x1, 0x1 },
  { "sub16",     ARM_QSUB16,   0x3, 0x1 },
  { "sub8",      ARM_QSUB8,    0x3, 0x9 },
  { "subaddx",   ARM_QSUBADDX, 0x2, 0x1 },
  /* Dummy field to end list. */
  { NULL,        ARM_UNKNOWN_INST, 0x0, 0x0 }
};

/* Define patterns for each parallel add/sub */
static const ModeledOpInfo kParallelAddSubPattern =
{ "",
  ARM_UNKNOWN_INST,
  kArmUndefinedAccessModeName,
  ARM_DP_RS,
  TRUE,
  "%C\t%x, %r, %z",
  ARM_WORD_LENGTH,
  NULLName,
  { CONTAINS_ZERO, 0x3, 0x0, CONTAINS_ZERO, CONTAINS_ZERO, 0xF,
    CONTAINS_ZERO, 0x0, 0x0 }};

/*
 * Model the prefix patterns to update the kMetaParallelAddSubPattern to
 * be a MetaParallelAddSubOp entry.
 */
typedef struct MetaPrefixParallelAddSubOp {
  const char* prefix;       /* The prefix to add to the meta name. */
  ArmInstKind kind;         /* The ADD16 form of the instruction kind. */
  int32_t opcode;           /* The opcode for all parallel patterns of
                             * this prefix.
                             */
} MetaPrefixParallelAddSubOp;


static const MetaPrefixParallelAddSubOp kMetaPrefixParallelAddSubPattern[] = {
  { "q",   ARM_QADD16,  0x2 },
  { "s",   ARM_SADD16,  0x1 },
  { "sh",  ARM_SHADD16, 0x3 },
  { "u",   ARM_UADD16,  0x5 },
  { "uh",  ARM_UHADD16, 0x7 },
  { "uq",  ARM_UQADD16, 0x6 },
  /* Dummy field to end list. */
  { NULL,  ARM_UNKNOWN_INST, 0x0 }
};

void BuildParallelOps() {
  int i;
  for (i = 0; ; ++i) {
    int j;
    char buffer[INST_BUFFER_SIZE];
    const MetaPrefixParallelAddSubOp* prefix_op =
        &(kMetaPrefixParallelAddSubPattern[i]);
    if (NULL == prefix_op->prefix) break;
    for (j = 0; ; ++j) {
      ModeledOpInfo* op;
      const MetaParallelAddSubOp* meta_op = &(kMetaParallelAddSubPattern[j]);
      if (NULL == meta_op->name) break;
      op = CopyOfOpInfo(&kParallelAddSubPattern);
      snprintf(buffer, sizeof(buffer),
               "%s%s", prefix_op->prefix, meta_op->name);
      op->name = strdup(buffer);
      op->inst_kind = prefix_op->kind + (meta_op->kind - ARM_QADD16);
      op->expected_values.opcode = prefix_op->opcode;
      op->expected_values.shift = meta_op->shift;
      op->expected_values.immediate = meta_op->immediate;
      AddInstruction(op);
    }
  }
}
