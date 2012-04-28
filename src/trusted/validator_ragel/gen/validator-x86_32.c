
#line 1 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "native_client/src/shared/utils/types.h"
#include "native_client/src/trusted/validator_ragel/unreviewed/validator.h"

#if defined(_MSC_VER)
#define inline __inline
#endif

#define check_jump_dest \
    if ((jump_dest & bundle_mask) != bundle_mask) { \
      if (jump_dest >= size) { \
        printf("direct jump out of range: %"NACL_PRIxS"\n", jump_dest); \
        result = 1; \
        goto error_detected; \
      } else { \
        BitmapSetBit(jump_dests, jump_dest + 1); \
      } \
    }


#line 96 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"



#line 39 "src/trusted/validator_ragel/gen/validator-x86_32.c"
static const int x86_64_decoder_start = 239;
static const int x86_64_decoder_first_final = 239;
static const int x86_64_decoder_error = 0;

static const int x86_64_decoder_en_main = 239;


#line 99 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"

/* Ignore this information for now.  */
#define GET_VEX_PREFIX3 0
#define SET_VEX_PREFIX3(P)
#define SET_DATA16_PREFIX(S)
#define SET_LOCK_PREFIX(S)
#define SET_REPZ_PREFIX(S)
#define SET_REPNZ_PREFIX(S)
#define SET_BRANCH_TAKEN(S)
#define SET_BRANCH_NOT_TAKEN(S)
#define SET_DISP_TYPE(T)
#define SET_DISP_PTR(P)

static const int kBitsPerByte = 8;

static inline uint8_t *BitmapAllocate(size_t indexes) {
  size_t byte_count = (indexes + kBitsPerByte - 1) / kBitsPerByte;
  uint8_t *bitmap = malloc(byte_count);
  if (bitmap != NULL) {
    memset(bitmap, 0, byte_count);
  }
  return bitmap;
}

static inline int BitmapIsBitSet(uint8_t *bitmap, size_t index) {
  return (bitmap[index / kBitsPerByte] & (1 << (index % kBitsPerByte))) != 0;
}

static inline void BitmapSetBit(uint8_t *bitmap, size_t index) {
  bitmap[index / kBitsPerByte] |= 1 << (index % kBitsPerByte);
}

static inline void BitmapClearBit(uint8_t *bitmap, size_t index) {
  bitmap[index / kBitsPerByte] &= ~(1 << (index % kBitsPerByte));
}

static int CheckJumpTargets(uint8_t *valid_targets, uint8_t *jump_dests,
                            size_t size) {
  size_t i;
  for (i = 0; i < size / 32; i++) {
    uint32_t jump_dest_mask = ((uint32_t *) jump_dests)[i];
    uint32_t valid_target_mask = ((uint32_t *) valid_targets)[i];
    if ((jump_dest_mask & ~valid_target_mask) != 0) {
      printf("bad jump to around %x\n", (unsigned)(i * 32));
      return 1;
    }
  }
  return 0;
}

int ValidateChunkIA32(const uint8_t *data, size_t size,
                      process_validation_error_func process_error,
                      void *userdata) {
  const size_t bundle_size = 32;
  const size_t bundle_mask = bundle_size - 1;

  uint8_t *valid_targets = BitmapAllocate(size);
  uint8_t *jump_dests = BitmapAllocate(size);

  const uint8_t *p = data;
  const uint8_t *begin = p;  /* Start of the instruction being processed.  */

  int result = 0;

  assert(size % bundle_size == 0);

  while (p < data + size) {
    const uint8_t *pe = p + bundle_size;
    const uint8_t *eof = pe;
    int cs;

    
#line 120 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	{
	cs = x86_64_decoder_start;
	}

#line 171 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
    
#line 127 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
tr0:
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr9:
#line 46 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP32);
    SET_DISP_PTR(p - 3);
  }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr10:
#line 42 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP8);
    SET_DISP_PTR(p);
  }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr11:
#line 53 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr15:
#line 55 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr58:
#line 43 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    int32_t offset =
           (uint32_t) (p[-3] + 256U * (p[-2] + 256U * (p[-1] + 256U * (p[0]))));
    size_t jump_dest = offset + (p - data);
    check_jump_dest;
  }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr61:
#line 35 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    int8_t offset = (uint8_t) (p[0]);
    size_t jump_dest = offset + (p - data);
    check_jump_dest;
  }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr82:
#line 54 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr108:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr228:
#line 52 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr267:
#line 39 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPZ_PREFIX(FALSE);
  }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr271:
#line 73 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
      BitmapClearBit(valid_targets, (p - data) - 1);
    }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
tr285:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st239;
st239:
	if ( ++p == pe )
		goto _test_eof239;
case 239:
#line 302 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto tr280;
		case 5u: goto tr281;
		case 12u: goto tr280;
		case 13u: goto tr281;
		case 14u: goto tr19;
		case 15u: goto tr282;
		case 20u: goto tr280;
		case 21u: goto tr281;
		case 28u: goto tr280;
		case 29u: goto tr281;
		case 36u: goto tr280;
		case 37u: goto tr281;
		case 44u: goto tr280;
		case 45u: goto tr281;
		case 46u: goto tr283;
		case 47u: goto tr19;
		case 52u: goto tr280;
		case 53u: goto tr281;
		case 60u: goto tr280;
		case 61u: goto tr281;
		case 62u: goto tr284;
		case 63u: goto tr19;
		case 101u: goto tr286;
		case 102u: goto tr287;
		case 104u: goto tr281;
		case 105u: goto tr288;
		case 106u: goto tr280;
		case 107u: goto tr289;
		case 128u: goto tr289;
		case 129u: goto tr288;
		case 130u: goto tr19;
		case 131u: goto tr291;
		case 141u: goto tr292;
		case 143u: goto tr293;
		case 154u: goto tr19;
		case 168u: goto tr280;
		case 169u: goto tr281;
		case 196u: goto tr296;
		case 197u: goto tr297;
		case 198u: goto tr298;
		case 199u: goto tr299;
		case 200u: goto tr300;
		case 202u: goto tr301;
		case 216u: goto tr279;
		case 217u: goto tr303;
		case 218u: goto tr304;
		case 219u: goto tr305;
		case 220u: goto tr306;
		case 221u: goto tr307;
		case 222u: goto tr308;
		case 223u: goto tr309;
		case 235u: goto tr290;
		case 240u: goto tr311;
		case 242u: goto tr312;
		case 243u: goto tr313;
		case 246u: goto tr314;
		case 247u: goto tr315;
		case 254u: goto tr316;
		case 255u: goto tr317;
	}
	if ( (*p) < 132u ) {
		if ( (*p) < 32u ) {
			if ( (*p) < 8u ) {
				if ( (*p) > 3u ) {
					if ( 6u <= (*p) && (*p) <= 7u )
						goto tr19;
				} else
					goto tr279;
			} else if ( (*p) > 19u ) {
				if ( (*p) < 24u ) {
					if ( 22u <= (*p) && (*p) <= 23u )
						goto tr19;
				} else if ( (*p) > 27u ) {
					if ( 30u <= (*p) && (*p) <= 31u )
						goto tr19;
				} else
					goto tr279;
			} else
				goto tr279;
		} else if ( (*p) > 35u ) {
			if ( (*p) < 54u ) {
				if ( (*p) > 39u ) {
					if ( 40u <= (*p) && (*p) <= 51u )
						goto tr279;
				} else if ( (*p) >= 38u )
					goto tr19;
			} else if ( (*p) > 55u ) {
				if ( (*p) < 96u ) {
					if ( 56u <= (*p) && (*p) <= 59u )
						goto tr279;
				} else if ( (*p) > 111u ) {
					if ( 112u <= (*p) && (*p) <= 127u )
						goto tr290;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr279;
	} else if ( (*p) > 139u ) {
		if ( (*p) < 194u ) {
			if ( (*p) < 160u ) {
				if ( (*p) > 142u ) {
					if ( 156u <= (*p) && (*p) <= 157u )
						goto tr19;
				} else if ( (*p) >= 140u )
					goto tr19;
			} else if ( (*p) > 163u ) {
				if ( (*p) < 184u ) {
					if ( 176u <= (*p) && (*p) <= 183u )
						goto tr280;
				} else if ( (*p) > 191u ) {
					if ( 192u <= (*p) && (*p) <= 193u )
						goto tr295;
				} else
					goto tr281;
			} else
				goto tr294;
		} else if ( (*p) > 195u ) {
			if ( (*p) < 212u ) {
				if ( (*p) > 207u ) {
					if ( 208u <= (*p) && (*p) <= 211u )
						goto tr302;
				} else if ( (*p) >= 204u )
					goto tr19;
			} else if ( (*p) > 231u ) {
				if ( (*p) < 234u ) {
					if ( 232u <= (*p) && (*p) <= 233u )
						goto tr310;
				} else if ( (*p) > 241u ) {
					if ( 250u <= (*p) && (*p) <= 251u )
						goto tr19;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr19;
	} else
		goto tr279;
	goto tr285;
tr85:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st1;
tr134:
#line 36 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPNZ_PREFIX(FALSE);
  }
	goto st1;
tr136:
#line 39 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPZ_PREFIX(FALSE);
  }
	goto st1;
tr279:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st1;
st1:
	if ( ++p == pe )
		goto _test_eof1;
case 1:
#line 474 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) > 127u ) {
		if ( 128u <= (*p) && (*p) <= 191u )
			goto st3;
	} else if ( (*p) >= 64u )
		goto st7;
	goto tr0;
tr123:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 525 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 5u: goto st3;
		case 13u: goto st3;
		case 21u: goto st3;
		case 29u: goto st3;
		case 37u: goto st3;
		case 45u: goto st3;
		case 53u: goto st3;
		case 61u: goto st3;
		case 69u: goto st3;
		case 77u: goto st3;
		case 85u: goto st3;
		case 93u: goto st3;
		case 101u: goto st3;
		case 109u: goto st3;
		case 117u: goto st3;
		case 125u: goto st3;
		case 133u: goto st3;
		case 141u: goto st3;
		case 149u: goto st3;
		case 157u: goto st3;
		case 165u: goto st3;
		case 173u: goto st3;
		case 181u: goto st3;
		case 189u: goto st3;
		case 197u: goto st3;
		case 205u: goto st3;
		case 213u: goto st3;
		case 221u: goto st3;
		case 229u: goto st3;
		case 237u: goto st3;
		case 245u: goto st3;
		case 253u: goto st3;
	}
	goto tr0;
tr124:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st3;
tr294:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 578 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	goto tr9;
tr125:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 605 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto tr10;
tr126:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 617 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto st7;
tr127:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 629 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto st3;
tr53:
#line 46 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP32);
    SET_DISP_PTR(p - 3);
  }
	goto st10;
tr54:
#line 42 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP8);
    SET_DISP_PTR(p);
  }
	goto st10;
tr95:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st10;
tr252:
#line 58 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
	goto st10;
tr264:
#line 57 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
	goto st10;
tr280:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 670 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto tr11;
tr146:
#line 46 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP32);
    SET_DISP_PTR(p - 3);
  }
	goto st11;
tr147:
#line 42 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP8);
    SET_DISP_PTR(p);
  }
	goto st11;
tr281:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 697 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	goto tr15;
tr19:
#line 90 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        process_error(begin, userdata);
        result = 1;
        goto error_detected;
    }
	goto st0;
#line 722 "src/trusted/validator_ragel/gen/validator-x86_32.c"
st0:
cs = 0;
	goto _out;
tr282:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
#line 737 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 0u: goto st16;
		case 1u: goto st17;
		case 11u: goto tr0;
		case 13u: goto st18;
		case 14u: goto tr0;
		case 15u: goto st19;
		case 19u: goto st29;
		case 23u: goto st29;
		case 24u: goto st30;
		case 31u: goto st31;
		case 43u: goto st29;
		case 49u: goto tr0;
		case 80u: goto st32;
		case 112u: goto st33;
		case 115u: goto st43;
		case 119u: goto tr0;
		case 162u: goto tr0;
		case 164u: goto st33;
		case 172u: goto st33;
		case 174u: goto st48;
		case 179u: goto tr19;
		case 195u: goto st29;
		case 197u: goto st49;
		case 199u: goto st50;
		case 208u: goto tr19;
		case 214u: goto tr19;
		case 215u: goto st32;
		case 240u: goto tr19;
		case 247u: goto st32;
		case 255u: goto tr19;
	}
	if ( (*p) < 128u ) {
		if ( (*p) < 48u ) {
			if ( (*p) > 12u ) {
				if ( 25u <= (*p) && (*p) <= 39u )
					goto tr19;
			} else if ( (*p) >= 4u )
				goto tr19;
		} else if ( (*p) > 63u ) {
			if ( (*p) < 113u ) {
				if ( 108u <= (*p) && (*p) <= 109u )
					goto tr19;
			} else if ( (*p) > 114u ) {
				if ( 120u <= (*p) && (*p) <= 125u )
					goto tr19;
			} else
				goto st42;
		} else
			goto tr19;
	} else if ( (*p) > 143u ) {
		if ( (*p) < 184u ) {
			if ( (*p) < 166u ) {
				if ( 160u <= (*p) && (*p) <= 163u )
					goto tr19;
			} else if ( (*p) > 171u ) {
				if ( 178u <= (*p) && (*p) <= 181u )
					goto st29;
			} else
				goto tr19;
		} else if ( (*p) > 187u ) {
			if ( (*p) < 200u ) {
				if ( 194u <= (*p) && (*p) <= 198u )
					goto st33;
			} else if ( (*p) > 207u ) {
				if ( 230u <= (*p) && (*p) <= 231u )
					goto tr19;
			} else
				goto tr0;
		} else
			goto tr19;
	} else
		goto st44;
	goto st1;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 12u: goto st2;
		case 13u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 76u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 140u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
	}
	if ( (*p) < 88u ) {
		if ( (*p) < 24u ) {
			if ( 8u <= (*p) && (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 47u ) {
			if ( 72u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 111u ) {
		if ( (*p) < 152u ) {
			if ( 136u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 175u ) {
			if ( (*p) > 207u ) {
				if ( 216u <= (*p) && (*p) <= 239u )
					goto tr0;
			} else if ( (*p) >= 200u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	switch( (*p) ) {
		case 36u: goto st2;
		case 37u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 100u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 164u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 160u ) {
		if ( (*p) < 48u ) {
			if ( 32u <= (*p) && (*p) <= 39u )
				goto tr0;
		} else if ( (*p) > 63u ) {
			if ( (*p) > 103u ) {
				if ( 112u <= (*p) && (*p) <= 127u )
					goto st7;
			} else if ( (*p) >= 96u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 167u ) {
		if ( (*p) < 208u ) {
			if ( (*p) > 191u ) {
				if ( 200u <= (*p) && (*p) <= 201u )
					goto tr0;
			} else if ( (*p) >= 176u )
				goto st3;
		} else if ( (*p) > 209u ) {
			if ( (*p) > 231u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 224u )
				goto tr0;
		} else
			goto tr0;
	} else
		goto st3;
	goto tr19;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 15u )
			goto tr0;
	} else if ( (*p) > 79u ) {
		if ( 128u <= (*p) && (*p) <= 143u )
			goto st3;
	} else
		goto st7;
	goto tr19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	switch( (*p) ) {
		case 4u: goto st21;
		case 5u: goto st22;
		case 12u: goto st21;
		case 13u: goto st22;
		case 20u: goto st21;
		case 21u: goto st22;
		case 28u: goto st21;
		case 29u: goto st22;
		case 36u: goto st21;
		case 37u: goto st22;
		case 44u: goto st21;
		case 45u: goto st22;
		case 52u: goto st21;
		case 53u: goto st22;
		case 60u: goto st21;
		case 61u: goto st22;
		case 68u: goto st27;
		case 76u: goto st27;
		case 84u: goto st27;
		case 92u: goto st27;
		case 100u: goto st27;
		case 108u: goto st27;
		case 116u: goto st27;
		case 124u: goto st27;
		case 132u: goto st28;
		case 140u: goto st28;
		case 148u: goto st28;
		case 156u: goto st28;
		case 164u: goto st28;
		case 172u: goto st28;
		case 180u: goto st28;
		case 188u: goto st28;
	}
	if ( (*p) > 127u ) {
		if ( 128u <= (*p) && (*p) <= 191u )
			goto st22;
	} else if ( (*p) >= 64u )
		goto st26;
	goto st20;
tr42:
#line 46 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP32);
    SET_DISP_PTR(p - 3);
  }
	goto st20;
tr43:
#line 42 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP8);
    SET_DISP_PTR(p);
  }
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 991 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 138u: goto tr0;
		case 142u: goto tr0;
		case 144u: goto tr0;
		case 148u: goto tr0;
		case 154u: goto tr0;
		case 158u: goto tr0;
		case 160u: goto tr0;
		case 164u: goto tr0;
		case 170u: goto tr0;
		case 174u: goto tr0;
		case 176u: goto tr0;
		case 180u: goto tr0;
		case 187u: goto tr0;
		case 191u: goto tr0;
	}
	if ( (*p) < 150u ) {
		if ( (*p) > 13u ) {
			if ( 28u <= (*p) && (*p) <= 29u )
				goto tr0;
		} else if ( (*p) >= 12u )
			goto tr0;
	} else if ( (*p) > 151u ) {
		if ( (*p) > 167u ) {
			if ( 182u <= (*p) && (*p) <= 183u )
				goto tr0;
		} else if ( (*p) >= 166u )
			goto tr0;
	} else
		goto tr0;
	goto tr19;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	switch( (*p) ) {
		case 5u: goto st22;
		case 13u: goto st22;
		case 21u: goto st22;
		case 29u: goto st22;
		case 37u: goto st22;
		case 45u: goto st22;
		case 53u: goto st22;
		case 61u: goto st22;
		case 69u: goto st22;
		case 77u: goto st22;
		case 85u: goto st22;
		case 93u: goto st22;
		case 101u: goto st22;
		case 109u: goto st22;
		case 117u: goto st22;
		case 125u: goto st22;
		case 133u: goto st22;
		case 141u: goto st22;
		case 149u: goto st22;
		case 157u: goto st22;
		case 165u: goto st22;
		case 173u: goto st22;
		case 181u: goto st22;
		case 189u: goto st22;
		case 197u: goto st22;
		case 205u: goto st22;
		case 213u: goto st22;
		case 221u: goto st22;
		case 229u: goto st22;
		case 237u: goto st22;
		case 245u: goto st22;
		case 253u: goto st22;
	}
	goto st20;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	goto st23;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	goto tr42;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	goto tr43;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	goto st26;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	goto st22;
tr86:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st29;
tr258:
#line 36 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPNZ_PREFIX(FALSE);
  }
	goto st29;
tr268:
#line 39 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPZ_PREFIX(FALSE);
  }
	goto st29;
tr292:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 1126 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 63u )
			goto tr0;
	} else if ( (*p) > 127u ) {
		if ( 128u <= (*p) && (*p) <= 191u )
			goto st3;
	} else
		goto st7;
	goto tr19;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 31u )
			goto tr0;
	} else if ( (*p) > 95u ) {
		if ( 128u <= (*p) && (*p) <= 159u )
			goto st3;
	} else
		goto st7;
	goto tr19;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 68u: goto st8;
		case 132u: goto st9;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 7u )
			goto tr0;
	} else if ( (*p) > 71u ) {
		if ( (*p) > 135u ) {
			if ( 192u <= (*p) && (*p) <= 199u )
				goto tr0;
		} else if ( (*p) >= 128u )
			goto st3;
	} else
		goto st7;
	goto tr19;
tr89:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st32;
tr262:
#line 36 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPNZ_PREFIX(FALSE);
  }
	goto st32;
tr270:
#line 39 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPZ_PREFIX(FALSE);
  }
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 1245 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( 192u <= (*p) )
		goto tr0;
	goto tr19;
tr90:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st33;
tr260:
#line 36 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPNZ_PREFIX(FALSE);
  }
	goto st33;
tr269:
#line 39 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPZ_PREFIX(FALSE);
  }
	goto st33;
tr289:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
#line 1278 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st34;
		case 5u: goto st35;
		case 12u: goto st34;
		case 13u: goto st35;
		case 20u: goto st34;
		case 21u: goto st35;
		case 28u: goto st34;
		case 29u: goto st35;
		case 36u: goto st34;
		case 37u: goto st35;
		case 44u: goto st34;
		case 45u: goto st35;
		case 52u: goto st34;
		case 53u: goto st35;
		case 60u: goto st34;
		case 61u: goto st35;
		case 68u: goto st40;
		case 76u: goto st40;
		case 84u: goto st40;
		case 92u: goto st40;
		case 100u: goto st40;
		case 108u: goto st40;
		case 116u: goto st40;
		case 124u: goto st40;
		case 132u: goto st41;
		case 140u: goto st41;
		case 148u: goto st41;
		case 156u: goto st41;
		case 164u: goto st41;
		case 172u: goto st41;
		case 180u: goto st41;
		case 188u: goto st41;
	}
	if ( (*p) > 127u ) {
		if ( 128u <= (*p) && (*p) <= 191u )
			goto st35;
	} else if ( (*p) >= 64u )
		goto st39;
	goto st10;
