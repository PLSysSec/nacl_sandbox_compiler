/*
 * Copyright 2010 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_SHARED_PPAPI_PROXY_PLUGIN_AUDIO_H_
#define NATIVE_CLIENT_SRC_SHARED_PPAPI_PROXY_PLUGIN_AUDIO_H_

#include <nacl/nacl_srpc.h>
#include <pthread.h>
#include "native_client/src/include/nacl_base.h"
#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/include/portability.h"
#include "native_client/src/include/ref_counted.h"
#include "native_client/src/shared/ppapi_proxy/plugin_resource.h"
#include "ppapi/c/dev/ppb_audio_dev.h"
#include "ppapi/c/pp_resource.h"

namespace ppapi_proxy {

enum PluginAudioState {
  AUDIO_INCOMPLETE = 0,  // StreamCreated not yet invoked
  AUDIO_PENDING,         // Incomplete and app requested StartPlayback
  AUDIO_READY,           // StreamCreated invoked, ready for playback
  AUDIO_PLAYING          // Audio in playback
};

// Implements the plugin (i.e., .nexe) side of the PPB_Audio interface.
// All methods in PluginAudio class will be invoked on the main thread.
// The only exception is AudioThread(), which will invoke the application
// supplied callback to periodically obtain more audio data.
class PluginAudio : public PluginResource {
 public:
  PluginAudio();
  virtual ~PluginAudio();
  void StreamCreated(NaClSrpcImcDescType socket,
      NaClSrpcImcDescType shm, size_t shm_size);
  void set_state(PluginAudioState state) { state_ = state; }
  void set_callback(PPB_Audio_Callback user_callback, void* user_data) {
      user_callback_ = user_callback;
      user_data_ = user_data;
  }
  PluginAudioState state() { return state_; }
  bool StartAudioThread();
  bool StopAudioThread();
  static void* AudioThread(void* self);
  static const PPB_Audio_Dev* GetInterface();
  virtual bool InitFromBrowserResource(PP_Resource resource);
 private:
  NaClSrpcImcDescType socket_;
  NaClSrpcImcDescType shm_;
  size_t shm_size_;
  void *shm_buffer_;
  PluginAudioState state_;
  pthread_t thread_id_;
  PPB_Audio_Callback user_callback_;
  void* user_data_;
  IMPLEMENT_RESOURCE(PluginAudio);
  NACL_DISALLOW_COPY_AND_ASSIGN(PluginAudio);
};

}  // namespace ppapi_proxy

#endif  // NATIVE_CLIENT_SRC_SHARED_PPAPI_PROXY_PLUGIN_AUDIO_H_
