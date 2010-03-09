/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


#include "native_client/src/include/portability.h"

#include <stdio.h>

#include <cctype>
#include <string>
#include <stdlib.h>
#include <algorithm>

#include "base/basictypes.h"
#include "native_client/src/trusted/plugin/origin.h"

#define NACL_SELENIUM_TEST "NACL_DISABLE_SECURITY_FOR_SELENIUM_TEST"

#ifdef  ORIGIN_DEBUG
# define dprintf(alist) do { printf alist; } while (0)
#else
# define dprintf(alist) do { if (0) { printf alist; } } while (0)
#endif

namespace nacl {

  std::string UrlToOrigin(std::string url) {
    std::string::iterator it = find(url.begin(), url.end(), ':');
    if (url.end() == it) {
      dprintf(("no protospec separator found\n"));
      return "";
    }
    for (int num_slashes = 0; num_slashes < 3; ++num_slashes) {
      it = find(it + 1, url.end(), '/');
      if (url.end() == it) {
        dprintf(("no start of pathspec found\n"));
        return "";
      }
    }

    std::string origin(url.begin(), it);

    //
    // Domain names are in ascii and case insensitive, so we can
    // canonicalize to all lower case.  NB: Internationalizing Domain
    // Names in Applications (IDNA) encodes unicode in this reduced
    // alphabet.
    //
    for (it = origin.begin(); origin.end() != it; ++it) {
      *it = tolower(*it);
    }
    //
    // cannonicalize empty hostname as "localhost"
    //
    if ("file://" == origin) {
      origin = "file://localhost";
    }
    return origin;
  }

  // For now we are just checking that NaCl modules are local, or on
  // code.google.com.  Beware NaCl modules in the browser cache!
  //
  // Eventually, after sufficient security testing, we will always
  // return true.
  bool OriginIsInWhitelist(std::string origin) {
    static char const *allowed_origin[] = {
      /*
      * do *NOT* add in file://localhost as a way to get old tests to
      * work.  The file://localhost was only for early stage testing
      * -- having it can be a security problem if an adversary can
      * guess browser cache file names.
      */
      "http://localhost",
      "http://localhost:80",
      "http://localhost:5103",
#if 0
      "http://code.google.com",  // for demos hosted on project website
#endif
    };
    dprintf(("OriginIsInWhitelist(%s)\n", origin.c_str()));
    for (size_t i = 0; i < ARRAYSIZE_UNSAFE(allowed_origin); ++i) {
      if (origin == allowed_origin[i]) {
        dprintf((" found at position %"NACL_PRIdS"\n", i));
        return true;
      }
    }
    // We disregard the origin whitelist when running Selenium tests.
    // The code below is temporary since eventually we will drop the
    // whitelist completely.
#if NACL_WINDOWS
    char buffer[2];
    size_t required_buffer_size;
    if (0 == getenv_s(&required_buffer_size, buffer, 2, NACL_SELENIUM_TEST)) {
      dprintf((" selenium test!\n"));
      return true;
    }
#else
    if (NULL != getenv(NACL_SELENIUM_TEST)) {
      dprintf((" selenium test!\n"));
      return true;
    }
#endif

    dprintf((" FAILED!\n"));
    return false;
  }
}