tr96:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 1329 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 5u: goto st35;
		case 13u: goto st35;
		case 21u: goto st35;
		case 29u: goto st35;
		case 37u: goto st35;
		case 45u: goto st35;
		case 53u: goto st35;
		case 61u: goto st35;
		case 69u: goto st35;
		case 77u: goto st35;
		case 85u: goto st35;
		case 93u: goto st35;
		case 101u: goto st35;
		case 109u: goto st35;
		case 117u: goto st35;
		case 125u: goto st35;
		case 133u: goto st35;
		case 141u: goto st35;
		case 149u: goto st35;
		case 157u: goto st35;
		case 165u: goto st35;
		case 173u: goto st35;
		case 181u: goto st35;
		case 189u: goto st35;
		case 197u: goto st35;
		case 205u: goto st35;
		case 213u: goto st35;
		case 221u: goto st35;
		case 229u: goto st35;
		case 237u: goto st35;
		case 245u: goto st35;
		case 253u: goto st35;
	}
	goto st10;
tr97:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 1375 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	goto tr53;
tr98:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 1402 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto tr54;
tr99:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st40;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
#line 1414 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto st39;
tr100:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
#line 1426 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto st35;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	if ( (*p) < 224u ) {
		if ( 208u <= (*p) && (*p) <= 215u )
			goto st10;
	} else if ( (*p) > 231u ) {
		if ( 240u <= (*p) && (*p) <= 247u )
			goto st10;
	} else
		goto st10;
	goto tr19;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( (*p) > 215u ) {
		if ( 240u <= (*p) && (*p) <= 247u )
			goto st10;
	} else if ( (*p) >= 208u )
		goto st10;
	goto tr19;
tr310:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st44;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
#line 1462 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	goto st46;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	goto st47;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	goto tr58;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	switch( (*p) ) {
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
		case 232u: goto tr0;
		case 240u: goto tr0;
		case 248u: goto tr0;
	}
	if ( (*p) < 80u ) {
		if ( 16u <= (*p) && (*p) <= 63u )
			goto tr0;
	} else if ( (*p) > 127u ) {
		if ( 144u <= (*p) && (*p) <= 191u )
			goto st3;
	} else
		goto st7;
	goto tr19;
tr94:
#line 25 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(FALSE);
  }
	goto st49;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
#line 1531 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( 192u <= (*p) )
		goto st10;
	goto tr19;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	switch( (*p) ) {
		case 12u: goto st2;
		case 13u: goto st3;
		case 76u: goto st8;
		case 140u: goto st9;
	}
	if ( (*p) < 72u ) {
		if ( 8u <= (*p) && (*p) <= 15u )
			goto tr0;
	} else if ( (*p) > 79u ) {
		if ( 136u <= (*p) && (*p) <= 143u )
			goto st3;
	} else
		goto st7;
	goto tr19;
tr283:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
#line 4 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_BRANCH_NOT_TAKEN(TRUE);
  }
	goto st51;
tr284:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
#line 7 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_BRANCH_TAKEN(TRUE);
  }
	goto st51;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
#line 1580 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( (*p) == 15u )
		goto st52;
	if ( 112u <= (*p) && (*p) <= 127u )
		goto st53;
	goto tr19;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	if ( 128u <= (*p) && (*p) <= 143u )
		goto st44;
	goto tr19;
tr290:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st53;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
#line 1604 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto tr61;
tr286:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st54;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
#line 1617 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 139u: goto st55;
		case 161u: goto st56;
	}
	goto tr19;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	switch( (*p) ) {
		case 5u: goto st56;
		case 13u: goto st56;
		case 21u: goto st56;
		case 29u: goto st56;
		case 37u: goto st56;
		case 45u: goto st56;
		case 53u: goto st56;
		case 61u: goto st56;
	}
	goto tr19;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( (*p) == 0u )
		goto st57;
	goto tr19;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( (*p) == 0u )
		goto st58;
	goto tr19;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( (*p) == 0u )
		goto st59;
	goto tr19;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( (*p) == 0u )
		goto tr0;
	goto tr19;
tr287:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
#line 10 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(TRUE);
  }
	goto st60;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
#line 1681 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 1u: goto st1;
		case 3u: goto st1;
		case 5u: goto st61;
		case 9u: goto st1;
		case 11u: goto st1;
		case 13u: goto st61;
		case 15u: goto st63;
		case 17u: goto st1;
		case 19u: goto st1;
		case 21u: goto st61;
		case 25u: goto st1;
		case 27u: goto st1;
		case 29u: goto st61;
		case 33u: goto st1;
		case 35u: goto st1;
		case 37u: goto st61;
		case 41u: goto st1;
		case 43u: goto st1;
		case 45u: goto st61;
		case 46u: goto st71;
		case 49u: goto st1;
		case 51u: goto st1;
		case 53u: goto st61;
		case 57u: goto st1;
		case 59u: goto st1;
		case 61u: goto st61;
		case 102u: goto st79;
		case 104u: goto st61;
		case 105u: goto st84;
		case 107u: goto st33;
		case 129u: goto st84;
		case 131u: goto st33;
		case 133u: goto st1;
		case 135u: goto st1;
		case 137u: goto st1;
		case 139u: goto st1;
		case 141u: goto st29;
		case 143u: goto st93;
		case 161u: goto st3;
		case 163u: goto st3;
		case 165u: goto tr0;
		case 167u: goto tr0;
		case 169u: goto st61;
		case 171u: goto tr0;
		case 173u: goto tr0;
		case 175u: goto tr0;
		case 193u: goto st94;
		case 199u: goto st95;
		case 209u: goto st96;
		case 211u: goto st96;
		case 240u: goto tr76;
		case 242u: goto tr77;
		case 243u: goto tr78;
		case 247u: goto st107;
		case 255u: goto st108;
	}
	if ( (*p) < 144u ) {
		if ( 64u <= (*p) && (*p) <= 79u )
			goto tr0;
	} else if ( (*p) > 153u ) {
		if ( (*p) > 157u ) {
			if ( 184u <= (*p) && (*p) <= 191u )
				goto st61;
		} else if ( (*p) >= 156u )
			goto tr0;
	} else
		goto tr0;
	goto tr19;
tr121:
#line 46 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP32);
    SET_DISP_PTR(p - 3);
  }
	goto st61;
tr122:
#line 42 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP8);
    SET_DISP_PTR(p);
  }
	goto st61;
tr301:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st61;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
#line 1776 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto st62;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	goto tr82;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	switch( (*p) ) {
		case 0u: goto st64;
		case 1u: goto st65;
		case 31u: goto st31;
		case 43u: goto tr86;
		case 56u: goto st66;
		case 58u: goto st67;
		case 80u: goto tr89;
		case 81u: goto tr85;
		case 112u: goto tr90;
		case 115u: goto st70;
		case 121u: goto tr89;
		case 164u: goto st33;
		case 165u: goto st1;
		case 172u: goto st33;
		case 173u: goto st1;
		case 175u: goto st1;
		case 177u: goto st1;
		case 178u: goto st29;
		case 182u: goto st1;
		case 193u: goto st1;
		case 194u: goto tr90;
		case 196u: goto st68;
		case 197u: goto tr94;
		case 198u: goto tr90;
		case 215u: goto tr89;
		case 231u: goto tr86;
		case 247u: goto tr89;
	}
	if ( (*p) < 84u ) {
		if ( (*p) < 20u ) {
			if ( (*p) < 16u ) {
				if ( 2u <= (*p) && (*p) <= 3u )
					goto st1;
			} else if ( (*p) > 17u ) {
				if ( 18u <= (*p) && (*p) <= 19u )
					goto tr86;
			} else
				goto tr85;
		} else if ( (*p) > 21u ) {
			if ( (*p) < 40u ) {
				if ( 22u <= (*p) && (*p) <= 23u )
					goto tr86;
			} else if ( (*p) > 47u ) {
				if ( 64u <= (*p) && (*p) <= 79u )
					goto st1;
			} else
				goto tr85;
		} else
			goto tr85;
	} else if ( (*p) > 111u ) {
		if ( (*p) < 180u ) {
			if ( (*p) < 116u ) {
				if ( 113u <= (*p) && (*p) <= 114u )
					goto st69;
			} else if ( (*p) > 118u ) {
				if ( 124u <= (*p) && (*p) <= 127u )
					goto tr85;
			} else
				goto tr85;
		} else if ( (*p) > 181u ) {
			if ( (*p) < 208u ) {
				if ( 188u <= (*p) && (*p) <= 190u )
					goto st1;
			} else if ( (*p) > 239u ) {
				if ( 241u <= (*p) && (*p) <= 254u )
					goto tr85;
			} else
				goto tr85;
		} else
			goto st29;
	} else
		goto tr85;
	goto tr19;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( 200u <= (*p) && (*p) <= 207u )
		goto tr0;
	goto tr19;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( 224u <= (*p) && (*p) <= 231u )
		goto tr0;
	goto tr19;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	switch( (*p) ) {
		case 16u: goto tr85;
		case 23u: goto tr85;
		case 42u: goto tr86;
	}
	if ( (*p) < 32u ) {
		if ( (*p) < 20u ) {
			if ( (*p) <= 11u )
				goto tr85;
		} else if ( (*p) > 21u ) {
			if ( 28u <= (*p) && (*p) <= 30u )
				goto tr85;
		} else
			goto tr85;
	} else if ( (*p) > 37u ) {
		if ( (*p) < 48u ) {
			if ( 40u <= (*p) && (*p) <= 43u )
				goto tr85;
		} else if ( (*p) > 53u ) {
			if ( (*p) > 65u ) {
				if ( 219u <= (*p) && (*p) <= 223u )
					goto tr85;
			} else if ( (*p) >= 55u )
				goto tr85;
		} else
			goto tr85;
	} else
		goto tr85;
	goto tr19;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	switch( (*p) ) {
		case 23u: goto tr90;
		case 32u: goto st68;
		case 68u: goto tr90;
		case 223u: goto tr90;
	}
	if ( (*p) < 33u ) {
		if ( (*p) > 15u ) {
			if ( 20u <= (*p) && (*p) <= 22u )
				goto st68;
		} else if ( (*p) >= 8u )
			goto tr90;
	} else if ( (*p) > 34u ) {
		if ( (*p) > 66u ) {
			if ( 96u <= (*p) && (*p) <= 99u )
				goto tr90;
		} else if ( (*p) >= 64u )
			goto tr90;
	} else
		goto tr90;
	goto tr19;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	switch( (*p) ) {
		case 4u: goto tr96;
		case 5u: goto tr97;
		case 12u: goto tr96;
		case 13u: goto tr97;
		case 20u: goto tr96;
		case 21u: goto tr97;
		case 28u: goto tr96;
		case 29u: goto tr97;
		case 36u: goto tr96;
		case 37u: goto tr97;
		case 44u: goto tr96;
		case 45u: goto tr97;
		case 52u: goto tr96;
		case 53u: goto tr97;
		case 60u: goto tr96;
		case 61u: goto tr97;
		case 68u: goto tr99;
		case 76u: goto tr99;
		case 84u: goto tr99;
		case 92u: goto tr99;
		case 100u: goto tr99;
		case 108u: goto tr99;
		case 116u: goto tr99;
		case 124u: goto tr99;
		case 132u: goto tr100;
		case 140u: goto tr100;
		case 148u: goto tr100;
		case 156u: goto tr100;
		case 164u: goto tr100;
		case 172u: goto tr100;
		case 180u: goto tr100;
		case 188u: goto tr100;
	}
	if ( (*p) > 127u ) {
		if ( 128u <= (*p) && (*p) <= 191u )
			goto tr97;
	} else if ( (*p) >= 64u )
		goto tr98;
	goto tr95;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	if ( (*p) < 224u ) {
		if ( 208u <= (*p) && (*p) <= 215u )
			goto tr95;
	} else if ( (*p) > 231u ) {
		if ( 240u <= (*p) && (*p) <= 247u )
			goto tr95;
	} else
		goto tr95;
	goto tr19;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( (*p) > 223u ) {
		if ( 240u <= (*p) )
			goto tr95;
	} else if ( (*p) >= 208u )
		goto tr95;
	goto tr19;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( (*p) == 15u )
		goto st72;
	goto tr19;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	if ( (*p) == 31u )
		goto st73;
	goto tr19;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( (*p) == 132u )
		goto st74;
	goto tr19;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	if ( (*p) == 0u )
		goto st75;
	goto tr19;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	if ( (*p) == 0u )
		goto st76;
	goto tr19;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	if ( (*p) == 0u )
		goto st77;
	goto tr19;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	if ( (*p) == 0u )
		goto st78;
	goto tr19;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	if ( (*p) == 0u )
		goto tr108;
	goto tr19;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	switch( (*p) ) {
		case 46u: goto st71;
		case 102u: goto st80;
	}
	goto tr19;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	switch( (*p) ) {
		case 46u: goto st71;
		case 102u: goto st81;
	}
	goto tr19;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	switch( (*p) ) {
		case 46u: goto st71;
		case 102u: goto st82;
	}
	goto tr19;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	switch( (*p) ) {
		case 46u: goto st71;
		case 102u: goto st83;
	}
	goto tr19;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	if ( (*p) == 46u )
		goto st71;
	goto tr19;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	switch( (*p) ) {
		case 4u: goto st85;
		case 5u: goto st86;
		case 12u: goto st85;
		case 13u: goto st86;
		case 20u: goto st85;
		case 21u: goto st86;
		case 28u: goto st85;
		case 29u: goto st86;
		case 36u: goto st85;
		case 37u: goto st86;
		case 44u: goto st85;
		case 45u: goto st86;
		case 52u: goto st85;
		case 53u: goto st86;
		case 60u: goto st85;
		case 61u: goto st86;
		case 68u: goto st91;
		case 76u: goto st91;
		case 84u: goto st91;
		case 92u: goto st91;
		case 100u: goto st91;
		case 108u: goto st91;
		case 116u: goto st91;
		case 124u: goto st91;
		case 132u: goto st92;
		case 140u: goto st92;
		case 148u: goto st92;
		case 156u: goto st92;
		case 164u: goto st92;
		case 172u: goto st92;
		case 180u: goto st92;
		case 188u: goto st92;
	}
	if ( (*p) > 127u ) {
		if ( 128u <= (*p) && (*p) <= 191u )
			goto st86;
	} else if ( (*p) >= 64u )
		goto st90;
	goto st61;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	switch( (*p) ) {
		case 5u: goto st86;
		case 13u: goto st86;
		case 21u: goto st86;
		case 29u: goto st86;
		case 37u: goto st86;
		case 45u: goto st86;
		case 53u: goto st86;
		case 61u: goto st86;
		case 69u: goto st86;
		case 77u: goto st86;
		case 85u: goto st86;
		case 93u: goto st86;
		case 101u: goto st86;
		case 109u: goto st86;
		case 117u: goto st86;
		case 125u: goto st86;
		case 133u: goto st86;
		case 141u: goto st86;
		case 149u: goto st86;
		case 157u: goto st86;
		case 165u: goto st86;
		case 173u: goto st86;
		case 181u: goto st86;
		case 189u: goto st86;
		case 197u: goto st86;
		case 205u: goto st86;
		case 213u: goto st86;
		case 221u: goto st86;
		case 229u: goto st86;
		case 237u: goto st86;
		case 245u: goto st86;
		case 253u: goto st86;
	}
	goto st61;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	goto st87;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	goto st88;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	goto st89;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	goto tr121;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	goto tr122;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	goto st90;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	goto st86;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	switch( (*p) ) {
		case 4u: goto tr123;
		case 5u: goto tr124;
		case 68u: goto tr126;
		case 132u: goto tr127;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 7u )
			goto tr108;
	} else if ( (*p) > 71u ) {
		if ( (*p) > 135u ) {
			if ( 192u <= (*p) && (*p) <= 199u )
				goto tr108;
		} else if ( (*p) >= 128u )
			goto tr124;
	} else
		goto tr125;
	goto tr19;
tr295:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st94;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
#line 2250 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st34;
		case 5u: goto st35;
		case 12u: goto st34;
		case 13u: goto st35;
		case 20u: goto st34;
		case 21u: goto st35;
		case 28u: goto st34;
		case 29u: goto st35;
		case 36u: goto st34;
		case 37u: goto st35;
		case 44u: goto st34;
		case 45u: goto st35;
		case 60u: goto st34;
		case 61u: goto st35;
		case 68u: goto st40;
		case 76u: goto st40;
		case 84u: goto st40;
		case 92u: goto st40;
		case 100u: goto st40;
		case 108u: goto st40;
		case 124u: goto st40;
		case 132u: goto st41;
		case 140u: goto st41;
		case 148u: goto st41;
		case 156u: goto st41;
		case 164u: goto st41;
		case 172u: goto st41;
		case 188u: goto st41;
	}
	if ( (*p) < 120u ) {
		if ( (*p) < 64u ) {
			if ( 48u <= (*p) && (*p) <= 55u )
				goto tr19;
		} else if ( (*p) > 111u ) {
			if ( 112u <= (*p) && (*p) <= 119u )
				goto tr19;
		} else
			goto st39;
	} else if ( (*p) > 127u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 175u )
				goto st35;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 191u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr19;
			} else if ( (*p) >= 184u )
				goto st35;
		} else
			goto tr19;
	} else
		goto st39;
	goto st10;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	switch( (*p) ) {
		case 4u: goto st85;
		case 5u: goto st86;
		case 68u: goto st91;
		case 132u: goto st92;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 7u )
			goto st61;
	} else if ( (*p) > 71u ) {
		if ( (*p) > 135u ) {
			if ( 192u <= (*p) && (*p) <= 199u )
				goto st61;
		} else if ( (*p) >= 128u )
			goto st86;
	} else
		goto st90;
	goto tr19;
tr302:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st96;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
#line 2338 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 124u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 120u ) {
		if ( (*p) < 64u ) {
			if ( 48u <= (*p) && (*p) <= 55u )
				goto tr19;
		} else if ( (*p) > 111u ) {
			if ( 112u <= (*p) && (*p) <= 119u )
				goto tr19;
		} else
			goto st7;
	} else if ( (*p) > 127u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 175u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 191u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr19;
			} else if ( (*p) >= 184u )
				goto st3;
		} else
			goto tr19;
	} else
		goto st7;
	goto tr0;
tr76:
#line 13 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_LOCK_PREFIX(TRUE);
  }
	goto st97;
tr254:
#line 10 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(TRUE);
  }
	goto st97;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
#line 2409 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 1u: goto st29;
		case 3u: goto st29;
		case 9u: goto st29;
		case 11u: goto st29;
		case 15u: goto st98;
		case 17u: goto st29;
		case 19u: goto st29;
		case 25u: goto st29;
		case 27u: goto st29;
		case 33u: goto st29;
		case 35u: goto st29;
		case 41u: goto st29;
		case 43u: goto st29;
		case 49u: goto st29;
		case 51u: goto st29;
		case 129u: goto st99;
		case 131u: goto st100;
		case 135u: goto st29;
		case 247u: goto st101;
		case 255u: goto st18;
	}
	goto tr19;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
	switch( (*p) ) {
		case 177u: goto st29;
		case 193u: goto st29;
	}
	goto tr19;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
	switch( (*p) ) {
		case 4u: goto st85;
		case 5u: goto st86;
		case 12u: goto st85;
		case 13u: goto st86;
		case 20u: goto st85;
		case 21u: goto st86;
		case 28u: goto st85;
		case 29u: goto st86;
		case 36u: goto st85;
		case 37u: goto st86;
		case 44u: goto st85;
		case 45u: goto st86;
		case 52u: goto st85;
		case 53u: goto st86;
		case 68u: goto st91;
		case 76u: goto st91;
		case 84u: goto st91;
		case 92u: goto st91;
		case 100u: goto st91;
		case 108u: goto st91;
		case 116u: goto st91;
		case 132u: goto st92;
		case 140u: goto st92;
		case 148u: goto st92;
		case 156u: goto st92;
		case 164u: goto st92;
		case 172u: goto st92;
		case 180u: goto st92;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 55u )
			goto st61;
	} else if ( (*p) > 119u ) {
		if ( 128u <= (*p) && (*p) <= 183u )
			goto st86;
	} else
		goto st90;
	goto tr19;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	switch( (*p) ) {
		case 4u: goto st34;
		case 5u: goto st35;
		case 12u: goto st34;
		case 13u: goto st35;
		case 20u: goto st34;
		case 21u: goto st35;
		case 28u: goto st34;
		case 29u: goto st35;
		case 36u: goto st34;
		case 37u: goto st35;
		case 44u: goto st34;
		case 45u: goto st35;
		case 52u: goto st34;
		case 53u: goto st35;
		case 68u: goto st40;
		case 76u: goto st40;
		case 84u: goto st40;
		case 92u: goto st40;
		case 100u: goto st40;
		case 108u: goto st40;
		case 116u: goto st40;
		case 132u: goto st41;
		case 140u: goto st41;
		case 148u: goto st41;
		case 156u: goto st41;
		case 164u: goto st41;
		case 172u: goto st41;
		case 180u: goto st41;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 55u )
			goto st10;
	} else if ( (*p) > 119u ) {
		if ( 128u <= (*p) && (*p) <= 183u )
			goto st35;
	} else
		goto st39;
	goto tr19;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	switch( (*p) ) {
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 84u: goto st8;
		case 92u: goto st8;
		case 148u: goto st9;
		case 156u: goto st9;
	}
	if ( (*p) < 80u ) {
		if ( 16u <= (*p) && (*p) <= 31u )
			goto tr0;
	} else if ( (*p) > 95u ) {
		if ( 144u <= (*p) && (*p) <= 159u )
			goto st3;
	} else
		goto st7;
	goto tr19;
