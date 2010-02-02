// Copyright (c) 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "native_client/src/include/portability.h"
#include "native_client/src/include/checked_cast.h"
#include "native_client/src/shared/npruntime/npmodule.h"

#include "gen/native_client/src/shared/npruntime/npmodule_rpc.h"
#include "gen/native_client/src/shared/npruntime/npnavigator_rpc.h"
#include "gpu/command_buffer/common/command_buffer.h"
#include "native_client/src/include/portability_io.h"
#include "native_client/src/include/portability_process.h"
#include "native_client/src/shared/npruntime/nacl_npapi.h"
#include "native_client/src/shared/npruntime/npobject_proxy.h"
#include "native_client/src/shared/npruntime/npobject_stub.h"
#include "native_client/src/shared/platform/nacl_threads.h"
#include "native_client/src/trusted/desc/nacl_desc_invalid.h"
#include "native_client/src/trusted/desc/nacl_desc_wrapper.h"
#include "native_client/src/trusted/plugin/origin.h"
#include "third_party/npapi/bindings/npapi_extensions.h"
#ifndef NACL_STANDALONE
#include "base/shared_memory.h"
#endif  // NACL_STANDALONE

using nacl::NPBridge;
using nacl::NPModule;
using nacl::RpcArg;

NaClSrpcError NPModuleRpcServer::NPN_GetValue(NaClSrpcChannel* channel,
                                              int32_t int_npp,
                                              int32_t var,
                                              int32_t* nperr,
                                              nacl_abi_size_t* result_bytes,
                                              char* result) {
  UNREFERENCED_PARAMETER(channel);
  NPNVariable variable = static_cast<NPNVariable>(var);
  NPObject* object;
  NPP npp = NPBridge::IntToNpp(int_npp);

  *nperr = ::NPN_GetValue(npp, variable, &object);
  nacl::RpcArg ret1(npp, result, *result_bytes);
  ret1.PutObject(object);

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModuleRpcServer::NPN_SetStatus(NaClSrpcChannel* channel,
                                               int32_t int_npp,
                                               char* status) {
  UNREFERENCED_PARAMETER(channel);

  if (NULL != status) {
    ::NPN_Status(NPBridge::IntToNpp(int_npp), status);
  }
  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModuleRpcServer::NPN_InvalidateRect(NaClSrpcChannel* channel,
                                                    int32_t int_npp,
                                                    nacl_abi_size_t rect_bytes,
                                                    char* rect) {
  UNREFERENCED_PARAMETER(channel);
  NPP npp = NPBridge::IntToNpp(int_npp);
  NPModule* module = NPModule::GetModule(int_npp);
  RpcArg arg1(npp, rect, rect_bytes);

  module->InvalidateRect(npp, arg1.GetRect());

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModuleRpcServer::NPN_ForceRedraw(NaClSrpcChannel* channel,
                                                 int32_t int_npp) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  module->ForceRedraw(NPBridge::IntToNpp(int_npp));

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModuleRpcServer::NPN_CreateArray(NaClSrpcChannel* channel,
                                                 int32_t int_npp,
                                                 int32_t* success,
                                                 nacl_abi_size_t* cap_bytes,
                                                 char* capability) {
  UNREFERENCED_PARAMETER(channel);
  NPP npp = NPBridge::IntToNpp(int_npp);
  NPObject* window;

  if (NPERR_NO_ERROR != ::NPN_GetValue(npp, NPNVWindowNPObject, &window)) {
    *success = 0;
    return NACL_SRPC_RESULT_OK;
  }
  NPString script;
  const char scriptText[] = "new Array();";
  script.UTF8Characters = scriptText;
  script.UTF8Length = nacl::assert_cast<uint32>(strlen(scriptText));
  NPVariant result;
  *success = ::NPN_Evaluate(npp, window, &script, &result) &&
             NPVARIANT_IS_OBJECT(result);
  if (*success) {
    RpcArg ret0(npp, capability, *cap_bytes);
    ret0.PutObject(NPVARIANT_TO_OBJECT(result));
    // TODO(sehr): We're leaking result here.
  }
  NPN_ReleaseObject(window);

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModuleRpcServer::NPN_OpenURL(NaClSrpcChannel* channel,
                                             int32_t int_npp,
                                             char* url,
                                             int32_t* nperr) {
  UNREFERENCED_PARAMETER(channel);

  if (NULL == url) {
    *nperr = NPERR_GENERIC_ERROR;
  } else {
    *nperr = ::NPN_GetURLNotify(NPBridge::IntToNpp(int_npp), url, NULL, NULL);
  }
  if (*nperr == NPERR_NO_ERROR) {
    // NPP_NewStream, NPP_DestroyStream, and NPP_URLNotify will be invoked
    // later.
  }

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModuleRpcServer::NPN_GetIntIdentifier(NaClSrpcChannel* channel,
                                                      int32_t intval,
                                                      int32_t* id) {
  UNREFERENCED_PARAMETER(channel);
  NPIdentifier identifier;

  if (NPModule::IsWebkit()) {
    // Webkit needs to look up integer IDs as strings.
    char index[11];
    SNPRINTF(index, sizeof(index), "%u", static_cast<unsigned>(intval));
    identifier = ::NPN_GetStringIdentifier(index);
  } else {
    identifier = ::NPN_GetIntIdentifier(intval);
  }
  *id = NPBridge::NpidentifierToInt(identifier);

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModuleRpcServer::NPN_IntFromIdentifier(NaClSrpcChannel* channel,
                                                       int32_t id,
                                                       int32_t* intval) {
  UNREFERENCED_PARAMETER(channel);

  *intval = ::NPN_IntFromIdentifier(NPBridge::IntToNpidentifier(id));

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModuleRpcServer::NPN_GetStringIdentifier(
    NaClSrpcChannel* channel,
    char* strval,
    int32_t* id) {
  UNREFERENCED_PARAMETER(channel);

  *id = NPBridge::NpidentifierToInt(::NPN_GetStringIdentifier(strval));

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModuleRpcServer::NPN_IdentifierIsString(
    NaClSrpcChannel* channel,
    int32_t id,
    int32_t* isstring) {
  UNREFERENCED_PARAMETER(channel);

  *isstring = ::NPN_IdentifierIsString(NPBridge::IntToNpidentifier(id));

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModuleRpcServer::NPN_UTF8FromIdentifier(
    NaClSrpcChannel* channel,
    int32_t id,
    int32_t* success,
    char** str) {
  UNREFERENCED_PARAMETER(channel);

  char* name = ::NPN_UTF8FromIdentifier(NPBridge::IntToNpidentifier(id));
  if (NULL == name) {
    *success = NPERR_GENERIC_ERROR;
    *str = strdup("");
  } else {
    *success = NPERR_NO_ERROR;
    // Need to use NPN_MemFree on returned value, whereas srpc will do free().
    *str = strdup(name);
    NPN_MemFree(name);
  }

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError Device2DRpcServer::Device2DInitialize(
    NaClSrpcChannel* channel,
    int32_t int_npp,
    NaClSrpcImcDescType* shm_desc,
    int32_t* stride,
    int32_t* left,
    int32_t* top,
    int32_t* right,
    int32_t* bottom) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  return module->Device2DInitialize(NPBridge::IntToNpp(int_npp),
                                    shm_desc,
                                    stride,
                                    left,
                                    top,
                                    right,
                                    bottom);
}

NaClSrpcError Device2DRpcServer::Device2DFlush(NaClSrpcChannel* channel,
                                               int32_t int_npp,
                                               int32_t* stride,
                                               int32_t* left,
                                               int32_t* top,
                                               int32_t* right,
                                               int32_t* bottom) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  return module->Device2DFlush(NPBridge::IntToNpp(int_npp),
                               stride,
                               left,
                               top,
                               right,
                               bottom);
}

NaClSrpcError Device2DRpcServer::Device2DDestroy(NaClSrpcChannel* channel,
                                                 int32_t int_npp) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  return module->Device2DDestroy(NPBridge::IntToNpp(int_npp));
}

NaClSrpcError Device3DRpcServer::Device3DInitialize(
    NaClSrpcChannel* channel,
    int32_t int_npp,
    int32_t entries_requested,
    NaClSrpcImcDescType* shm_desc,
    int32_t* entries_obtained,
    int32_t* get_offset,
    int32_t* put_offset) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  return module->Device3DInitialize(NPBridge::IntToNpp(int_npp),
                                    entries_requested,
                                    shm_desc,
                                    entries_obtained,
                                    get_offset,
                                    put_offset);
}

NaClSrpcError Device3DRpcServer::Device3DFlush(NaClSrpcChannel* channel,
                                               int32_t int_npp,
                                               int32_t put_offset,
                                               int32_t* get_offset) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  return module->Device3DFlush(NPBridge::IntToNpp(int_npp),
                               put_offset,
                               get_offset);
}

NaClSrpcError Device3DRpcServer::Device3DDestroy(NaClSrpcChannel* channel,
                                                 int32_t int_npp) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  return module->Device3DDestroy(NPBridge::IntToNpp(int_npp));
}

NaClSrpcError Device3DRpcServer::Device3DGetState(NaClSrpcChannel* channel,
                                                  int32_t int_npp,
                                                  int32_t state,
                                                  int32_t* value) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  return module->Device3DGetState(NPBridge::IntToNpp(int_npp), state, value);
}

NaClSrpcError Device3DRpcServer::Device3DSetState(NaClSrpcChannel* channel,
                                                  int32_t int_npp,
                                                  int32_t state,
                                                  int32_t value) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  return module->Device3DSetState(NPBridge::IntToNpp(int_npp), state, value);
}

NaClSrpcError Device3DRpcServer::Device3DCreateBuffer(
    NaClSrpcChannel* channel,
    int32_t int_npp,
    int32_t size,
    NaClSrpcImcDescType* shm_desc,
    int32_t* id) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  return module->Device3DCreateBuffer(NPBridge::IntToNpp(int_npp),
                                      size,
                                      shm_desc,
                                      id);
}

NaClSrpcError Device3DRpcServer::Device3DDestroyBuffer(NaClSrpcChannel* channel,
                                                       int32_t int_npp,
                                                       int32_t id) {
  UNREFERENCED_PARAMETER(channel);
  NPModule* module = nacl::NPModule::GetModule(int_npp);

  return module->Device3DDestroyBuffer(NPBridge::IntToNpp(int_npp), id);
}
