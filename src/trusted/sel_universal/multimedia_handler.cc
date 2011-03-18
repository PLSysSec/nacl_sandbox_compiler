/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

// This file exports a single function used to setup the
// multimedia sub-system for use with sel_universal
// It was inpspired by src/trusted/plugin/srpc/multimedia_socket.cc
// On the untrusted side it interface with: src/untrusted/av/nacl_av.c
//
// NOTE: this is experimentation and testing. We are not concerned
//       about descriptor and memory leaks

#if (NACL_LINUX)
// for shmem
#include <sys/ipc.h>
#include <sys/shm.h>
#include "native_client/src/trusted/desc/linux/nacl_desc_sysv_shm.h"
// for sync_sockets
#include <sys/socket.h>
#endif

#include <assert.h>
#include <string.h>

#include <string>

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_input_event.h"
#include "ppapi/c/pp_size.h"
#include "ppapi/c/ppb_audio.h"
#include "ppapi/c/ppb_audio_config.h"
#include "ppapi/c/ppb_image_data.h"

#include "native_client/src/shared/imc/nacl_imc.h"
#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"
#include "native_client/src/trusted/desc/nacl_desc_sync_socket.h"
#include "native_client/src/trusted/desc/nacl_desc_wrapper.h"

#include "native_client/src/trusted/sel_universal/rpc_universal.h"
#include "native_client/src/trusted/sel_universal/multimedia.h"
#include "native_client/src/trusted/sel_universal/parsing.h"

using nacl::DescWrapperFactory;
using nacl::DescWrapper;

#define SRPC_PARAMS NaClSrpcRpc* rpc, \
                    NaClSrpcArg** ins, \
                    NaClSrpcArg** outs, \
                    NaClSrpcClosure* done

// ======================================================================
const int kInvalidInstance = 0;
const int kInvalidHandle = 0;
const int kFirstImageDataHandle = 100;
const int kFirstGraphicsHandle = 200;
const int kFirstAudioHandle = 300;
const int kFirstAudioConfigHandle = 400;
const int kBytesPerSample = 4; // 16bit stereo

const int kRecommendSampleFrameCount = 2048;
const int kMaxAudioBufferSize = 0x10000;
const int kBytesPerPixel = 4;

const int MY_EVENT_FLUSH_CALL_BACK = 88;
const int MY_EVENT_INIT_AUDIO = 89;
const int MY_EVENT_AUDIO_CALLBACK = 90;
// ======================================================================

// Note: Just a bunch of fairly unrelated global variables,
// we expect them to be zero initialized.
static struct {
  int instance;

  // video stuff
  int screen_width;
  int screen_height;

  int handle_graphics;
  int handle_image_data;
  int image_data_size;

  nacl::DescWrapper* desc_video_shmem;
  void* addr_video;

  // audio stuff
  int handle_audio;
  int handle_audio_config;

  int sample_frequency;
  int sample_frame_count;

  nacl::DescWrapper* desc_audio_shmem;
  nacl::DescWrapper* desc_audio_sync_in;
  nacl::DescWrapper* desc_audio_sync_out;
  void* addr_audio;

  IMultimedia* sdl_engine;
  std::string title;
} Global;

// ======================================================================
static void AudioCallBack(void* data, unsigned char* buffer, int length) {
  NaClLog(2, "AudioCallBack(%p, %p, %d)\n", data, buffer, length);
  UNREFERENCED_PARAMETER(data);
  CHECK(length == Global.sample_frame_count * kBytesPerSample);
  // NOTE: we copy the previously filled buffer.
  //       This introduces extra latency but simplifies the design
  //       as we do not have to wait for the nexe to generate the data.
  memcpy(buffer, Global.addr_audio, length);

  // ping sync socket
  int value = 0;
  Global.desc_audio_sync_in->Write(&value, sizeof value);
}
// ======================================================================
// NOTE: these are not fully supported at this time
//       they undoubtedly need to be updated when ppapi changes
static bool IsSupportedInterface(string interface) {
  return
    interface == "PPB_Audio;0.5" ||
    interface == "PPB_AudioConfig;0.5" ||
    interface == "PPB_Core;0.3" ||
    interface == "PPB_FileIO(Dev);0.3" ||
    interface == "PPB_Graphics2D;0.3" ||
    interface == "PPB_ImageData;0.3" ||
    interface == "PPB_Instance;0.4" ||
    interface == "PPB_URLLoader;0.1" ||
    interface == "PPB_URLRequestInfo;0.2" ||
    interface == "PPB_URLResponseInfo;0.1";
}

