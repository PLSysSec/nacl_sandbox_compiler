// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NATIVE_CLIENT_SRC_SHARED_PPAPI_PROXY_BROWSER_GLOBALS_H_
#define NATIVE_CLIENT_SRC_SHARED_PPAPI_PROXY_BROWSER_GLOBALS_H_

#include "native_client/src/third_party/ppapi/c/dev/ppb_context_3d_dev.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_context_3d_trusted_dev.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_cursor_control_dev.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_find_dev.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_font_dev.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_memory_dev.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_scrollbar_dev.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_testing_dev.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_surface_3d_dev.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_var_deprecated.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_widget_dev.h"
#include "native_client/src/third_party/ppapi/c/dev/ppb_zoom_dev.h"
#include "native_client/src/third_party/ppapi/c/pp_instance.h"
#include "native_client/src/third_party/ppapi/c/pp_module.h"
#include "native_client/src/third_party/ppapi/c/ppb.h"
#include "native_client/src/third_party/ppapi/c/ppb_core.h"
#include "native_client/src/third_party/ppapi/c/ppb_file_io.h"
#include "native_client/src/third_party/ppapi/c/ppb_file_system.h"
#include "native_client/src/third_party/ppapi/c/ppb_graphics_2d.h"
#include "native_client/src/third_party/ppapi/c/ppb_image_data.h"
#include "native_client/src/third_party/ppapi/c/ppb_instance.h"
#include "native_client/src/third_party/ppapi/c/ppb_messaging.h"
#include "native_client/src/third_party/ppapi/c/ppb_url_loader.h"
#include "native_client/src/third_party/ppapi/c/ppb_url_request_info.h"
#include "native_client/src/third_party/ppapi/c/ppb_url_response_info.h"
#include "native_client/src/third_party/ppapi/c/ppb_var.h"
#include "native_client/src/third_party/ppapi/c/private/ppb_pdf.h"
#include "native_client/src/third_party/ppapi/c/trusted/ppb_image_data_trusted.h"

struct NaClSrpcRpc;
struct NaClSrpcChannel;

namespace ppapi_proxy {

// These functions handle the browser-side (trusted code) mapping of a browser
// Instance to instance-specific data, such as the SRPC communication channel.
// These functions are called by the in-browser (trusted) plugin code, and are
// always called from the main (foreground, UI, ...) thread. As such, they are
// not thread-safe (they do not need to be).

// BrowserPpp keeps browser side PPP_Instance specific information, such as the
// channel used to talk to the instance.
class BrowserPpp;

// Associate a particular BrowserPpp with a PP_Instance value.  This allows the
// browser side to look up information it needs to communicate with the stub.
void SetBrowserPppForInstance(PP_Instance instance,
                              BrowserPpp* browser_ppp);
// When an instance is destroyed, this is called to remove the association, as
// the stub will be destroyed by a call to Shutdown.
void UnsetBrowserPppForInstance(PP_Instance instance);
// Gets the BrowserPpp information remembered for a particular instance.
BrowserPpp* LookupBrowserPppForInstance(PP_Instance instance);

// To keep track of memory allocated by a particular module, we need to remember
// the PP_Module corresponding to a particular NaClSrpcChannel*.
void SetModuleIdForSrpcChannel(NaClSrpcChannel* channel, PP_Module module_id);
// To call remote callbacks only when the proxy is up and running, we need to
// remember the PP_Instance corresponding to a particular NaClSrpcChannel*.
void SetInstanceIdForSrpcChannel(NaClSrpcChannel* channel,
                                 PP_Instance instance_id);
// Removes the association with a given channel.
void UnsetModuleIdForSrpcChannel(NaClSrpcChannel* channel);
void UnsetInstanceIdForSrpcChannel(NaClSrpcChannel* channel);
// Looks up the association with a given channel.
PP_Module LookupModuleIdForSrpcChannel(NaClSrpcChannel* channel);
PP_Instance LookupInstanceIdForSrpcChannel(NaClSrpcChannel* channel);

// Helpers for getting a pointer to the "main channel" for a specific nexe.
NaClSrpcChannel* GetMainSrpcChannel(NaClSrpcRpc* upcall_rpc);
NaClSrpcChannel* GetMainSrpcChannel(PP_Instance);

// Support for getting PPB_ browser interfaces.
// Safe version CHECK's for NULL.
void SetPPBGetInterface(PPB_GetInterface get_interface_function);
const void* GetBrowserInterface(const char* interface_name);
const void* GetBrowserInterfaceSafe(const char* interface_name);
// Functions marked "shared" are to be provided by both the browser and the
// plugin side of the proxy, so they can be used by the shared proxy code
// under both trusted and untrusted compilation.
const PPB_Context3D_Dev* PPBContext3DInterface();
const PPB_Context3DTrusted_Dev* PPBContext3DTrustedInterface();
const PPB_Core* PPBCoreInterface();  // shared
const PPB_CursorControl_Dev* PPBCursorControlInterface();
const PPB_FileIO* PPBFileIOInterface();
const PPB_FileSystem* PPBFileSystemInterface();
const PPB_Find_Dev* PPBFindInterface();
const PPB_Font_Dev* PPBFontInterface();
const PPB_Graphics2D* PPBGraphics2DInterface();
const PPB_ImageData* PPBImageDataInterface();
const PPB_ImageDataTrusted* PPBImageDataTrustedInterface();
const PPB_Instance* PPBInstanceInterface();
const PPB_Memory_Dev* PPBMemoryInterface();  // shared
const PPB_Messaging* PPBMessagingInterface();
const PPB_PDF* PPBPDFInterface();
const PPB_Scrollbar_Dev* PPBScrollbarInterface();
const PPB_Surface3D_Dev* PPBSurface3DInterface();
const PPB_Testing_Dev* PPBTestingInterface();
const PPB_URLLoader* PPBURLLoaderInterface();
const PPB_URLRequestInfo* PPBURLRequestInfoInterface();
const PPB_URLResponseInfo* PPBURLResponseInfoInterface();
const PPB_Var* PPBVarInterface();  // shared
const PPB_Widget_Dev* PPBWidgetInterface();
const PPB_Zoom_Dev* PPBZoomInterface();

// PPAPI constants used in the proxy.
extern const PP_Resource kInvalidResourceId;

}  // namespace ppapi_proxy

#endif  // NATIVE_CLIENT_SRC_SHARED_PPAPI_PROXY_BROWSER_GLOBALS_H_
