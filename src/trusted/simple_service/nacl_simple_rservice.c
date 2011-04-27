/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "native_client/src/trusted/simple_service/nacl_simple_rservice.h"

#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_threads.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"

#include "native_client/src/trusted/desc/nacl_desc_base.h"
#include "native_client/src/trusted/desc/nacl_desc_conn_cap.h"
#include "native_client/src/trusted/desc/nrd_xfer.h"

#include "native_client/src/trusted/nacl_base/nacl_refcount.h"

#include "native_client/src/trusted/service_runtime/nacl_config.h"
/* NACL_KERN_STACK_SIZE */
#include "native_client/src/trusted/service_runtime/include/sys/errno.h"

int NaClSimpleRevClientCtor(struct NaClSimpleRevClient  *self,
                            void                        (*callback)(
                                void                        *state,
                                struct NaClDesc             *conn),
                            void                        *state) {
  NaClLog(4,
          "NaClSimpleRevClientCtor: this 0x%"NACL_PRIxPTR"\n",
          (uintptr_t) self);
  if (!NaClRefCountCtor(&self->base)) {
    NaClLog(4, "NaClSimpleRevClientCtor: NaClRefCountCtor failed\n");
    goto done;
  }
  if (0 != NaClCommonDescMakeBoundSock(self->bound_and_cap)) {
    goto bound_failed;
  }

  self->acceptor_spawned = 0;
  self->client_callback = callback;
  self->state = state;
  NaClLog(4,
          ("NaClSimpleRevClientCtor: callback 0x%"NACL_PRIxPTR
           ", state 0x%"NACL_PRIxPTR"\n"),
          (uintptr_t) callback,
          (uintptr_t) state);
  NACL_VTBL(NaClRefCount, self) =
      (struct NaClRefCountVtbl *) &kNaClSimpleRevClientVtbl;
  return 1;

bound_failed:
  (*NACL_VTBL(NaClRefCount, self)->Dtor)(&self->base);
done:
  return 0;
}

static void NaClSimpleRevClientDtor(struct NaClRefCount *vself) {
  struct NaClSimpleRevClient *self =
      (struct NaClSimpleRevClient *) vself;

  NaClDescUnref(self->bound_and_cap[0]);
  NaClDescUnref(self->bound_and_cap[1]);
  if (self->acceptor_spawned) {
    NaClThreadDtor(&self->acceptor);
  }
  self->acceptor_spawned = 0;

  NACL_VTBL(NaClRefCount, self) = &kNaClRefCountVtbl;
  (*NACL_VTBL(NaClRefCount, self)->Dtor)(&self->base);
}

struct NaClSimpleRevClientVtbl const kNaClSimpleRevClientVtbl = {
  {
    NaClSimpleRevClientDtor,
  },
};

static void WINAPI RevRpcHandlerBase(void *thread_state) {
  struct NaClSimpleRevClient *self =
      (struct NaClSimpleRevClient *) thread_state;

  int             status;
  struct NaClDesc *conn;

  NaClLog(4, "Entered RevRpcHandlerBase\n");
  while (0 == (status =
               (*NACL_VTBL(NaClDesc, self->bound_and_cap[0])->AcceptConn)(
                   self->bound_and_cap[0], &conn))) {
    NaClLog(4,
            ("RevRpcHandlerBase: Accept returned success,"
             " invoking callback"
             " 0x%"NACL_PRIxPTR"(0x%"NACL_PRIxPTR",0x%"NACL_PRIxPTR")\n"),
            (uintptr_t) self->client_callback,
            (uintptr_t) self->state,
            (uintptr_t) conn);
    /*
     * The callback should not block; if needed, it should spawn
     * another thread.  The callback takes ownership of |conn| -- it
     * may Unref |conn| prior to returning or the ownership of |conn|
     * may be passed to another thread (e.g., newly spawned, or waking
     * up and giving |conn| to the thread that made the connection
     * request in the first place).
     */
    (*self->client_callback)(self->state, conn);
    NaClLog(4,
            "RevRpcHandlerBase: callback finished.\n");
  }
  NaClLog(LOG_INFO,
          ("NaClSimpleRevClient::RevRpcHandlerBase:"
           " AcceptConn failed, status %d\n"),
          status);
  /*
   * The unref of self may Dtor the currently running thread.  This is
   * okay, since this only removes the ability to use the thread
   * handle (in Windows) but does not otherwise affect the thread.  We
   * don't log afterwards, just in case the logging code (is later
   * modified to) use thread info.
   */
  NaClRefCountUnref((struct NaClRefCount *) self);
}