tr77:
#line 22 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPNZ_PREFIX(TRUE);
  }
	goto st102;
tr257:
#line 10 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(TRUE);
  }
	goto st102;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
#line 2567 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 15u: goto st103;
		case 167u: goto tr0;
		case 175u: goto tr0;
	}
	goto tr19;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	if ( (*p) == 56u )
		goto st104;
	goto tr19;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
	if ( (*p) == 241u )
		goto tr134;
	goto tr19;
tr78:
#line 19 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPZ_PREFIX(TRUE);
  }
#line 16 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPZ_PREFIX(TRUE);
  }
	goto st105;
tr266:
#line 10 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DATA16_PREFIX(TRUE);
  }
	goto st105;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
#line 2608 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 15u: goto st106;
		case 165u: goto tr0;
		case 167u: goto tr0;
		case 171u: goto tr0;
		case 173u: goto tr0;
		case 175u: goto tr0;
	}
	goto tr19;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	if ( (*p) == 184u )
		goto tr136;
	if ( 188u <= (*p) && (*p) <= 189u )
		goto tr136;
	goto tr19;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	switch( (*p) ) {
		case 4u: goto st85;
		case 5u: goto st86;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st91;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st92;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 80u ) {
		if ( (*p) < 8u ) {
			if ( (*p) <= 7u )
				goto st61;
		} else if ( (*p) > 15u ) {
			if ( (*p) > 71u ) {
				if ( 72u <= (*p) && (*p) <= 79u )
					goto tr19;
			} else if ( (*p) >= 64u )
				goto st90;
		} else
			goto tr19;
	} else if ( (*p) > 127u ) {
		if ( (*p) < 144u ) {
			if ( (*p) > 135u ) {
				if ( 136u <= (*p) && (*p) <= 143u )
					goto tr19;
			} else if ( (*p) >= 128u )
				goto st86;
		} else if ( (*p) > 191u ) {
			if ( (*p) > 199u ) {
				if ( 200u <= (*p) && (*p) <= 207u )
					goto tr19;
			} else if ( (*p) >= 192u )
				goto st61;
		} else
			goto st3;
	} else
		goto st7;
	goto tr0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto tr123;
		case 53u: goto tr124;
		case 68u: goto st8;
		case 76u: goto st8;
		case 116u: goto tr126;
		case 132u: goto st9;
		case 140u: goto st9;
		case 180u: goto tr127;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 64u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr108;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr108;
			} else if ( (*p) >= 192u )
				goto tr0;
		} else
			goto tr124;
	} else
		goto tr125;
	goto tr19;
tr288:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st109;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
#line 2744 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st110;
		case 5u: goto st111;
		case 12u: goto st110;
		case 13u: goto st111;
		case 20u: goto st110;
		case 21u: goto st111;
		case 28u: goto st110;
		case 29u: goto st111;
		case 36u: goto st110;
		case 37u: goto st111;
		case 44u: goto st110;
		case 45u: goto st111;
		case 52u: goto st110;
		case 53u: goto st111;
		case 60u: goto st110;
		case 61u: goto st111;
		case 68u: goto st116;
		case 76u: goto st116;
		case 84u: goto st116;
		case 92u: goto st116;
		case 100u: goto st116;
		case 108u: goto st116;
		case 116u: goto st116;
		case 124u: goto st116;
		case 132u: goto st117;
		case 140u: goto st117;
		case 148u: goto st117;
		case 156u: goto st117;
		case 164u: goto st117;
		case 172u: goto st117;
		case 180u: goto st117;
		case 188u: goto st117;
	}
	if ( (*p) > 127u ) {
		if ( 128u <= (*p) && (*p) <= 191u )
			goto st111;
	} else if ( (*p) >= 64u )
		goto st115;
	goto st11;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	switch( (*p) ) {
		case 5u: goto st111;
		case 13u: goto st111;
		case 21u: goto st111;
		case 29u: goto st111;
		case 37u: goto st111;
		case 45u: goto st111;
		case 53u: goto st111;
		case 61u: goto st111;
		case 69u: goto st111;
		case 77u: goto st111;
		case 85u: goto st111;
		case 93u: goto st111;
		case 101u: goto st111;
		case 109u: goto st111;
		case 117u: goto st111;
		case 125u: goto st111;
		case 133u: goto st111;
		case 141u: goto st111;
		case 149u: goto st111;
		case 157u: goto st111;
		case 165u: goto st111;
		case 173u: goto st111;
		case 181u: goto st111;
		case 189u: goto st111;
		case 197u: goto st111;
		case 205u: goto st111;
		case 213u: goto st111;
		case 221u: goto st111;
		case 229u: goto st111;
		case 237u: goto st111;
		case 245u: goto st111;
		case 253u: goto st111;
	}
	goto st11;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
	goto st112;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
	goto st113;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	goto st114;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	goto tr146;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	goto tr147;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
	goto st115;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	goto st111;
tr291:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st118;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
#line 2870 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st34;
		case 5u: goto st35;
		case 12u: goto st34;
		case 13u: goto st35;
		case 20u: goto st34;
		case 21u: goto st35;
		case 28u: goto st34;
		case 29u: goto st35;
		case 36u: goto st34;
		case 37u: goto st35;
		case 44u: goto st34;
		case 45u: goto st35;
		case 52u: goto st34;
		case 53u: goto st35;
		case 60u: goto st34;
		case 61u: goto st35;
		case 68u: goto st40;
		case 76u: goto st40;
		case 84u: goto st40;
		case 92u: goto st40;
		case 100u: goto st40;
		case 108u: goto st40;
		case 116u: goto st40;
		case 124u: goto st40;
		case 132u: goto st41;
		case 140u: goto st41;
		case 148u: goto st41;
		case 156u: goto st41;
		case 164u: goto st41;
		case 172u: goto st41;
		case 180u: goto st41;
		case 188u: goto st41;
		case 224u: goto st119;
		case 225u: goto st224;
		case 226u: goto st226;
		case 227u: goto st228;
		case 228u: goto st230;
		case 229u: goto st232;
		case 230u: goto st234;
		case 231u: goto st236;
	}
	if ( (*p) > 127u ) {
		if ( 128u <= (*p) && (*p) <= 191u )
			goto st35;
	} else if ( (*p) >= 64u )
		goto st39;
	goto st10;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
	if ( (*p) == 224u )
		goto tr156;
	goto tr11;
tr156:
#line 53 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st240;
st240:
	if ( ++p == pe )
		goto _test_eof240;
case 240:
#line 2941 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto tr280;
		case 5u: goto tr281;
		case 12u: goto tr280;
		case 13u: goto tr281;
		case 14u: goto tr19;
		case 15u: goto tr282;
		case 20u: goto tr280;
		case 21u: goto tr281;
		case 28u: goto tr280;
		case 29u: goto tr281;
		case 36u: goto tr280;
		case 37u: goto tr281;
		case 44u: goto tr280;
		case 45u: goto tr281;
		case 46u: goto tr283;
		case 47u: goto tr19;
		case 52u: goto tr280;
		case 53u: goto tr281;
		case 60u: goto tr280;
		case 61u: goto tr281;
		case 62u: goto tr284;
		case 63u: goto tr19;
		case 101u: goto tr286;
		case 102u: goto tr287;
		case 104u: goto tr281;
		case 105u: goto tr288;
		case 106u: goto tr280;
		case 107u: goto tr289;
		case 128u: goto tr289;
		case 129u: goto tr288;
		case 130u: goto tr19;
		case 131u: goto tr291;
		case 141u: goto tr292;
		case 143u: goto tr293;
		case 154u: goto tr19;
		case 168u: goto tr280;
		case 169u: goto tr281;
		case 196u: goto tr296;
		case 197u: goto tr297;
		case 198u: goto tr298;
		case 199u: goto tr299;
		case 200u: goto tr300;
		case 202u: goto tr301;
		case 216u: goto tr279;
		case 217u: goto tr303;
		case 218u: goto tr304;
		case 219u: goto tr305;
		case 220u: goto tr306;
		case 221u: goto tr307;
		case 222u: goto tr308;
		case 223u: goto tr309;
		case 235u: goto tr290;
		case 240u: goto tr311;
		case 242u: goto tr312;
		case 243u: goto tr313;
		case 246u: goto tr314;
		case 247u: goto tr315;
		case 254u: goto tr316;
		case 255u: goto tr318;
	}
	if ( (*p) < 132u ) {
		if ( (*p) < 32u ) {
			if ( (*p) < 8u ) {
				if ( (*p) > 3u ) {
					if ( 6u <= (*p) && (*p) <= 7u )
						goto tr19;
				} else
					goto tr279;
			} else if ( (*p) > 19u ) {
				if ( (*p) < 24u ) {
					if ( 22u <= (*p) && (*p) <= 23u )
						goto tr19;
				} else if ( (*p) > 27u ) {
					if ( 30u <= (*p) && (*p) <= 31u )
						goto tr19;
				} else
					goto tr279;
			} else
				goto tr279;
		} else if ( (*p) > 35u ) {
			if ( (*p) < 54u ) {
				if ( (*p) > 39u ) {
					if ( 40u <= (*p) && (*p) <= 51u )
						goto tr279;
				} else if ( (*p) >= 38u )
					goto tr19;
			} else if ( (*p) > 55u ) {
				if ( (*p) < 96u ) {
					if ( 56u <= (*p) && (*p) <= 59u )
						goto tr279;
				} else if ( (*p) > 111u ) {
					if ( 112u <= (*p) && (*p) <= 127u )
						goto tr290;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr279;
	} else if ( (*p) > 139u ) {
		if ( (*p) < 194u ) {
			if ( (*p) < 160u ) {
				if ( (*p) > 142u ) {
					if ( 156u <= (*p) && (*p) <= 157u )
						goto tr19;
				} else if ( (*p) >= 140u )
					goto tr19;
			} else if ( (*p) > 163u ) {
				if ( (*p) < 184u ) {
					if ( 176u <= (*p) && (*p) <= 183u )
						goto tr280;
				} else if ( (*p) > 191u ) {
					if ( 192u <= (*p) && (*p) <= 193u )
						goto tr295;
				} else
					goto tr281;
			} else
				goto tr294;
		} else if ( (*p) > 195u ) {
			if ( (*p) < 212u ) {
				if ( (*p) > 207u ) {
					if ( 208u <= (*p) && (*p) <= 211u )
						goto tr302;
				} else if ( (*p) >= 204u )
					goto tr19;
			} else if ( (*p) > 231u ) {
				if ( (*p) < 234u ) {
					if ( 232u <= (*p) && (*p) <= 233u )
						goto tr310;
				} else if ( (*p) > 241u ) {
					if ( 250u <= (*p) && (*p) <= 251u )
						goto tr19;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr19;
	} else
		goto tr279;
	goto tr285;
tr293:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st120;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
#line 3095 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 68u: goto st8;
		case 132u: goto st9;
		case 232u: goto st121;
		case 233u: goto st136;
		case 234u: goto st144;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 7u )
			goto tr0;
	} else if ( (*p) > 71u ) {
		if ( (*p) > 135u ) {
			if ( 192u <= (*p) && (*p) <= 199u )
				goto tr0;
		} else if ( (*p) >= 128u )
			goto st3;
	} else
		goto st7;
	goto tr19;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	switch( (*p) ) {
		case 64u: goto tr160;
		case 68u: goto tr161;
		case 72u: goto tr160;
		case 76u: goto tr161;
		case 80u: goto tr160;
		case 84u: goto tr161;
		case 88u: goto tr160;
		case 92u: goto tr161;
		case 96u: goto tr160;
		case 100u: goto tr161;
		case 104u: goto tr160;
		case 108u: goto tr161;
		case 112u: goto tr160;
		case 116u: goto tr161;
		case 120u: goto tr162;
		case 124u: goto tr161;
		case 192u: goto tr163;
		case 196u: goto tr161;
		case 200u: goto tr163;
		case 204u: goto tr161;
		case 208u: goto tr163;
		case 212u: goto tr161;
		case 216u: goto tr163;
		case 220u: goto tr161;
		case 224u: goto tr163;
		case 228u: goto tr161;
		case 232u: goto tr163;
		case 236u: goto tr161;
		case 240u: goto tr163;
		case 244u: goto tr161;
		case 248u: goto tr163;
		case 252u: goto tr161;
	}
	goto tr19;
tr160:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st122;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
#line 3166 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 166u: goto st123;
		case 182u: goto st123;
	}
	if ( (*p) < 158u ) {
		if ( (*p) < 142u ) {
			if ( 133u <= (*p) && (*p) <= 135u )
				goto st123;
		} else if ( (*p) > 143u ) {
			if ( 149u <= (*p) && (*p) <= 151u )
				goto st123;
		} else
			goto st123;
	} else if ( (*p) > 159u ) {
		if ( (*p) < 204u ) {
			if ( 162u <= (*p) && (*p) <= 163u )
				goto st123;
		} else if ( (*p) > 207u ) {
			if ( 236u <= (*p) && (*p) <= 239u )
				goto st33;
		} else
			goto st33;
	} else
		goto st123;
	goto tr19;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
	switch( (*p) ) {
		case 4u: goto st125;
		case 5u: goto st126;
		case 12u: goto st125;
		case 13u: goto st126;
		case 20u: goto st125;
		case 21u: goto st126;
		case 28u: goto st125;
		case 29u: goto st126;
		case 36u: goto st125;
		case 37u: goto st126;
		case 44u: goto st125;
		case 45u: goto st126;
		case 52u: goto st125;
		case 53u: goto st126;
		case 60u: goto st125;
		case 61u: goto st126;
		case 68u: goto st131;
		case 76u: goto st131;
		case 84u: goto st131;
		case 92u: goto st131;
		case 100u: goto st131;
		case 108u: goto st131;
		case 116u: goto st131;
		case 124u: goto st131;
		case 132u: goto st132;
		case 140u: goto st132;
		case 148u: goto st132;
		case 156u: goto st132;
		case 164u: goto st132;
		case 172u: goto st132;
		case 180u: goto st132;
		case 188u: goto st132;
	}
	if ( (*p) > 127u ) {
		if ( 128u <= (*p) && (*p) <= 191u )
			goto st126;
	} else if ( (*p) >= 64u )
		goto st130;
	goto st124;
tr174:
#line 46 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP32);
    SET_DISP_PTR(p - 3);
  }
	goto st124;
tr175:
#line 42 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP8);
    SET_DISP_PTR(p);
  }
	goto st124;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
#line 3254 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 0u: goto tr0;
		case 16u: goto tr0;
		case 32u: goto tr0;
		case 48u: goto tr0;
		case 64u: goto tr0;
		case 80u: goto tr0;
		case 96u: goto tr0;
		case 112u: goto tr0;
	}
	goto tr19;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	switch( (*p) ) {
		case 5u: goto st126;
		case 13u: goto st126;
		case 21u: goto st126;
		case 29u: goto st126;
		case 37u: goto st126;
		case 45u: goto st126;
		case 53u: goto st126;
		case 61u: goto st126;
		case 69u: goto st126;
		case 77u: goto st126;
		case 85u: goto st126;
		case 93u: goto st126;
		case 101u: goto st126;
		case 109u: goto st126;
		case 117u: goto st126;
		case 125u: goto st126;
		case 133u: goto st126;
		case 141u: goto st126;
		case 149u: goto st126;
		case 157u: goto st126;
		case 165u: goto st126;
		case 173u: goto st126;
		case 181u: goto st126;
		case 189u: goto st126;
		case 197u: goto st126;
		case 205u: goto st126;
		case 213u: goto st126;
		case 221u: goto st126;
		case 229u: goto st126;
		case 237u: goto st126;
		case 245u: goto st126;
		case 253u: goto st126;
	}
	goto st124;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
	goto st127;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	goto st128;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
	goto st129;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
	goto tr174;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
	goto tr175;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
	goto st130;
st132:
	if ( ++p == pe )
		goto _test_eof132;
case 132:
	goto st126;
tr161:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st133;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
#line 3350 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( (*p) == 162u )
		goto st123;
	goto tr19;
tr162:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st134;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
#line 3364 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 166u: goto st123;
		case 182u: goto st123;
	}
	if ( (*p) < 158u ) {
		if ( (*p) < 142u ) {
			if ( 133u <= (*p) && (*p) <= 135u )
				goto st123;
		} else if ( (*p) > 143u ) {
			if ( 149u <= (*p) && (*p) <= 151u )
				goto st123;
		} else
			goto st123;
	} else if ( (*p) > 159u ) {
		if ( (*p) < 192u ) {
			if ( 162u <= (*p) && (*p) <= 163u )
				goto st123;
		} else if ( (*p) > 195u ) {
			if ( (*p) > 207u ) {
				if ( 236u <= (*p) && (*p) <= 239u )
					goto st33;
			} else if ( (*p) >= 204u )
				goto st33;
		} else
			goto st33;
	} else
		goto st123;
	goto tr19;
tr163:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st135;
st135:
	if ( ++p == pe )
		goto _test_eof135;
case 135:
#line 3403 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( 162u <= (*p) && (*p) <= 163u )
		goto st123;
	goto tr19;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
	switch( (*p) ) {
		case 64u: goto tr176;
		case 72u: goto tr176;
		case 80u: goto tr176;
		case 88u: goto tr176;
		case 96u: goto tr176;
		case 104u: goto tr176;
		case 112u: goto tr176;
		case 120u: goto tr177;
		case 124u: goto tr178;
		case 192u: goto tr179;
		case 200u: goto tr179;
		case 208u: goto tr179;
		case 216u: goto tr179;
		case 224u: goto tr179;
		case 232u: goto tr179;
		case 240u: goto tr179;
		case 248u: goto tr179;
	}
	goto tr19;
tr176:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st137;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
#line 3441 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 1u: goto st138;
		case 2u: goto st139;
	}
	if ( 144u <= (*p) && (*p) <= 155u )
		goto st1;
	goto tr19;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
	switch( (*p) ) {
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 72u ) {
		if ( (*p) > 7u ) {
			if ( 64u <= (*p) && (*p) <= 71u )
				goto tr19;
		} else
			goto tr19;
	} else if ( (*p) > 127u ) {
		if ( (*p) < 136u ) {
			if ( 128u <= (*p) && (*p) <= 135u )
				goto tr19;
		} else if ( (*p) > 191u ) {
			if ( 192u <= (*p) && (*p) <= 199u )
				goto tr19;
		} else
			goto st3;
	} else
		goto st7;
	goto tr0;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
	switch( (*p) ) {
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 76u: goto st8;
		case 116u: goto st8;
		case 140u: goto st9;
		case 180u: goto st9;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( 8u <= (*p) && (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 72u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 136u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 200u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
tr177:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st140;
st140:
	if ( ++p == pe )
		goto _test_eof140;
case 140:
#line 3549 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 1u: goto st138;
		case 2u: goto st139;
		case 18u: goto st141;
		case 203u: goto st1;
		case 219u: goto st1;
	}
	if ( (*p) < 198u ) {
		if ( (*p) < 144u ) {
			if ( 128u <= (*p) && (*p) <= 131u )
				goto st1;
		} else if ( (*p) > 155u ) {
			if ( 193u <= (*p) && (*p) <= 195u )
				goto st1;
		} else
			goto st1;
	} else if ( (*p) > 199u ) {
		if ( (*p) < 214u ) {
			if ( 209u <= (*p) && (*p) <= 211u )
				goto st1;
		} else if ( (*p) > 215u ) {
			if ( 225u <= (*p) && (*p) <= 227u )
				goto st1;
		} else
			goto st1;
	} else
		goto st1;
	goto tr19;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
	if ( 192u <= (*p) && (*p) <= 207u )
		goto tr0;
	goto tr19;
tr178:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st142;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
#line 3595 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( 128u <= (*p) && (*p) <= 129u )
		goto st1;
	goto tr19;
tr179:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st143;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
#line 3609 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( 144u <= (*p) && (*p) <= 155u )
		goto st1;
	goto tr19;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
	switch( (*p) ) {
		case 64u: goto tr183;
		case 72u: goto tr183;
		case 80u: goto tr183;
		case 88u: goto tr183;
		case 96u: goto tr183;
		case 104u: goto tr183;
		case 112u: goto tr183;
		case 120u: goto tr184;
	}
	goto tr19;
tr183:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st145;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
#line 3638 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( (*p) == 18u )
		goto st146;
	goto tr19;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
	switch( (*p) ) {
		case 4u: goto st110;
		case 5u: goto st111;
		case 12u: goto st110;
		case 13u: goto st111;
		case 68u: goto st116;
		case 76u: goto st116;
		case 132u: goto st117;
		case 140u: goto st117;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 15u )
			goto st11;
	} else if ( (*p) > 79u ) {
		if ( (*p) > 143u ) {
			if ( 192u <= (*p) && (*p) <= 207u )
				goto st11;
		} else if ( (*p) >= 128u )
			goto st111;
	} else
		goto st115;
	goto tr19;
tr184:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st147;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
#line 3678 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 16u: goto st109;
		case 18u: goto st146;
	}
	goto tr19;
