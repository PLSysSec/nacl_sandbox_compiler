// Copyright (c) 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is a build test that includes the basic OpenGL ES 2 headers, to make
// sure that the header layout in the NaCl toolchain is correct.  Note that
// unlike the cpp header test files, this is not generated by a script.

#include <GLES2/gl2.h>
// TODO(dspringer): Uncomment this #include when the toolchain actually
// includes gl2ext.h.  See bug:
// http://code.google.com/p/nativeclient/issues/detail?id=1273
// #include <GLES2/gl2ext.h>
