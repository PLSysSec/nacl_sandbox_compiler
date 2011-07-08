// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVE_CLIENT_SRC_SHARED_PPAPI_PROXY_PLUGIN_PPB_CURSOR_CONTROL_H_
#define NATIVE_CLIENT_SRC_SHARED_PPAPI_PROXY_PLUGIN_PPB_CURSOR_CONTROL_H_

#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_cursor_control_dev.h"

namespace ppapi_proxy {

// Implements the untrusted side of the PPB_CursorControl_Dev interface.
class PluginCursorControl {
 public:
  PluginCursorControl();
  static const PPB_CursorControl_Dev* GetInterface();

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(PluginCursorControl);
};

}  // namespace ppapi_proxy

#endif  // NATIVE_CLIENT_SRC_SHARED_PPAPI_PROXY_PLUGIN_PPB_CURSOR_CONTROL_H_

