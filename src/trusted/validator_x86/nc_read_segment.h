/*
 * Copyright 2009, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Reads in text file of hexidecimal values, and build a corresponding segment.
 *
 * Note: To see what the segment contains, run ncdis on the corresponding
 * segment to disassemble it.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NC_READ_SEGMENT_H_
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NC_READ_SEGMENT_H_

#include <stdio.h>
#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/validator_x86/types_memory_model.h"

/* Given a file, and a byte array of the given size, this function
 * opens the corresponding file, reads the text of hexidecimal values, puts
 * them in the byte array, and returns how many bytes were read. If any
 * line begins with a pound sign (#), it is assumed to be a comment and
 * ignored. If the file contains more hex values than the size of the byte
 * array, the read is truncated to the size of the byte array. If the number
 * of non-blank hex values aren't even, the single hex value is used as the
 * the corresponding byte value.
 */
size_t NcReadHexText(FILE* input, uint8_t* mbase, size_t mbase_size);

/* Same as NcReadHexText, except if the first (non-comment) line has
 * an at sign (@) in column 1, it assumes that the first line is specify
 * a value for the initial program counter (pc). If the first line doesn't
 * specify a value for the pc, zero is assumed. All other lines are
 * assumed to be hex values to be converted to byte values and placed
 * into the byte array.
 */
size_t NcReadHexTextWithPc(FILE* input, PcAddress* pc,
                           uint8_t* mbase, size_t mbase_size);

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NC_READ_SEGMENT_H_ */