tr296:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st148;
st148:
	if ( ++p == pe )
		goto _test_eof148;
case 148:
#line 3695 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 225u: goto st149;
		case 226u: goto st167;
		case 227u: goto st176;
	}
	goto tr19;
st149:
	if ( ++p == pe )
		goto _test_eof149;
case 149:
	switch( (*p) ) {
		case 65u: goto tr191;
		case 66u: goto tr192;
		case 67u: goto tr193;
		case 68u: goto tr194;
		case 69u: goto tr195;
		case 70u: goto tr196;
		case 71u: goto tr197;
		case 73u: goto tr191;
		case 74u: goto tr192;
		case 75u: goto tr193;
		case 76u: goto tr194;
		case 77u: goto tr195;
		case 78u: goto tr196;
		case 79u: goto tr197;
		case 81u: goto tr191;
		case 82u: goto tr192;
		case 83u: goto tr193;
		case 84u: goto tr194;
		case 85u: goto tr195;
		case 86u: goto tr196;
		case 87u: goto tr197;
		case 89u: goto tr191;
		case 90u: goto tr192;
		case 91u: goto tr193;
		case 92u: goto tr194;
		case 93u: goto tr195;
		case 94u: goto tr196;
		case 95u: goto tr197;
		case 97u: goto tr191;
		case 98u: goto tr192;
		case 99u: goto tr193;
		case 100u: goto tr194;
		case 101u: goto tr195;
		case 102u: goto tr196;
		case 103u: goto tr197;
		case 105u: goto tr191;
		case 106u: goto tr192;
		case 107u: goto tr193;
		case 108u: goto tr194;
		case 109u: goto tr195;
		case 110u: goto tr196;
		case 111u: goto tr197;
		case 113u: goto tr191;
		case 114u: goto tr192;
		case 115u: goto tr193;
		case 116u: goto tr194;
		case 117u: goto tr195;
		case 118u: goto tr196;
		case 119u: goto tr197;
		case 120u: goto tr198;
		case 121u: goto tr199;
		case 122u: goto tr200;
		case 123u: goto tr201;
		case 124u: goto tr202;
		case 125u: goto tr203;
		case 126u: goto tr204;
		case 127u: goto tr205;
	}
	if ( 64u <= (*p) && (*p) <= 112u )
		goto tr190;
	goto tr19;
tr190:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st150;
tr235:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st150;
st150:
	if ( ++p == pe )
		goto _test_eof150;
case 150:
#line 3785 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 18u: goto st1;
		case 23u: goto st29;
		case 81u: goto st1;
		case 194u: goto st33;
		case 198u: goto st33;
	}
	if ( (*p) < 46u ) {
		if ( 20u <= (*p) && (*p) <= 22u )
			goto st1;
	} else if ( (*p) > 47u ) {
		if ( (*p) > 89u ) {
			if ( 92u <= (*p) && (*p) <= 95u )
				goto st1;
		} else if ( (*p) >= 84u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr191:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st151;
tr236:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st151;
st151:
	if ( ++p == pe )
		goto _test_eof151;
case 151:
#line 3822 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 18u: goto st29;
		case 81u: goto st1;
		case 115u: goto st152;
		case 194u: goto st33;
		case 198u: goto st33;
	}
	if ( (*p) < 116u ) {
		if ( (*p) < 46u ) {
			if ( (*p) > 21u ) {
				if ( 22u <= (*p) && (*p) <= 23u )
					goto st29;
			} else if ( (*p) >= 20u )
				goto st1;
		} else if ( (*p) > 47u ) {
			if ( (*p) < 92u ) {
				if ( 84u <= (*p) && (*p) <= 89u )
					goto st1;
			} else if ( (*p) > 109u ) {
				if ( 113u <= (*p) && (*p) <= 114u )
					goto st42;
			} else
				goto st1;
		} else
			goto st1;
	} else if ( (*p) > 118u ) {
		if ( (*p) < 216u ) {
			if ( (*p) > 125u ) {
				if ( 208u <= (*p) && (*p) <= 213u )
					goto st1;
			} else if ( (*p) >= 124u )
				goto st1;
		} else if ( (*p) > 229u ) {
			if ( (*p) < 241u ) {
				if ( 232u <= (*p) && (*p) <= 239u )
					goto st1;
			} else if ( (*p) > 246u ) {
				if ( 248u <= (*p) && (*p) <= 254u )
					goto st1;
			} else
				goto st1;
		} else
			goto st1;
	} else
		goto st1;
	goto tr19;
st152:
	if ( ++p == pe )
		goto _test_eof152;
case 152:
	if ( (*p) > 223u ) {
		if ( 240u <= (*p) )
			goto st10;
	} else if ( (*p) >= 208u )
		goto st10;
	goto tr19;
tr192:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st153;
st153:
	if ( ++p == pe )
		goto _test_eof153;
case 153:
#line 3889 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 42u: goto st1;
		case 81u: goto st1;
		case 83u: goto st1;
		case 194u: goto st33;
	}
	if ( (*p) > 90u ) {
		if ( 92u <= (*p) && (*p) <= 95u )
			goto st1;
	} else if ( (*p) >= 88u )
		goto st1;
	goto tr19;
tr193:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st154;
st154:
	if ( ++p == pe )
		goto _test_eof154;
case 154:
#line 3912 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 42u: goto st1;
		case 81u: goto st1;
		case 194u: goto st33;
		case 208u: goto st1;
	}
	if ( (*p) < 92u ) {
		if ( 88u <= (*p) && (*p) <= 90u )
			goto st1;
	} else if ( (*p) > 95u ) {
		if ( 124u <= (*p) && (*p) <= 125u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr194:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st155;
tr239:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st155;
st155:
	if ( ++p == pe )
		goto _test_eof155;
case 155:
#line 3945 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 81u: goto st1;
		case 194u: goto st33;
		case 198u: goto st33;
	}
	if ( (*p) < 84u ) {
		if ( 20u <= (*p) && (*p) <= 21u )
			goto st1;
	} else if ( (*p) > 89u ) {
		if ( 92u <= (*p) && (*p) <= 95u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr195:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st156;
tr240:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st156;
st156:
	if ( ++p == pe )
		goto _test_eof156;
case 156:
#line 3977 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 81u: goto st1;
		case 194u: goto st33;
		case 198u: goto st33;
		case 208u: goto st1;
	}
	if ( (*p) < 84u ) {
		if ( 20u <= (*p) && (*p) <= 21u )
			goto st1;
	} else if ( (*p) > 89u ) {
		if ( (*p) > 95u ) {
			if ( 124u <= (*p) && (*p) <= 125u )
				goto st1;
		} else if ( (*p) >= 92u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr196:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st157;
tr241:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st157;
st157:
	if ( ++p == pe )
		goto _test_eof157;
case 157:
#line 4013 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( 16u <= (*p) && (*p) <= 17u )
		goto st32;
	goto tr19;
tr197:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st158;
tr242:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st158;
st158:
	if ( ++p == pe )
		goto _test_eof158;
case 158:
#line 4034 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( (*p) == 208u )
		goto st1;
	if ( (*p) > 17u ) {
		if ( 124u <= (*p) && (*p) <= 125u )
			goto st1;
	} else if ( (*p) >= 16u )
		goto st32;
	goto tr19;
tr198:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st159;
tr243:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st159;
st159:
	if ( ++p == pe )
		goto _test_eof159;
case 159:
#line 4060 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 19u: goto st29;
		case 23u: goto st29;
		case 43u: goto st29;
		case 80u: goto st32;
		case 119u: goto tr0;
		case 174u: goto st101;
		case 194u: goto st33;
		case 198u: goto st33;
	}
	if ( (*p) < 40u ) {
		if ( 16u <= (*p) && (*p) <= 22u )
			goto st1;
	} else if ( (*p) > 41u ) {
		if ( (*p) > 47u ) {
			if ( 81u <= (*p) && (*p) <= 95u )
				goto st1;
		} else if ( (*p) >= 46u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr199:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st160;
tr244:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st160;
st160:
	if ( ++p == pe )
		goto _test_eof160;
case 160:
#line 4100 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 43u: goto st29;
		case 80u: goto st32;
		case 81u: goto st1;
		case 112u: goto st33;
		case 115u: goto st152;
		case 127u: goto st1;
		case 194u: goto st33;
		case 197u: goto st49;
		case 215u: goto st32;
		case 231u: goto st29;
		case 247u: goto st32;
	}
	if ( (*p) < 84u ) {
		if ( (*p) < 20u ) {
			if ( (*p) > 17u ) {
				if ( 18u <= (*p) && (*p) <= 19u )
					goto st29;
			} else if ( (*p) >= 16u )
				goto st1;
		} else if ( (*p) > 21u ) {
			if ( (*p) < 40u ) {
				if ( 22u <= (*p) && (*p) <= 23u )
					goto st29;
			} else if ( (*p) > 41u ) {
				if ( 46u <= (*p) && (*p) <= 47u )
					goto st1;
			} else
				goto st1;
		} else
			goto st1;
	} else if ( (*p) > 111u ) {
		if ( (*p) < 196u ) {
			if ( (*p) < 116u ) {
				if ( 113u <= (*p) && (*p) <= 114u )
					goto st42;
			} else if ( (*p) > 118u ) {
				if ( 124u <= (*p) && (*p) <= 125u )
					goto st1;
			} else
				goto st1;
		} else if ( (*p) > 198u ) {
			if ( (*p) < 216u ) {
				if ( 208u <= (*p) && (*p) <= 213u )
					goto st1;
			} else if ( (*p) > 239u ) {
				if ( 241u <= (*p) && (*p) <= 254u )
					goto st1;
			} else
				goto st1;
		} else
			goto st33;
	} else
		goto st1;
	goto tr19;
tr200:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st161;
st161:
	if ( ++p == pe )
		goto _test_eof161;
case 161:
#line 4166 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 18u: goto st1;
		case 22u: goto st1;
		case 42u: goto st1;
		case 111u: goto st1;
		case 112u: goto st33;
		case 194u: goto st33;
		case 230u: goto st1;
	}
	if ( (*p) < 81u ) {
		if ( (*p) > 17u ) {
			if ( 44u <= (*p) && (*p) <= 45u )
				goto st1;
		} else if ( (*p) >= 16u )
			goto st29;
	} else if ( (*p) > 83u ) {
		if ( (*p) > 95u ) {
			if ( 126u <= (*p) && (*p) <= 127u )
				goto st1;
		} else if ( (*p) >= 88u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr201:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st162;
st162:
	if ( ++p == pe )
		goto _test_eof162;
case 162:
#line 4201 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 18u: goto st1;
		case 42u: goto st1;
		case 81u: goto st1;
		case 112u: goto st33;
		case 194u: goto st33;
		case 208u: goto st1;
		case 230u: goto st1;
		case 240u: goto st29;
	}
	if ( (*p) < 88u ) {
		if ( (*p) > 17u ) {
			if ( 44u <= (*p) && (*p) <= 45u )
				goto st1;
		} else if ( (*p) >= 16u )
			goto st29;
	} else if ( (*p) > 90u ) {
		if ( (*p) > 95u ) {
			if ( 124u <= (*p) && (*p) <= 125u )
				goto st1;
		} else if ( (*p) >= 92u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr202:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st163;
tr247:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st163;
st163:
	if ( ++p == pe )
		goto _test_eof163;
case 163:
#line 4244 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 43u: goto st29;
		case 80u: goto st32;
		case 119u: goto tr0;
		case 194u: goto st33;
		case 198u: goto st33;
	}
	if ( (*p) < 20u ) {
		if ( 16u <= (*p) && (*p) <= 17u )
			goto st1;
	} else if ( (*p) > 21u ) {
		if ( (*p) > 41u ) {
			if ( 81u <= (*p) && (*p) <= 95u )
				goto st1;
		} else if ( (*p) >= 40u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr203:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st164;
tr248:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st164;
st164:
	if ( ++p == pe )
		goto _test_eof164;
case 164:
#line 4281 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 43u: goto st29;
		case 80u: goto st32;
		case 81u: goto st1;
		case 194u: goto st33;
		case 198u: goto st33;
		case 208u: goto st1;
		case 214u: goto st1;
		case 230u: goto st1;
		case 231u: goto st29;
	}
	if ( (*p) < 40u ) {
		if ( (*p) > 17u ) {
			if ( 20u <= (*p) && (*p) <= 21u )
				goto st1;
		} else if ( (*p) >= 16u )
			goto st1;
	} else if ( (*p) > 41u ) {
		if ( (*p) > 95u ) {
			if ( 124u <= (*p) && (*p) <= 127u )
				goto st1;
		} else if ( (*p) >= 84u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr204:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st165;
tr249:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st165;
st165:
	if ( ++p == pe )
		goto _test_eof165;
case 165:
#line 4325 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 18u: goto st1;
		case 22u: goto st1;
		case 91u: goto st1;
		case 127u: goto st1;
		case 230u: goto st1;
	}
	if ( 16u <= (*p) && (*p) <= 17u )
		goto st32;
	goto tr19;
tr205:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st166;
tr250:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st166;
st166:
	if ( ++p == pe )
		goto _test_eof166;
case 166:
#line 4353 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 18u: goto st1;
		case 208u: goto st1;
		case 230u: goto st1;
		case 240u: goto st29;
	}
	if ( (*p) > 17u ) {
		if ( 124u <= (*p) && (*p) <= 125u )
			goto st1;
	} else if ( (*p) >= 16u )
		goto st32;
	goto tr19;
st167:
	if ( ++p == pe )
		goto _test_eof167;
case 167:
	switch( (*p) ) {
		case 64u: goto tr207;
		case 65u: goto tr208;
		case 69u: goto tr209;
		case 72u: goto tr207;
		case 73u: goto tr208;
		case 77u: goto tr209;
		case 80u: goto tr207;
		case 81u: goto tr208;
		case 85u: goto tr209;
		case 88u: goto tr207;
		case 89u: goto tr208;
		case 93u: goto tr209;
		case 96u: goto tr207;
		case 97u: goto tr208;
		case 101u: goto tr209;
		case 104u: goto tr207;
		case 105u: goto tr208;
		case 109u: goto tr209;
		case 112u: goto tr207;
		case 113u: goto tr208;
		case 117u: goto tr209;
		case 120u: goto tr207;
		case 121u: goto tr210;
		case 125u: goto tr211;
		case 193u: goto tr212;
		case 197u: goto tr213;
		case 201u: goto tr212;
		case 205u: goto tr213;
		case 209u: goto tr212;
		case 213u: goto tr213;
		case 217u: goto tr212;
		case 221u: goto tr213;
		case 225u: goto tr212;
		case 229u: goto tr213;
		case 233u: goto tr212;
		case 237u: goto tr213;
		case 241u: goto tr212;
		case 245u: goto tr213;
		case 249u: goto tr212;
		case 253u: goto tr213;
	}
	goto tr19;
tr207:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st168;
st168:
	if ( ++p == pe )
		goto _test_eof168;
case 168:
#line 4423 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 242u: goto st1;
		case 243u: goto st169;
		case 247u: goto st1;
	}
	goto tr19;
st169:
	if ( ++p == pe )
		goto _test_eof169;
case 169:
	switch( (*p) ) {
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
	}
	if ( (*p) < 72u ) {
		if ( 8u <= (*p) && (*p) <= 31u )
			goto tr0;
	} else if ( (*p) > 95u ) {
		if ( (*p) > 159u ) {
			if ( 200u <= (*p) && (*p) <= 223u )
				goto tr0;
		} else if ( (*p) >= 136u )
			goto st3;
	} else
		goto st7;
	goto tr19;
tr208:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st170;
st170:
	if ( ++p == pe )
		goto _test_eof170;
case 170:
#line 4470 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( (*p) == 43u )
		goto st1;
	if ( (*p) < 55u ) {
		if ( (*p) < 40u ) {
			if ( (*p) <= 13u )
				goto st1;
		} else if ( (*p) > 41u ) {
			if ( 44u <= (*p) && (*p) <= 47u )
				goto st29;
		} else
			goto st1;
	} else if ( (*p) > 64u ) {
		if ( (*p) < 166u ) {
			if ( 150u <= (*p) && (*p) <= 159u )
				goto st1;
		} else if ( (*p) > 175u ) {
			if ( (*p) > 191u ) {
				if ( 219u <= (*p) && (*p) <= 223u )
					goto st1;
			} else if ( (*p) >= 182u )
				goto st1;
		} else
			goto st1;
	} else
		goto st1;
	goto tr19;
tr209:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st171;
st171:
	if ( ++p == pe )
		goto _test_eof171;
case 171:
#line 4507 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 154u: goto st1;
		case 156u: goto st1;
		case 158u: goto st1;
		case 170u: goto st1;
		case 172u: goto st1;
		case 174u: goto st1;
		case 186u: goto st1;
		case 188u: goto st1;
		case 190u: goto st1;
	}
	if ( (*p) < 150u ) {
		if ( (*p) > 13u ) {
			if ( 44u <= (*p) && (*p) <= 47u )
				goto st29;
		} else if ( (*p) >= 12u )
			goto st1;
	} else if ( (*p) > 152u ) {
		if ( (*p) > 168u ) {
			if ( 182u <= (*p) && (*p) <= 184u )
				goto st1;
		} else if ( (*p) >= 166u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr210:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st172;
st172:
	if ( ++p == pe )
		goto _test_eof172;
case 172:
#line 4544 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 19u: goto st1;
		case 23u: goto st1;
		case 24u: goto st29;
		case 42u: goto st29;
	}
	if ( (*p) < 48u ) {
		if ( (*p) < 32u ) {
			if ( (*p) > 15u ) {
				if ( 28u <= (*p) && (*p) <= 30u )
					goto st1;
			} else
				goto st1;
		} else if ( (*p) > 37u ) {
			if ( (*p) > 43u ) {
				if ( 44u <= (*p) && (*p) <= 47u )
					goto st29;
			} else if ( (*p) >= 40u )
				goto st1;
		} else
			goto st1;
	} else if ( (*p) > 53u ) {
		if ( (*p) < 166u ) {
			if ( (*p) > 65u ) {
				if ( 150u <= (*p) && (*p) <= 159u )
					goto st1;
			} else if ( (*p) >= 55u )
				goto st1;
		} else if ( (*p) > 175u ) {
			if ( (*p) > 191u ) {
				if ( 219u <= (*p) && (*p) <= 223u )
					goto st1;
			} else if ( (*p) >= 182u )
				goto st1;
		} else
			goto st1;
	} else
		goto st1;
	goto tr19;
tr211:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st173;
st173:
	if ( ++p == pe )
		goto _test_eof173;
case 173:
#line 4594 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 19u: goto st1;
		case 23u: goto st1;
		case 154u: goto st1;
		case 156u: goto st1;
		case 158u: goto st1;
		case 170u: goto st1;
		case 172u: goto st1;
		case 174u: goto st1;
		case 186u: goto st1;
		case 188u: goto st1;
		case 190u: goto st1;
	}
	if ( (*p) < 44u ) {
		if ( (*p) > 15u ) {
			if ( 24u <= (*p) && (*p) <= 26u )
				goto st29;
		} else if ( (*p) >= 12u )
			goto st1;
	} else if ( (*p) > 47u ) {
		if ( (*p) < 166u ) {
			if ( 150u <= (*p) && (*p) <= 152u )
				goto st1;
		} else if ( (*p) > 168u ) {
			if ( 182u <= (*p) && (*p) <= 184u )
				goto st1;
		} else
			goto st1;
	} else
		goto st29;
	goto tr19;
tr212:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st174;
st174:
	if ( ++p == pe )
		goto _test_eof174;
case 174:
#line 4636 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( (*p) < 166u ) {
		if ( 150u <= (*p) && (*p) <= 159u )
			goto st1;
	} else if ( (*p) > 175u ) {
		if ( 182u <= (*p) && (*p) <= 191u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr213:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st175;
st175:
	if ( ++p == pe )
		goto _test_eof175;
case 175:
#line 4656 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 154u: goto st1;
		case 156u: goto st1;
		case 158u: goto st1;
		case 170u: goto st1;
		case 172u: goto st1;
		case 174u: goto st1;
		case 186u: goto st1;
		case 188u: goto st1;
		case 190u: goto st1;
	}
	if ( (*p) < 166u ) {
		if ( 150u <= (*p) && (*p) <= 152u )
			goto st1;
	} else if ( (*p) > 168u ) {
		if ( 182u <= (*p) && (*p) <= 184u )
			goto st1;
	} else
		goto st1;
	goto tr19;
st176:
	if ( ++p == pe )
		goto _test_eof176;
case 176:
	switch( (*p) ) {
		case 65u: goto tr215;
		case 69u: goto tr216;
		case 73u: goto tr215;
		case 77u: goto tr216;
		case 81u: goto tr215;
		case 85u: goto tr216;
		case 89u: goto tr215;
		case 93u: goto tr216;
		case 97u: goto tr215;
		case 101u: goto tr216;
		case 105u: goto tr215;
		case 109u: goto tr216;
		case 113u: goto tr215;
		case 117u: goto tr216;
		case 121u: goto tr217;
		case 125u: goto tr218;
		case 193u: goto tr219;
		case 197u: goto tr220;
		case 201u: goto tr219;
		case 205u: goto tr220;
		case 209u: goto tr219;
		case 213u: goto tr220;
		case 217u: goto tr219;
		case 221u: goto tr220;
		case 225u: goto tr219;
		case 229u: goto tr220;
		case 233u: goto tr219;
		case 237u: goto tr220;
		case 241u: goto tr219;
		case 245u: goto tr220;
		case 249u: goto tr219;
		case 253u: goto tr220;
	}
	goto tr19;
tr215:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st177;
st177:
	if ( ++p == pe )
		goto _test_eof177;
case 177:
#line 4726 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 33u: goto st33;
		case 68u: goto st33;
		case 223u: goto st33;
	}
	if ( (*p) < 74u ) {
		if ( (*p) < 64u ) {
			if ( 8u <= (*p) && (*p) <= 15u )
				goto st33;
		} else if ( (*p) > 66u ) {
			if ( 72u <= (*p) && (*p) <= 73u )
				goto st178;
		} else
			goto st33;
	} else if ( (*p) > 76u ) {
		if ( (*p) < 104u ) {
			if ( 92u <= (*p) && (*p) <= 95u )
				goto st123;
		} else if ( (*p) > 111u ) {
			if ( 120u <= (*p) && (*p) <= 127u )
				goto st123;
		} else
			goto st123;
	} else
		goto st123;
	goto tr19;
st178:
	if ( ++p == pe )
		goto _test_eof178;
case 178:
	switch( (*p) ) {
		case 4u: goto st180;
		case 5u: goto st181;
		case 12u: goto st180;
		case 13u: goto st181;
		case 20u: goto st180;
		case 21u: goto st181;
		case 28u: goto st180;
		case 29u: goto st181;
		case 36u: goto st180;
		case 37u: goto st181;
		case 44u: goto st180;
		case 45u: goto st181;
		case 52u: goto st180;
		case 53u: goto st181;
		case 60u: goto st180;
		case 61u: goto st181;
		case 68u: goto st186;
		case 76u: goto st186;
		case 84u: goto st186;
		case 92u: goto st186;
		case 100u: goto st186;
		case 108u: goto st186;
		case 116u: goto st186;
		case 124u: goto st186;
		case 132u: goto st187;
		case 140u: goto st187;
		case 148u: goto st187;
		case 156u: goto st187;
		case 164u: goto st187;
		case 172u: goto st187;
		case 180u: goto st187;
		case 188u: goto st187;
	}
	if ( (*p) > 127u ) {
		if ( 128u <= (*p) && (*p) <= 191u )
			goto st181;
	} else if ( (*p) >= 64u )
		goto st185;
	goto st179;
tr232:
#line 46 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP32);
    SET_DISP_PTR(p - 3);
  }
	goto st179;
tr233:
#line 42 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_DISP_TYPE(DISP8);
    SET_DISP_PTR(p);
  }
	goto st179;
st179:
	if ( ++p == pe )
		goto _test_eof179;
case 179:
#line 4815 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( (*p) < 48u ) {
		if ( (*p) < 16u ) {
			if ( (*p) <= 3u )
				goto tr228;
		} else if ( (*p) > 19u ) {
			if ( 32u <= (*p) && (*p) <= 35u )
				goto tr228;
		} else
			goto tr228;
	} else if ( (*p) > 51u ) {
		if ( (*p) < 80u ) {
			if ( 64u <= (*p) && (*p) <= 67u )
				goto tr228;
		} else if ( (*p) > 83u ) {
			if ( (*p) > 99u ) {
				if ( 112u <= (*p) && (*p) <= 115u )
					goto tr228;
			} else if ( (*p) >= 96u )
				goto tr228;
		} else
			goto tr228;
	} else
		goto tr228;
	goto tr19;
