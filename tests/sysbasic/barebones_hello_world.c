/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl test for simple hello world not using newlib
 */

/*
 * these were lifted from src/trusted/service_runtime/nacl_config.h
 * NOTE: we cannot include this file here
 * TODO(robertm): make this file available in the sdk
 */

#define NACL_INSTR_BLOCK_SHIFT         5
#define NACL_PAGESHIFT                12
#define NACL_SYSCALL_START_ADDR       (16 << NACL_PAGESHIFT)
#define NACL_SYSCALL_ADDR(syscall_number)                               \
     (NACL_SYSCALL_START_ADDR + (syscall_number << NACL_INSTR_BLOCK_SHIFT))

#define NACL_SYSCALL(s) ((TYPE_nacl_ ## s) NACL_SYSCALL_ADDR(NACL_sys_ ## s))

typedef int (*TYPE_nacl_write) (int desc, void const *buf, int count);
typedef void (*TYPE_nacl_null) (void);
typedef void (*TYPE_nacl_exit) (int status);

#include <bits/nacl_syscalls.h>


#define myprint(s) NACL_SYSCALL(write)(1, s, mystrlen(s))


static int mystrlen(const char* s) {
  int count = 0;
  while(*s++) ++count;
  return count;
}


int main() {
  myprint("@null\n");
  NACL_SYSCALL(null)();

  myprint("@write\n");
  myprint("hello worldn");

  myprint("@exit\n");
  NACL_SYSCALL(exit)(69);
  return 0;
}