int NaClSimpleRevClientStartServiceThread(struct NaClSimpleRevClient *self) {
  NaClLog(4, "Entered NaClSimpleRevClientStartServiceThread\n");
  if (self->acceptor_spawned) {
    NaClLog(LOG_FATAL,
            "NaClSimpleRevClientStartServiceThread: dup - already started\n");
  }
  if (!NaClThreadCtor(&self->acceptor,
                      RevRpcHandlerBase,
                      (struct NaClSimpleRevClient *) NaClRefCountRef(
                          (struct NaClRefCount *) self),
                      NACL_KERN_STACK_SIZE)) {
    NaClRefCountUnref((struct NaClRefCount *) self);
    return 0;
  }
  return 1;
}

int NaClSimpleRevServiceCtor(struct NaClSimpleRevService      *self,
                             struct NaClDesc                  *conn_cap,
                             struct NaClSrpcHandlerDesc const *handlers) {
  NaClLog(4,
          "NaClSimpleRevServiceCtor: this 0x%"NACL_PRIxPTR"\n",
          (uintptr_t) self);
  if (!NaClRefCountCtor(&self->base)) {
    NaClLog(4, "NaClSimpleRevServiceCtor: NaClRefCountCtor failed\n");
    return 0;
  }
  self->conn_cap = conn_cap;  /* take ownership, if ctor succeeds */
  self->handlers = handlers;
  /* caller ensures lifetime of handlers is at least that of self */

  NACL_VTBL(NaClRefCount, self) =
      (struct NaClRefCountVtbl *) &kNaClSimpleRevServiceVtbl;

  NaClLog(4, "Leaving NaClSimpleRevServiceCtor\n");
  return 1;
}

void NaClSimpleRevServiceDtor(struct NaClRefCount *vself) {
  struct NaClSimpleRevService *self =
      (struct NaClSimpleRevService *) vself;

  NaClDescUnref(self->conn_cap);
  self->conn_cap = NULL;
  self->handlers = NULL;

  NACL_VTBL(NaClRefCount, self) = &kNaClRefCountVtbl;
  (*NACL_VTBL(NaClRefCount, self)->Dtor)(vself);
}

static void WINAPI ConnRpcBase(void *thread_state) {
  struct NaClSimpleRevConnection *rev_conn =
      (struct NaClSimpleRevConnection *) thread_state;

  NaClLog(4, "Entered ConnRpcBase, invoking RpcHandler vfn\n");
  (*NACL_VTBL(NaClSimpleRevService, rev_conn->service)->RpcHandler)(
      rev_conn->service, rev_conn);
  if (NULL != rev_conn->instance_data_cleanup) {
    (*rev_conn->instance_data_cleanup)(rev_conn->instance_data);
  }
  NaClThreadDtor(&rev_conn->thread);
  NaClLog(4, "Leaving ConnRpcBase\n");
  NaClRefCountUnref((struct NaClRefCount *) rev_conn);
}

