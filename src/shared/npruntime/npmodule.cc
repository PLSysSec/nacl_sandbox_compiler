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

namespace nacl {

// Class static variable declarations.
bool NPModule::is_webkit = false;

NPModule::NPModule(NaClSrpcChannel* channel)
    : proxy_(NULL),
      window_(NULL),
      extensions_(NULL),
      device2d_(NULL),
      context2d_(NULL),
      device3d_(NULL),
      context3d_(NULL) {
  // Remember the channel we will be communicating over.
  channel_ = channel;
  // Remember the bridge for this channel.
  channel->server_instance_data = static_cast<void*>(this);
  // All NPVariants will be transferred in the format of the browser.
  set_peer_npvariant_size(sizeof(NPVariant));
  // Set up a service for the browser-provided NPN methods.
  NaClSrpcService* service = new(std::nothrow) NaClSrpcService;
  if (NULL == service) {
    DebugPrintf("Couldn't create upcall services.\n");
    return;
  }
  if (!NaClSrpcServiceHandlerCtor(service, NPModuleRpcs::srpc_methods)) {
    DebugPrintf("Couldn't construct upcall services.\n");
    return;
  }
  // Export the service on the channel.
  channel->server = service;
  // And inform the client of the available services.
  char* str = const_cast<char*>(service->service_string);
  if (NACL_SRPC_RESULT_OK !=
      NPNavigatorRpcClient::NP_SetUpcallServices(channel, str)) {
    DebugPrintf("Couldn't set upcall services.\n");
  }
}

NPModule::~NPModule() {
  // The corresponding stub is released by the Navigator.
  if (proxy_) {
    NPN_ReleaseObject(proxy_);
  }
  // TODO(sehr): release contexts, etc., here.
}

NPModule* NPModule::GetModule(int32_t int_npp) {
  NPP npp = NPBridge::IntToNpp(int_npp);
  return static_cast<NPModule*>(NPBridge::LookupBridge(npp));
}

void NPModule::InvalidateRect(NPP npp, const NPRect* nprect) {
  if (window_ && window_->window && nprect) {
    NPN_InvalidateRect(npp, const_cast<NPRect*>(nprect));
  }
}

// NPN_PluginThreadAsyncCall support

namespace {

class NppClosure {
 public:
  NppClosure(uint32_t number, NPModule* module) {
    number_ = number;
    module_ = module;
  }
  uint32_t number() const { return number_; }
  NPModule* module() const { return module_; }
 private:
  uint32_t number_;
  NPModule* module_;
};

// The thunk enqueued on the browser's foreground thread.
static void doNppAsyncCall(void* arg) {
  NppClosure* closure = reinterpret_cast<NppClosure*>(arg);
  if (NULL != closure) {
    NPNavigatorRpcClient::NPP_DoAsyncCall(closure->module()->channel(),
                                          closure->number());
  }
  delete closure;
}

static NaClSrpcError handleAsyncCall(NaClSrpcChannel* channel,
                                     NaClSrpcArg** inputs,
                                     NaClSrpcArg** outputs) {
  UNREFERENCED_PARAMETER(channel);
  UNREFERENCED_PARAMETER(outputs);
  DebugPrintf("handleAsyncCall\n");
  NPP npp = NPBridge::IntToNpp(inputs[0]->u.ival);
  NPModule* module = static_cast<NPModule*>(NPBridge::LookupBridge(npp));
  uint32_t number = static_cast<uint32_t>(inputs[1]->u.ival);

  // Place a closure on the browser's javascript foreground thread.
  NppClosure* closure = new(std::nothrow) NppClosure(number, module);
  if (NULL == closure) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  NPN_PluginThreadAsyncCall(npp,
                            doNppAsyncCall,
                            static_cast<void*>(closure));
  return NACL_SRPC_RESULT_OK;
}

}  // namespace

// Structure for passing information to the thread.  Shares ownership of
// the descriptor with the creating routine.  This allows passing ownership
// to the upcall thread.
struct UpcallInfo {
 public:
  DescWrapper* desc_;
  NPModule* module_;
  UpcallInfo(DescWrapper* desc, NPModule* module) {
    desc_ = desc;
    module_ = module;
  }
  ~UpcallInfo() {
    DebugPrintf("deleting upcall info\n");
    desc_->Delete();
  }
};

static void WINAPI UpcallThread(void* arg) {
  UpcallInfo* info = reinterpret_cast<UpcallInfo*>(arg);
  NaClSrpcHandlerDesc handlers[] = {
    { "NPN_PluginThreadAsyncCall:ii:", handleAsyncCall },
    { NULL, NULL }
  };
  DebugPrintf("UpcallThread(%p)\n", arg);
  // Run the SRPC server.
  NaClSrpcServerLoop(info->desc_->desc(), handlers, info->module_);
  // Free the info node.
  delete info;
  DebugPrintf("UpcallThread: End\n");
}

NPError NPModule::Initialize() {
  NaClSrpcError retval;
  DescWrapperFactory factory;
  DescWrapper* pair[2] = { NULL, NULL };
  UpcallInfo* info = NULL;
  NPError err = NPERR_GENERIC_ERROR;

  DebugPrintf("Initialize\n");

  // Create a socket pair for the upcall server.
  if (factory.MakeSocketPair(pair)) {
    goto done;
  }
  // Create an info node to pass to the thread.
  info = new(std::nothrow) UpcallInfo(pair[0], this);
  if (NULL == info) {
    goto done;
  }
  // info takes ownership of pair[0].
  pair[0] = NULL;
  // Create a thread and an SRPC "upcall" server.
  if (!NaClThreadCtor(&upcall_thread_, UpcallThread, info, 128 << 10)) {
    goto done;
  }
  // On success, ownership of info passes to the thread.
  info = NULL;
  // Invoke the NaCl module's NP_Initialize function.
  retval =
      NPNavigatorRpcClient::NP_Initialize(channel(),
                                          GETPID(),
                                          static_cast<int>(sizeof(NPVariant)),
                                          pair[1]->desc());
  // Return the appropriate error code.
  if (NACL_SRPC_RESULT_OK != retval) {
    goto done;
  }
  err = NPERR_NO_ERROR;

 done:
  DebugPrintf("deleting pairs\n");
  DescWrapper::SafeDelete(pair[0]);
  DescWrapper::SafeDelete(pair[1]);
  delete info;
  return err;
}

static bool SerializeArgArray(int argc,
                              char* array[],
                              char* serial_array,
                              uint32_t* serial_size) {
  size_t used = 0;

  for (int i = 0; i < argc; ++i) {
    // Note that strlen() cannot ever return SIZE_T_MAX, since
    // that would imply that there were no nulls anywhere in memory,
    // which would lead to strlen() never terminating. So this
    // assignment is safe.
    size_t len = strlen(array[i]) + 1;

    if (len > std::numeric_limits<uint32_t>::max()) {
      // overflow, input string is too long
      return false;
    }

    if (used > std::numeric_limits<uint32_t>::max() - len) {
      // overflow, output string is too long
      return false;
    }

    if (used > *serial_size - len) {
      // Length of the serialized array was exceeded.
      return false;
    }
    strncpy(serial_array + used, array[i], len);
    used += len;
  }
  // Note that there is a check against numeric_limits<uint32_t> in
  // the code above, which is why this cast is safe.
  *serial_size = static_cast<uint32_t>(used);
  return true;
}

NPError NPModule::New(char* mimetype,
                      NPP npp,
                      int argc,
                      char* argn[],
                      char* argv[]) {
  // NPError is shorter than an int, causing stack corruption reports
  // on Windows, because SRPC doesn't have an int16 type.
  int nperr;
  char argn_serial[kMaxArgc * kMaxArgLength];
  char argv_serial[kMaxArgc * kMaxArgLength];

  DebugPrintf("New\n");
  for (int i = 0; i < argc; ++i) {
    DebugPrintf("  %"PRIu32": argn=%s argv=%s\n", i, argn[i], argv[i]);
  }
  uint32_t argn_size = static_cast<uint32_t>(sizeof(argn_serial));
  uint32_t argv_size = static_cast<uint32_t>(sizeof(argv_serial));
  if (!SerializeArgArray(argc, argn, argn_serial, &argn_size) ||
      !SerializeArgArray(argc, argv, argv_serial, &argv_size)) {
    DebugPrintf("New: serialize failed\n");
    return NPERR_GENERIC_ERROR;
  }
  NaClSrpcError retval = NPNavigatorRpcClient::NPP_New(channel(),
                                                       mimetype,
                                                       NPBridge::NppToInt(npp),
                                                       argc,
                                                       argn_size,
                                                       argn_serial,
                                                       argv_size,
                                                       argv_serial,
                                                       &nperr);
  if (NACL_SRPC_RESULT_OK != retval) {
    DebugPrintf("New: invocation returned %x, %d\n", retval, nperr);
    return NPERR_GENERIC_ERROR;
  }
  return static_cast<NPError>(nperr);
}

//
// NPInstance methods
//

NPError NPModule::Destroy(NPP npp, NPSavedData** save) {
  UNREFERENCED_PARAMETER(save);
  int nperr;
  NaClSrpcError retval =
      NPNavigatorRpcClient::NPP_Destroy(channel(),
                                        NPBridge::NppToInt(npp),
                                        &nperr);
  if (NACL_SRPC_RESULT_OK != retval) {
    return NPERR_GENERIC_ERROR;
  }
  return static_cast<NPError>(nperr);
}

NPError NPModule::SetWindow(NPP npp, NPWindow* window) {
  if (NULL == window) {
    return NPERR_NO_ERROR;
  }
  int nperr;
  NaClSrpcError retval =
      NPNavigatorRpcClient::NPP_SetWindow(channel(),
                                          NPBridge::NppToInt(npp),
                                          window->height,
                                          window->width,
                                          &nperr);
  if (NACL_SRPC_RESULT_OK != retval) {
    return NPERR_GENERIC_ERROR;
  }
  return static_cast<NPError>(nperr);
}

NPError NPModule::GetValue(NPP npp, NPPVariable variable, void *value) {
  // NOTE: we do not use a switch statement because of compiler warnings */
  // TODO(sehr): RPC to module for most.
  if (NPPVpluginNameString == variable) {
    *static_cast<const char**>(value) = "NativeClient NPAPI bridge plug-in";
    return NPERR_NO_ERROR;
  } else if (NPPVpluginDescriptionString == variable) {
    *static_cast<const char**>(value) =
      "A plug-in for NPAPI based NativeClient modules.";
    return NPERR_NO_ERROR;
  } else if (NPPVpluginScriptableNPObject == variable) {
    *reinterpret_cast<NPObject**>(value) = GetScriptableInstance(npp);
    if (*reinterpret_cast<NPObject**>(value)) {
      return NPERR_NO_ERROR;
    } else {
      return NPERR_GENERIC_ERROR;
    }
  } else {
    return NPERR_INVALID_PARAM;
  }
}

int16_t NPModule::HandleEvent(NPP npp, void* event) {
  static const uint32_t kEventSize =
      static_cast<uint32_t>(sizeof(NPPepperEvent));
  int32_t return_int16;

  NaClSrpcError retval =
      NPNavigatorRpcClient::NPP_HandleEvent(channel(),
                                            NPBridge::NppToInt(npp),
                                            kEventSize,
                                            reinterpret_cast<char*>(event),
                                            &return_int16);
  if (NACL_SRPC_RESULT_OK == retval) {
    return static_cast<int16_t>(return_int16 & 0xffff);
  } else {
    return -1;
  }
}

NPObject* NPModule::GetScriptableInstance(NPP npp) {
  DebugPrintf("GetScriptableInstance: npp %p\n", npp);
  if (NULL == proxy_) {
    // TODO(sehr): Not clear we should be caching on the browser plugin side.
    NPCapability capability;
    nacl_abi_size_t cap_size = static_cast<nacl_abi_size_t>(sizeof(capability));
    char* cap_ptr = reinterpret_cast<char*>(&capability);
    NaClSrpcError retval =
        NPNavigatorRpcClient::NPP_GetScriptableInstance(channel(),
                                                        NPBridge::NppToInt(npp),
                                                        &cap_size,
                                                        cap_ptr);
    if (NACL_SRPC_RESULT_OK != retval) {
      DebugPrintf("    Got return code %x\n", retval);
      return NULL;
    }
    proxy_ = NPBridge::CreateProxy(npp, capability);
    DebugPrintf("    Proxy is %p\n", reinterpret_cast<void*>(proxy_));
  }
  if (NULL != proxy_) {
    NPN_RetainObject(proxy_);
  }
  return proxy_;
}

NPError NPModule::NewStream(NPP npp,
                            NPMIMEType type,
                            NPStream* stream,
                            NPBool seekable,
                            uint16_t* stype) {
  UNREFERENCED_PARAMETER(npp);
  UNREFERENCED_PARAMETER(type);
  UNREFERENCED_PARAMETER(stream);
  UNREFERENCED_PARAMETER(seekable);
  *stype = NP_ASFILEONLY;
  return NPERR_NO_ERROR;
}

void NPModule::StreamAsFile(NPP npp, NPStream* stream, const char* filename) {
  UNREFERENCED_PARAMETER(npp);
  UNREFERENCED_PARAMETER(stream);
  UNREFERENCED_PARAMETER(filename);
  // TODO(sehr): Implement using UrlAsNaClDesc.
}

NPError NPModule::DestroyStream(NPP npp, NPStream *stream, NPError reason) {
  UNREFERENCED_PARAMETER(npp);
  UNREFERENCED_PARAMETER(stream);
  UNREFERENCED_PARAMETER(reason);
  return NPERR_NO_ERROR;
}

void NPModule::URLNotify(NPP npp,
                         const char* url,
                         NPReason reason,
                         void* notify_data) {
  UNREFERENCED_PARAMETER(url);
  UNREFERENCED_PARAMETER(notify_data);
  DebugPrintf("URLNotify: npp %p, rsn %d\n", static_cast<void*>(npp), reason);
  // TODO(sehr): need a call when reason is failure.
  if (NPRES_DONE != reason) {
    return;
  }
  // TODO(sehr): Need to set the descriptor appropriately and call.
  // NPNavigatorRpcClient::NPP_URLNotify(channel(), desc, reason);
}

void NPModule::ForceRedraw(NPP npp) {
  if (window_ && window_->window) {
    NPN_ForceRedraw(npp);
  }
}

class TransportDIB;

struct Device2DImpl {
  ::TransportDIB* dib;
};

NaClSrpcError NPModule::Device2DInitialize(NPP npp,
                                           NaClSrpcImcDescType* shm_desc,
                                           int32_t* stride,
                                           int32_t* left,
                                           int32_t* top,
                                           int32_t* right,
                                           int32_t* bottom) {
  // Initialize the return values in case of failure.
  *shm_desc =
      const_cast<NaClDesc*>(
          reinterpret_cast<const NaClDesc*>(NaClDescInvalidMake()));
  *stride = -1;
  *left = -1;
  *top = -1;
  *right = -1;
  *bottom = -1;

  if (NULL == extensions_) {
    if (NPERR_NO_ERROR !=
        NPN_GetValue(npp, NPNVPepperExtensions, &extensions_)) {
      // Because this variable is not implemented in other browsers, this path
      // should always be taken except in Pepper-enabled browsers.
      return NACL_SRPC_RESULT_APP_ERROR;
    }
    if (NULL == extensions_) {
      return NACL_SRPC_RESULT_APP_ERROR;
    }
  }
  if (NULL == device2d_) {
    device2d_ = extensions_->acquireDevice(npp, NPPepper2DDevice);
    if (NULL == device2d_) {
      return NACL_SRPC_RESULT_APP_ERROR;
    }
  }
  if (NULL == context2d_) {
    context2d_ = new(std::nothrow) NPDeviceContext2D;
    if (NULL == context2d_) {
      return NACL_SRPC_RESULT_APP_ERROR;
    }
    NPError retval = device2d_->initializeContext(npp, NULL, context2d_);
    if (NPERR_NO_ERROR != retval) {
      return NACL_SRPC_RESULT_APP_ERROR;
    }
  }
  DescWrapperFactory factory;
  Device2DImpl* impl =
      reinterpret_cast<Device2DImpl*>(context2d_->reserved);
  DescWrapper* wrapper = factory.ImportTransportDIB(impl->dib);
  if (NULL == wrapper) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  // Increase reference count for SRPC return value, since wrapper Delete
  // would cause Dtor to fire.
  *shm_desc = NaClDescRef(wrapper->desc());
  // Free the wrapper.
  wrapper->Delete();
  *stride = context2d_->stride;
  *left = context2d_->dirty.left;
  *top = context2d_->dirty.top;
  *right = context2d_->dirty.right;
  *bottom = context2d_->dirty.bottom;

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModule::Device2DFlush(NPP npp,
                                      int32_t* stride,
                                      int32_t* left,
                                      int32_t* top,
                                      int32_t* right,
                                      int32_t* bottom) {
  if (NULL == extensions_) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  NPError retval = device2d_->flushContext(npp, context2d_, NULL, NULL);
  if (NPERR_NO_ERROR != retval) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  *stride = context2d_->stride;
  *left = context2d_->dirty.left;
  *top = context2d_->dirty.top;
  *right = context2d_->dirty.right;
  *bottom = context2d_->dirty.bottom;

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModule::Device2DDestroy(NPP npp) {
  if (NULL == extensions_) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  NPError retval = device2d_->destroyContext(npp, context2d_);
  if (NPERR_NO_ERROR != retval) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }

  return NACL_SRPC_RESULT_OK;
}

namespace base {
class SharedMemory;
}  // namespace base

struct Device3DImpl {
  gpu::CommandBuffer* command_buffer;
};

NaClSrpcError NPModule::Device3DInitialize(NPP npp,
                                           int32_t entries_requested,
                                           NaClSrpcImcDescType* shm_desc,
                                           int32_t* entries_obtained,
                                           int32_t* get_offset,
                                           int32_t* put_offset) {
  // Initialize the return values in case of failure.
  *shm_desc =
      const_cast<NaClDesc*>(
          reinterpret_cast<const NaClDesc*>(NaClDescInvalidMake()));
  *entries_obtained = -1;
  *get_offset = -1;
  *put_offset = -1;

#if defined(NACL_STANDALONE)
  UNREFERENCED_PARAMETER(npp);
  UNREFERENCED_PARAMETER(entries_requested);
  UNREFERENCED_PARAMETER(shm_desc);
  UNREFERENCED_PARAMETER(entries_obtained);
  UNREFERENCED_PARAMETER(get_offset);
  UNREFERENCED_PARAMETER(put_offset);

  return NACL_SRPC_RESULT_APP_ERROR;
#else
  if (NULL == extensions_) {
    if (NPERR_NO_ERROR !=
        NPN_GetValue(npp, NPNVPepperExtensions, &extensions_)) {
      // Because this variable is not implemented in other browsers, this path
      // should always be taken except in Pepper-enabled browsers.
      return NACL_SRPC_RESULT_APP_ERROR;
    }
    if (NULL == extensions_) {
      return NACL_SRPC_RESULT_APP_ERROR;
    }
  }
  if (NULL == device3d_) {
    device3d_ = extensions_->acquireDevice(npp, NPPepper3DDevice);
    if (NULL == device3d_) {
      return NACL_SRPC_RESULT_APP_ERROR;
    }
  }
  if (NULL == context3d_) {
    context3d_ = new(std::nothrow) NPDeviceContext3D;
    if (NULL == context3d_) {
      return NACL_SRPC_RESULT_APP_ERROR;
    }
    static NPDeviceContext3DConfig config;
    config.commandBufferSize = entries_requested;
    NPError retval =
        device3d_->initializeContext(npp, &config, context3d_);
    if (NPERR_NO_ERROR != retval) {
      return NACL_SRPC_RESULT_APP_ERROR;
    }
  }
  Device3DImpl* impl =
      reinterpret_cast<Device3DImpl*>(context3d_->reserved);
  ::base::SharedMemory* shm =
      impl->command_buffer->GetRingBuffer().shared_memory;
  if (NULL == shm) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  DescWrapperFactory factory;
  size_t shm_size = context3d_->commandBufferSize * sizeof(int32_t);
  DescWrapper* wrapper =
      factory.ImportSharedMemory(shm, static_cast<size_t>(shm_size));
  if (NULL == wrapper) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  // Increase reference count for SRPC return value, since wrapper Delete
  // would cause Dtor to fire.
  *shm_desc = NaClDescRef(wrapper->desc());
  // Free the wrapper.
  wrapper->Delete();
  *entries_obtained = context3d_->commandBufferSize;
  *get_offset = context3d_->getOffset;
  *put_offset = context3d_->putOffset;

  return NACL_SRPC_RESULT_OK;
#endif  // defined(NACL_STANDALONE)
}

NaClSrpcError NPModule::Device3DFlush(NPP npp,
                                      int32_t put_offset,
                                      int32_t* get_offset) {
  if (NULL == extensions_) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  context3d_->putOffset = put_offset;
  NPError retval = device3d_->flushContext(npp, context3d_, NULL, NULL);
  if (NPERR_NO_ERROR != retval) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  *get_offset = context3d_->getOffset;

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModule::Device3DDestroy(NPP npp) {
  if (NULL == extensions_) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  NPError retval = device3d_->destroyContext(npp, context3d_);
  if (NPERR_NO_ERROR != retval) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }

  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModule::Device3DGetState(NPP npp,
                                         int32_t state,
                                         int32_t* value) {
  UNREFERENCED_PARAMETER(npp);
  UNREFERENCED_PARAMETER(state);
  UNREFERENCED_PARAMETER(value);
  /* TODO(sehr): re-enable this.
  NPError retval = device3d_->getStateContext(npp, context3d_, state, value);
  if (NPERR_NO_ERROR != retval) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  */
  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModule::Device3DSetState(NPP npp,
                                         int32_t state,
                                         int32_t value) {
  UNREFERENCED_PARAMETER(npp);
  UNREFERENCED_PARAMETER(state);
  UNREFERENCED_PARAMETER(value);
  /* TODO(sehr): re-enable this.
  NPError retval = device3d_->setStateContext(npp, context3d_, state, value);
  if (NPERR_NO_ERROR != retval) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  */
  return NACL_SRPC_RESULT_OK;
}

NaClSrpcError NPModule::Device3DCreateBuffer(NPP npp,
                                             int32_t size,
                                             NaClSrpcImcDescType* shm_desc,
                                             int32_t* id) {
  // Initialize buffer id and returned handle to allow error returns.
  int buffer_id = -1;
  *shm_desc =
      const_cast<NaClDesc*>(
          reinterpret_cast<const NaClDesc*>(NaClDescInvalidMake()));
  *id = buffer_id;

#if defined(NACL_STANDALONE)
  UNREFERENCED_PARAMETER(npp);
  UNREFERENCED_PARAMETER(size);
  UNREFERENCED_PARAMETER(shm_desc);
  UNREFERENCED_PARAMETER(id);
  return NACL_SRPC_RESULT_APP_ERROR;
#else
  // Call the Pepper API.
  NPError retval = device3d_->createBuffer(npp, context3d_, size, &buffer_id);
  if (NPERR_NO_ERROR != retval) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  // Look up the base::SharedMemory for the returned id.
  Device3DImpl* impl =
      reinterpret_cast<Device3DImpl*>(context3d_->reserved);
  ::base::SharedMemory* shm =
      impl->command_buffer->GetTransferBuffer(buffer_id).shared_memory;
  if (NULL == shm) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  // Create a NaCl descriptor to return.
  DescWrapperFactory factory;
  DescWrapper* wrapper =
      factory.ImportSharedMemory(shm, static_cast<size_t>(size));
  if (NULL == wrapper) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }
  // Increase reference count for SRPC return value, since wrapper Delete
  // would cause Dtor to fire.
  *shm_desc = NaClDescRef(wrapper->desc());
  *id = buffer_id;
  // Clean up.
  wrapper->Delete();

  return NACL_SRPC_RESULT_OK;
#endif  // defined(NACL_STANDALONE)
}

NaClSrpcError NPModule::Device3DDestroyBuffer(NPP npp, int32_t id) {
  NPError retval = device3d_->destroyBuffer(npp, context3d_, id);
  if (NPERR_NO_ERROR != retval) {
    return NACL_SRPC_RESULT_APP_ERROR;
  }

  return NACL_SRPC_RESULT_OK;
}

}  // namespace nacl