st180:
	if ( ++p == pe )
		goto _test_eof180;
case 180:
	switch( (*p) ) {
		case 5u: goto st181;
		case 13u: goto st181;
		case 21u: goto st181;
		case 29u: goto st181;
		case 37u: goto st181;
		case 45u: goto st181;
		case 53u: goto st181;
		case 61u: goto st181;
		case 69u: goto st181;
		case 77u: goto st181;
		case 85u: goto st181;
		case 93u: goto st181;
		case 101u: goto st181;
		case 109u: goto st181;
		case 117u: goto st181;
		case 125u: goto st181;
		case 133u: goto st181;
		case 141u: goto st181;
		case 149u: goto st181;
		case 157u: goto st181;
		case 165u: goto st181;
		case 173u: goto st181;
		case 181u: goto st181;
		case 189u: goto st181;
		case 197u: goto st181;
		case 205u: goto st181;
		case 213u: goto st181;
		case 221u: goto st181;
		case 229u: goto st181;
		case 237u: goto st181;
		case 245u: goto st181;
		case 253u: goto st181;
	}
	goto st179;
st181:
	if ( ++p == pe )
		goto _test_eof181;
case 181:
	goto st182;
st182:
	if ( ++p == pe )
		goto _test_eof182;
case 182:
	goto st183;
st183:
	if ( ++p == pe )
		goto _test_eof183;
case 183:
	goto st184;
st184:
	if ( ++p == pe )
		goto _test_eof184;
case 184:
	goto tr232;
st185:
	if ( ++p == pe )
		goto _test_eof185;
case 185:
	goto tr233;
st186:
	if ( ++p == pe )
		goto _test_eof186;
case 186:
	goto st185;
st187:
	if ( ++p == pe )
		goto _test_eof187;
case 187:
	goto st181;
tr216:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st188;
st188:
	if ( ++p == pe )
		goto _test_eof188;
case 188:
#line 4924 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 6u: goto st33;
		case 64u: goto st33;
	}
	if ( (*p) < 92u ) {
		if ( (*p) < 12u ) {
			if ( 8u <= (*p) && (*p) <= 9u )
				goto st33;
		} else if ( (*p) > 13u ) {
			if ( (*p) > 73u ) {
				if ( 74u <= (*p) && (*p) <= 75u )
					goto st123;
			} else if ( (*p) >= 72u )
				goto st178;
		} else
			goto st33;
	} else if ( (*p) > 95u ) {
		if ( (*p) < 108u ) {
			if ( 104u <= (*p) && (*p) <= 105u )
				goto st123;
		} else if ( (*p) > 109u ) {
			if ( (*p) > 121u ) {
				if ( 124u <= (*p) && (*p) <= 125u )
					goto st123;
			} else if ( (*p) >= 120u )
				goto st123;
		} else
			goto st123;
	} else
		goto st123;
	goto tr19;
tr217:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st189;
st189:
	if ( ++p == pe )
		goto _test_eof189;
case 189:
#line 4966 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 23u: goto st190;
		case 29u: goto st33;
		case 68u: goto st33;
		case 223u: goto st33;
	}
	if ( (*p) < 72u ) {
		if ( (*p) < 20u ) {
			if ( (*p) > 5u ) {
				if ( 8u <= (*p) && (*p) <= 15u )
					goto st33;
			} else if ( (*p) >= 4u )
				goto st33;
		} else if ( (*p) > 22u ) {
			if ( (*p) > 34u ) {
				if ( 64u <= (*p) && (*p) <= 66u )
					goto st33;
			} else if ( (*p) >= 32u )
				goto st33;
		} else
			goto st33;
	} else if ( (*p) > 73u ) {
		if ( (*p) < 96u ) {
			if ( (*p) > 76u ) {
				if ( 92u <= (*p) && (*p) <= 95u )
					goto st123;
			} else if ( (*p) >= 74u )
				goto st123;
		} else if ( (*p) > 99u ) {
			if ( (*p) > 111u ) {
				if ( 120u <= (*p) && (*p) <= 127u )
					goto st123;
			} else if ( (*p) >= 104u )
				goto st123;
		} else
			goto st33;
	} else
		goto st178;
	goto tr19;
st190:
	if ( ++p == pe )
		goto _test_eof190;
case 190:
	switch( (*p) ) {
		case 4u: goto st34;
		case 12u: goto st34;
		case 20u: goto st34;
		case 28u: goto st34;
		case 36u: goto st34;
		case 44u: goto st34;
		case 52u: goto st34;
		case 60u: goto st34;
		case 68u: goto st40;
		case 76u: goto st40;
		case 84u: goto st40;
		case 92u: goto st40;
		case 100u: goto st40;
		case 108u: goto st40;
		case 116u: goto st40;
		case 124u: goto st40;
		case 132u: goto st41;
		case 140u: goto st41;
		case 148u: goto st41;
		case 156u: goto st41;
		case 164u: goto st41;
		case 172u: goto st41;
		case 180u: goto st41;
		case 188u: goto st41;
	}
	if ( (*p) < 38u ) {
		if ( (*p) < 14u ) {
			if ( (*p) > 3u ) {
				if ( 6u <= (*p) && (*p) <= 11u )
					goto st10;
			} else
				goto st10;
		} else if ( (*p) > 19u ) {
			if ( (*p) > 27u ) {
				if ( 30u <= (*p) && (*p) <= 35u )
					goto st10;
			} else if ( (*p) >= 22u )
				goto st10;
		} else
			goto st10;
	} else if ( (*p) > 43u ) {
		if ( (*p) < 62u ) {
			if ( (*p) > 51u ) {
				if ( 54u <= (*p) && (*p) <= 59u )
					goto st10;
			} else if ( (*p) >= 46u )
				goto st10;
		} else if ( (*p) > 63u ) {
			if ( (*p) > 127u ) {
				if ( 192u <= (*p) )
					goto tr19;
			} else if ( (*p) >= 64u )
				goto st39;
		} else
			goto st10;
	} else
		goto st10;
	goto st35;
tr218:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st191;
st191:
	if ( ++p == pe )
		goto _test_eof191;
case 191:
#line 5079 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 29u: goto st33;
		case 64u: goto st33;
	}
	if ( (*p) < 74u ) {
		if ( (*p) < 12u ) {
			if ( (*p) > 6u ) {
				if ( 8u <= (*p) && (*p) <= 9u )
					goto st33;
			} else if ( (*p) >= 4u )
				goto st33;
		} else if ( (*p) > 13u ) {
			if ( (*p) > 25u ) {
				if ( 72u <= (*p) && (*p) <= 73u )
					goto st178;
			} else if ( (*p) >= 24u )
				goto st33;
		} else
			goto st33;
	} else if ( (*p) > 75u ) {
		if ( (*p) < 108u ) {
			if ( (*p) > 95u ) {
				if ( 104u <= (*p) && (*p) <= 105u )
					goto st123;
			} else if ( (*p) >= 92u )
				goto st123;
		} else if ( (*p) > 109u ) {
			if ( (*p) > 121u ) {
				if ( 124u <= (*p) && (*p) <= 125u )
					goto st123;
			} else if ( (*p) >= 120u )
				goto st123;
		} else
			goto st123;
	} else
		goto st123;
	goto tr19;
tr219:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st192;
st192:
	if ( ++p == pe )
		goto _test_eof192;
case 192:
#line 5127 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( (*p) < 92u ) {
		if ( 72u <= (*p) && (*p) <= 73u )
			goto st178;
	} else if ( (*p) > 95u ) {
		if ( (*p) > 111u ) {
			if ( 120u <= (*p) && (*p) <= 127u )
				goto st123;
		} else if ( (*p) >= 104u )
			goto st123;
	} else
		goto st123;
	goto tr19;
tr220:
#line 161 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_VEX_PREFIX3(*p);
  }
	goto st193;
st193:
	if ( ++p == pe )
		goto _test_eof193;
case 193:
#line 5150 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( (*p) < 104u ) {
		if ( (*p) > 73u ) {
			if ( 92u <= (*p) && (*p) <= 95u )
				goto st123;
		} else if ( (*p) >= 72u )
			goto st178;
	} else if ( (*p) > 105u ) {
		if ( (*p) < 120u ) {
			if ( 108u <= (*p) && (*p) <= 109u )
				goto st123;
		} else if ( (*p) > 121u ) {
			if ( 124u <= (*p) && (*p) <= 125u )
				goto st123;
		} else
			goto st123;
	} else
		goto st123;
	goto tr19;
tr297:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st194;
st194:
	if ( ++p == pe )
		goto _test_eof194;
case 194:
#line 5180 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 193u: goto tr236;
		case 194u: goto tr237;
		case 195u: goto tr238;
		case 196u: goto tr239;
		case 197u: goto tr240;
		case 198u: goto tr241;
		case 199u: goto tr242;
		case 201u: goto tr236;
		case 202u: goto tr237;
		case 203u: goto tr238;
		case 204u: goto tr239;
		case 205u: goto tr240;
		case 206u: goto tr241;
		case 207u: goto tr242;
		case 209u: goto tr236;
		case 210u: goto tr237;
		case 211u: goto tr238;
		case 212u: goto tr239;
		case 213u: goto tr240;
		case 214u: goto tr241;
		case 215u: goto tr242;
		case 217u: goto tr236;
		case 218u: goto tr237;
		case 219u: goto tr238;
		case 220u: goto tr239;
		case 221u: goto tr240;
		case 222u: goto tr241;
		case 223u: goto tr242;
		case 225u: goto tr236;
		case 226u: goto tr237;
		case 227u: goto tr238;
		case 228u: goto tr239;
		case 229u: goto tr240;
		case 230u: goto tr241;
		case 231u: goto tr242;
		case 233u: goto tr236;
		case 234u: goto tr237;
		case 235u: goto tr238;
		case 236u: goto tr239;
		case 237u: goto tr240;
		case 238u: goto tr241;
		case 239u: goto tr242;
		case 241u: goto tr236;
		case 242u: goto tr237;
		case 243u: goto tr238;
		case 244u: goto tr239;
		case 245u: goto tr240;
		case 246u: goto tr241;
		case 247u: goto tr242;
		case 248u: goto tr243;
		case 249u: goto tr244;
		case 250u: goto tr245;
		case 251u: goto tr246;
		case 252u: goto tr247;
		case 253u: goto tr248;
		case 254u: goto tr249;
		case 255u: goto tr250;
	}
	if ( 192u <= (*p) && (*p) <= 240u )
		goto tr235;
	goto tr19;
tr237:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st195;
st195:
	if ( ++p == pe )
		goto _test_eof195;
case 195:
#line 5254 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 81u: goto st1;
		case 83u: goto st1;
		case 194u: goto st33;
	}
	if ( (*p) > 90u ) {
		if ( 92u <= (*p) && (*p) <= 95u )
			goto st1;
	} else if ( (*p) >= 88u )
		goto st1;
	goto tr19;
tr238:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st196;
st196:
	if ( ++p == pe )
		goto _test_eof196;
case 196:
#line 5277 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 81u: goto st1;
		case 194u: goto st33;
		case 208u: goto st1;
	}
	if ( (*p) < 92u ) {
		if ( 88u <= (*p) && (*p) <= 90u )
			goto st1;
	} else if ( (*p) > 95u ) {
		if ( 124u <= (*p) && (*p) <= 125u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr245:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st197;
st197:
	if ( ++p == pe )
		goto _test_eof197;
case 197:
#line 5303 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 18u: goto st1;
		case 22u: goto st1;
		case 111u: goto st1;
		case 112u: goto st33;
		case 194u: goto st33;
		case 230u: goto st1;
	}
	if ( (*p) < 81u ) {
		if ( 16u <= (*p) && (*p) <= 17u )
			goto st29;
	} else if ( (*p) > 83u ) {
		if ( (*p) > 95u ) {
			if ( 126u <= (*p) && (*p) <= 127u )
				goto st1;
		} else if ( (*p) >= 88u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr246:
#line 165 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    /* VEX.R is not used in ia32 mode.  */
    SET_VEX_PREFIX3(p[0] & 0x7f);
  }
	goto st198;
st198:
	if ( ++p == pe )
		goto _test_eof198;
case 198:
#line 5335 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 18u: goto st1;
		case 81u: goto st1;
		case 112u: goto st33;
		case 194u: goto st33;
		case 208u: goto st1;
		case 230u: goto st1;
		case 240u: goto st29;
	}
	if ( (*p) < 88u ) {
		if ( 16u <= (*p) && (*p) <= 17u )
			goto st29;
	} else if ( (*p) > 90u ) {
		if ( (*p) > 95u ) {
			if ( 124u <= (*p) && (*p) <= 125u )
				goto st1;
		} else if ( (*p) >= 92u )
			goto st1;
	} else
		goto st1;
	goto tr19;
tr298:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st199;
st199:
	if ( ++p == pe )
		goto _test_eof199;
case 199:
#line 5368 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st34;
		case 5u: goto st35;
		case 68u: goto st40;
		case 132u: goto st41;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 7u )
			goto st10;
	} else if ( (*p) > 71u ) {
		if ( (*p) > 135u ) {
			if ( 192u <= (*p) && (*p) <= 199u )
				goto st10;
		} else if ( (*p) >= 128u )
			goto st35;
	} else
		goto st39;
	goto tr19;
tr299:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st200;
st200:
	if ( ++p == pe )
		goto _test_eof200;
case 200:
#line 5398 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st110;
		case 5u: goto st111;
		case 68u: goto st116;
		case 132u: goto st117;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 7u )
			goto st11;
	} else if ( (*p) > 71u ) {
		if ( (*p) > 135u ) {
			if ( 192u <= (*p) && (*p) <= 199u )
				goto st11;
		} else if ( (*p) >= 128u )
			goto st111;
	} else
		goto st115;
	goto tr19;
tr300:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st201;
st201:
	if ( ++p == pe )
		goto _test_eof201;
case 201:
#line 5428 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	goto st202;
st202:
	if ( ++p == pe )
		goto _test_eof202;
case 202:
	goto tr252;
tr303:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st203;
st203:
	if ( ++p == pe )
		goto _test_eof203;
case 203:
#line 5446 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
		case 239u: goto tr19;
	}
	if ( (*p) < 128u ) {
		if ( (*p) < 64u ) {
			if ( 8u <= (*p) && (*p) <= 15u )
				goto tr19;
		} else if ( (*p) > 71u ) {
			if ( (*p) > 79u ) {
				if ( 80u <= (*p) && (*p) <= 127u )
					goto st7;
			} else if ( (*p) >= 72u )
				goto tr19;
		} else
			goto st7;
	} else if ( (*p) > 135u ) {
		if ( (*p) < 209u ) {
			if ( (*p) > 143u ) {
				if ( 144u <= (*p) && (*p) <= 191u )
					goto st3;
			} else if ( (*p) >= 136u )
				goto tr19;
		} else if ( (*p) > 223u ) {
			if ( (*p) > 227u ) {
				if ( 230u <= (*p) && (*p) <= 231u )
					goto tr19;
			} else if ( (*p) >= 226u )
				goto tr19;
		} else
			goto tr19;
	} else
		goto st3;
	goto tr0;
tr304:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st204;
st204:
	if ( ++p == pe )
		goto _test_eof204;
