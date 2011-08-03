// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>

#include "native_client/src/trusted/plugin/browser_interface.h"

#include "native_client/src/include/checked_cast.h"
#include "native_client/src/include/elf.h"
#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/plugin/api_defines.h"
#include "native_client/src/trusted/plugin/scriptable_handle.h"

#include "native_client/src/third_party/ppapi/c/dev/ppb_console_dev.h"
#include "native_client/src/third_party/ppapi/c/ppb_var.h"
#include "native_client/src/third_party/ppapi/cpp/module.h"
#include "native_client/src/third_party/ppapi/cpp/private/instance_private.h"
#include "native_client/src/third_party/ppapi/cpp/private/var_private.h"

using nacl::assert_cast;

namespace plugin {

uintptr_t BrowserInterfacePpapi::StringToIdentifier(const nacl::string& str) {
  StringToIdentifierMap::iterator iter = string_to_identifier_map_.find(str);
  if (iter == string_to_identifier_map_.end()) {
    uintptr_t id = next_identifier++;
    string_to_identifier_map_.insert(make_pair(str, id));
    identifier_to_string_map_.insert(make_pair(id, str));
    return id;
  }
  return string_to_identifier_map_[str];
}


nacl::string BrowserInterfacePpapi::IdentifierToString(uintptr_t ident) {
  assert(identifier_to_string_map_.find(ident) !=
         identifier_to_string_map_.end());
  return identifier_to_string_map_[ident];
}


void BrowserInterfacePpapi::AddToConsole(InstanceIdentifier instance_id,
                                         const nacl::string& text) {
  pp::InstancePrivate* instance = InstanceIdentifierToPPInstance(instance_id);
  pp::Module* module = pp::Module::Get();
  const PPB_Var* var_interface =
      static_cast<const struct PPB_Var*>(
          module->GetBrowserInterface(PPB_VAR_INTERFACE));
  nacl::string prefix_string("NativeClient");
  PP_Var prefix =
      var_interface->VarFromUtf8(module->pp_module(),
                                 prefix_string.c_str(),
                                 static_cast<uint32_t>(prefix_string.size()));
  PP_Var str = var_interface->VarFromUtf8(module->pp_module(),
                                          text.c_str(),
                                          static_cast<uint32_t>(text.size()));
  const PPB_Console_Dev* console_interface =
      static_cast<const struct PPB_Console_Dev*>(
          module->GetBrowserInterface(PPB_CONSOLE_DEV_INTERFACE));
  console_interface->LogWithSource(instance->pp_instance(),
                                   PP_LOGLEVEL_LOG,
                                   prefix,
                                   str);
  var_interface->Release(prefix);
  var_interface->Release(str);
}


ScriptableHandle* BrowserInterfacePpapi::NewScriptableHandle(
    PortableHandle* handle) {
  return ScriptableHandlePpapi::New(handle);
}


pp::InstancePrivate* InstanceIdentifierToPPInstance(
    InstanceIdentifier instance_id) {
  return reinterpret_cast<pp::InstancePrivate*>(
      assert_cast<intptr_t>(instance_id));
}


InstanceIdentifier PPInstanceToInstanceIdentifier(
    pp::InstancePrivate* instance) {
  return assert_cast<InstanceIdentifier>(reinterpret_cast<intptr_t>(instance));
}

}  // namespace plugin
