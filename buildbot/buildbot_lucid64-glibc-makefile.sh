#!/bin/bash
# Copyright 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Script assumed to be run in native_client/
if [[ ${PWD} != */native_client ]]; then
  echo "ERROR: must be run in native_client!"
  exit 1
fi

set -x
set -e
set -u


echo @@@BUILD_STEP gclient_runhooks@@@
gclient runhooks --force

echo @@@BUILD_STEP clobber@@@
rm -rf scons-out tools/SRC tools/BUILD tools/out tools/toolchain \
  tools/toolchain.tgz toolchain .tmp || echo already_clean

echo @@@BUILD_STEP compile_toolchain@@@
(
  cd tools
  make -j8 buildbot-build-with-glibc TOOLCHAINLOC=toolchain SDKNAME=linux_x86
)

echo @@@BUILD_STEP tar_toolchain@@@
(
  cd tools
  tar cSvfz toolchain.tgz toolchain/ && chmod a+r toolchain.tgz
)

echo @@@BUILD_STEP untar_toolchain@@@
(
  mkdir -p .tmp
  cd .tmp
  tar zxf ../tools/toolchain.tgz
  mv toolchain ..
)

echo @@@BUILD_STEP gyp_compile@@@
(
  cd ..
  make -k -j8 V=1 BUILDTYPE=Release
)

RETCODE=0

echo @@@BUILD_STEP gyp_tests@@@
python trusted_test.py --config Release || \
  (RETCODE=$? && echo @@@BUILD_FAILED@@@)

echo @@@BUILD_STEP small_tests32@@@
./scons -k -j 8 \
  naclsdk_mode=custom:"${PWD}"/toolchain/linux_x86 \
  --mode=dbg-host,nacl platform=x86-32 \
  --nacl_glibc --verbose small_tests || \
  (RETCODE=$? && echo @@@BUILD_FAILED@@@)

echo @@@BUILD_STEP small_tests64@@@
./scons -k -j 8 \
  naclsdk_mode=custom:"${PWD}"/toolchain/linux_x86 \
  --mode=dbg-host,nacl platform=x86-64 \
  --nacl_glibc --verbose small_tests || \
  (RETCODE=$? && echo @@@BUILD_FAILED@@@)

# TODO(pasko): add medium_tests, large_tests, {chrome_}browser_tests.

echo @@@BUILD_STEP archive_build@@@
if [[ ${RETCODE} == 0 ]]; then
  /b/build/scripts/slave/gsutil -h Cache-Control:no-cache cp -a public-read \
    tools/out/toolchain.tgz \
    gs://nativeclient-archive2/x86_toolchain/r${BUILDBOT_GOT_REVISION}/toolchain_linux_x86.tar.gz
fi

exit ${RETCODE}
