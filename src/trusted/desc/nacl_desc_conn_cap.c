/*
 * Copyright 2008  The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

/*
 * NaCl Service Runtime.  I/O Descriptor / Handle abstraction.
 * Connection capabilities.
 */

#include <stdlib.h>
#include <string.h>

#include "native_client/src/include/portability.h"
#include "native_client/src/include/nacl_macros.h"

#include "native_client/src/shared/imc/nacl_imc_c.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/desc/nacl_desc_base.h"
#include "native_client/src/trusted/desc/nacl_desc_conn_cap.h"
#include "native_client/src/trusted/desc/nacl_desc_imc.h"
#include "native_client/src/trusted/desc/nacl_desc_imc_bound_desc.h"

#include "native_client/src/trusted/service_runtime/nacl_config.h"
#include "native_client/src/trusted/service_runtime/include/sys/errno.h"
#include "native_client/src/trusted/service_runtime/include/sys/stat.h"

static struct NaClDescVtbl const kNaClDescConnCapVtbl;  /* fwd */

int NaClDescConnCapCtor(struct NaClDescConnCap          *self,
                        struct NaClSocketAddress const  *nsap) {
  struct NaClDesc *basep = (struct NaClDesc *) self;

  basep->base.vtbl = (struct NaClRefCountVtbl const *) NULL;
  if (!NaClDescCtor(basep)) {
    return 0;
  }
  self->cap = *nsap;
  basep->base.vtbl = (struct NaClRefCountVtbl const *) &kNaClDescConnCapVtbl;
  return 1;
}

static void NaClDescConnCapDtor(struct NaClDesc *vself) {
  vself->base.vtbl = (struct NaClRefCountVtbl const *) &kNaClDescVtbl;
  (*vself->base.vtbl->Dtor)(&vself->base);
  return;
}

static int NaClDescConnCapFstat(struct NaClDesc          *vself,
                                struct nacl_abi_stat     *statbuf) {
  UNREFERENCED_PARAMETER(vself);

  memset(statbuf, 0, sizeof *statbuf);
  statbuf->nacl_abi_st_mode = NACL_ABI_S_IFSOCKADDR | NACL_ABI_S_IRWXU;
  return 0;
}

static int NaClDescConnCapClose(struct NaClDesc          *vself) {
  NaClDescUnref(vself);
  return 0;
}

static int NaClDescConnCapExternalizeSize(struct NaClDesc  *vself,
                                          size_t           *nbytes,
                                          size_t           *nhandles) {
  UNREFERENCED_PARAMETER(vself);

  *nbytes = NACL_PATH_MAX;
  *nhandles = 0;

  return 0;
}

static int NaClDescConnCapExternalize(struct NaClDesc          *vself,
                                      struct NaClDescXferState *xfer) {
  struct NaClDescConnCap    *self;

  self = (struct NaClDescConnCap *) vself;
  memcpy(xfer->next_byte, self->cap.path, NACL_PATH_MAX);
  xfer->next_byte += NACL_PATH_MAX;

  return 0;
}

