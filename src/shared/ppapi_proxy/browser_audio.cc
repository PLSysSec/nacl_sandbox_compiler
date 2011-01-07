// Copyright (c) 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "native_client/src/include/nacl_scoped_ptr.h"
#include "native_client/src/include/portability.h"
#include "native_client/src/shared/ppapi_proxy/browser_globals.h"
#include "native_client/src/shared/ppapi_proxy/browser_ppp.h"
#include "native_client/src/shared/ppapi_proxy/plugin_globals.h"
#include "native_client/src/trusted/desc/nacl_desc_invalid.h"
#include "native_client/src/trusted/desc/nacl_desc_wrapper.h"
#include "ppapi/c/dev/ppb_audio_config_dev.h"
#include "ppapi/c/dev/ppb_audio_dev.h"
#include "ppapi/c/dev/ppb_audio_trusted_dev.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/core.h"
#include "ppapi/cpp/module.h"
#include "srpcgen/ppb_rpc.h"
#include "srpcgen/ppp_rpc.h"

namespace {

const PPB_AudioTrusted_Dev* GetAudioTrustedInterface() {
  static const PPB_AudioTrusted_Dev* audioTrusted =
      static_cast<const PPB_AudioTrusted_Dev*>
          (ppapi_proxy::GetBrowserInterface(PPB_AUDIO_TRUSTED_DEV_INTERFACE));
  return audioTrusted;
}

const PPB_Audio_Dev* GetAudioInterface() {
  static const PPB_Audio_Dev* audio =
      static_cast<const PPB_Audio_Dev*>
          (ppapi_proxy::GetBrowserInterface(PPB_AUDIO_DEV_INTERFACE));
  return audio;
}

struct StreamCreatedCallbackData {
  PP_Instance instance_id;
  PP_Resource audio_id;
  StreamCreatedCallbackData(PP_Instance i, PP_Resource a) :
      instance_id(i),
      audio_id(a) { }
};

// This completion callback will be invoked when the sync socket and shared
// memory handles become available.
void StreamCreatedCallback(void* user_data, int32_t result) {
  if (NULL == user_data)
    return;
  nacl::scoped_ptr<StreamCreatedCallbackData> data(
      static_cast<StreamCreatedCallbackData*>(user_data));
  if (result < 0) {
    return;
  }
  const PPB_AudioTrusted_Dev* audioTrusted = GetAudioTrustedInterface();
  if (NULL == audioTrusted) {
    return;
  }
  int sync_socket_handle;
  int shared_memory_handle;
  uint32_t shared_memory_size;
  if (PP_OK != audioTrusted->GetSyncSocket(data->audio_id,
                                           &sync_socket_handle)) {
    return;
  }
  if (PP_OK != audioTrusted->GetSharedMemory(data->audio_id,
                                             &shared_memory_handle,
                                             &shared_memory_size)) {
    return;
  }
  nacl::DescWrapperFactory factory;
  NaClHandle nacl_shm_handle = (NaClHandle)shared_memory_handle;
  NaClHandle nacl_sync_handle = (NaClHandle)sync_socket_handle;
  nacl::scoped_ptr<nacl::DescWrapper> shm_wrapper(factory.ImportShmHandle(
      nacl_shm_handle, shared_memory_size));
  nacl::scoped_ptr<nacl::DescWrapper> socket_wrapper(
      factory.ImportSyncSocketHandle(nacl_sync_handle));
  NaClDesc *nacl_shm = NaClDescRef(shm_wrapper->desc());
  NaClDesc *nacl_socket = NaClDescRef(socket_wrapper->desc());
  int r;
  r = PppAudioDevRpcClient::PPP_Audio_Dev_StreamCreated(
      ppapi_proxy::GetMainSrpcChannel(data->instance_id),
      data->audio_id,
      nacl_shm,
      shared_memory_size,
      nacl_socket);
}

}  // namespace