int NaClSimpleRevServiceConnectAndSpawnHandler(
    struct NaClSimpleRevService *self,
    void                        *instance_data,
    void                        (*instance_data_cleanup)(void *instance_data)) {
  int                             status;
  struct NaClDesc                 *conn = NULL;
  struct NaClSimpleRevConnection  *rev_conn;

  NaClLog(4, "Entered NaClSimpleRevServiceConnectAndSpawnHandler\n");
  if (0 != (status =
            (*NACL_VTBL(NaClDesc, self->conn_cap)->ConnectAddr)(
                self->conn_cap, &conn))) {
    /* failed */
    NaClLog(4, "NaClSimpleRevServiceConnectAndSpawnHandler: connect failed\n");
    return status;
  }
  if (0 != (status =
            (*NACL_VTBL(NaClSimpleRevService, self)->RevConnectionFactory)(
                self, conn, instance_data, instance_data_cleanup, &rev_conn))) {
    NaClDescUnref(conn);
    NaClLog(4,
            ("NaClSimpleRevServiceConnectAndSpawnHandler: factory failed,"
             " error %d\n"),
            status);
    return status;
  }
  conn = NULL;  /* rev_conn owns the ref in conn now */
  /*
   * Spawn thread using NaClSimpleRevConnection.
   */
  /* rev_conn not visible to other threads, so okay */
  if (!NaClThreadCtor(&rev_conn->thread,
                      ConnRpcBase, rev_conn, NACL_KERN_STACK_SIZE)) {
    /*
     * no thread, clean up
     */
    NaClRefCountUnref((struct NaClRefCount *) rev_conn);
    NaClLog(4, "NaClSimpleRevServiceConnectAndSpawnHandler: no thread\n");
    return -NACL_ABI_EAGAIN;
  }
  /* thread owns rev_conn */
  NaClLog(4, "Leaving NaClSimpleRevServiceConnectAndSpawnHandler\n");
  return 0;
}

int NaClSimpleRevServiceConnectionFactory(
    struct NaClSimpleRevService     *self,
    struct NaClDesc                 *conn,
    void                            *instance_data,
    void                            (*instance_data_cleanup)(
        void *instance_data),
    struct NaClSimpleRevConnection  **out) {
  struct NaClSimpleRevConnection *rconn;

  rconn = (struct NaClSimpleRevConnection *) malloc(sizeof *rconn);
  if (NULL == rconn) {
    NaClLog(4, "NaClSimpleRevServiceConnectionFactoryWithInstanceData:"
            " no memory\n");
    return -NACL_ABI_EAGAIN;
  }
  if (!NaClSimpleRevConnectionCtor(rconn, self, conn, instance_data,
                                   instance_data_cleanup)) {
    NaClLog(4, "NaClSimpleRevServiceConnectionFactoryWithInstanceData:"
            " NaClSimpleRevConnectionCtor failed\n");
    free(rconn);
    return -NACL_ABI_EINVAL;
  }

  *out = rconn;
  return 0;
}

void NaClSimpleRevServiceRpcHandler(
    struct NaClSimpleRevService     *self,
    struct NaClSimpleRevConnection  *conn) {
  int retval;

  NaClLog(4, "Entered NaClSimpleRevServiceRpcHandler: ServerLoop!\n");
  retval = NaClSrpcServerLoop(conn->connected_socket,
                              self->handlers,
                              conn->instance_data);
  NaClLog(4, "Leaving NaClSimpleRevServiceRpcHandler\n");
}

struct NaClSimpleRevServiceVtbl const kNaClSimpleRevServiceVtbl = {
  {
    NaClSimpleRevServiceDtor,
  },
  NaClSimpleRevServiceConnectAndSpawnHandler,
  NaClSimpleRevServiceConnectionFactory,
  NaClSimpleRevServiceRpcHandler,
};

int NaClSimpleRevConnectionCtor(
    struct NaClSimpleRevConnection  *self,
    struct NaClSimpleRevService     *service,
    struct NaClDesc                 *conn,
    void                            *instance_data,
    void                            (*instance_data_cleanup)(
        void *instance_data)) {
  NaClLog(4,
          "NaClSimpleRevConnectionCtor: this 0x%"NACL_PRIxPTR"\n",
          (uintptr_t) self);
  if (!NaClRefCountCtor(&self->base)) {
    return 0;
  }

  self->service = service;
  self->connected_socket = conn;
  self->instance_data = instance_data;
  self->instance_data_cleanup = instance_data_cleanup;

  return 1;
}

void NaClSimpleRevConnectionDtor(struct NaClRefCount *vself) {
  struct NaClSimpleRevConnection *self =
      (struct NaClSimpleRevConnection *) vself;

  NaClRefCountUnref((struct NaClRefCount *) self->service);
  NaClRefCountUnref((struct NaClRefCount *) self->connected_socket);

  NACL_VTBL(NaClRefCount, self) = &kNaClRefCountVtbl;
  (*NACL_VTBL(NaClRefCount, self)->Dtor)(vself);
}

struct NaClRefCountVtbl kNaClSimpleRevConnectionVtbl = {
  NaClSimpleRevConnectionDtor,
};
