// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "native_client/src/include/nacl_macros.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"

#include "ppapi/c/ppb.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp.h"

PP_EXPORT int32_t PPP_InitializeModule(PP_Module module_id,
                                       PPB_GetInterface get_browser_interface) {
  return PP_OK;
}

PP_EXPORT void PPP_ShutdownModule() {
  printf("--- PPP_ShutdownModule\n");
  // Crash in a way that won't get optimized out by LLVM.
  *(volatile int *) 0 = 0;
  NACL_NOTREACHED();
}

namespace {

PP_Bool DidCreate(PP_Instance /*instance*/,
                  uint32_t /*argc*/,
                  const char* /*argn*/[],
                  const char* /*argv*/[]) {
  return PP_TRUE;
}

void DidDestroy(PP_Instance /*instance*/) {
}

void DidChangeView(PP_Instance /*instance*/,
                   const struct PP_Rect* /*position*/,
                   const struct PP_Rect* /*clip*/) {
}

void DidChangeFocus(PP_Instance /*instance*/, PP_Bool /*has_focus*/) {
}

PP_Bool HandleDocumentLoad(PP_Instance /*instance*/, PP_Resource /*loader*/) {
  return PP_FALSE;
}

const struct PPP_Instance instance_interface = {
  DidCreate,
  DidDestroy,
  DidChangeView,
  DidChangeFocus,
  HandleDocumentLoad
};

}  // namespace

PP_EXPORT const void* PPP_GetInterface(const char* interface_name) {
  printf("PPP_GetInterface(%s)\n", interface_name);
  if (0 == strcmp(interface_name, PPP_INSTANCE_INTERFACE))  // Required.
    return reinterpret_cast<const void*>(&instance_interface);
  return NULL;
}
