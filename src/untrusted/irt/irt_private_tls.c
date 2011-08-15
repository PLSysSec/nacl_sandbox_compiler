/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "native_client/src/untrusted/nacl/nacl_thread.h"
#include "native_client/src/untrusted/nacl/syscall_bindings_trampoline.h"
#include "native_client/src/untrusted/nacl/tls.h"


/*
 * The integrated runtime (IRT) needs to have Thread Local Storage
 * isolated from the TLS of the user executable.
 *
 * This exists mainly:
 *  1) to make errno (and newlib's stdio) work inside the IRT without
 *     having to rebuild newlib;
 *  2) to make pthread_mutex_lock() work inside the IRT, since it
 *     expects to be able to get the current thread's ID.
 *
 * These functions override functions in libnacl (src/untrusted/nacl).
 */

int nacl_tls_init(void *thread_ptr) {
  return -NACL_SYSCALL(second_tls_set)(thread_ptr);
}

void *nacl_tls_get() {
  return NACL_SYSCALL(second_tls_get)();
}

void *__nacl_read_tp() {
  return nacl_tls_get();
}
