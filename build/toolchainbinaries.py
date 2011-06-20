#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Utility functions and common data for downloading NaCl toolchain binaries.
"""

import sys


BASE_DOWNLOAD_URL = \
    'http://commondatastorage.googleapis.com/nativeclient-archive2'

PLATFORM_MAPPING = {
    'windows': {
        'x86-32': ['win_x86', 'win_x86_newlib'],
        'x86-64': ['win_x86', 'win_x86_newlib'],
    },
    'linux': {
        'x86-32': ['linux_x86', 'linux_x86_newlib',
                   'pnacl_linux_i686_newlib', 'linux_arm-trusted'],
        'x86-64': ['linux_x86', 'linux_x86_newlib',
                   'pnacl_linux_x86_64_newlib', 'linux_arm-trusted'],
        'arm'   : ['linux_x86_newlib', 'pnacl_linux_x86_64_newlib',
                   'linux_arm-trusted'],
    },
    'mac': {
        'x86-32': ['mac_x86', 'mac_x86_newlib'],
        'x86-64': ['mac_x86', 'mac_x86_newlib'],
    },
}


def EncodeToolchainUrl(base_url, version, flavor):
  if 'pnacl' in flavor:
    return '%s/toolchain/%s/naclsdk_%s.tgz' % (
        base_url, version, flavor)
  elif flavor.endswith('_newlib'):
    return '%s/toolchain/%s/naclsdk_%s.tgz' % (
      base_url, version, flavor[:-len('_newlib')])
  elif 'x86' in flavor:
    # TODO(pasko): the URL must depend on the 'flavor' provided, but not the
    # platform we are running on.
    if sys.platform == 'win32':
      return '%s/x86_toolchain/r%s/toolchain_%s.tar.xz' % (
        base_url, version, flavor)
    else:
      return '%s/x86_toolchain/r%s/toolchain_%s.tar.bz2' % (
        base_url, version, flavor)
  else:
    return '%s/toolchain/%s/naclsdk_%s.tgz' % (
      base_url, version, flavor)


def IsArmFlavor(flavor):
  return 'arm' in flavor or 'pnacl' in flavor