int NaClDescConnCapConnectAddr(struct NaClDesc *vself,
                               struct NaClDesc **out_desc) {
  /*
   * See NaClDescImcBoundDescAcceptConn code in
   * nacl_desc_imc_bound_desc.c
   */
  struct NaClDescConnCap      *self;
  int                         retval;
  NaClHandle                  nh[2];
  size_t                      ix;
  struct NaClMessageHeader    conn_msg;
  struct NaClDescImcDesc      *peer;

  NaClLog(3, "Entered NaClDescConnCapConnectAddr\n");
  self = (struct NaClDescConnCap *) vself;

  peer = NULL;
  for (ix = 0; ix < NACL_ARRAY_SIZE(nh); ++ix) {
    nh[ix] = NACL_INVALID_HANDLE;
  }

  NaClLog(4, " socket address %.*s\n", NACL_PATH_MAX, self->cap.path);

  if (NULL == (peer = malloc(sizeof *peer))) {
    retval = -NACL_ABI_ENOMEM;
    goto cleanup;
  }

  if (0 != NaClSocketPair(nh)) {
    retval = -NACL_ABI_EMFILE;
    goto cleanup;
  }

  conn_msg.iov_length = 0;
  conn_msg.iov = NULL;
  conn_msg.handles = &nh[0];
  conn_msg.handle_count = 1;  /* send nh[0], keep nh[1] */
  conn_msg.flags = 0;

  NaClLog(4, " sending connection message\n");
  if (-1 == NaClSendDatagramTo(&conn_msg, 0, &self->cap)) {
    NaClLog(LOG_ERROR, ("NaClDescConnCapConnectAddr:"
                        " initial connect message could not be sent.\n"));
    retval = -NACL_ABI_EIO;
    goto cleanup;
  }

  (void) NaClClose(nh[0]);
  nh[0] = NACL_INVALID_HANDLE;
  NaClLog(4, " creating NaClDescImcDesc for local end of socketpair\n");
  if (!NaClDescImcDescCtor(peer, nh[1])) {
    retval = -NACL_ABI_EMFILE;  /* TODO(bsy): is this the right errno? */
    goto cleanup;
  }
  nh[1] = NACL_INVALID_HANDLE;

  *out_desc = (struct NaClDesc *) peer;
  retval = 0;

cleanup:
  if (retval < 0) {
    NaClLog(4, " error return; cleaning up\n");
    if (NACL_INVALID_HANDLE != nh[0])
      (void) NaClClose(nh[0]);
    if (NACL_INVALID_HANDLE != nh[1])
      (void) NaClClose(nh[1]);
    /* peer is not constructed, so we need only to free the memory */
    free(peer);
  }
  return retval;
}

static int NaClDescConnCapAcceptConn(struct NaClDesc *vself,
                                     struct NaClDesc **out_desc) {
  UNREFERENCED_PARAMETER(vself);
  UNREFERENCED_PARAMETER(out_desc);

  NaClLog(LOG_ERROR, "NaClDescConnCapAcceptConn: not IMC\n");
  return -NACL_ABI_EINVAL;
}

static struct NaClDescVtbl const kNaClDescConnCapVtbl = {
  {
    (void (*)(struct NaClRefCount *)) NaClDescConnCapDtor,
  },
  NaClDescMapNotImplemented,
  NaClDescUnmapUnsafeNotImplemented,
  NaClDescUnmapNotImplemented,
  NaClDescReadNotImplemented,
  NaClDescWriteNotImplemented,
  NaClDescSeekNotImplemented,
  NaClDescIoctlNotImplemented,
  NaClDescConnCapFstat,
  NaClDescConnCapClose,
  NaClDescGetdentsNotImplemented,
  NACL_DESC_CONN_CAP,
  NaClDescConnCapExternalizeSize,
  NaClDescConnCapExternalize,
  NaClDescLockNotImplemented,
  NaClDescTryLockNotImplemented,
  NaClDescUnlockNotImplemented,
  NaClDescWaitNotImplemented,
  NaClDescTimedWaitAbsNotImplemented,
  NaClDescSignalNotImplemented,
  NaClDescBroadcastNotImplemented,
  NaClDescSendMsgNotImplemented,
  NaClDescRecvMsgNotImplemented,
  NaClDescConnCapConnectAddr,
  NaClDescConnCapAcceptConn,
  NaClDescPostNotImplemented,
  NaClDescSemWaitNotImplemented,
  NaClDescGetValueNotImplemented,
};

int NaClDescConnCapInternalize(struct NaClDesc          **out_desc,
                               struct NaClDescXferState *xfer) {
  int                       rv;
  struct NaClSocketAddress  nsa;
  struct NaClDescConnCap    *ndccp;

  rv = -NACL_ABI_EIO;  /* catch-all */
  ndccp = NULL;

  if (xfer->next_byte + NACL_PATH_MAX > xfer->byte_buffer_end) {
    rv = -NACL_ABI_EIO;
    goto cleanup;
  }
  memcpy(nsa.path, xfer->next_byte, NACL_PATH_MAX);
  ndccp = malloc(sizeof *ndccp);
  if (NULL == ndccp) {
    rv = -NACL_ABI_ENOMEM;
    goto cleanup;
  }
  if (!NaClDescConnCapCtor(ndccp, &nsa)) {
    rv = -NACL_ABI_ENOMEM;
    goto cleanup;
  }
  *out_desc = (struct NaClDesc *) ndccp;
  rv = 0;
  xfer->next_byte += NACL_PATH_MAX;
cleanup:
  if (rv < 0) {
    free(ndccp);
  }
  return rv;
}
