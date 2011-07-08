// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/shared/ppapi_proxy/object_serialize.h"
#include "native_client/src/shared/ppapi_proxy/plugin_globals.h"
#include "native_client/src/third_party/ppapi/c/ppp.h"
#include "native_client/src/third_party/ppapi/c/ppp_messaging.h"
#include "srpcgen/ppp_rpc.h"

using ppapi_proxy::DeserializeTo;
using ppapi_proxy::PPPMessagingInterface;

void PppMessagingRpcServer::PPP_Messaging_HandleMessage(
      NaClSrpcRpc* rpc,
      NaClSrpcClosure* done,
      PP_Instance instance,
      nacl_abi_size_t message_size, char* message_bytes) {
  rpc->result = NACL_SRPC_RESULT_APP_ERROR;
  NaClSrpcClosureRunner runner(done);

  const PPP_Messaging* ppp_messaging = PPPMessagingInterface();
  PP_Var message;
  if (!DeserializeTo(rpc->channel, message_bytes, message_size, 1, &message))
    return;
  ppp_messaging->HandleMessage(instance, message);
  rpc->result = NACL_SRPC_RESULT_OK;
}

