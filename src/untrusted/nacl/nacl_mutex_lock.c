/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "native_client/src/untrusted/nacl/nacl_thread.h"
#include "native_client/src/untrusted/nacl/syscall_bindings_trampoline.h"

int nacl_mutex_lock(int mutex_handle) {
  return -NACL_GC_WRAP_SYSCALL(NACL_SYSCALL(mutex_lock)(mutex_handle));
}
