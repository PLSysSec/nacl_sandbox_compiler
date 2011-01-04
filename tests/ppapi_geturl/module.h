// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.
#ifndef TESTS_PPAPI_GETURL_MODULE_H_
#define TESTS_PPAPI_GETURL_MODULE_H_

#include <string>

#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/dev/ppb_var_deprecated.h"
#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"

// ppapi_geturl example is deliberately using C PPAPI interface.
// C++ PPAPI layer has pp::Module wrapper class.
class Module {
 public:
  static Module* Create(PP_Module module_id,
                        PPB_GetInterface get_browser_interface);
  static Module* Get();
  static void Free();

  const void* GetPluginInterface(const char* interface_name);
  const void* GetBrowserInterface(const char* interface_name);
  PP_Module module_id() { return module_id_; }
  const PPB_Core* ppb_core_interface() { return ppb_core_interface_; }
  const PPB_Var_Deprecated* ppb_var_interface() { return ppb_var_interface_; }

  static char* VarToCStr(const PP_Var& var);
  static std::string VarToStr(const PP_Var& var);
  static PP_Var StrToVar(const char* str);
  static PP_Var StrToVar(const std::string& str);
  static std::string ErrorCodeToStr(int32_t error_code);

  // Calls JS ReportResult() defined in ppapi_geturl.html
  void ReportResult(PP_Instance pp_instance,
                    const char* url,
                    bool as_file,
                    const char* text,
                    bool success);
 private:
  PP_Module module_id_;
  PPB_GetInterface get_browser_interface_;
  const PPB_Core* ppb_core_interface_;
  const PPB_Instance* ppb_instance_interface_;
  const PPB_Var_Deprecated* ppb_var_interface_;

  Module(PP_Module module_id, PPB_GetInterface get_browser_interface);
  ~Module() { }
  Module(const Module&);
  void operator=(const Module&);
};

#endif  // TESTS_PPAPI_GETURL_MODULE_H_