void PpbAudioDevRpcServer::PPB_Audio_Dev_Create(
    NaClSrpcRpc* rpc,
    NaClSrpcClosure* done,
    PP_Instance instance,
    PP_Resource config,
    PP_Resource* resource) {
  NaClSrpcClosureRunner runner(done);
  const PPB_AudioTrusted_Dev* audio = GetAudioTrustedInterface();
  PP_Resource audio_id;
  PP_CompletionCallback callback;
  StreamCreatedCallbackData* data;
  int32_t r;
  rpc->result = NACL_SRPC_RESULT_APP_ERROR;
  if (ppapi_proxy::kInvalidResourceId == config)
    return;
  if (NULL == audio)
    return;
  *resource = audio->CreateTrusted(instance);
  audio_id = *resource;
  if (ppapi_proxy::kInvalidResourceId == audio_id)
    return;
  data = new StreamCreatedCallbackData(instance, audio_id);
  callback = PP_MakeCompletionCallback(StreamCreatedCallback, data);
  r = audio->Open(audio_id, config, callback);
  // if the Open() call failed, pass failure code and explicitly
  // invoke the completion callback, giving it a chance to release data.
  if ((r != PP_ERROR_WOULDBLOCK)) {
    PP_RunCompletionCallback(&callback, r);
    return;
  }
  rpc->result = NACL_SRPC_RESULT_OK;
}

void PpbAudioDevRpcServer::PPB_Audio_Dev_StartPlayback(
    NaClSrpcRpc* rpc,
    NaClSrpcClosure* done,
    PP_Resource resource,
    int32_t* out_bool) {
  NaClSrpcClosureRunner runner(done);
  const PPB_Audio_Dev* audio = GetAudioInterface();
  rpc->result = NACL_SRPC_RESULT_APP_ERROR;
  if (NULL == audio) {
    *out_bool = false;
    return;
  }
  *out_bool = static_cast<int32_t>(audio->StartPlayback(resource));
  rpc->result = NACL_SRPC_RESULT_OK;
}

void PpbAudioDevRpcServer::PPB_Audio_Dev_StopPlayback(
    NaClSrpcRpc* rpc,
    NaClSrpcClosure* done,
    PP_Resource resource,
    int32_t* out_bool) {
  NaClSrpcClosureRunner runner(done);
  const PPB_Audio_Dev* audio = GetAudioInterface();
  rpc->result = NACL_SRPC_RESULT_APP_ERROR;
  if (NULL == audio) {
    *out_bool = false;
    return;
  }
  *out_bool = static_cast<int32_t>(audio->StopPlayback(resource));
  rpc->result = NACL_SRPC_RESULT_OK;
}

void PpbAudioDevRpcServer::PPB_Audio_Dev_IsAudio(
    NaClSrpcRpc* rpc,
    NaClSrpcClosure* done,
    PP_Resource resource,
    int32_t* out_bool) {
  NaClSrpcClosureRunner runner(done);
  const PPB_Audio_Dev* audio = GetAudioInterface();
  rpc->result = NACL_SRPC_RESULT_APP_ERROR;
  if (NULL == audio) {
    *out_bool = false;
    return;
  }
  *out_bool = static_cast<int32_t>(audio->IsAudio(resource));
  rpc->result = NACL_SRPC_RESULT_OK;
}

void PpbAudioDevRpcServer::PPB_Audio_Dev_GetCurrentConfig(
    NaClSrpcRpc* rpc,
    NaClSrpcClosure* done,
    PP_Resource resource,
    PP_Resource* config) {
  NaClSrpcClosureRunner runner(done);
  const PPB_Audio_Dev* audio = GetAudioInterface();
  rpc->result = NACL_SRPC_RESULT_APP_ERROR;
  if (NULL == audio) {
    return;
  }
  if (ppapi_proxy::kInvalidResourceId == resource) {
    return;
  }
  *config = audio->GetCurrentConfig(resource);
  rpc->result = NACL_SRPC_RESULT_OK;
}