case 204:
#line 5519 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 128u ) {
		if ( 64u <= (*p) && (*p) <= 127u )
			goto st7;
	} else if ( (*p) > 191u ) {
		if ( (*p) > 232u ) {
			if ( 234u <= (*p) )
				goto tr19;
		} else if ( (*p) >= 224u )
			goto tr19;
	} else
		goto st3;
	goto tr0;
tr305:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st205;
st205:
	if ( ++p == pe )
		goto _test_eof205;
case 205:
#line 5577 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 108u: goto st8;
		case 124u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 172u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 128u ) {
		if ( (*p) < 96u ) {
			if ( (*p) < 48u ) {
				if ( 32u <= (*p) && (*p) <= 39u )
					goto tr19;
			} else if ( (*p) > 55u ) {
				if ( 64u <= (*p) && (*p) <= 95u )
					goto st7;
			} else
				goto tr19;
		} else if ( (*p) > 103u ) {
			if ( (*p) < 112u ) {
				if ( 104u <= (*p) && (*p) <= 111u )
					goto st7;
			} else if ( (*p) > 119u ) {
				if ( 120u <= (*p) && (*p) <= 127u )
					goto st7;
			} else
				goto tr19;
		} else
			goto tr19;
	} else if ( (*p) > 159u ) {
		if ( (*p) < 184u ) {
			if ( (*p) < 168u ) {
				if ( 160u <= (*p) && (*p) <= 167u )
					goto tr19;
			} else if ( (*p) > 175u ) {
				if ( 176u <= (*p) && (*p) <= 183u )
					goto tr19;
			} else
				goto st3;
		} else if ( (*p) > 191u ) {
			if ( (*p) < 228u ) {
				if ( 224u <= (*p) && (*p) <= 225u )
					goto tr19;
			} else if ( (*p) > 231u ) {
				if ( 248u <= (*p) )
					goto tr19;
			} else
				goto tr19;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
tr306:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st206;
st206:
	if ( ++p == pe )
		goto _test_eof206;
case 206:
#line 5660 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 128u ) {
		if ( 64u <= (*p) && (*p) <= 127u )
			goto st7;
	} else if ( (*p) > 191u ) {
		if ( 208u <= (*p) && (*p) <= 223u )
			goto tr19;
	} else
		goto st3;
	goto tr0;
tr307:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st207;
st207:
	if ( ++p == pe )
		goto _test_eof207;
case 207:
#line 5715 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 128u ) {
		if ( (*p) < 64u ) {
			if ( 40u <= (*p) && (*p) <= 47u )
				goto tr19;
		} else if ( (*p) > 103u ) {
			if ( (*p) > 111u ) {
				if ( 112u <= (*p) && (*p) <= 127u )
					goto st7;
			} else if ( (*p) >= 104u )
				goto tr19;
		} else
			goto st7;
	} else if ( (*p) > 167u ) {
		if ( (*p) < 176u ) {
			if ( 168u <= (*p) && (*p) <= 175u )
				goto tr19;
		} else if ( (*p) > 191u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) )
					goto tr19;
			} else if ( (*p) >= 200u )
				goto tr19;
		} else
			goto st3;
	} else
		goto st3;
	goto tr0;
tr308:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st208;
st208:
	if ( ++p == pe )
		goto _test_eof208;
case 208:
#line 5784 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 128u ) {
		if ( 64u <= (*p) && (*p) <= 127u )
			goto st7;
	} else if ( (*p) > 191u ) {
		if ( (*p) > 216u ) {
			if ( 218u <= (*p) && (*p) <= 223u )
				goto tr19;
		} else if ( (*p) >= 208u )
			goto tr19;
	} else
		goto st3;
	goto tr0;
tr309:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st209;
st209:
	if ( ++p == pe )
		goto _test_eof209;
case 209:
#line 5842 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 192u ) {
		if ( (*p) > 127u ) {
			if ( 128u <= (*p) && (*p) <= 191u )
				goto st3;
		} else if ( (*p) >= 64u )
			goto st7;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 231u ) {
			if ( 248u <= (*p) )
				goto tr19;
		} else if ( (*p) >= 225u )
			goto tr19;
	} else
		goto tr19;
	goto tr0;
tr311:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
#line 13 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_LOCK_PREFIX(TRUE);
  }
	goto st210;
st210:
	if ( ++p == pe )
		goto _test_eof210;
case 210:
#line 5907 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 15u: goto st211;
		case 102u: goto tr254;
		case 128u: goto st100;
		case 129u: goto st212;
		case 131u: goto st100;
	}
	if ( (*p) < 32u ) {
		if ( (*p) < 8u ) {
			if ( (*p) <= 3u )
				goto st29;
		} else if ( (*p) > 11u ) {
			if ( (*p) > 19u ) {
				if ( 24u <= (*p) && (*p) <= 27u )
					goto st29;
			} else if ( (*p) >= 16u )
				goto st29;
		} else
			goto st29;
	} else if ( (*p) > 35u ) {
		if ( (*p) < 134u ) {
			if ( (*p) > 43u ) {
				if ( 48u <= (*p) && (*p) <= 51u )
					goto st29;
			} else if ( (*p) >= 40u )
				goto st29;
		} else if ( (*p) > 135u ) {
			if ( (*p) > 247u ) {
				if ( 254u <= (*p) )
					goto st18;
			} else if ( (*p) >= 246u )
				goto st101;
		} else
			goto st29;
	} else
		goto st29;
	goto tr19;
st211:
	if ( ++p == pe )
		goto _test_eof211;
case 211:
	if ( (*p) == 199u )
		goto st50;
	if ( (*p) > 177u ) {
		if ( 192u <= (*p) && (*p) <= 193u )
			goto st29;
	} else if ( (*p) >= 176u )
		goto st29;
	goto tr19;
st212:
	if ( ++p == pe )
		goto _test_eof212;
case 212:
	switch( (*p) ) {
		case 4u: goto st110;
		case 5u: goto st111;
		case 12u: goto st110;
		case 13u: goto st111;
		case 20u: goto st110;
		case 21u: goto st111;
		case 28u: goto st110;
		case 29u: goto st111;
		case 36u: goto st110;
		case 37u: goto st111;
		case 44u: goto st110;
		case 45u: goto st111;
		case 52u: goto st110;
		case 53u: goto st111;
		case 68u: goto st116;
		case 76u: goto st116;
		case 84u: goto st116;
		case 92u: goto st116;
		case 100u: goto st116;
		case 108u: goto st116;
		case 116u: goto st116;
		case 132u: goto st117;
		case 140u: goto st117;
		case 148u: goto st117;
		case 156u: goto st117;
		case 164u: goto st117;
		case 172u: goto st117;
		case 180u: goto st117;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 55u )
			goto st11;
	} else if ( (*p) > 119u ) {
		if ( 128u <= (*p) && (*p) <= 183u )
			goto st111;
	} else
		goto st115;
	goto tr19;
tr312:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
#line 22 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPNZ_PREFIX(TRUE);
  }
	goto st213;
st213:
	if ( ++p == pe )
		goto _test_eof213;
case 213:
#line 6015 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 15u: goto st214;
		case 102u: goto tr257;
	}
	if ( (*p) > 167u ) {
		if ( 174u <= (*p) && (*p) <= 175u )
			goto tr0;
	} else if ( (*p) >= 166u )
		goto tr0;
	goto tr19;
st214:
	if ( ++p == pe )
		goto _test_eof214;
case 214:
	switch( (*p) ) {
		case 43u: goto tr258;
		case 56u: goto st215;
		case 81u: goto tr134;
		case 112u: goto tr260;
		case 120u: goto tr261;
		case 121u: goto tr262;
		case 194u: goto tr260;
		case 208u: goto tr134;
		case 214u: goto tr262;
		case 230u: goto tr134;
		case 240u: goto tr258;
	}
	if ( (*p) < 88u ) {
		if ( (*p) > 18u ) {
			if ( 42u <= (*p) && (*p) <= 45u )
				goto tr134;
		} else if ( (*p) >= 16u )
			goto tr134;
	} else if ( (*p) > 90u ) {
		if ( (*p) > 95u ) {
			if ( 124u <= (*p) && (*p) <= 125u )
				goto tr134;
		} else if ( (*p) >= 92u )
			goto tr134;
	} else
		goto tr134;
	goto tr19;
st215:
	if ( ++p == pe )
		goto _test_eof215;
case 215:
	if ( 240u <= (*p) && (*p) <= 241u )
		goto tr134;
	goto tr19;
tr261:
#line 36 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPNZ_PREFIX(FALSE);
  }
	goto st216;
st216:
	if ( ++p == pe )
		goto _test_eof216;
case 216:
#line 6075 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	if ( 192u <= (*p) )
		goto st217;
	goto tr19;
st217:
	if ( ++p == pe )
		goto _test_eof217;
case 217:
	goto tr264;
tr313:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
#line 19 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPZ_PREFIX(TRUE);
  }
#line 16 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
    SET_REPZ_PREFIX(TRUE);
  }
	goto st218;
st218:
	if ( ++p == pe )
		goto _test_eof218;
case 218:
#line 6103 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 15u: goto st219;
		case 102u: goto tr266;
		case 144u: goto tr267;
	}
	if ( (*p) > 167u ) {
		if ( 170u <= (*p) && (*p) <= 175u )
			goto tr0;
	} else if ( (*p) >= 164u )
		goto tr0;
	goto tr19;
st219:
	if ( ++p == pe )
		goto _test_eof219;
case 219:
	switch( (*p) ) {
		case 22u: goto tr136;
		case 43u: goto tr268;
		case 111u: goto tr136;
		case 112u: goto tr269;
		case 184u: goto tr136;
		case 194u: goto tr269;
		case 214u: goto tr270;
		case 230u: goto tr136;
	}
	if ( (*p) < 81u ) {
		if ( (*p) > 18u ) {
			if ( 42u <= (*p) && (*p) <= 45u )
				goto tr136;
		} else if ( (*p) >= 16u )
			goto tr136;
	} else if ( (*p) > 83u ) {
		if ( (*p) < 126u ) {
			if ( 88u <= (*p) && (*p) <= 95u )
				goto tr136;
		} else if ( (*p) > 127u ) {
			if ( 188u <= (*p) && (*p) <= 189u )
				goto tr136;
		} else
			goto tr136;
	} else
		goto tr136;
	goto tr19;
tr314:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st220;
st220:
	if ( ++p == pe )
		goto _test_eof220;
case 220:
#line 6158 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st34;
		case 5u: goto st35;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st40;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st41;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 80u ) {
		if ( (*p) < 8u ) {
			if ( (*p) <= 7u )
				goto st10;
		} else if ( (*p) > 15u ) {
			if ( (*p) > 71u ) {
				if ( 72u <= (*p) && (*p) <= 79u )
					goto tr19;
			} else if ( (*p) >= 64u )
				goto st39;
		} else
			goto tr19;
	} else if ( (*p) > 127u ) {
		if ( (*p) < 144u ) {
			if ( (*p) > 135u ) {
				if ( 136u <= (*p) && (*p) <= 143u )
					goto tr19;
			} else if ( (*p) >= 128u )
				goto st35;
		} else if ( (*p) > 191u ) {
			if ( (*p) > 199u ) {
				if ( 200u <= (*p) && (*p) <= 207u )
					goto tr19;
			} else if ( (*p) >= 192u )
				goto st10;
		} else
			goto st3;
	} else
		goto st7;
	goto tr0;
tr315:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st221;
st221:
	if ( ++p == pe )
		goto _test_eof221;
case 221:
#line 6230 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st110;
		case 5u: goto st111;
		case 20u: goto st2;
		case 21u: goto st3;
		case 28u: goto st2;
		case 29u: goto st3;
		case 36u: goto st2;
		case 37u: goto st3;
		case 44u: goto st2;
		case 45u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 60u: goto st2;
		case 61u: goto st3;
		case 68u: goto st116;
		case 84u: goto st8;
		case 92u: goto st8;
		case 100u: goto st8;
		case 108u: goto st8;
		case 116u: goto st8;
		case 124u: goto st8;
		case 132u: goto st117;
		case 148u: goto st9;
		case 156u: goto st9;
		case 164u: goto st9;
		case 172u: goto st9;
		case 180u: goto st9;
		case 188u: goto st9;
	}
	if ( (*p) < 80u ) {
		if ( (*p) < 8u ) {
			if ( (*p) <= 7u )
				goto st11;
		} else if ( (*p) > 15u ) {
			if ( (*p) > 71u ) {
				if ( 72u <= (*p) && (*p) <= 79u )
					goto tr19;
			} else if ( (*p) >= 64u )
				goto st115;
		} else
			goto tr19;
	} else if ( (*p) > 127u ) {
		if ( (*p) < 144u ) {
			if ( (*p) > 135u ) {
				if ( 136u <= (*p) && (*p) <= 143u )
					goto tr19;
			} else if ( (*p) >= 128u )
				goto st111;
		} else if ( (*p) > 191u ) {
			if ( (*p) > 199u ) {
				if ( 200u <= (*p) && (*p) <= 207u )
					goto tr19;
			} else if ( (*p) >= 192u )
				goto st11;
		} else
			goto st3;
	} else
		goto st7;
	goto tr0;
tr316:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st222;
st222:
	if ( ++p == pe )
		goto _test_eof222;
case 222:
#line 6302 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
	}
	if ( (*p) < 64u ) {
		if ( (*p) <= 15u )
			goto tr0;
	} else if ( (*p) > 79u ) {
		if ( (*p) > 143u ) {
			if ( 192u <= (*p) && (*p) <= 207u )
				goto tr0;
		} else if ( (*p) >= 128u )
			goto st3;
	} else
		goto st7;
	goto tr19;
tr318:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st223;
st223:
	if ( ++p == pe )
		goto _test_eof223;
case 223:
#line 6336 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 116u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 180u: goto st9;
		case 208u: goto tr271;
		case 224u: goto tr271;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 64u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 192u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
st224:
	if ( ++p == pe )
		goto _test_eof224;
case 224:
	if ( (*p) == 224u )
		goto tr272;
	goto tr11;
tr272:
#line 53 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st241;
st241:
	if ( ++p == pe )
		goto _test_eof241;
