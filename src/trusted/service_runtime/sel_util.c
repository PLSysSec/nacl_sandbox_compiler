/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Simple/secure ELF loader (NaCl SEL).
 */
#include "native_client/src/include/portability.h"
#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"


/*
 * return the smallest power of 2 that is greater than or equal to max_addr
 *
 * pre: the result is representable as a size_t.
 *
 * in concrete terms, max_addr <= ~((~0U)>>1) = (unsigned int) INT_MIN
 * (assuming size_t is unsigned int).
 */
size_t NaClAppPow2Ceil(size_t max_addr) {
#if 0
  size_t  addr, t;

  /* check precondition */
  if ((max_addr & ~((~0U)>>1)) != 0 && (max_addr & ((~0U)>>1)) != 0) {
    return 0;
  }

  /* O( number of one bits in max_addr ) */
  for (addr = max_addr; (t = (addr & (addr-1))) != 0; addr = t)
    ;
  if (addr < max_addr) addr = addr << 1;
#else
  size_t  addr;

  /* check precondition */
  if ((max_addr & ~((~0U)>>1)) != 0 && (max_addr & ((~0U)>>1)) != 0) {
    return 0;
  }

  /* O( number of bits in max_addr ) */
  for (addr = 1; addr < max_addr; addr = addr << 1)
    ;
#endif

  CHECK(addr >= max_addr);
  CHECK(addr != 0);
  CHECK((addr & (addr-1)) == 0);
  return addr;
}