// void* PPB_GetInterface(const char* interface_name);
// PPB_GetInterface:s:i
static void PPB_GetInterface(SRPC_PARAMS) {
  string interface(ins[0]->arrays.str);
  NaClLog(LOG_INFO, "PPB_GetInterface(%s)\n", interface.c_str());
  bool supported = IsSupportedInterface(interface);
  if (!supported) {
    NaClLog(LOG_ERROR, "unsupported interface\n");
  }
  outs[0]->u.ival = supported ? 1 : 0;

  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the Core API
// void ReleaseResource(PP_Resource resource);
// PPB_Core_ReleaseResource:i:
static void PPB_Core_ReleaseResource(SRPC_PARAMS) {
  UNREFERENCED_PARAMETER(ins);
  UNREFERENCED_PARAMETER(outs);
  NaClLog(LOG_INFO, "PPB_Core_ReleaseResource\n");
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// This appears to have no equivalent in the ppapi world
// ReleaseResourceMultipleTimes:ii:
static void ReleaseResourceMultipleTimes(SRPC_PARAMS) {
  UNREFERENCED_PARAMETER(outs);
  NaClLog(LOG_INFO, "ReleaseResourceMultipleTimes(%d, %d)\n",
          ins[0]->u.ival, ins[1]->u.ival);
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the ImageData API
// PP_Resource Create(PP_Instance instance,
//                    PP_ImageDataFormat format,
//                    const struct PP_Size* size,
//                    PP_Bool init_to_zero);
// PPB_ImageData_Create:iiCi:i
//
// TODO(robertm) this function can currently be called only once
//               and the dimension must match the global values
//               and the format is fixed.
static void PPB_ImageData_Create(SRPC_PARAMS) {
  const int instance = ins[0]->u.ival;
  const int format = ins[1]->u.ival;
  CHECK(ins[2]->u.count == sizeof(PP_Size));
  PP_Size* img_size = (PP_Size*) ins[2]->arrays.carr;
  NaClLog(LOG_INFO, "PPB_ImageData_Create(%d, %d, %d, %d)\n",
          instance, format, img_size->width, img_size->height);

  CHECK(Global.handle_image_data == kInvalidHandle);
  Global.handle_image_data = kFirstImageDataHandle;
  CHECK(Global.instance != kInvalidInstance);
  CHECK(instance == Global.instance);
  CHECK(format == PP_IMAGEDATAFORMAT_BGRA_PREMUL);

  CHECK(Global.screen_width == img_size->width);
  CHECK(Global.screen_height == img_size->height);
  Global.image_data_size = kBytesPerPixel * img_size->width * img_size->height;


  nacl::DescWrapperFactory factory;
  Global.desc_video_shmem = factory.MakeShm(Global.image_data_size);
  size_t dummy_size;

  if (Global.desc_video_shmem->Map(&Global.addr_video, &dummy_size)) {
    NaClLog(LOG_FATAL, "cannot map video shmem\n");
  }

  if (ins[3]->u.ival) {
    memset(Global.addr_video, 0, Global.image_data_size);
  }

  outs[0]->u.ival = Global.handle_image_data;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the ImageData API
// PP_Bool Describe(PP_Resource image_data,
//                  struct PP_ImageDataDesc* desc);
// PPB_ImageData_Describe:i:Chii
static void PPB_ImageData_Describe(SRPC_PARAMS) {
  int handle = ins[0]->u.ival;
  NaClLog(LOG_INFO, "PPB_ImageData_Describe(%d)\n", handle);
  CHECK(handle == Global.handle_image_data);

  PP_ImageDataDesc d;
  d.format = PP_IMAGEDATAFORMAT_BGRA_PREMUL;
  d.size.width =  Global.screen_width;
  d.size.height = Global.screen_height;
  // we handle only rgba data -> each pixel is 4 bytes.
  d.stride = Global.screen_width * kBytesPerPixel;
  outs[0]->u.count = sizeof(d);
  outs[0]->arrays.carr = reinterpret_cast<char*>(calloc(1, sizeof(d)));
  memcpy(outs[0]->arrays.carr, &d, sizeof(d));

  outs[1]->u.hval = Global.desc_video_shmem->desc();
  outs[2]->u.ival = Global.image_data_size;
  outs[3]->u.ival = 1;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the Graphics2D API
// PP_Resource Create(PP_Instance instance,
//                    const struct PP_Size* size,
//                    PP_Bool is_always_opaque);
// PPB_Graphics2D_Create:iCi:i
//
// TODO(robertm) This function can currently be called only once
//               The size must be the same as the one provided via
//                HandlerSDLInitialize()
static void PPB_Graphics2D_Create(SRPC_PARAMS) {
  int instance = ins[0]->u.ival;
  NaClLog(LOG_INFO, "PPB_Graphics2D_Create(%d)\n", instance);
  CHECK(Global.handle_graphics == kInvalidHandle);
  Global.handle_graphics = kFirstGraphicsHandle;
  CHECK(instance == Global.instance);
  PP_Size* img_size = reinterpret_cast<PP_Size*>(ins[1]->arrays.carr);
  CHECK(Global.screen_width == img_size->width);
  CHECK(Global.screen_height == img_size->height);
  // TODO(robertm):  is_always_opaque is currently ignored
  outs[0]->u.ival = Global.handle_graphics;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// PP_Bool BindGraphics(PP_Instance instance, PP_Resource device);
// PPB_Instance_BindGraphics:ii:i
static void PPB_Instance_BindGraphics(SRPC_PARAMS) {
  int instance = ins[0]->u.ival;
  int handle = ins[1]->u.ival;
  NaClLog(LOG_INFO, "PPB_Instance_BindGraphics(%d, %d)\n",
          instance, handle);
  CHECK(instance == Global.instance);
  CHECK(handle == Global.handle_graphics);
  outs[0]->u.ival = 1;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the Graphics2D API
// void ReplaceContents(PP_Resource graphics_2d,
//                      PP_Resource image_data);
// PPB_Graphics2D_ReplaceContents:ii:
//
// NOTE: this is completely ignored and we postpone all action to "Flush"
static void PPB_Graphics2D_ReplaceContents(SRPC_PARAMS) {
  int handle_graphics = ins[0]->u.ival;
  int handle_image_data = ins[1]->u.ival;
  UNREFERENCED_PARAMETER(outs);
  NaClLog(LOG_INFO, "PPB_Graphics2D_ReplaceContents(%d, %d)\n",
          handle_graphics, handle_image_data);
  CHECK(handle_graphics == Global.handle_graphics);
  CHECK(handle_image_data == Global.handle_image_data);

  // For now assume this will be immediately followed by a Flush
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the Graphics2D API
// void PaintImageData(PP_Resource graphics_2d,
//                     PP_Resource image_data,
//                     const struct PP_Point* top_left,
//                     const struct PP_Rect* src_rect);
// PPB_Graphics2D_PaintImageData:iiCC:
//
// NOTE: this is completely ignored and we postpone all action to "Flush"
//       Furhermore we assume that entire image is painted
static void PPB_Graphics2D_PaintImageData(SRPC_PARAMS) {
  int handle_graphics = ins[0]->u.ival;
  int handle_image_data = ins[1]->u.ival;
  UNREFERENCED_PARAMETER(outs);
  NaClLog(LOG_INFO, "PPB_Graphics2D_PaintImageData(%d, %d)\n",
          handle_graphics, handle_image_data);
  CHECK(handle_graphics == Global.handle_graphics);
  CHECK(handle_image_data == Global.handle_image_data);

  // For now assume this will be immediately followed by a Flush
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the Graphics2D API
// int32_t Flush(PP_Resource graphics_2d,
//               struct PP_CompletionCallback callback);
// PPB_Graphics2D_Flush:ii:i
static void PPB_Graphics2D_Flush(SRPC_PARAMS) {
  int handle = ins[0]->u.ival;
  int callback_id = ins[1]->u.ival;
  NaClLog(LOG_INFO, "PPB_Graphics2D_Flush(%d, %d)\n", handle, callback_id);
  CHECK(handle == Global.handle_graphics);
  outs[0]->u.ival = -1;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);

  Global.sdl_engine->VideoUpdate(Global.addr_video);
  NaClLog(LOG_INFO, "pushing user event for callback (%d)\n", callback_id);
  Global.sdl_engine->PushUserEvent(MY_EVENT_FLUSH_CALL_BACK, callback_id);
}

// From the PPB_Audio API
// PP_Resource Create(PP_Instance instance, PP_Resource config,
//                    PPB_Audio_Callback audio_callback, void* user_data);
// PPB_Audio_Create:ii:i
static void PPB_Audio_Create(SRPC_PARAMS) {
  int instance = ins[0]->u.ival;
  int handle = ins[1]->u.ival;
  NaClLog(LOG_INFO, "PPB_Audio_Create(%d, %d)\n", instance, handle);
  CHECK(instance == Global.instance);
  CHECK(handle == Global.handle_audio_config);

  nacl::DescWrapperFactory factory;
#if (NACL_WINDOWS || NACL_OSX)
  NaClLog(LOG_ERROR, "HandlerSyncSocketCreate NYI for windows/mac\n");
#else
  nacl::Handle handles[2] = {nacl::kInvalidHandle, nacl::kInvalidHandle};
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, handles) != 0) {
    NaClLog(LOG_FATAL, "cannot create syn sockets\n");
  }
  Global.desc_audio_sync_in = factory.ImportSyncSocketHandle(handles[0]);
  Global.desc_audio_sync_out = factory.ImportSyncSocketHandle(handles[1]);
#endif
  Global.desc_audio_shmem = factory.MakeShm(kMaxAudioBufferSize);
  size_t dummy_size;

  if (Global.desc_audio_shmem->Map(&Global.addr_audio, &dummy_size)) {
    NaClLog(LOG_FATAL, "cannot map audio shmem\n");
  }
  NaClLog(LOG_INFO, "PPB_Audio_Create: buffer is %p\n", Global.addr_audio);

  Global.sdl_engine->AudioInit16Bit(Global.sample_frequency,
                                    2,
                                    Global.sample_frame_count,
                                    &AudioCallBack);

  CHECK(Global.handle_audio == kInvalidHandle);
  Global.handle_audio = kFirstAudioHandle;
  outs[0]->u.ival = Global.handle_audio;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);

  Global.sdl_engine->PushUserEvent(MY_EVENT_INIT_AUDIO, 0);
}

// From the PPB_Audio API
// PP_Bool IsAudio(PP_Resource resource)
// PPB_Audio_IsAudio:i:i
static void PPB_Audio_IsAudio(SRPC_PARAMS) {
  int handle = ins[0]->u.ival;
  NaClLog(LOG_INFO, "PPB_Audio_IsAudio(%d)\n", handle);
  CHECK(handle == Global.handle_audio);

  outs[0]->u.ival = 1;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the PPB_Audio API
// PP_Resource GetCurrentConfig(PP_Resource audio);
// PPB_Audio_GetCurrentConfig:i:i
static void PPB_Audio_GetCurrentConfig(SRPC_PARAMS) {
  int handle = ins[0]->u.ival;
  NaClLog(LOG_INFO, "PPB_Audio_GetCurrentConfig(%d)\n", handle);
  CHECK(handle == Global.handle_audio);
  CHECK(Global.handle_audio_config != kInvalidHandle);

  outs[0]->u.ival = Global.handle_audio_config;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the PPB_Audio API
// PP_Bool StartPlayback(PP_Resource audio);
// PPB_Audio_StopPlayback:i:i
static void PPB_Audio_StopPlayback(SRPC_PARAMS) {
  int handle = ins[0]->u.ival;
  NaClLog(LOG_INFO, "PPB_Audio_StopPlayback(%d)\n", handle);
  CHECK(handle == Global.handle_audio);
  Global.sdl_engine->AudioStop();

  outs[0]->u.ival = 1;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the PPB_Audio API
// PP_Bool StopPlayback(PP_Resource audio);
// PPB_Audio_StartPlayback:i:i
static void PPB_Audio_StartPlayback(SRPC_PARAMS) {
  int handle = ins[0]->u.ival;
  NaClLog(LOG_INFO, "PPB_Audio_StartPlayback(%d)\n", handle);
  CHECK(handle == Global.handle_audio);
  Global.sdl_engine->AudioStart();

  outs[0]->u.ival = 1;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// From the PPB_AudioConfig API
// PP_Resource CreateStereo16Bit(PP_Instance instance,
//                               PP_AudioSampleRate sample_rate,
//                               uint32_t sample_frame_count);
// PPB_AudioConfig_CreateStereo16Bit:iii:i
static void PPB_AudioConfig_CreateStereo16Bit(SRPC_PARAMS) {
  int instance = ins[0]->u.ival;
  Global.sample_frequency = ins[1]->u.ival;
  Global.sample_frame_count = ins[2]->u.ival;
  NaClLog(LOG_INFO, "PPB_AudioConfig_CreateStereo16Bit(%d, %d, %d)\n",
          instance, Global.sample_frequency, Global.sample_frame_count);
  CHECK(instance == Global.instance);
  CHECK(Global.sample_frame_count * kBytesPerSample < kMaxAudioBufferSize);
  CHECK(Global.handle_audio_config == kInvalidHandle);
  Global.handle_audio_config = kFirstAudioConfigHandle;
  outs[0]->u.ival = Global.handle_audio_config;

  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// PPB_AudioConfig_IsAudioConfig:i:i
// PP_Bool IsAudioConfig(PP_Resource resource);
static void PPB_AudioConfig_IsAudioConfig(SRPC_PARAMS) {
  int handle = ins[0]->u.ival;
  NaClLog(LOG_INFO, "PPB_AudioConfig_IsAudioConfig(%d)\n", handle);
  outs[0]->u.ival = (handle == Global.handle_audio_config);
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// PPB_AudioConfig_RecommendSampleFrameCount:ii:i
// uint32_t RecommendSampleFrameCount(PP_AudioSampleRate sample_rate,
//                                    uint32_t requested_sample_frame_count);
static void PPB_AudioConfig_RecommendSampleFrameCount(SRPC_PARAMS) {
  int sample_frequency = ins[0]->u.ival;
  int sample_frame_count = ins[1]->u.ival;
  NaClLog(LOG_INFO, "PPB_AudioConfig_RecommendSampleFrameCount(%d, %d)\n",
          sample_frequency, sample_frame_count);
  // This is clearly imperfect.
  // TODO(robertm): Consider using SDL's negotiation mechanism here
  outs[0]->u.ival = kRecommendSampleFrameCount;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// PPB_AudioConfig_GetSampleRate:i:i
// PP_AudioSampleRate GetSampleRate(PP_Resource config);
static void PPB_AudioConfig_GetSampleRate(SRPC_PARAMS) {
  int handle = ins[0]->u.ival;
  NaClLog(LOG_INFO, "PPB_AudioConfig_GetSampleRate(%d)\n", handle);
  CHECK(handle == Global.handle_audio_config);

  outs[0]->u.ival = Global.sample_frequency;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}

// PPB_AudioConfig_GetSampleFrameCount:i:i
// uint32_t GetSampleFrameCount(PP_Resource config);
static void PPB_AudioConfig_GetSampleFrameCount(SRPC_PARAMS) {
  int handle = ins[0]->u.ival;
  NaClLog(LOG_INFO, "PPB_AudioConfig_GetSampleFrameCount(%d)\n", handle);
  CHECK(handle == Global.handle_audio_config);

  outs[0]->u.ival = Global.sample_frame_count;
  rpc->result = NACL_SRPC_RESULT_OK;
  done->Run(done);
}


#define TUPLE(a, b) #a #b, a
bool HandlerSDLInitialize(NaClCommandLoop* ncl, const vector<string>& args) {
  NaClLog(LOG_INFO, "HandlerSDLInitialize\n");
  if (args.size() < 5) {
    NaClLog(LOG_ERROR, "Insufficient arguments to 'rpc' command.\n");
    return false;
  }

  UNREFERENCED_PARAMETER(ncl);
  ncl->AddUpcallRpc(TUPLE(PPB_ImageData_Describe, :i:Chii));
  ncl->AddUpcallRpc(TUPLE(PPB_ImageData_Create, :iiCi:i));
  ncl->AddUpcallRpc(TUPLE(PPB_Instance_BindGraphics, :ii:i));
  ncl->AddUpcallRpc(TUPLE(PPB_Graphics2D_Create, :iCi:i));
  ncl->AddUpcallRpc(TUPLE(PPB_Graphics2D_ReplaceContents, :ii:));
  ncl->AddUpcallRpc(TUPLE(PPB_Graphics2D_PaintImageData, :iiCC:));
  ncl->AddUpcallRpc(TUPLE(PPB_Graphics2D_Flush, :ii:i));
  ncl->AddUpcallRpc(TUPLE(PPB_GetInterface, :s:i));
  ncl->AddUpcallRpc(TUPLE(PPB_Core_ReleaseResource, :i:));
  ncl->AddUpcallRpc(TUPLE(ReleaseResourceMultipleTimes, :ii:));
  ncl->AddUpcallRpc(TUPLE(PPB_Audio_Create, :ii:i));
  ncl->AddUpcallRpc(TUPLE(PPB_Audio_IsAudio, :i:i));
  ncl->AddUpcallRpc(TUPLE(PPB_Audio_GetCurrentConfig, :i:i));
  ncl->AddUpcallRpc(TUPLE(PPB_Audio_StopPlayback, :i:i));
  ncl->AddUpcallRpc(TUPLE(PPB_Audio_StartPlayback, :i:i));

  ncl->AddUpcallRpc(TUPLE(PPB_AudioConfig_CreateStereo16Bit, :iii:i));
  ncl->AddUpcallRpc(TUPLE(PPB_AudioConfig_IsAudioConfig, :i:i));
  ncl->AddUpcallRpc(TUPLE(PPB_AudioConfig_RecommendSampleFrameCount, :ii:i));
  ncl->AddUpcallRpc(TUPLE(PPB_AudioConfig_GetSampleRate, :i:i));
  ncl->AddUpcallRpc(TUPLE(PPB_AudioConfig_GetSampleFrameCount, :i:i));

  Global.instance = ExtractInt32(args[1]);
  Global.screen_width = ExtractInt32(args[2]);
  Global.screen_height = ExtractInt32(args[3]);
  Global.title = args[4];

  Global.sdl_engine = MakeMultimediaSDL(Global.screen_width,
                                        Global.screen_height,
                                        Global.title.c_str());
  return true;
}

// uncomment the line below if you want to use a non-blocking
// event processing loop.
// This can be sometime useful for debugging but is wasting cycles
// #define USE_POLLING

bool HandlerSDLEventLoop(NaClCommandLoop* ncl, const vector<string>& args) {
  NaClLog(LOG_INFO, "HandlerSDLEventLoop\n");
  UNREFERENCED_PARAMETER(args);
  UNREFERENCED_PARAMETER(ncl);
  PP_InputEvent event;
  while (true) {
    NaClLog(LOG_INFO, "event wait\n");
#if defined(USE_POLLING)
    Global.sdl_engine->EventPoll(&event);
    if (IsInvalidEvent(&event)) continue;
#else
    Global.sdl_engine->EventGet(&event);
#endif

    NaClLog(LOG_INFO, "Got event of type %d\n", event.type);

    NaClSrpcArg  in[NACL_SRPC_MAX_ARGS];
    NaClSrpcArg* ins[NACL_SRPC_MAX_ARGS + 1];
    NaClSrpcArg  out[NACL_SRPC_MAX_ARGS];
    NaClSrpcArg* outs[NACL_SRPC_MAX_ARGS + 1];

    if (IsTerminationEvent(&event)) {
      break;
    } else if (IsUserEvent(&event)) {
      // A user event is a non-standard event
      // TODO(robertm): fix this hack
      switch (GetData1FromUserEvent(&event)) {
        // This event gets created so that we can invoke
        // RunCompletionCallback after PPB_Graphics2D_Flush
        case MY_EVENT_FLUSH_CALL_BACK: {
          int callback = GetData2FromUserEvent(&event);
          NaClLog(LOG_INFO, "Completion callback(%d)\n", callback);
          int dummy_exception[2] = {0, 0};
          BuildArgVec(ins, in, 3);
          ins[0]->tag = NACL_SRPC_ARG_TYPE_INT;
          ins[0]->u.ival = callback;
          ins[1]->tag = NACL_SRPC_ARG_TYPE_INT;
          ins[1]->u.ival = 0;
          ins[2]->tag = NACL_SRPC_ARG_TYPE_CHAR_ARRAY;
          ins[2]->u.count = sizeof(dummy_exception);
          ins[2]->arrays.carr = reinterpret_cast<char*>(&dummy_exception);
          BuildArgVec(outs, out, 0);
          ncl->InvokeNexeRpc("RunCompletionCallback:iiC:", ins, outs);
          break;
        }
        // This event gets created so that we can invoke
        // PPP_Audio_StreamCreated after PPB_Audio_Create
        case MY_EVENT_INIT_AUDIO:
          NaClLog(LOG_INFO, "audio init callback\n");
          BuildArgVec(ins, in, 4);
          ins[0]->tag = NACL_SRPC_ARG_TYPE_INT;
          ins[0]->u.ival = Global.handle_audio;
          ins[1]->tag = NACL_SRPC_ARG_TYPE_HANDLE;
          ins[1]->u.hval = Global.desc_audio_shmem->desc();
          ins[2]->tag = NACL_SRPC_ARG_TYPE_INT;
          ins[2]->u.ival = Global.sample_frame_count * kBytesPerSample;
          ins[3]->tag = NACL_SRPC_ARG_TYPE_HANDLE;
          ins[3]->u.hval = Global.desc_audio_sync_out->desc();

          BuildArgVec(outs, out, 0);
          sleep(1);
          ncl->InvokeNexeRpc("PPP_Audio_StreamCreated:ihih:", ins, outs);
          break;
        default:
          NaClLog(LOG_FATAL, "unknown event type %d\n",
                  GetData1FromUserEvent(&event));
          break;
      }
    } else {
      BuildArgVec(ins, in, 2);
      ins[0]->tag = NACL_SRPC_ARG_TYPE_INT;
      ins[0]->u.ival = Global.instance;
      ins[1]->tag = NACL_SRPC_ARG_TYPE_CHAR_ARRAY;
      ins[1]->u.count = sizeof(event);
      ins[1]->arrays.carr = reinterpret_cast<char*>(&event);

      BuildArgVec(outs, out, 1);
      outs[0]->tag = NACL_SRPC_ARG_TYPE_INT;
      ncl->InvokeNexeRpc("PPP_Instance_HandleInputEvent:iC:i", ins, outs);
    }
  }
  NaClLog(LOG_INFO, "Exiting event loop\n");
  return true;
}
