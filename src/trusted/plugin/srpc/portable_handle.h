/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


// The abstract portable scriptable object base class.

#ifndef NATIVE_CLIENT_SRC_TRUSTED_PLUGIN_SRPC_PORTABLE_HANDLE_H_
#define NATIVE_CLIENT_SRC_TRUSTED_PLUGIN_SRPC_PORTABLE_HANDLE_H_

#include <stdio.h>
#include <map>

#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/trusted/plugin/srpc/method_map.h"
#include "native_client/src/trusted/plugin/srpc/utility.h"


namespace plugin {

// Forward declarations for externals.
class BrowserInterface;
class Plugin;

typedef enum {
  METHOD_CALL = 0,
  PROPERTY_GET,
  PROPERTY_SET
} CallType;


// PortableHandle represents scriptable objects used by the browser plugin.
// The classes in this hierarchy are independent of the browser plugin API
// used to implement them.  PortableHandle is an abstract base class.
class PortableHandle {
 public:
  // Delete this object.
  void Delete() { delete this; }

  // Generic NPAPI/IDispatch interface
  bool Invoke(uintptr_t method_id, CallType call_type, SrpcParams* params);
  bool HasMethod(uintptr_t method_id, CallType call_type);

  // Get the method signature so ScriptableHandle can marshal the inputs
  bool InitParams(uintptr_t method_id, CallType call_type, SrpcParams* params);

  // DescBasedHandles can be conveyed over SRPC channels.
  virtual bool IsDescBasedHandle() const { return false; }

  // The interface to the browser.
  virtual BrowserInterface* browser_interface() const = 0;

  // Every portable object has a pointer to the root plugin object.
  virtual Plugin* plugin() const = 0;

 protected:
  PortableHandle();
  virtual ~PortableHandle();

  // Derived classes can set the properties and methods they export by
  // the following three methods.
  void AddPropertyGet(RpcFunction function_ptr,
                      const char *name,
                      const char *outs);
  void AddPropertySet(RpcFunction function_ptr,
                      const char *name,
                      const char *ins);
  void AddMethodCall(RpcFunction function_ptr,
                     const char *name,
                     const char *ins,
                     const char *outs);

  // Every derived class should provide an implementation for these functions
  // to allow handling of method calls that cannot be registered at build time.
  virtual bool InitParamsEx(uintptr_t method_id,
                            CallType call_type,
                            SrpcParams* params);
  virtual bool InvokeEx(uintptr_t method_id,
                        CallType call_type,
                        SrpcParams* params);
  virtual bool HasMethodEx(uintptr_t method_id, CallType call_type);

 private:
  NACL_DISALLOW_COPY_AND_ASSIGN(PortableHandle);
  MethodInfo* GetMethodInfo(uintptr_t method_id, CallType call_type);
  MethodMap methods_;
  MethodMap property_get_methods_;
  MethodMap property_set_methods_;
};

}  // namespace plugin

#endif  // NATIVE_CLIENT_SRC_TRUSTED_PLUGIN_SRPC_PORTABLE_HANDLE_H_
