/*
 * Copyright 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_SEL_UNIVERASAL_MULTIMEDIA_HANDLER_H_
#define NATIVE_CLIENT_SRC_TRUSTED_SEL_UNIVERASAL_MULTIMEDIA_HANDLER_H_

#include <string>
#include <vector>

class NaClCommandLoop;

// initialize multimedia (SDL) subsystem
bool HandlerSDLInitialize(NaClCommandLoop* ncl,
                          const std::vector<std::string>& args);

// start multimedia event loop
bool HandlerSDLEventLoop(NaClCommandLoop* ncl,
                         const std::vector<std::string>& args);


#endif  /* NATIVE_CLIENT_SRC_TRUSTED_SEL_UNIVERASAL_MULTIMEDIA_HANDLER_H_ */
