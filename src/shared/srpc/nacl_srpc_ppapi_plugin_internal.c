/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "native_client/src/shared/srpc/nacl_srpc_ppapi_plugin_internal.h"

#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/nacl_syscalls.h>
#if 0  /* until toolchain propagation */
#include <sys/nacl_kern_rpc.h>
#else
#include "native_client/src/trusted/service_runtime/include/sys/nacl_kern_rpc.h"
#endif
#include <sys/nacl_name_service.h>

#include "native_client/src/shared/platform/nacl_log.h"

#include "native_client/src/shared/srpc/nacl_srpc.h"
#include "native_client/src/shared/srpc/nacl_srpc_internal.h"

static int gNaClNameServiceConnCapDesc = -1;

void NaClPluginLowLevelInitializationCompleteInternal(void) {
  int                     nameservice_conn_desc;
  int                     kern_service_conn_cap_desc = -1;
  int                     kern_service_desc;
  struct NaClSrpcChannel  srpc_channel;
  int                     status;

  NaClLog(4, "Entered NaClPluginLowLevelInitializationComplete\n");

  if (-1 != gNaClNameServiceConnCapDesc) {
    NaClLog(LOG_ERROR,
            "Double call to NaClPluginLowLevelInitializationComplete?\n");
    return;
  }
  /*
   * The existence of the bootstrap nameservice is independent of
   * whether NaCl is running as a standalone application or running as
   * a untrusted Pepper plugin, browser extension environment.
   */
  if (-1 == nacl_nameservice(&gNaClNameServiceConnCapDesc)) {
    NaClLog(LOG_FATAL,
            "NaClPluginLowLevelInitializationComplete: no name service?!?\n");
  }

  nameservice_conn_desc = imc_connect(gNaClNameServiceConnCapDesc);
  if (-1 == nameservice_conn_desc) {
    NaClLog(LOG_FATAL,
            "Could not connect to bootstrap name service\n");
  }
  if (!NaClSrpcClientCtor(&srpc_channel, nameservice_conn_desc)) {
    (void) close(nameservice_conn_desc);
    NaClLog(LOG_FATAL, "SRPC channel ctor to name service failed\n");
  }
  if (NACL_SRPC_RESULT_OK != NaClSrpcInvokeBySignature(
          &srpc_channel,
          NACL_NAME_SERVICE_LOOKUP,
          "KernelService",
          O_RDWR,
          &status,
          &kern_service_conn_cap_desc)) {
    NaClSrpcDtor(&srpc_channel);
    NaClLog(LOG_FATAL, "Name service lookup RPC for KernelService failed\n");
  }
  NaClSrpcDtor(&srpc_channel);
  if (-1 == kern_service_conn_cap_desc) {
    NaClLog(LOG_FATAL, "Name service lookup for KernelService failed, %d\n",
            status);
  }
  if (-1 == (kern_service_desc = imc_connect(kern_service_conn_cap_desc))) {
    (void) close(kern_service_conn_cap_desc);
    NaClLog(LOG_FATAL, "Connect to KernelService failed\n");
  }
  (void) close(kern_service_conn_cap_desc);
  if (!NaClSrpcClientCtor(&srpc_channel, kern_service_desc)) {
    (void) close(kern_service_desc);
    NaClLog(LOG_FATAL, "SRPC channel ctor to KernelService failed\n");
  }
  if (NACL_SRPC_RESULT_OK != NaClSrpcInvokeBySignature(
          &srpc_channel,
          NACL_KERN_SERVICE_INITIALIZATION_COMPLETE)) {
    NaClLog(LOG_FATAL, "KernelService init_done RPC failed!\n");
  }
  NaClSrpcDtor(&srpc_channel);
}
