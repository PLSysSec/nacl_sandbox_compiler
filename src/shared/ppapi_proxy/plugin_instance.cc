/*
 * Copyright 2010 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "native_client/src/shared/ppapi_proxy/plugin_instance.h"

#include <stdio.h>
#include "native_client/src/include/portability.h"
#include "native_client/src/shared/ppapi_proxy/utility.h"
#include "ppapi/c/ppb_instance.h"

namespace ppapi_proxy {

namespace {

static PluginInstance* GetInstancePointer(PP_Instance instance) {
  return reinterpret_cast<PluginInstance*>(static_cast<uintptr_t>(instance));
}

static PP_Var GetWindowObjectThunk(PP_Instance instance) {
  DebugPrintf("PluginInstance::GetWindowObject: instance=%" NACL_PRIx64"\n",
              instance);
  return GetInstancePointer(instance)->GetWindowObject();
}

static PP_Var GetOwnerElementObjectThunk(PP_Instance instance) {
  DebugPrintf("PluginInstance::GetOwnerElementObject: instance=%"
              NACL_PRIx64 "\n",
              instance);
  return GetInstancePointer(instance)->GetOwnerElementObject();
}

static bool BindGraphicsDeviceContextThunk(PP_Instance instance,
                                           PP_Resource device) {
  DebugPrintf("PluginInstance::BindGraphicsDeviceContext: instance=%"
              NACL_PRIx64 ", device=%" NACL_PRIu64 "\n",
              instance, device);
  return GetInstancePointer(instance)->BindGraphicsDeviceContext(device);
}

static bool IsFullFrameThunk(PP_Instance instance) {
  DebugPrintf("PluginInstance::IsFullFrame: instance=%" NACL_PRIx64 "\n",
              instance);
  return GetInstancePointer(instance)->IsFullFrame();
}

PP_Var ExecuteScript(PP_Instance instance, PP_Var script,  PP_Var* exception) {
  UNREFERENCED_PARAMETER(script);
  UNREFERENCED_PARAMETER(exception);
  DebugPrintf("PluginInstance::ExecuteScript: instance=%"
              NACL_PRIx64 "\n",
              instance);
  NACL_UNIMPLEMENTED();
  return PP_MakeVoid();
}

}  // namespace

const PPB_Instance* PluginInstance::GetInterface() {
  static const PPB_Instance intf = {
    GetWindowObjectThunk,
    GetOwnerElementObjectThunk,
    BindGraphicsDeviceContextThunk,
    IsFullFrameThunk,
    ExecuteScript
  };
  return &intf;
}

}  // namespace ppapi_proxy