case 241:
#line 6399 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto tr280;
		case 5u: goto tr281;
		case 12u: goto tr280;
		case 13u: goto tr281;
		case 14u: goto tr19;
		case 15u: goto tr282;
		case 20u: goto tr280;
		case 21u: goto tr281;
		case 28u: goto tr280;
		case 29u: goto tr281;
		case 36u: goto tr280;
		case 37u: goto tr281;
		case 44u: goto tr280;
		case 45u: goto tr281;
		case 46u: goto tr283;
		case 47u: goto tr19;
		case 52u: goto tr280;
		case 53u: goto tr281;
		case 60u: goto tr280;
		case 61u: goto tr281;
		case 62u: goto tr284;
		case 63u: goto tr19;
		case 101u: goto tr286;
		case 102u: goto tr287;
		case 104u: goto tr281;
		case 105u: goto tr288;
		case 106u: goto tr280;
		case 107u: goto tr289;
		case 128u: goto tr289;
		case 129u: goto tr288;
		case 130u: goto tr19;
		case 131u: goto tr291;
		case 141u: goto tr292;
		case 143u: goto tr293;
		case 154u: goto tr19;
		case 168u: goto tr280;
		case 169u: goto tr281;
		case 196u: goto tr296;
		case 197u: goto tr297;
		case 198u: goto tr298;
		case 199u: goto tr299;
		case 200u: goto tr300;
		case 202u: goto tr301;
		case 216u: goto tr279;
		case 217u: goto tr303;
		case 218u: goto tr304;
		case 219u: goto tr305;
		case 220u: goto tr306;
		case 221u: goto tr307;
		case 222u: goto tr308;
		case 223u: goto tr309;
		case 235u: goto tr290;
		case 240u: goto tr311;
		case 242u: goto tr312;
		case 243u: goto tr313;
		case 246u: goto tr314;
		case 247u: goto tr315;
		case 254u: goto tr316;
		case 255u: goto tr319;
	}
	if ( (*p) < 132u ) {
		if ( (*p) < 32u ) {
			if ( (*p) < 8u ) {
				if ( (*p) > 3u ) {
					if ( 6u <= (*p) && (*p) <= 7u )
						goto tr19;
				} else
					goto tr279;
			} else if ( (*p) > 19u ) {
				if ( (*p) < 24u ) {
					if ( 22u <= (*p) && (*p) <= 23u )
						goto tr19;
				} else if ( (*p) > 27u ) {
					if ( 30u <= (*p) && (*p) <= 31u )
						goto tr19;
				} else
					goto tr279;
			} else
				goto tr279;
		} else if ( (*p) > 35u ) {
			if ( (*p) < 54u ) {
				if ( (*p) > 39u ) {
					if ( 40u <= (*p) && (*p) <= 51u )
						goto tr279;
				} else if ( (*p) >= 38u )
					goto tr19;
			} else if ( (*p) > 55u ) {
				if ( (*p) < 96u ) {
					if ( 56u <= (*p) && (*p) <= 59u )
						goto tr279;
				} else if ( (*p) > 111u ) {
					if ( 112u <= (*p) && (*p) <= 127u )
						goto tr290;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr279;
	} else if ( (*p) > 139u ) {
		if ( (*p) < 194u ) {
			if ( (*p) < 160u ) {
				if ( (*p) > 142u ) {
					if ( 156u <= (*p) && (*p) <= 157u )
						goto tr19;
				} else if ( (*p) >= 140u )
					goto tr19;
			} else if ( (*p) > 163u ) {
				if ( (*p) < 184u ) {
					if ( 176u <= (*p) && (*p) <= 183u )
						goto tr280;
				} else if ( (*p) > 191u ) {
					if ( 192u <= (*p) && (*p) <= 193u )
						goto tr295;
				} else
					goto tr281;
			} else
				goto tr294;
		} else if ( (*p) > 195u ) {
			if ( (*p) < 212u ) {
				if ( (*p) > 207u ) {
					if ( 208u <= (*p) && (*p) <= 211u )
						goto tr302;
				} else if ( (*p) >= 204u )
					goto tr19;
			} else if ( (*p) > 231u ) {
				if ( (*p) < 234u ) {
					if ( 232u <= (*p) && (*p) <= 233u )
						goto tr310;
				} else if ( (*p) > 241u ) {
					if ( 250u <= (*p) && (*p) <= 251u )
						goto tr19;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr19;
	} else
		goto tr279;
	goto tr285;
tr319:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st225;
st225:
	if ( ++p == pe )
		goto _test_eof225;
case 225:
#line 6553 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 116u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 180u: goto st9;
		case 209u: goto tr271;
		case 225u: goto tr271;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 64u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 192u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
st226:
	if ( ++p == pe )
		goto _test_eof226;
case 226:
	if ( (*p) == 224u )
		goto tr273;
	goto tr11;
tr273:
#line 53 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st242;
st242:
	if ( ++p == pe )
		goto _test_eof242;
case 242:
#line 6616 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto tr280;
		case 5u: goto tr281;
		case 12u: goto tr280;
		case 13u: goto tr281;
		case 14u: goto tr19;
		case 15u: goto tr282;
		case 20u: goto tr280;
		case 21u: goto tr281;
		case 28u: goto tr280;
		case 29u: goto tr281;
		case 36u: goto tr280;
		case 37u: goto tr281;
		case 44u: goto tr280;
		case 45u: goto tr281;
		case 46u: goto tr283;
		case 47u: goto tr19;
		case 52u: goto tr280;
		case 53u: goto tr281;
		case 60u: goto tr280;
		case 61u: goto tr281;
		case 62u: goto tr284;
		case 63u: goto tr19;
		case 101u: goto tr286;
		case 102u: goto tr287;
		case 104u: goto tr281;
		case 105u: goto tr288;
		case 106u: goto tr280;
		case 107u: goto tr289;
		case 128u: goto tr289;
		case 129u: goto tr288;
		case 130u: goto tr19;
		case 131u: goto tr291;
		case 141u: goto tr292;
		case 143u: goto tr293;
		case 154u: goto tr19;
		case 168u: goto tr280;
		case 169u: goto tr281;
		case 196u: goto tr296;
		case 197u: goto tr297;
		case 198u: goto tr298;
		case 199u: goto tr299;
		case 200u: goto tr300;
		case 202u: goto tr301;
		case 216u: goto tr279;
		case 217u: goto tr303;
		case 218u: goto tr304;
		case 219u: goto tr305;
		case 220u: goto tr306;
		case 221u: goto tr307;
		case 222u: goto tr308;
		case 223u: goto tr309;
		case 235u: goto tr290;
		case 240u: goto tr311;
		case 242u: goto tr312;
		case 243u: goto tr313;
		case 246u: goto tr314;
		case 247u: goto tr315;
		case 254u: goto tr316;
		case 255u: goto tr320;
	}
	if ( (*p) < 132u ) {
		if ( (*p) < 32u ) {
			if ( (*p) < 8u ) {
				if ( (*p) > 3u ) {
					if ( 6u <= (*p) && (*p) <= 7u )
						goto tr19;
				} else
					goto tr279;
			} else if ( (*p) > 19u ) {
				if ( (*p) < 24u ) {
					if ( 22u <= (*p) && (*p) <= 23u )
						goto tr19;
				} else if ( (*p) > 27u ) {
					if ( 30u <= (*p) && (*p) <= 31u )
						goto tr19;
				} else
					goto tr279;
			} else
				goto tr279;
		} else if ( (*p) > 35u ) {
			if ( (*p) < 54u ) {
				if ( (*p) > 39u ) {
					if ( 40u <= (*p) && (*p) <= 51u )
						goto tr279;
				} else if ( (*p) >= 38u )
					goto tr19;
			} else if ( (*p) > 55u ) {
				if ( (*p) < 96u ) {
					if ( 56u <= (*p) && (*p) <= 59u )
						goto tr279;
				} else if ( (*p) > 111u ) {
					if ( 112u <= (*p) && (*p) <= 127u )
						goto tr290;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr279;
	} else if ( (*p) > 139u ) {
		if ( (*p) < 194u ) {
			if ( (*p) < 160u ) {
				if ( (*p) > 142u ) {
					if ( 156u <= (*p) && (*p) <= 157u )
						goto tr19;
				} else if ( (*p) >= 140u )
					goto tr19;
			} else if ( (*p) > 163u ) {
				if ( (*p) < 184u ) {
					if ( 176u <= (*p) && (*p) <= 183u )
						goto tr280;
				} else if ( (*p) > 191u ) {
					if ( 192u <= (*p) && (*p) <= 193u )
						goto tr295;
				} else
					goto tr281;
			} else
				goto tr294;
		} else if ( (*p) > 195u ) {
			if ( (*p) < 212u ) {
				if ( (*p) > 207u ) {
					if ( 208u <= (*p) && (*p) <= 211u )
						goto tr302;
				} else if ( (*p) >= 204u )
					goto tr19;
			} else if ( (*p) > 231u ) {
				if ( (*p) < 234u ) {
					if ( 232u <= (*p) && (*p) <= 233u )
						goto tr310;
				} else if ( (*p) > 241u ) {
					if ( 250u <= (*p) && (*p) <= 251u )
						goto tr19;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr19;
	} else
		goto tr279;
	goto tr285;
tr320:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st227;
st227:
	if ( ++p == pe )
		goto _test_eof227;
case 227:
#line 6770 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 116u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 180u: goto st9;
		case 210u: goto tr271;
		case 226u: goto tr271;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 64u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 192u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
st228:
	if ( ++p == pe )
		goto _test_eof228;
case 228:
	if ( (*p) == 224u )
		goto tr274;
	goto tr11;
tr274:
#line 53 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st243;
st243:
	if ( ++p == pe )
		goto _test_eof243;
case 243:
#line 6833 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto tr280;
		case 5u: goto tr281;
		case 12u: goto tr280;
		case 13u: goto tr281;
		case 14u: goto tr19;
		case 15u: goto tr282;
		case 20u: goto tr280;
		case 21u: goto tr281;
		case 28u: goto tr280;
		case 29u: goto tr281;
		case 36u: goto tr280;
		case 37u: goto tr281;
		case 44u: goto tr280;
		case 45u: goto tr281;
		case 46u: goto tr283;
		case 47u: goto tr19;
		case 52u: goto tr280;
		case 53u: goto tr281;
		case 60u: goto tr280;
		case 61u: goto tr281;
		case 62u: goto tr284;
		case 63u: goto tr19;
		case 101u: goto tr286;
		case 102u: goto tr287;
		case 104u: goto tr281;
		case 105u: goto tr288;
		case 106u: goto tr280;
		case 107u: goto tr289;
		case 128u: goto tr289;
		case 129u: goto tr288;
		case 130u: goto tr19;
		case 131u: goto tr291;
		case 141u: goto tr292;
		case 143u: goto tr293;
		case 154u: goto tr19;
		case 168u: goto tr280;
		case 169u: goto tr281;
		case 196u: goto tr296;
		case 197u: goto tr297;
		case 198u: goto tr298;
		case 199u: goto tr299;
		case 200u: goto tr300;
		case 202u: goto tr301;
		case 216u: goto tr279;
		case 217u: goto tr303;
		case 218u: goto tr304;
		case 219u: goto tr305;
		case 220u: goto tr306;
		case 221u: goto tr307;
		case 222u: goto tr308;
		case 223u: goto tr309;
		case 235u: goto tr290;
		case 240u: goto tr311;
		case 242u: goto tr312;
		case 243u: goto tr313;
		case 246u: goto tr314;
		case 247u: goto tr315;
		case 254u: goto tr316;
		case 255u: goto tr321;
	}
	if ( (*p) < 132u ) {
		if ( (*p) < 32u ) {
			if ( (*p) < 8u ) {
				if ( (*p) > 3u ) {
					if ( 6u <= (*p) && (*p) <= 7u )
						goto tr19;
				} else
					goto tr279;
			} else if ( (*p) > 19u ) {
				if ( (*p) < 24u ) {
					if ( 22u <= (*p) && (*p) <= 23u )
						goto tr19;
				} else if ( (*p) > 27u ) {
					if ( 30u <= (*p) && (*p) <= 31u )
						goto tr19;
				} else
					goto tr279;
			} else
				goto tr279;
		} else if ( (*p) > 35u ) {
			if ( (*p) < 54u ) {
				if ( (*p) > 39u ) {
					if ( 40u <= (*p) && (*p) <= 51u )
						goto tr279;
				} else if ( (*p) >= 38u )
					goto tr19;
			} else if ( (*p) > 55u ) {
				if ( (*p) < 96u ) {
					if ( 56u <= (*p) && (*p) <= 59u )
						goto tr279;
				} else if ( (*p) > 111u ) {
					if ( 112u <= (*p) && (*p) <= 127u )
						goto tr290;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr279;
	} else if ( (*p) > 139u ) {
		if ( (*p) < 194u ) {
			if ( (*p) < 160u ) {
				if ( (*p) > 142u ) {
					if ( 156u <= (*p) && (*p) <= 157u )
						goto tr19;
				} else if ( (*p) >= 140u )
					goto tr19;
			} else if ( (*p) > 163u ) {
				if ( (*p) < 184u ) {
					if ( 176u <= (*p) && (*p) <= 183u )
						goto tr280;
				} else if ( (*p) > 191u ) {
					if ( 192u <= (*p) && (*p) <= 193u )
						goto tr295;
				} else
					goto tr281;
			} else
				goto tr294;
		} else if ( (*p) > 195u ) {
			if ( (*p) < 212u ) {
				if ( (*p) > 207u ) {
					if ( 208u <= (*p) && (*p) <= 211u )
						goto tr302;
				} else if ( (*p) >= 204u )
					goto tr19;
			} else if ( (*p) > 231u ) {
				if ( (*p) < 234u ) {
					if ( 232u <= (*p) && (*p) <= 233u )
						goto tr310;
				} else if ( (*p) > 241u ) {
					if ( 250u <= (*p) && (*p) <= 251u )
						goto tr19;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr19;
	} else
		goto tr279;
	goto tr285;
tr321:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st229;
st229:
	if ( ++p == pe )
		goto _test_eof229;
case 229:
#line 6987 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 116u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 180u: goto st9;
		case 211u: goto tr271;
		case 227u: goto tr271;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 64u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 192u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
st230:
	if ( ++p == pe )
		goto _test_eof230;
case 230:
	if ( (*p) == 224u )
		goto tr275;
	goto tr11;
tr275:
#line 53 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st244;
st244:
	if ( ++p == pe )
		goto _test_eof244;
case 244:
#line 7050 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto tr280;
		case 5u: goto tr281;
		case 12u: goto tr280;
		case 13u: goto tr281;
		case 14u: goto tr19;
		case 15u: goto tr282;
		case 20u: goto tr280;
		case 21u: goto tr281;
		case 28u: goto tr280;
		case 29u: goto tr281;
		case 36u: goto tr280;
		case 37u: goto tr281;
		case 44u: goto tr280;
		case 45u: goto tr281;
		case 46u: goto tr283;
		case 47u: goto tr19;
		case 52u: goto tr280;
		case 53u: goto tr281;
		case 60u: goto tr280;
		case 61u: goto tr281;
		case 62u: goto tr284;
		case 63u: goto tr19;
		case 101u: goto tr286;
		case 102u: goto tr287;
		case 104u: goto tr281;
		case 105u: goto tr288;
		case 106u: goto tr280;
		case 107u: goto tr289;
		case 128u: goto tr289;
		case 129u: goto tr288;
		case 130u: goto tr19;
		case 131u: goto tr291;
		case 141u: goto tr292;
		case 143u: goto tr293;
		case 154u: goto tr19;
		case 168u: goto tr280;
		case 169u: goto tr281;
		case 196u: goto tr296;
		case 197u: goto tr297;
		case 198u: goto tr298;
		case 199u: goto tr299;
		case 200u: goto tr300;
		case 202u: goto tr301;
		case 216u: goto tr279;
		case 217u: goto tr303;
		case 218u: goto tr304;
		case 219u: goto tr305;
		case 220u: goto tr306;
		case 221u: goto tr307;
		case 222u: goto tr308;
		case 223u: goto tr309;
		case 235u: goto tr290;
		case 240u: goto tr311;
		case 242u: goto tr312;
		case 243u: goto tr313;
		case 246u: goto tr314;
		case 247u: goto tr315;
		case 254u: goto tr316;
		case 255u: goto tr322;
	}
	if ( (*p) < 132u ) {
		if ( (*p) < 32u ) {
			if ( (*p) < 8u ) {
				if ( (*p) > 3u ) {
					if ( 6u <= (*p) && (*p) <= 7u )
						goto tr19;
				} else
					goto tr279;
			} else if ( (*p) > 19u ) {
				if ( (*p) < 24u ) {
					if ( 22u <= (*p) && (*p) <= 23u )
						goto tr19;
				} else if ( (*p) > 27u ) {
					if ( 30u <= (*p) && (*p) <= 31u )
						goto tr19;
				} else
					goto tr279;
			} else
				goto tr279;
		} else if ( (*p) > 35u ) {
			if ( (*p) < 54u ) {
				if ( (*p) > 39u ) {
					if ( 40u <= (*p) && (*p) <= 51u )
						goto tr279;
				} else if ( (*p) >= 38u )
					goto tr19;
			} else if ( (*p) > 55u ) {
				if ( (*p) < 96u ) {
					if ( 56u <= (*p) && (*p) <= 59u )
						goto tr279;
				} else if ( (*p) > 111u ) {
					if ( 112u <= (*p) && (*p) <= 127u )
						goto tr290;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr279;
	} else if ( (*p) > 139u ) {
		if ( (*p) < 194u ) {
			if ( (*p) < 160u ) {
				if ( (*p) > 142u ) {
					if ( 156u <= (*p) && (*p) <= 157u )
						goto tr19;
				} else if ( (*p) >= 140u )
					goto tr19;
			} else if ( (*p) > 163u ) {
				if ( (*p) < 184u ) {
					if ( 176u <= (*p) && (*p) <= 183u )
						goto tr280;
				} else if ( (*p) > 191u ) {
					if ( 192u <= (*p) && (*p) <= 193u )
						goto tr295;
				} else
					goto tr281;
			} else
				goto tr294;
		} else if ( (*p) > 195u ) {
			if ( (*p) < 212u ) {
				if ( (*p) > 207u ) {
					if ( 208u <= (*p) && (*p) <= 211u )
						goto tr302;
				} else if ( (*p) >= 204u )
					goto tr19;
			} else if ( (*p) > 231u ) {
				if ( (*p) < 234u ) {
					if ( 232u <= (*p) && (*p) <= 233u )
						goto tr310;
				} else if ( (*p) > 241u ) {
					if ( 250u <= (*p) && (*p) <= 251u )
						goto tr19;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr19;
	} else
		goto tr279;
	goto tr285;
tr322:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st231;
st231:
	if ( ++p == pe )
		goto _test_eof231;
case 231:
#line 7204 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 116u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 180u: goto st9;
		case 212u: goto tr271;
		case 228u: goto tr271;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 64u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 192u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
st232:
	if ( ++p == pe )
		goto _test_eof232;
case 232:
	if ( (*p) == 224u )
		goto tr276;
	goto tr11;
tr276:
#line 53 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st245;
st245:
	if ( ++p == pe )
		goto _test_eof245;
case 245:
#line 7267 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto tr280;
		case 5u: goto tr281;
		case 12u: goto tr280;
		case 13u: goto tr281;
		case 14u: goto tr19;
		case 15u: goto tr282;
		case 20u: goto tr280;
		case 21u: goto tr281;
		case 28u: goto tr280;
		case 29u: goto tr281;
		case 36u: goto tr280;
		case 37u: goto tr281;
		case 44u: goto tr280;
		case 45u: goto tr281;
		case 46u: goto tr283;
		case 47u: goto tr19;
		case 52u: goto tr280;
		case 53u: goto tr281;
		case 60u: goto tr280;
		case 61u: goto tr281;
		case 62u: goto tr284;
		case 63u: goto tr19;
		case 101u: goto tr286;
		case 102u: goto tr287;
		case 104u: goto tr281;
		case 105u: goto tr288;
		case 106u: goto tr280;
		case 107u: goto tr289;
		case 128u: goto tr289;
		case 129u: goto tr288;
		case 130u: goto tr19;
		case 131u: goto tr291;
		case 141u: goto tr292;
		case 143u: goto tr293;
		case 154u: goto tr19;
		case 168u: goto tr280;
		case 169u: goto tr281;
		case 196u: goto tr296;
		case 197u: goto tr297;
		case 198u: goto tr298;
		case 199u: goto tr299;
		case 200u: goto tr300;
		case 202u: goto tr301;
		case 216u: goto tr279;
		case 217u: goto tr303;
		case 218u: goto tr304;
		case 219u: goto tr305;
		case 220u: goto tr306;
		case 221u: goto tr307;
		case 222u: goto tr308;
		case 223u: goto tr309;
		case 235u: goto tr290;
		case 240u: goto tr311;
		case 242u: goto tr312;
		case 243u: goto tr313;
		case 246u: goto tr314;
		case 247u: goto tr315;
		case 254u: goto tr316;
		case 255u: goto tr323;
	}
	if ( (*p) < 132u ) {
		if ( (*p) < 32u ) {
			if ( (*p) < 8u ) {
				if ( (*p) > 3u ) {
					if ( 6u <= (*p) && (*p) <= 7u )
						goto tr19;
				} else
					goto tr279;
			} else if ( (*p) > 19u ) {
				if ( (*p) < 24u ) {
					if ( 22u <= (*p) && (*p) <= 23u )
						goto tr19;
				} else if ( (*p) > 27u ) {
					if ( 30u <= (*p) && (*p) <= 31u )
						goto tr19;
				} else
					goto tr279;
			} else
				goto tr279;
		} else if ( (*p) > 35u ) {
			if ( (*p) < 54u ) {
				if ( (*p) > 39u ) {
					if ( 40u <= (*p) && (*p) <= 51u )
						goto tr279;
				} else if ( (*p) >= 38u )
					goto tr19;
			} else if ( (*p) > 55u ) {
				if ( (*p) < 96u ) {
					if ( 56u <= (*p) && (*p) <= 59u )
						goto tr279;
				} else if ( (*p) > 111u ) {
					if ( 112u <= (*p) && (*p) <= 127u )
						goto tr290;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr279;
	} else if ( (*p) > 139u ) {
		if ( (*p) < 194u ) {
			if ( (*p) < 160u ) {
				if ( (*p) > 142u ) {
					if ( 156u <= (*p) && (*p) <= 157u )
						goto tr19;
				} else if ( (*p) >= 140u )
					goto tr19;
			} else if ( (*p) > 163u ) {
				if ( (*p) < 184u ) {
					if ( 176u <= (*p) && (*p) <= 183u )
						goto tr280;
				} else if ( (*p) > 191u ) {
					if ( 192u <= (*p) && (*p) <= 193u )
						goto tr295;
				} else
					goto tr281;
			} else
				goto tr294;
		} else if ( (*p) > 195u ) {
			if ( (*p) < 212u ) {
				if ( (*p) > 207u ) {
					if ( 208u <= (*p) && (*p) <= 211u )
						goto tr302;
				} else if ( (*p) >= 204u )
					goto tr19;
			} else if ( (*p) > 231u ) {
				if ( (*p) < 234u ) {
					if ( 232u <= (*p) && (*p) <= 233u )
						goto tr310;
				} else if ( (*p) > 241u ) {
					if ( 250u <= (*p) && (*p) <= 251u )
						goto tr19;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr19;
	} else
		goto tr279;
	goto tr285;
tr323:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st233;
st233:
	if ( ++p == pe )
		goto _test_eof233;
case 233:
#line 7421 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 116u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 180u: goto st9;
		case 213u: goto tr271;
		case 229u: goto tr271;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 64u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 192u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
st234:
	if ( ++p == pe )
		goto _test_eof234;
case 234:
	if ( (*p) == 224u )
		goto tr277;
	goto tr11;
tr277:
#line 53 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st246;
st246:
	if ( ++p == pe )
		goto _test_eof246;
case 246:
#line 7484 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto tr280;
		case 5u: goto tr281;
		case 12u: goto tr280;
		case 13u: goto tr281;
		case 14u: goto tr19;
		case 15u: goto tr282;
		case 20u: goto tr280;
		case 21u: goto tr281;
		case 28u: goto tr280;
		case 29u: goto tr281;
		case 36u: goto tr280;
		case 37u: goto tr281;
		case 44u: goto tr280;
		case 45u: goto tr281;
		case 46u: goto tr283;
		case 47u: goto tr19;
		case 52u: goto tr280;
		case 53u: goto tr281;
		case 60u: goto tr280;
		case 61u: goto tr281;
		case 62u: goto tr284;
		case 63u: goto tr19;
		case 101u: goto tr286;
		case 102u: goto tr287;
		case 104u: goto tr281;
		case 105u: goto tr288;
		case 106u: goto tr280;
		case 107u: goto tr289;
		case 128u: goto tr289;
		case 129u: goto tr288;
		case 130u: goto tr19;
		case 131u: goto tr291;
		case 141u: goto tr292;
		case 143u: goto tr293;
		case 154u: goto tr19;
		case 168u: goto tr280;
		case 169u: goto tr281;
		case 196u: goto tr296;
		case 197u: goto tr297;
		case 198u: goto tr298;
		case 199u: goto tr299;
		case 200u: goto tr300;
		case 202u: goto tr301;
		case 216u: goto tr279;
		case 217u: goto tr303;
		case 218u: goto tr304;
		case 219u: goto tr305;
		case 220u: goto tr306;
		case 221u: goto tr307;
		case 222u: goto tr308;
		case 223u: goto tr309;
		case 235u: goto tr290;
		case 240u: goto tr311;
		case 242u: goto tr312;
		case 243u: goto tr313;
		case 246u: goto tr314;
		case 247u: goto tr315;
		case 254u: goto tr316;
		case 255u: goto tr324;
	}
	if ( (*p) < 132u ) {
		if ( (*p) < 32u ) {
			if ( (*p) < 8u ) {
				if ( (*p) > 3u ) {
					if ( 6u <= (*p) && (*p) <= 7u )
						goto tr19;
				} else
					goto tr279;
			} else if ( (*p) > 19u ) {
				if ( (*p) < 24u ) {
					if ( 22u <= (*p) && (*p) <= 23u )
						goto tr19;
				} else if ( (*p) > 27u ) {
					if ( 30u <= (*p) && (*p) <= 31u )
						goto tr19;
				} else
					goto tr279;
			} else
				goto tr279;
		} else if ( (*p) > 35u ) {
			if ( (*p) < 54u ) {
				if ( (*p) > 39u ) {
					if ( 40u <= (*p) && (*p) <= 51u )
						goto tr279;
				} else if ( (*p) >= 38u )
					goto tr19;
			} else if ( (*p) > 55u ) {
				if ( (*p) < 96u ) {
					if ( 56u <= (*p) && (*p) <= 59u )
						goto tr279;
				} else if ( (*p) > 111u ) {
					if ( 112u <= (*p) && (*p) <= 127u )
						goto tr290;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr279;
	} else if ( (*p) > 139u ) {
		if ( (*p) < 194u ) {
			if ( (*p) < 160u ) {
				if ( (*p) > 142u ) {
					if ( 156u <= (*p) && (*p) <= 157u )
						goto tr19;
				} else if ( (*p) >= 140u )
					goto tr19;
			} else if ( (*p) > 163u ) {
				if ( (*p) < 184u ) {
					if ( 176u <= (*p) && (*p) <= 183u )
						goto tr280;
				} else if ( (*p) > 191u ) {
					if ( 192u <= (*p) && (*p) <= 193u )
						goto tr295;
				} else
					goto tr281;
			} else
				goto tr294;
		} else if ( (*p) > 195u ) {
			if ( (*p) < 212u ) {
				if ( (*p) > 207u ) {
					if ( 208u <= (*p) && (*p) <= 211u )
						goto tr302;
				} else if ( (*p) >= 204u )
					goto tr19;
			} else if ( (*p) > 231u ) {
				if ( (*p) < 234u ) {
					if ( 232u <= (*p) && (*p) <= 233u )
						goto tr310;
				} else if ( (*p) > 241u ) {
					if ( 250u <= (*p) && (*p) <= 251u )
						goto tr19;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr19;
	} else
		goto tr279;
	goto tr285;
tr324:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st235;
st235:
	if ( ++p == pe )
		goto _test_eof235;
case 235:
#line 7638 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 116u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 180u: goto st9;
		case 214u: goto tr271;
		case 230u: goto tr271;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 64u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 192u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
st236:
	if ( ++p == pe )
		goto _test_eof236;
case 236:
	if ( (*p) == 224u )
		goto tr278;
	goto tr11;
tr278:
#line 53 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{ }
#line 84 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
       /* On successful match the instruction start must point to the next byte
        * to be able to report the new offset as the start of instruction
        * causing error.  */
       begin = p + 1;
     }
	goto st247;
st247:
	if ( ++p == pe )
		goto _test_eof247;
case 247:
#line 7701 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto tr280;
		case 5u: goto tr281;
		case 12u: goto tr280;
		case 13u: goto tr281;
		case 14u: goto tr19;
		case 15u: goto tr282;
		case 20u: goto tr280;
		case 21u: goto tr281;
		case 28u: goto tr280;
		case 29u: goto tr281;
		case 36u: goto tr280;
		case 37u: goto tr281;
		case 44u: goto tr280;
		case 45u: goto tr281;
		case 46u: goto tr283;
		case 47u: goto tr19;
		case 52u: goto tr280;
		case 53u: goto tr281;
		case 60u: goto tr280;
		case 61u: goto tr281;
		case 62u: goto tr284;
		case 63u: goto tr19;
		case 101u: goto tr286;
		case 102u: goto tr287;
		case 104u: goto tr281;
		case 105u: goto tr288;
		case 106u: goto tr280;
		case 107u: goto tr289;
		case 128u: goto tr289;
		case 129u: goto tr288;
		case 130u: goto tr19;
		case 131u: goto tr291;
		case 141u: goto tr292;
		case 143u: goto tr293;
		case 154u: goto tr19;
		case 168u: goto tr280;
		case 169u: goto tr281;
		case 196u: goto tr296;
		case 197u: goto tr297;
		case 198u: goto tr298;
		case 199u: goto tr299;
		case 200u: goto tr300;
		case 202u: goto tr301;
		case 216u: goto tr279;
		case 217u: goto tr303;
		case 218u: goto tr304;
		case 219u: goto tr305;
		case 220u: goto tr306;
		case 221u: goto tr307;
		case 222u: goto tr308;
		case 223u: goto tr309;
		case 235u: goto tr290;
		case 240u: goto tr311;
		case 242u: goto tr312;
		case 243u: goto tr313;
		case 246u: goto tr314;
		case 247u: goto tr315;
		case 254u: goto tr316;
		case 255u: goto tr325;
	}
	if ( (*p) < 132u ) {
		if ( (*p) < 32u ) {
			if ( (*p) < 8u ) {
				if ( (*p) > 3u ) {
					if ( 6u <= (*p) && (*p) <= 7u )
						goto tr19;
				} else
					goto tr279;
			} else if ( (*p) > 19u ) {
				if ( (*p) < 24u ) {
					if ( 22u <= (*p) && (*p) <= 23u )
						goto tr19;
				} else if ( (*p) > 27u ) {
					if ( 30u <= (*p) && (*p) <= 31u )
						goto tr19;
				} else
					goto tr279;
			} else
				goto tr279;
		} else if ( (*p) > 35u ) {
			if ( (*p) < 54u ) {
				if ( (*p) > 39u ) {
					if ( 40u <= (*p) && (*p) <= 51u )
						goto tr279;
				} else if ( (*p) >= 38u )
					goto tr19;
			} else if ( (*p) > 55u ) {
				if ( (*p) < 96u ) {
					if ( 56u <= (*p) && (*p) <= 59u )
						goto tr279;
				} else if ( (*p) > 111u ) {
					if ( 112u <= (*p) && (*p) <= 127u )
						goto tr290;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr279;
	} else if ( (*p) > 139u ) {
		if ( (*p) < 194u ) {
			if ( (*p) < 160u ) {
				if ( (*p) > 142u ) {
					if ( 156u <= (*p) && (*p) <= 157u )
						goto tr19;
				} else if ( (*p) >= 140u )
					goto tr19;
			} else if ( (*p) > 163u ) {
				if ( (*p) < 184u ) {
					if ( 176u <= (*p) && (*p) <= 183u )
						goto tr280;
				} else if ( (*p) > 191u ) {
					if ( 192u <= (*p) && (*p) <= 193u )
						goto tr295;
				} else
					goto tr281;
			} else
				goto tr294;
		} else if ( (*p) > 195u ) {
			if ( (*p) < 212u ) {
				if ( (*p) > 207u ) {
					if ( 208u <= (*p) && (*p) <= 211u )
						goto tr302;
				} else if ( (*p) >= 204u )
					goto tr19;
			} else if ( (*p) > 231u ) {
				if ( (*p) < 234u ) {
					if ( 232u <= (*p) && (*p) <= 233u )
						goto tr310;
				} else if ( (*p) > 241u ) {
					if ( 250u <= (*p) && (*p) <= 251u )
						goto tr19;
				} else
					goto tr19;
			} else
				goto tr19;
		} else
			goto tr19;
	} else
		goto tr279;
	goto tr285;
tr325:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st237;
st237:
	if ( ++p == pe )
		goto _test_eof237;
case 237:
#line 7855 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 116u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 180u: goto st9;
		case 215u: goto tr271;
		case 231u: goto tr271;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 64u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 192u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
tr317:
#line 80 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        begin = p;
        BitmapSetBit(valid_targets, p - data);
     }
	goto st238;
st238:
	if ( ++p == pe )
		goto _test_eof238;
case 238:
#line 7907 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	switch( (*p) ) {
		case 4u: goto st2;
		case 5u: goto st3;
		case 12u: goto st2;
		case 13u: goto st3;
		case 52u: goto st2;
		case 53u: goto st3;
		case 68u: goto st8;
		case 76u: goto st8;
		case 116u: goto st8;
		case 132u: goto st9;
		case 140u: goto st9;
		case 180u: goto st9;
	}
	if ( (*p) < 112u ) {
		if ( (*p) < 48u ) {
			if ( (*p) <= 15u )
				goto tr0;
		} else if ( (*p) > 55u ) {
			if ( 64u <= (*p) && (*p) <= 79u )
				goto st7;
		} else
			goto tr0;
	} else if ( (*p) > 119u ) {
		if ( (*p) < 176u ) {
			if ( 128u <= (*p) && (*p) <= 143u )
				goto st3;
		} else if ( (*p) > 183u ) {
			if ( (*p) > 207u ) {
				if ( 240u <= (*p) && (*p) <= 247u )
					goto tr0;
			} else if ( (*p) >= 192u )
				goto tr0;
		} else
			goto st3;
	} else
		goto st7;
	goto tr19;
	}
	_test_eof239: cs = 239; goto _test_eof; 
	_test_eof1: cs = 1; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
	_test_eof93: cs = 93; goto _test_eof; 
	_test_eof94: cs = 94; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof96: cs = 96; goto _test_eof; 
	_test_eof97: cs = 97; goto _test_eof; 
	_test_eof98: cs = 98; goto _test_eof; 
	_test_eof99: cs = 99; goto _test_eof; 
	_test_eof100: cs = 100; goto _test_eof; 
	_test_eof101: cs = 101; goto _test_eof; 
	_test_eof102: cs = 102; goto _test_eof; 
	_test_eof103: cs = 103; goto _test_eof; 
	_test_eof104: cs = 104; goto _test_eof; 
	_test_eof105: cs = 105; goto _test_eof; 
	_test_eof106: cs = 106; goto _test_eof; 
	_test_eof107: cs = 107; goto _test_eof; 
	_test_eof108: cs = 108; goto _test_eof; 
	_test_eof109: cs = 109; goto _test_eof; 
	_test_eof110: cs = 110; goto _test_eof; 
	_test_eof111: cs = 111; goto _test_eof; 
	_test_eof112: cs = 112; goto _test_eof; 
	_test_eof113: cs = 113; goto _test_eof; 
	_test_eof114: cs = 114; goto _test_eof; 
	_test_eof115: cs = 115; goto _test_eof; 
	_test_eof116: cs = 116; goto _test_eof; 
	_test_eof117: cs = 117; goto _test_eof; 
	_test_eof118: cs = 118; goto _test_eof; 
	_test_eof119: cs = 119; goto _test_eof; 
	_test_eof240: cs = 240; goto _test_eof; 
	_test_eof120: cs = 120; goto _test_eof; 
	_test_eof121: cs = 121; goto _test_eof; 
	_test_eof122: cs = 122; goto _test_eof; 
	_test_eof123: cs = 123; goto _test_eof; 
	_test_eof124: cs = 124; goto _test_eof; 
	_test_eof125: cs = 125; goto _test_eof; 
	_test_eof126: cs = 126; goto _test_eof; 
	_test_eof127: cs = 127; goto _test_eof; 
	_test_eof128: cs = 128; goto _test_eof; 
	_test_eof129: cs = 129; goto _test_eof; 
	_test_eof130: cs = 130; goto _test_eof; 
	_test_eof131: cs = 131; goto _test_eof; 
	_test_eof132: cs = 132; goto _test_eof; 
	_test_eof133: cs = 133; goto _test_eof; 
	_test_eof134: cs = 134; goto _test_eof; 
	_test_eof135: cs = 135; goto _test_eof; 
	_test_eof136: cs = 136; goto _test_eof; 
	_test_eof137: cs = 137; goto _test_eof; 
	_test_eof138: cs = 138; goto _test_eof; 
	_test_eof139: cs = 139; goto _test_eof; 
	_test_eof140: cs = 140; goto _test_eof; 
	_test_eof141: cs = 141; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 
	_test_eof146: cs = 146; goto _test_eof; 
	_test_eof147: cs = 147; goto _test_eof; 
	_test_eof148: cs = 148; goto _test_eof; 
	_test_eof149: cs = 149; goto _test_eof; 
	_test_eof150: cs = 150; goto _test_eof; 
	_test_eof151: cs = 151; goto _test_eof; 
	_test_eof152: cs = 152; goto _test_eof; 
	_test_eof153: cs = 153; goto _test_eof; 
	_test_eof154: cs = 154; goto _test_eof; 
	_test_eof155: cs = 155; goto _test_eof; 
	_test_eof156: cs = 156; goto _test_eof; 
	_test_eof157: cs = 157; goto _test_eof; 
	_test_eof158: cs = 158; goto _test_eof; 
	_test_eof159: cs = 159; goto _test_eof; 
	_test_eof160: cs = 160; goto _test_eof; 
	_test_eof161: cs = 161; goto _test_eof; 
	_test_eof162: cs = 162; goto _test_eof; 
	_test_eof163: cs = 163; goto _test_eof; 
	_test_eof164: cs = 164; goto _test_eof; 
	_test_eof165: cs = 165; goto _test_eof; 
	_test_eof166: cs = 166; goto _test_eof; 
	_test_eof167: cs = 167; goto _test_eof; 
	_test_eof168: cs = 168; goto _test_eof; 
	_test_eof169: cs = 169; goto _test_eof; 
	_test_eof170: cs = 170; goto _test_eof; 
	_test_eof171: cs = 171; goto _test_eof; 
	_test_eof172: cs = 172; goto _test_eof; 
	_test_eof173: cs = 173; goto _test_eof; 
	_test_eof174: cs = 174; goto _test_eof; 
	_test_eof175: cs = 175; goto _test_eof; 
	_test_eof176: cs = 176; goto _test_eof; 
	_test_eof177: cs = 177; goto _test_eof; 
	_test_eof178: cs = 178; goto _test_eof; 
	_test_eof179: cs = 179; goto _test_eof; 
	_test_eof180: cs = 180; goto _test_eof; 
	_test_eof181: cs = 181; goto _test_eof; 
	_test_eof182: cs = 182; goto _test_eof; 
	_test_eof183: cs = 183; goto _test_eof; 
	_test_eof184: cs = 184; goto _test_eof; 
	_test_eof185: cs = 185; goto _test_eof; 
	_test_eof186: cs = 186; goto _test_eof; 
	_test_eof187: cs = 187; goto _test_eof; 
	_test_eof188: cs = 188; goto _test_eof; 
	_test_eof189: cs = 189; goto _test_eof; 
	_test_eof190: cs = 190; goto _test_eof; 
	_test_eof191: cs = 191; goto _test_eof; 
	_test_eof192: cs = 192; goto _test_eof; 
	_test_eof193: cs = 193; goto _test_eof; 
	_test_eof194: cs = 194; goto _test_eof; 
	_test_eof195: cs = 195; goto _test_eof; 
	_test_eof196: cs = 196; goto _test_eof; 
	_test_eof197: cs = 197; goto _test_eof; 
	_test_eof198: cs = 198; goto _test_eof; 
	_test_eof199: cs = 199; goto _test_eof; 
	_test_eof200: cs = 200; goto _test_eof; 
	_test_eof201: cs = 201; goto _test_eof; 
	_test_eof202: cs = 202; goto _test_eof; 
	_test_eof203: cs = 203; goto _test_eof; 
	_test_eof204: cs = 204; goto _test_eof; 
	_test_eof205: cs = 205; goto _test_eof; 
	_test_eof206: cs = 206; goto _test_eof; 
	_test_eof207: cs = 207; goto _test_eof; 
	_test_eof208: cs = 208; goto _test_eof; 
	_test_eof209: cs = 209; goto _test_eof; 
	_test_eof210: cs = 210; goto _test_eof; 
	_test_eof211: cs = 211; goto _test_eof; 
	_test_eof212: cs = 212; goto _test_eof; 
	_test_eof213: cs = 213; goto _test_eof; 
	_test_eof214: cs = 214; goto _test_eof; 
	_test_eof215: cs = 215; goto _test_eof; 
	_test_eof216: cs = 216; goto _test_eof; 
	_test_eof217: cs = 217; goto _test_eof; 
	_test_eof218: cs = 218; goto _test_eof; 
	_test_eof219: cs = 219; goto _test_eof; 
	_test_eof220: cs = 220; goto _test_eof; 
	_test_eof221: cs = 221; goto _test_eof; 
	_test_eof222: cs = 222; goto _test_eof; 
	_test_eof223: cs = 223; goto _test_eof; 
	_test_eof224: cs = 224; goto _test_eof; 
	_test_eof241: cs = 241; goto _test_eof; 
	_test_eof225: cs = 225; goto _test_eof; 
	_test_eof226: cs = 226; goto _test_eof; 
	_test_eof242: cs = 242; goto _test_eof; 
	_test_eof227: cs = 227; goto _test_eof; 
	_test_eof228: cs = 228; goto _test_eof; 
	_test_eof243: cs = 243; goto _test_eof; 
	_test_eof229: cs = 229; goto _test_eof; 
	_test_eof230: cs = 230; goto _test_eof; 
	_test_eof244: cs = 244; goto _test_eof; 
	_test_eof231: cs = 231; goto _test_eof; 
	_test_eof232: cs = 232; goto _test_eof; 
	_test_eof245: cs = 245; goto _test_eof; 
	_test_eof233: cs = 233; goto _test_eof; 
	_test_eof234: cs = 234; goto _test_eof; 
	_test_eof246: cs = 246; goto _test_eof; 
	_test_eof235: cs = 235; goto _test_eof; 
	_test_eof236: cs = 236; goto _test_eof; 
	_test_eof247: cs = 247; goto _test_eof; 
	_test_eof237: cs = 237; goto _test_eof; 
	_test_eof238: cs = 238; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 1: 
	case 2: 
	case 3: 
	case 4: 
	case 5: 
	case 6: 
	case 7: 
	case 8: 
	case 9: 
	case 10: 
	case 11: 
	case 12: 
	case 13: 
	case 14: 
	case 15: 
	case 16: 
	case 17: 
	case 18: 
	case 19: 
	case 20: 
	case 21: 
	case 22: 
	case 23: 
	case 24: 
	case 25: 
	case 26: 
	case 27: 
	case 28: 
	case 29: 
	case 30: 
	case 31: 
	case 32: 
	case 33: 
	case 34: 
	case 35: 
	case 36: 
	case 37: 
	case 38: 
	case 39: 
	case 40: 
	case 41: 
	case 42: 
	case 43: 
	case 44: 
	case 45: 
	case 46: 
	case 47: 
	case 48: 
	case 49: 
	case 50: 
	case 51: 
	case 52: 
	case 53: 
	case 54: 
	case 55: 
	case 56: 
	case 57: 
	case 58: 
	case 59: 
	case 60: 
	case 61: 
	case 62: 
	case 63: 
	case 64: 
	case 65: 
	case 66: 
	case 67: 
	case 68: 
	case 69: 
	case 70: 
	case 71: 
	case 72: 
	case 73: 
	case 74: 
	case 75: 
	case 76: 
	case 77: 
	case 78: 
	case 79: 
	case 80: 
	case 81: 
	case 82: 
	case 83: 
	case 84: 
	case 85: 
	case 86: 
	case 87: 
	case 88: 
	case 89: 
	case 90: 
	case 91: 
	case 92: 
	case 93: 
	case 94: 
	case 95: 
	case 96: 
	case 97: 
	case 98: 
	case 99: 
	case 100: 
	case 101: 
	case 102: 
	case 103: 
	case 104: 
	case 105: 
	case 106: 
	case 107: 
	case 108: 
	case 109: 
	case 110: 
	case 111: 
	case 112: 
	case 113: 
	case 114: 
	case 115: 
	case 116: 
	case 117: 
	case 118: 
	case 119: 
	case 120: 
	case 121: 
	case 122: 
	case 123: 
	case 124: 
	case 125: 
	case 126: 
	case 127: 
	case 128: 
	case 129: 
	case 130: 
	case 131: 
	case 132: 
	case 133: 
	case 134: 
	case 135: 
	case 136: 
	case 137: 
	case 138: 
	case 139: 
	case 140: 
	case 141: 
	case 142: 
	case 143: 
	case 144: 
	case 145: 
	case 146: 
	case 147: 
	case 148: 
	case 149: 
	case 150: 
	case 151: 
	case 152: 
	case 153: 
	case 154: 
	case 155: 
	case 156: 
	case 157: 
	case 158: 
	case 159: 
	case 160: 
	case 161: 
	case 162: 
	case 163: 
	case 164: 
	case 165: 
	case 166: 
	case 167: 
	case 168: 
	case 169: 
	case 170: 
	case 171: 
	case 172: 
	case 173: 
	case 174: 
	case 175: 
	case 176: 
	case 177: 
	case 178: 
	case 179: 
	case 180: 
	case 181: 
	case 182: 
	case 183: 
	case 184: 
	case 185: 
	case 186: 
	case 187: 
	case 188: 
	case 189: 
	case 190: 
	case 191: 
	case 192: 
	case 193: 
	case 194: 
	case 195: 
	case 196: 
	case 197: 
	case 198: 
	case 199: 
	case 200: 
	case 201: 
	case 202: 
	case 203: 
	case 204: 
	case 205: 
	case 206: 
	case 207: 
	case 208: 
	case 209: 
	case 210: 
	case 211: 
	case 212: 
	case 213: 
	case 214: 
	case 215: 
	case 216: 
	case 217: 
	case 218: 
	case 219: 
	case 220: 
	case 221: 
	case 222: 
	case 223: 
	case 224: 
	case 225: 
	case 226: 
	case 227: 
	case 228: 
	case 229: 
	case 230: 
	case 231: 
	case 232: 
	case 233: 
	case 234: 
	case 235: 
	case 236: 
	case 237: 
	case 238: 
#line 90 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
	{
        process_error(begin, userdata);
        result = 1;
        goto error_detected;
    }
	break;
#line 8444 "src/trusted/validator_ragel/gen/validator-x86_32.c"
	}
	}

	_out: {}
	}

#line 172 "src/trusted/validator_ragel/unreviewed/validator-x86_32.rl"
  }

  if (CheckJumpTargets(valid_targets, jump_dests, size)) {
    return 1;
  }

error_detected:
  return result;
}
