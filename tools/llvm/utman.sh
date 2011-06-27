#!/bin/bash
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
#@                 Untrusted Toolchain Manager
#@-------------------------------------------------------------------
#@ This script builds the ARM and PNaCl untrusted toolchains.
#@ It MUST be run from the native_client/ directory.
#
######################################################################
# Directory Layout Description
######################################################################
# All directories are relative to BASE which is
# On Linux X86-64: native_client/toolchain/pnacl_linux_x86_64/
# On Linux X86-32: native_client/toolchain/pnacl_linux_i686/
# On Mac X86-32  : native_client/toolchain/pnacl_darwin_i386/
#
# /x86-32sfi-lib   [experimental] x86 sandboxed libraries and object files
# /x86-32sfi-tools [experimental] x86-32 crosstool binaries for building
#                  and linking  x86-32 nexes
#
######################################################################
# Config
######################################################################

set -o nounset
set -o errexit

# The script is located in "native_client/tools/llvm".
# Set pwd to native_client/
cd "$(dirname "$0")"/../..
if [[ $(basename "$(pwd)") != "native_client" ]] ; then
  echo "ERROR: cannot find native_client/ directory"
  exit -1
fi
readonly NACL_ROOT="$(pwd)"

source tools/llvm/common-tools.sh

SetScriptPath "${NACL_ROOT}/tools/llvm/utman.sh"
SetLogDirectory "${NACL_ROOT}/toolchain/hg-log"

# NOTE: gcc and llvm have to be synchronized
#       we have chosen toolchains which both are based on gcc-4.2.1

# For different levels of make parallelism change this in your env
readonly UTMAN_CONCURRENCY=${UTMAN_CONCURRENCY:-8}
UTMAN_BUILD_ARM=true

if ${BUILD_PLATFORM_MAC} || ${BUILD_PLATFORM_WIN}; then
  # We don't yet support building ARM tools for mac or windows.
  UTMAN_BUILD_ARM=false
fi

# Set the library mode
readonly LIBMODE=${LIBMODE:-newlib}

LIBMODE_NEWLIB=false
LIBMODE_GLIBC=false
if [ ${LIBMODE} == "newlib" ]; then
  LIBMODE_NEWLIB=true
elif [ ${LIBMODE} == "glibc" ]; then
  LIBMODE_GLIBC=true
  UTMAN_BUILD_ARM=false
else
  Fatal "Unknown library mode ${LIBMODE}"
fi
readonly LIBMODE_NEWLIB
readonly LIBMODE_GLIBC

# TODO(pdox): Decide what the target should really permanently be
readonly CROSS_TARGET_ARM=arm-none-linux-gnueabi
readonly BINUTILS_TARGET=arm-pc-nacl
readonly REAL_CROSS_TARGET=pnacl

readonly TC_ROOT="${NACL_ROOT}/toolchain"
readonly PNACL_ROOT="${TC_ROOT}/pnacl_${BUILD_PLATFORM}_${HOST_ARCH}_${LIBMODE}"
readonly PNACL_BIN="${PNACL_ROOT}/bin"
readonly DRIVER_DIR="${NACL_ROOT}/tools/llvm/driver"
readonly ARM_ARCH=armv7-a
readonly ARM_FPU=vfp

readonly BASE_INSTALL_DIR="${PNACL_ROOT}/pkg"
readonly NEWLIB_INSTALL_DIR="${BASE_INSTALL_DIR}/newlib"
readonly GLIBC_INSTALL_DIR="${BASE_INSTALL_DIR}/glibc"
readonly LLVM_INSTALL_DIR="${BASE_INSTALL_DIR}/llvm"
readonly LLVM_GCC_INSTALL_DIR="${BASE_INSTALL_DIR}/llvm-gcc"
readonly LIBSTDCPP_INSTALL_DIR="${BASE_INSTALL_DIR}/libstdcpp"
readonly BINUTILS_INSTALL_DIR="${BASE_INSTALL_DIR}/binutils"
readonly SYSROOT_DIR="${PNACL_ROOT}/sysroot"
readonly LDSCRIPTS_DIR="${PNACL_ROOT}/ldscripts"
readonly GCC_VER="4.2.1"

readonly NNACL_ROOT="${TC_ROOT}/${SCONS_BUILD_PLATFORM}_x86_newlib"
readonly NNACL_GLIBC_ROOT="${TC_ROOT}/${SCONS_BUILD_PLATFORM}_x86"

readonly BFD_PLUGIN_DIR="${BINUTILS_INSTALL_DIR}/lib/bfd-plugins"

readonly MAKE_OPTS="-j${UTMAN_CONCURRENCY} VERBOSE=1"

readonly NONEXISTENT_PATH="/going/down/the/longest/road/to/nowhere"

# For speculative build status output. ( see status function )
# Leave this blank, it will be filled during processing.
SPECULATIVE_REBUILD_SET=""

# The directory in which we we keep src dirs (from hg repos)
# and objdirs. These should be ABSOLUTE paths.

readonly TC_SRC="${NACL_ROOT}/hg"
readonly TC_BUILD="${TC_ROOT}/hg-build-${LIBMODE}"

# The location of sources (absolute)
readonly TC_SRC_LLVM="${TC_SRC}/llvm"
readonly TC_SRC_LLVM_GCC="${TC_SRC}/llvm-gcc"
readonly TC_SRC_BINUTILS="${TC_SRC}/binutils"
readonly TC_SRC_NEWLIB="${TC_SRC}/newlib"
readonly TC_SRC_COMPILER_RT="${TC_SRC}/compiler-rt"
readonly TC_SRC_LIBSTDCPP="${TC_SRC_LLVM_GCC}/llvm-gcc-4.2/libstdc++-v3"

# Unfortunately, binutils/configure generates this untracked file
# in the binutils source directory
readonly BINUTILS_MESS="${TC_SRC_BINUTILS}/binutils-2.20/opcodes/i386-tbl.h"

readonly SERVICE_RUNTIME_SRC="${NACL_ROOT}/src/trusted/service_runtime"
readonly EXPORT_HEADER_SCRIPT="${SERVICE_RUNTIME_SRC}/export_header.py"
readonly NACL_SYS_HEADERS="${SERVICE_RUNTIME_SRC}/include"
readonly NACL_SYS_TS="${TC_SRC}/nacl.sys.timestamp"
readonly NEWLIB_INCLUDE_DIR="${TC_SRC_NEWLIB}/newlib-trunk/newlib/libc/include"

# The location of each project. These should be absolute paths.
# If a tool below depends on a certain libc, then the build
# directory should have ${LIBMODE} in it to distinguish them.

readonly TC_BUILD_LLVM="${TC_BUILD}/llvm"
readonly TC_BUILD_LLVM_GCC="${TC_BUILD}/llvm-gcc"
readonly TC_BUILD_BINUTILS="${TC_BUILD}/binutils"
readonly TC_BUILD_BINUTILS_LIBERTY="${TC_BUILD}/binutils-liberty"
readonly TC_BUILD_NEWLIB="${TC_BUILD}/newlib"
readonly TC_BUILD_LIBSTDCPP="${TC_BUILD_LLVM_GCC}-arm/libstdcpp"
readonly TC_BUILD_COMPILER_RT="${TC_BUILD}/compiler_rt"

# These are fake directories, for storing the timestamp only
readonly TC_BUILD_EXTRASDK="${TC_BUILD}/extrasdk"

readonly TIMESTAMP_FILENAME="make-timestamp"

# PNaCl toolchain locations (absolute!)
readonly PNACL_TOOLCHAIN_ROOT="${PNACL_ROOT}"
readonly PNACL_BITCODE_ROOT="${PNACL_TOOLCHAIN_ROOT}/libs-bitcode"

readonly PNACL_ARM_ROOT="${PNACL_TOOLCHAIN_ROOT}/libs-arm"
readonly PNACL_X8632_ROOT="${PNACL_TOOLCHAIN_ROOT}/libs-x8632"
readonly PNACL_X8664_ROOT="${PNACL_TOOLCHAIN_ROOT}/libs-x8664"

# PNaCl client-translators (sandboxed) binary locations
readonly PNACL_SB_ROOT="${PNACL_ROOT}/tools-sb"
readonly PNACL_SB_X8632="${PNACL_SB_ROOT}/x8632"
readonly PNACL_SB_X8664="${PNACL_SB_ROOT}/x8664"
readonly PNACL_SB_UNIVERSAL="${PNACL_SB_ROOT}/universal"

# Location of PNaCl gcc/g++/as
readonly PNACL_GCC="${PNACL_BIN}/pnacl-gcc"
readonly PNACL_GPP="${PNACL_BIN}/pnacl-g++"
readonly PNACL_AR="${PNACL_BIN}/pnacl-ar"
readonly PNACL_RANLIB="${PNACL_BIN}/pnacl-ranlib"
readonly PNACL_AS="${PNACL_BIN}/pnacl-as"
readonly PNACL_LD="${PNACL_BIN}/pnacl-ld"
readonly PNACL_NM="${PNACL_BIN}/pnacl-nm"
readonly PNACL_TRANSLATE="${PNACL_BIN}/pnacl-translate"
readonly PNACL_READELF="${PNACL_BIN}/readelf"
readonly PNACL_SIZE="${PNACL_BIN}/size"

readonly PNACL_AS_ARM="${PNACL_BIN}/pnacl-arm-as"
readonly PNACL_AS_X8632="${PNACL_BIN}/pnacl-i686-as"
readonly PNACL_AS_X8664="${PNACL_BIN}/pnacl-x86_64-as"

# For a production (release) build, we want the sandboxed
# translator to only contain the code needed to handle
# its own architecture. For example, the translator shipped with
# an X86-32 browser would only be able to translate to X86-32 code.
# This is so that the translator binary is as small as possible.
#
# If SBTC_PRODUCTION is true, then the translators are built
# separately, one for each architecture, so that each translator
# can only target its own architecture.
#
# If SBTC_PRODUCTION is false, then we instead use PNaCl to
# build a `fat` translator which can target all supported
# architectures. This translator is built as a .pexe
# which can then be translated to each individual architecture.
SBTC_PRODUCTION=${SBTC_PRODUCTION:-false}

# Which toolchain to use for each arch.
if ${LIBMODE_NEWLIB}; then
  SBTC_BUILD_WITH_PNACL="arm x8632 x8664"
else
  SBTC_BUILD_WITH_PNACL="x8632 x8664"
fi

# Current milestones in each repo
# hg-update-all uses these
readonly LLVM_REV=69da56e5597a
readonly LLVM_GCC_REV=8713c8955201
readonly NEWLIB_REV=9bef47f82918
readonly BINUTILS_REV=0afa2765184c
readonly COMPILER_RT_REV=1a3a6ffb31ea

# Repositories
readonly REPO_LLVM_GCC="llvm-gcc.nacl-llvm-branches"
readonly REPO_LLVM="nacl-llvm-branches"
readonly REPO_NEWLIB="newlib.nacl-llvm-branches"
readonly REPO_BINUTILS="binutils.nacl-llvm-branches"
readonly REPO_COMPILER_RT="compiler-rt.nacl-llvm-branches"


# TODO(espindola): This should be ${CXX:-}, but llvm-gcc's configure has a
# bug that brakes the build if we do that.
CC=${CC:-gcc}
CXX=${CXX:-g++}
if ${HOST_ARCH_X8632} ; then
  # These are simple compiler wrappers to force 32bit builds
  # For bots and releases we build the toolchains
  # on the oldest system we care to support. Currently
  # that is a 32 bit hardy. The advantage of this is that we can build
  # the toolchain shared, reducing its size and allowing the use of
  # plugins. You can test them on your system by setting the
  # environment variable HOST_ARCH=x86_32 on a 64 bit system.
  # Make sure you clean all your build dirs
  # before switching arches.
  CC="${NACL_ROOT}/tools/llvm/mygcc32"
  CXX="${NACL_ROOT}/tools/llvm/myg++32"
fi

readonly CROSS_TARGET_AR=${BINUTILS_INSTALL_DIR}/bin/${BINUTILS_TARGET}-ar
readonly CROSS_TARGET_NM=${BINUTILS_INSTALL_DIR}/bin/${BINUTILS_TARGET}-nm
readonly CROSS_TARGET_RANLIB=\
${BINUTILS_INSTALL_DIR}/bin/${BINUTILS_TARGET}-ranlib
readonly ILLEGAL_TOOL=${PNACL_BIN}/pnacl-illegal

# NOTE: we do not expect the assembler or linker to be used for libs
#       hence the use of ILLEGAL_TOOL.
STD_ENV_FOR_LIBSTDCPP=(
  CC_FOR_BUILD="${CC}"
  CC="${PNACL_GCC}"
  CXX="${PNACL_GPP}"
  RAW_CXX_FOR_TARGET="${PNACL_GPP}"
  LD="${ILLEGAL_TOOL}"
  CFLAGS="--pnacl-arm-bias"
  CPPFLAGS="--pnacl-arm-bias"
  CXXFLAGS="--pnacl-arm-bias"
  CFLAGS_FOR_TARGET="--pnacl-arm-bias"
  CPPFLAGS_FOR_TARGET="--pnacl-arm-bias"
  CC_FOR_TARGET="${PNACL_GCC}"
  GCC_FOR_TARGET="${PNACL_GCC}"
  CXX_FOR_TARGET="${PNACL_GPP}"
  AR="${PNACL_AR}"
  AR_FOR_TARGET="${PNACL_AR}"
  NM_FOR_TARGET="${PNACL_NM}"
  RANLIB="${PNACL_RANLIB}"
  RANLIB_FOR_TARGET="${PNACL_RANLIB}"
  AS_FOR_TARGET="${ILLEGAL_TOOL}"
  LD_FOR_TARGET="${ILLEGAL_TOOL}"
  OBJDUMP_FOR_TARGET="${ILLEGAL_TOOL}" )

STD_ENV_FOR_NEWLIB=(
  CFLAGS_FOR_TARGET="--pnacl-arm-bias"
  CPPFLAGS_FOR_TARGET="--pnacl-arm-bias"
  CC_FOR_TARGET="${PNACL_GCC}"
  GCC_FOR_TARGET="${PNACL_GCC}"
  CXX_FOR_TARGET="${PNACL_GPP}"
  AR_FOR_TARGET="${PNACL_AR}"
  NM_FOR_TARGET="${PNACL_NM}"
  RANLIB_FOR_TARGET="${PNACL_RANLIB}"
  OBJDUMP_FOR_TARGET="${ILLEGAL_TOOL}"
  AS_FOR_TARGET="${ILLEGAL_TOOL}"
  LD_FOR_TARGET="${ILLEGAL_TOOL}"
  STRIP_FOR_TARGET="${ILLEGAL_TOOL}" )


# The gold plugin that we use is documented at
# http://llvm.org/docs/GoldPlugin.html
# Despite its name it is actually used by both gold and bfd. The changes to
# this file to enable its use are:
# * Build shared
# * --enable-gold and --enable-plugin when building binutils
# * --with-binutils-include when building binutils
# * linking the plugin in bfd-plugins

######################################################################
######################################################################
#
#                     < USER ACCESSIBLE FUNCTIONS >
#
######################################################################
######################################################################

#@-------------------------------------------------------------------------

#@ hg-info-all         - Show status of repositories
hg-info-all() {
  hg-pull-all

  hg-info "${TC_SRC_LLVM}"       ${LLVM_REV}
  hg-info "${TC_SRC_LLVM_GCC}"   ${LLVM_GCC_REV}
  hg-info "${TC_SRC_NEWLIB}"     ${NEWLIB_REV}
  hg-info "${TC_SRC_BINUTILS}"   ${BINUTILS_REV}
  hg-info "${TC_SRC_COMPILER_RT}" ${COMPILER_RT_REV}
}

#@ hg-update-all      - Update all repos to the latest stable rev
hg-update-all() {
  hg-update-llvm-gcc
  hg-update-llvm
  hg-update-newlib
  hg-update-binutils
  hg-update-compiler-rt
}

hg-assert-safe-to-update() {
  local name="$1"
  local dir="$2"
  local rev="$3"
  local defstr=$(echo "${name}" | tr '[a-z]-' '[A-Z]_')

  if ! hg-on-branch "${dir}" pnacl-sfi ; then
    Banner "hg/${name} is not on branch pnacl-sfi"
    exit -1
  fi

  if ! hg-has-changes "${dir}"; then
    return 0
  fi

  if hg-at-revision "${dir}" "${rev}" ; then
    return 0
  fi

  Banner \
    "                         ERROR                          " \
    "                                                        " \
    " hg/${name} needs to be updated to the stable revision  " \
    " but has local modifications.                           " \
    "                                                        " \
    " If your repository is behind stable, update it using:  " \
    "                                                        " \
    "        cd hg/${name}; hg update ${rev}                 " \
    "        (you may need to resolve conflicts)             " \
    "                                                        " \
    " If your repository is ahead of stable, then modify:    " \
    "   ${defstr}_REV   (in tools/llvm/utman.sh)             " \
    " to suppress this error message.                        "
  exit -1
}


hg-bot-sanity() {
  local name="$1"
  local dir="$2"

  if ! ${UTMAN_BUILDBOT} ; then
    return 0
  fi

  if ! hg-on-branch "${dir}" pnacl-sfi ||
     hg-has-changes "${dir}" ||
     hg-has-untracked "${dir}" ; then
    Banner "WARNING: hg/${name} is in an illegal state." \
           "         Wiping and trying again."
    rm -rf "${dir}"
    hg-checkout-${name}
  fi
}

hg-update-common() {
  local name="$1"
  local rev="$2"
  local dir="$3"

  # If this is a buildbot, do sanity checks here.
  hg-bot-sanity "${name}" "${dir}"

  # Make sure it is safe to update
  hg-assert-safe-to-update "${name}" "${dir}" "${rev}"

  if hg-at-revision "${dir}" "${rev}" ; then
    StepBanner "HG-UPDATE" "Repo ${name} already at ${rev}"
  else
    StepBanner "HG-UPDATE" "Updating ${name} to ${rev}"
    hg-pull "${dir}"
    hg-update "${dir}" ${rev}
  fi
}

#@ hg-update-llvm-gcc    - Update LLVM-GCC to the stable revision
hg-update-llvm-gcc() {
  hg-update-common "llvm-gcc" ${LLVM_GCC_REV} "${TC_SRC_LLVM_GCC}"
}

#@ hg-update-llvm        - Update LLVM to the stable revision
hg-update-llvm() {
  hg-update-common "llvm" ${LLVM_REV} "${TC_SRC_LLVM}"
}

#@ hg-update-newlib      - Update NEWLIB To the stable revision
hg-update-newlib() {
  # Clean the headers first, so that sanity checks inside
  # hg-update-common do not see any local modifications.
  newlib-nacl-headers-check
  newlib-nacl-headers-clean
  hg-update-common "newlib" ${NEWLIB_REV} "${TC_SRC_NEWLIB}"
  newlib-nacl-headers
}

#@ hg-update-binutils    - Update BINUTILS to the stable revision
hg-update-binutils() {
  # Clean the binutils generated file first, so that sanity checks
  # inside hg-update-common do not see any local modifications.
  binutils-mess-hide
  hg-update-common "binutils" ${BINUTILS_REV} "${TC_SRC_BINUTILS}"
  binutils-mess-unhide
}

#@ hg-update-compiler-rt - Update compiler-rt to the stable revision
hg-update-compiler-rt() {
  hg-update-common "compiler-rt" ${COMPILER_RT_REV} "${TC_SRC_COMPILER_RT}"
}

#@ hg-pull-all           - Pull all repos. (but do not update working copy)
#@ hg-pull-REPO          - Pull repository REPO.
#@                         (REPO can be llvm-gcc, llvm, newlib, binutils)
hg-pull-all() {
  StepBanner "HG-PULL" "Running 'hg pull' in all repos..."
  hg-pull-llvm-gcc
  hg-pull-llvm
  hg-pull-newlib
  hg-pull-binutils
  hg-pull-compiler-rt
}

hg-pull-llvm-gcc() {
  hg-pull "${TC_SRC_LLVM_GCC}"
}

hg-pull-llvm() {
  hg-pull "${TC_SRC_LLVM}"
}

hg-pull-newlib() {
  hg-pull "${TC_SRC_NEWLIB}"
}

hg-pull-binutils() {
  hg-pull "${TC_SRC_BINUTILS}"
}

hg-pull-compiler-rt() {
  hg-pull "${TC_SRC_COMPILER_RT}"
}

#@ hg-checkout-all       - check out mercurial repos needed to build toolchain
#@                          (skips repos which are already checked out)
hg-checkout-all() {
  StepBanner "HG-CHECKOUT-ALL"
  hg-checkout-llvm-gcc
  hg-checkout-llvm
  hg-checkout-binutils
  hg-checkout-newlib
  hg-checkout-compiler-rt
}

hg-checkout-llvm-gcc() {
  hg-checkout ${REPO_LLVM_GCC} ${TC_SRC_LLVM_GCC} ${LLVM_GCC_REV}
}

hg-checkout-llvm() {
  hg-checkout ${REPO_LLVM}     ${TC_SRC_LLVM}     ${LLVM_REV}
}

hg-checkout-binutils() {
  hg-checkout ${REPO_BINUTILS} ${TC_SRC_BINUTILS} ${BINUTILS_REV}
}

hg-checkout-newlib() {
  hg-checkout ${REPO_NEWLIB}   ${TC_SRC_NEWLIB}   ${NEWLIB_REV}
  newlib-nacl-headers
}

hg-checkout-compiler-rt() {
  hg-checkout ${REPO_COMPILER_RT}   ${TC_SRC_COMPILER_RT}   ${COMPILER_RT_REV}
}

#@ hg-clean              - Remove all repos. (WARNING: local changes are lost)
hg-clean() {
  StepBanner "HG-CLEAN"

  echo "Are you sure?"
  echo "This will DELETE all source repositories under 'native_client/hg'"
  echo "Any local changes will be lost."
  echo ""
  echo "Type YES to confirm: "
  read CONFIRM_TEXT

  if [ $CONFIRM_TEXT == "YES" ]; then
    StepBanner "HG-CLEAN" "Cleaning Mercurial repositories"
    rm -rf "${TC_SRC}"
  else
    StepBanner "HG-CLEAN" "Clean cancelled by user"
  fi
}

#@-------------------------------------------------------------------------

#@ download-trusted      - Download and Install trusted SDKs (arm,x86-32,x86-64)
#@                         (your untrusted build will not be modified)
download-trusted() {
  StepBanner "DOWNLOAD-TRUSTED" "Downloading trusted toolchains"
  local installdir="${PNACL_ROOT}"
  local tmpdir="${installdir}-backup"
  local dldir="${installdir}-downloaded"

  rm -rf "${dldir}"

  if [ -d "${tmpdir}" ]; then
    echo "ERROR: It is not expected that directory '${tmpdir}' exists."
    echo "       Please delete it if you don't need it"
    exit -1
  fi

  if [ -d "${installdir}" ]; then
    mv "${installdir}" "${tmpdir}"
  fi

  download-toolchains

  if [ -d "${installdir}" ]; then
    mv "${installdir}" "${dldir}"
  fi

  if [ -d "${tmpdir}" ]; then
    mv "${tmpdir}" "${installdir}"
  fi
}

#@-------------------------------------------------------------------------

#@ download-toolchains   - Download and Install all SDKs (arm,x86-32,x86-64)

download-toolchains() {
  gclient runhooks --force
}

#@-------------------------------------------------------------------------
#@ libs                  - install native libs and build bitcode libs
libs() {
  libs-clean
  libc

  # Build native versions of libgcc
  # TODO(robertm): change these to bitcode libs
  build-compiler-rt
  # NOTE: this currently depends on "llvm-gcc arm"
  build-libgcc_eh-bitcode arm


  libehsupport
  libstdcpp
  extrasdk
}

libc() {
  if ${LIBMODE_NEWLIB} ; then
    # TODO(pdox): Why is this step needed?
    sysroot
    newlib
  elif ${LIBMODE_GLIBC} ; then
    glibc
  fi
}


#@ everything            - Build and install untrusted SDK.
everything() {

  mkdir -p "${PNACL_ROOT}"

  # This is needed to build misc-tools and run ARM tests.
  # We check this early so that there are no surprises later, and we can
  # handle all user interaction early.
  check-for-trusted

  hg-checkout-all
  hg-update-all

  clean-install

  clean-logs

  binutils
  llvm
  driver
  llvm-gcc arm

  libs

  # NOTE: we delay the tool building till after the sdk is essentially
  #      complete, so that sdk sanity checks don't fail
  misc-tools

  verify
}

glibc() {
  StepBanner "GLIBC" "Copying glibc from NNaCl toolchain"

  mkdir -p "${PNACL_X8632_ROOT}"
  mkdir -p "${PNACL_X8664_ROOT}"
  mkdir -p "${GLIBC_INSTALL_DIR}"

  # Files in: lib/gcc/nacl64/4.4.3/[32]/
  local LIBS1="crtbegin.o crtbeginT.o crtbeginS.o crtend.o crtendS.o"

  # Files in: nacl64/lib[32]/
  local LIBS2="crt1.o crti.o crtn.o \
               libc.a libc_nonshared.a \
               libc-2.9.so libc.so libc.so.6 \
               libm-2.9.so libm.a libm.so libm.so.6 \
               libdl-2.9.so libdl.so.2 libdl.so libdl.a \
               libpthread-2.9.so libpthread.a libpthread.so \
               libpthread.so.0 libpthread_nonshared.a \
               runnable-ld.so \
               ld-2.9.so \
               ldscripts/elf_nacl.x ldscripts/elf64_nacl.x \
               ldscripts/elf_nacl.xs ldscripts/elf64_nacl.xs \
               ldscripts/elf_nacl.x.static ldscripts/elf64_nacl.x.static"

  for lib in ${LIBS1} ; do
    cp -a "${NNACL_GLIBC_ROOT}/lib/gcc/nacl64/4.4.3/32/${lib}" \
       "${PNACL_X8632_ROOT}"
    cp -a "${NNACL_GLIBC_ROOT}/lib/gcc/nacl64/4.4.3/${lib}" \
       "${PNACL_X8664_ROOT}"
  done

  for lib in ${LIBS2} ; do
    cp -a "${NNACL_GLIBC_ROOT}/nacl64/lib32/${lib}" "${PNACL_X8632_ROOT}"
    cp -a "${NNACL_GLIBC_ROOT}/nacl64/lib/${lib}" "${PNACL_X8664_ROOT}"
  done

  # ld-linux has different sonames across 32/64.
  # Create symlinks to make them look the same.
  # TODO(pdox): Can this be fixed in glibc?
  cp -a "${NNACL_GLIBC_ROOT}"/nacl64/lib32/ld-linux.so.2 \
     "${PNACL_X8632_ROOT}"
  ln -sf ld-linux.so.2 "${PNACL_X8632_ROOT}"/ld-linux-x86-64.so.2

  cp -a "${NNACL_GLIBC_ROOT}"/nacl64/lib/ld-linux-x86-64.so.2 \
     "${PNACL_X8664_ROOT}"
  ln -sf ld-linux-x86-64.so.2 "${PNACL_X8664_ROOT}"/ld-linux.so.2

  # Copy the glibc headers
  cp -a "${NNACL_GLIBC_ROOT}"/nacl64/include \
        "${GLIBC_INSTALL_DIR}"

  # We build our own C++, so we have our own headers.
  rm -rf "${GLIBC_INSTALL_DIR}"/include/c++
}

#@ all                   - Alias for 'everything'
all() {
  everything
}

#@ status                - Show status of build directories
status() {
  # TODO(robertm): this is currently broken
  StepBanner "BUILD STATUS"

  status-helper "BINUTILS"          binutils
  status-helper "LLVM"              llvm
  status-helper "GCC-STAGE1"        llvm-gcc

  status-helper "NEWLIB"            newlib
  status-helper "EXTRASDK"          extrasdk
  status-helper "LIBSTDCPP"         libstdcpp

}

status-helper() {
  local title="$1"
  local mod="$2"

  if ${mod}-needs-configure; then
    StepBanner "$title" "NEEDS FULL REBUILD"
    speculative-add "${mod}"
  elif ${mod}-needs-make; then
    StepBanner "$title" "NEEDS MAKE (INCREMENTAL)"
    speculative-add "${mod}"
  else
    StepBanner "$title" "OK (UP TO DATE)"
  fi
}

speculative-add() {
  local mod="$1"
  SPECULATIVE_REBUILD_SET="${SPECULATIVE_REBUILD_SET} ${mod}"
}

speculative-check() {
  local mod="$1"
  local search=$(echo "${SPECULATIVE_REBUILD_SET}" | grep -F "$mod")
  [ ${#search} -gt 0 ]
  return $?
}



#@ clean                 - Clean the build and install directories.
clean() {
  StepBanner "CLEAN" "Cleaning build, log, and install directories."

  clean-logs
  clean-build
  clean-install
  clean-scons
}

#@ fast-clean            - Clean everything except LLVM.
fast-clean() {
  local did_backup=false
  local backup_dir="${NACL_ROOT}/llvm-build-backup"

  if [ -d "${TC_BUILD_LLVM}" ]; then
    rm -rf "${backup_dir}"
    mv "${TC_BUILD_LLVM}" "${backup_dir}"
    did_backup=true
  fi

  clean

  if ${did_backup} ; then
    mkdir -p "${TC_BUILD}"
    mv "${backup_dir}" "${TC_BUILD_LLVM}"
  fi
}

binutils-mess-hide() {
  local messtmp="${TC_SRC}/binutils.tmp"
  if [ -f "${BINUTILS_MESS}" ] ; then
    mv "${BINUTILS_MESS}" "${messtmp}"
  fi
}

binutils-mess-unhide() {
  local messtmp="${TC_SRC}/binutils.tmp"
  if [ -f "${messtmp}" ] ; then
    mv "${messtmp}" "${BINUTILS_MESS}"
  fi
}

#+ clean-scons           - Clean scons-out directory
clean-scons() {
  rm -rf scons-out
}

#+ clean-build           - Clean all build directories
clean-build() {
  rm -rf "${TC_BUILD}"
}

#+ clean-install         - Clean install directories
clean-install() {
  rm -rf "${PNACL_ROOT}"
}

#+ libs-clean            - Removes the library directories
libs-clean() {
  StepBanner "LIBS-CLEAN" "Cleaning ${PNACL_TOOLCHAIN_ROOT}/libs-*"
  rm -rf ${PNACL_ARM_ROOT} \
         ${PNACL_BITCODE_ROOT} \
         ${PNACL_X8632_ROOT} \
         ${PNACL_X8664_ROOT}
}


#@-------------------------------------------------------------------------

#@ untrusted_sdk <file>  - Create untrusted SDK tarball from scratch
#@                          (clean + all + prune + tarball)
untrusted_sdk() {
  if [ ! -n "${1:-}" ]; then
    echo "Error: untrusted_sdk needs a tarball name." >&2
    exit 1
  fi

  clean
  everything
  prune

  install-translators srpc
  prune-translator-install srpc
  prune

  tarball $1
}

#+ prune                 - Prune toolchain
prune() {
  StepBanner "PRUNE" "Pruning toolchain"
  # ACCEPTABLE_SIZE should be much lower for real release,
  # but we are currently doing a debug build and not pruning
  # as aggressively as we could.
  local ACCEPTABLE_SIZE=300
  local dir_size_before=$(get_dir_size_in_mb ${PNACL_ROOT})

  SubBanner "Size before: ${PNACL_ROOT} ${dir_size_before}MB"
  echo "removing some static libs we do not have any use for"
  rm  -f "${NEWLIB_INSTALL_DIR}"/lib/lib*.a

  echo "stripping binaries"
  strip "${LLVM_GCC_INSTALL_DIR}"/libexec/gcc/${CROSS_TARGET_ARM}/${GCC_VER}/c*
  if ! strip "${LLVM_GCC_INSTALL_DIR}"/bin/* ||
     ! strip "${LLVM_INSTALL_DIR}"/bin/* ; then
    echo "NOTE: some failures during stripping are expected"
  fi

  echo "removing llvm headers"
  rm -rf "${LLVM_INSTALL_DIR}"/include/llvm*

  echo "removing .pyc files"
  rm -f "${PNACL_BIN}"/*.pyc

  if ${LIBMODE_GLIBC}; then
    echo "remove pnacl_cache directory"
    rm -rf "${PNACL_BITCODE_ROOT}"/pnacl_cache
  fi

  echo "remove driver log"
  rm -f "${PNACL_ROOT}"/driver.log

  local dir_size_after=$(get_dir_size_in_mb "${PNACL_ROOT}")
  SubBanner "Size after: ${PNACL_ROOT} ${dir_size_after}MB"

  if [[ ${dir_size_after} -gt ${ACCEPTABLE_SIZE} ]] ; then
    # TODO(pdox): Move this to the buildbot script so that
    # it can make the bot red without ruining the toolchain archive.
    echo "WARNING: size of toolchain exceeds ${ACCEPTABLE_SIZE}MB"
  fi

}

#+ tarball <filename>    - Produce tarball file
tarball() {
  if [ ! -n "${1:-}" ]; then
    echo "Error: tarball needs a tarball name." >&2
    exit 1
  fi

  RecordRevisionInfo
  local tarball="$1"
  StepBanner "TARBALL" "Creating tar ball ${tarball}"

  tar zcf "${tarball}" -C "${PNACL_ROOT}" .
}


#########################################################################
#                              < LLVM >
#########################################################################


#+-------------------------------------------------------------------------
#+ llvm                  - Configure, build and install LLVM.

llvm() {
  StepBanner "LLVM (HOST)"

  local srcdir="${TC_SRC_LLVM}"

  assert-dir "${srcdir}" "You need to checkout LLVM."

  if llvm-needs-configure; then
    llvm-configure
  else
    SkipBanner "LLVM" "configure"
  fi

  if llvm-needs-make; then
    llvm-make
  else
    SkipBanner "LLVM" "make"
  fi

  llvm-install
}

#+ llvm-clean            - Clean LLVM completely
llvm-clean() {
  StepBanner "LLVM" "Clean"
  local objdir="${TC_BUILD_LLVM}"
  rm -rf ${objdir}
  mkdir -p ${objdir}
}

# Default case - Optimized configure
LLVM_EXTRA_OPTIONS="--enable-optimized"

#+ llvm-configure        - Run LLVM configure
llvm-configure() {
  StepBanner "LLVM" "Configure"

  local srcdir="${TC_SRC_LLVM}"
  local objdir="${TC_BUILD_LLVM}"

  mkdir -p "${objdir}"
  spushd "${objdir}"

  # The --with-binutils-include is to allow llvm to build the gold plugin
  local binutils_include="${TC_SRC_BINUTILS}/binutils-2.20/include"
  RunWithLog "llvm.configure" \
      env -i PATH=/usr/bin/:/bin \
             MAKE_OPTS=${MAKE_OPTS} \
             CC="${CC}" \
             CXX="${CXX}" \
             ${srcdir}/llvm-trunk/configure \
             --disable-jit \
             --with-binutils-include=${binutils_include} \
             --enable-targets=x86,x86_64,arm \
             --target=${CROSS_TARGET_ARM} \
             --prefix="${LLVM_INSTALL_DIR}" \
             --with-llvmgccdir="${LLVM_GCC_INSTALL_DIR}" \
             ${LLVM_EXTRA_OPTIONS}


  spopd
}

#+ llvm-configure-dbg        - Run LLVM configure
#  Not used by default. Call manually.
llvm-configure-dbg() {
  StepBanner "LLVM" "Configure With Debugging"
  LLVM_EXTRA_OPTIONS="--disable-optimized \
        --enable-debug-runtime \
        --enable-assertions "
  llvm-configure
}


llvm-needs-configure() {
  [ ! -f "${TC_BUILD_LLVM}/config.status" ]
  return $?
}

llvm-needs-make() {
  local srcdir="${TC_SRC_LLVM}"
  local objdir="${TC_BUILD_LLVM}"

  ts-modified "$srcdir" "$objdir"
  return $?
}

#+ llvm-make             - Run LLVM 'make'
llvm-make() {
  StepBanner "LLVM" "Make"

  local srcdir="${TC_SRC_LLVM}"
  local objdir="${TC_BUILD_LLVM}"

  spushd "${objdir}"

  ts-touch-open "${objdir}"

  RunWithLog llvm.make \
    env -i PATH=/usr/bin/:/bin \
           MAKE_OPTS="${MAKE_OPTS}" \
           NACL_SANDBOX=0 \
           CC="${CC}" \
           CXX="${CXX}" \
           make ${MAKE_OPTS} all

  ts-touch-commit  "${objdir}"

  spopd
}

#+ llvm-install          - Install LLVM
llvm-install() {
  StepBanner "LLVM" "Install"

  spushd "${TC_BUILD_LLVM}"
  RunWithLog llvm.install \
       make ${MAKE_OPTS} install

  mkdir -p "${BFD_PLUGIN_DIR}"

  ln -sf ../../../llvm/lib/${SO_PREFIX}LLVMgold${SO_EXT} "${BFD_PLUGIN_DIR}"
  ln -sf ../../../llvm/lib/${SO_PREFIX}LTO${SO_EXT} "${BFD_PLUGIN_DIR}"

  # On Mac, libLTO seems to be loaded from lib/
  ln -sf ../../llvm/lib/${SO_PREFIX}LTO${SO_EXT} "${BINUTILS_INSTALL_DIR}"/lib

  spopd
}



#########################################################################
#     < GCC STAGE 1 >
#########################################################################

# Build "pregcc" which is a gcc that does not depend on having glibc/newlib
# already compiled. This also generates some important headers (confirm this).
#
# NOTE: depends on newlib source being set up so we can use it to set
#       up a sysroot.
#


LLVM_GCC_SETUP=false
llvm-gcc-setup() {
  # If this is an internal invocation, don't setup again.
  if ${LLVM_GCC_SETUP} && [ $# -eq 0 ]; then
    return 0
  fi

  if [ $# -ne 1 ] ; then
    Fatal "Please specify architecture: x86-32, x86-64, arm"
  fi
  local arch=$1

  case ${arch} in
    arm) LLVM_GCC_TARGET=${CROSS_TARGET_ARM} ;;
    x86-32) LLVM_GCC_TARGET=${CROSS_TARGET_X86_32} ;;
    x86-64) LLVM_GCC_TARGET=${CROSS_TARGET_X86_64} ;;
    *) Fatal "Unrecognized architecture ${arch}" ;;
  esac
  LLVM_GCC_SETUP=true
  LLVM_GCC_ARCH=${arch}
  LLVM_GCC_BUILD_DIR="${TC_BUILD_LLVM_GCC}-${LLVM_GCC_ARCH}"
  return 0
}

#+-------------------------------------------------------------------------
#+ llvm-gcc            - build and install pre-gcc
llvm-gcc() {
  llvm-gcc-setup "$@"
  StepBanner "LLVM-GCC (HOST) for ${LLVM_GCC_ARCH}"

  if llvm-gcc-needs-configure; then
    llvm-gcc-clean
    llvm-gcc-configure
  else
    SkipBanner "LLVM-GCC ${LLVM_GCC_ARCH}" "configure"
  fi

  # We must always make before we do make install, because
  # the build must occur in a patched environment.
  # http://code.google.com/p/nativeclient/issues/detail?id=1128
  llvm-gcc-make

  llvm-gcc-install
}

#+ sysroot               - setup initial sysroot
sysroot() {
  StepBanner "LLVM-GCC" "Setting up initial sysroot"

  local sys_include="${SYSROOT_DIR}/include"
  local sys_include2="${SYSROOT_DIR}/sys-include"

  rm -rf "${sys_include}" "${sys_include2}"
  mkdir -p "${sys_include}"
  ln -sf "${sys_include}" "${sys_include2}"
  cp -r "${NEWLIB_INCLUDE_DIR}"/* "${sys_include}"
}

#+ llvm-gcc-clean      - Clean gcc stage 1
llvm-gcc-clean() {
  llvm-gcc-setup "$@"
  StepBanner "LLVM-GCC" "Clean"
  local objdir="${LLVM_GCC_BUILD_DIR}"
  rm -rf "${objdir}"
}

llvm-gcc-needs-configure() {
  llvm-gcc-setup "$@"
  speculative-check "llvm" && return 0
  ts-newer-than "${TC_BUILD_LLVM}" \
                "${LLVM_GCC_BUILD_DIR}" && return 0
  [ ! -f "${LLVM_GCC_BUILD_DIR}/config.status" ]
  return $?
}

#+ llvm-gcc-configure  - Configure GCC stage 1
llvm-gcc-configure() {
  llvm-gcc-setup "$@"
  StepBanner "LLVM-GCC" "Configure ${LLVM_GCC_TARGET}"

  local srcdir="${TC_SRC_LLVM_GCC}"
  local objdir="${LLVM_GCC_BUILD_DIR}"

  mkdir -p "${objdir}"
  spushd "${objdir}"

  # NOTE: hack, assuming presence of x86/32 toolchain (used for both 32/64)
  local config_opts=""
  case ${LLVM_GCC_ARCH} in
      arm)
          config_opts="--with-as=${PNACL_AS_ARM} \
                       --with-arch=${ARM_ARCH} \
                       --with-fpu=${ARM_FPU}"
          ;;
      x86-32)
          config_opts="--with-as=${PNACL_AS_X8632}"
          ;;
      x86-64)
          config_opts="--with-as=${PNACL_AS_X8664}"
          ;;
  esac

  local flags=""
  if ${LIBMODE_NEWLIB}; then
    flags+="--with-newlib"
  fi

  RunWithLog llvm-pregcc-${LLVM_GCC_ARCH}.configure \
      env -i PATH=/usr/bin/:/bin \
             CC="${CC}" \
             CXX="${CXX}" \
             CFLAGS="-Dinhibit_libc" \
             AR_FOR_TARGET="${CROSS_TARGET_AR}" \
             RANLIB_FOR_TARGET="${CROSS_TARGET_RANLIB}" \
             NM_FOR_TARGET="${CROSS_TARGET_NM}" \
             ${srcdir}/llvm-gcc-4.2/configure \
               --prefix="${LLVM_GCC_INSTALL_DIR}" \
               --enable-llvm="${LLVM_INSTALL_DIR}" \
               ${flags} \
               --disable-libmudflap \
               --disable-decimal-float \
               --disable-libssp \
               --disable-libgomp \
               --disable-multilib \
               --enable-languages=c,c++ \
               --disable-threads \
               --disable-libstdcxx-pch \
               --disable-shared \
               --without-headers \
               ${config_opts} \
               --target=${LLVM_GCC_TARGET}

  spopd
}

llvm-gcc-make() {
  llvm-gcc-setup "$@"
  local srcdir="${TC_SRC_LLVM_GCC}"
  local objdir="${LLVM_GCC_BUILD_DIR}"
  spushd ${objdir}

  StepBanner "LLVM-GCC" "Make (Stage 1)"

  ts-touch-open "${objdir}"

  RunWithLog llvm-pregcc-${LLVM_GCC_ARCH}.make \
       env -i PATH=/usr/bin/:/bin \
              CC="${CC}" \
              CXX="${CXX}" \
              CFLAGS="-Dinhibit_libc" \
              make ${MAKE_OPTS} all-gcc

  ts-touch-commit "${objdir}"

  spopd
}

#+ build-libgcc_eh-bitcode - build/install bitcode version of libgcc_eh
build-libgcc_eh-bitcode() {
  # NOTE: For simplicity we piggyback the libgcc_eh build onto a preconfigured
  #       objdir. So, to be safe, you have to run gcc-stage1-make first
  local target=$1
  local srcdir="${TC_SRC_LLVM_GCC}"
  local objdir="${TC_BUILD_LLVM_GCC}-${target}"
  spushd ${objdir}/gcc
  StepBanner "bitcode libgcc_eh" "cleaning"
  RunWithLog libgcc_eh.clean \
      env -i PATH=/usr/bin/:/bin \
             make clean-target-libgcc

  # NOTE: usually gcc/libgcc.mk is generate and invoked implicitly by
  #       gcc/Makefile.
  #       Since we are calling it directly we need to make up for some
  #       missing flags, e.g.  include paths ann defines like
  #       'ATTRIBUTE_UNUSED' which is used to mark unused function
  #       parameters.
  #       The arguments were gleaned from build logs.
  StepBanner "bitcode libgcc_eh" "building"
  RunWithLog libgcc_eh.bitcode.make \
       env -i PATH=/usr/bin/:/bin \
              "${STD_ENV_FOR_LIBSTDCPP[@]}" \
              "INCLUDES=-I${srcdir}/llvm-gcc-4.2/include -I${srcdir}/llvm-gcc-4.2/gcc -I." \
              "LIBGCC2_CFLAGS=-DATTRIBUTE_UNUSED= -DHOST_BITS_PER_INT=32 -Dinhibit_libc  -DIN_GCC -DCROSS_DIRECTORY_STRUCTURE " \
              "AR_CREATE_FOR_TARGET=${PNACL_AR} rc" \
              make ${MAKE_OPTS} -f libgcc.mk libgcc_eh.a

  StepBanner "bitcode libgcc_eh" "installing"
  # removed the old native versions of libgcc_eh if any
  rm -f "${PNACL_ROOT}"/libs-*/libgcc_eh.a
  # install the new bitcode version
  mkdir -p "${PNACL_BITCODE_ROOT}"
  cp libgcc_eh.a "${PNACL_BITCODE_ROOT}"
  spopd
}

#+ build-compiler-rt - build/install llvm's replacement for libgcc.a
build-compiler-rt() {
  src="${TC_SRC_COMPILER_RT}/compiler-rt/lib"
  mkdir -p "${TC_BUILD_COMPILER_RT}"
  spushd "${TC_BUILD_COMPILER_RT}"

  StepBanner "COMPILER-RT (LIBGCC)"
  for arch in arm x86-32 x86-64; do
    StepBanner "compiler rt" "build ${arch}"
    rm -rf "${arch}"
    mkdir -p "${arch}"
    spushd "${arch}"
    RunWithLog libgcc.${arch}.make \
        make -j ${UTMAN_CONCURRENCY} -f ${src}/Makefile-pnacl libgcc.a \
          "SRC_DIR=${src}" \
          "CC=${PNACL_GCC}" \
          "AR=${PNACL_AR}" \
          "CFLAGS=-arch ${arch} --pnacl-allow-translate -O3 -fPIC"
    spopd
  done

  StepBanner "compiler rt" "install all"
  ls -l */libgcc.a

  mkdir -p "${PNACL_ARM_ROOT}"
  cp arm/libgcc.a "${PNACL_ARM_ROOT}/"

  mkdir -p "${PNACL_X8632_ROOT}"
  cp x86-32/libgcc.a "${PNACL_X8632_ROOT}/"

  mkdir -p "${PNACL_X8664_ROOT}"
  cp x86-64/libgcc.a "${PNACL_X8664_ROOT}/"
  spopd
}

#+ build-compiler-rt-bitcode - build/install llvm's replacement for libgcc.a
#                              as bitcode - this is EXPERIMENTAL
build-compiler-rt-bitcode() {
  src="${TC_SRC_COMPILER_RT}/compiler-rt/lib"
  mkdir -p "${TC_BUILD_COMPILER_RT}"
  spushd "${TC_BUILD_COMPILER_RT}"

  StepBanner "COMPILER-RT (LIBGCC) bitcode"
  StepBanner "compiler rt bitcode" "build"
  mkdir -p bitcode
  spushd bitcode
  RunWithLog libgcc.bitcode.make \
    make -j ${UTMAN_CONCURRENCY} -f ${src}/Makefile-pnacl libgcc.a \
      "SRC_DIR=${src}" \
      "CC=${PNACL_GCC}" \
      "AR=${PNACL_AR}" \
      "CFLAGS=-O3"
  spopd


  StepBanner "compiler rt bitcode" "install"
  rm -f "${PNACL_ARM_ROOT}/libgcc.a" \
        "${PNACL_X8632_ROOT}/libgcc.a" \
        "${PNACL_X8664_ROOT}/libgcc.a" \
        "${PNACL_BITCODE_ROOT}/libgcc.a"

  mkdir -p "${PNACL_BITCODE_ROOT}"
  ls -l bitcode/libgcc.a
  cp bitcode/libgcc.a "${PNACL_BITCODE_ROOT}"

  spopd
}

#+ llvm-gcc-install    - Install GCC stage 1
llvm-gcc-install() {
  llvm-gcc-setup "$@"
  StepBanner "LLVM-GCC" "Install ${LLVM_GCC_ARCH}"

  local objdir="${LLVM_GCC_BUILD_DIR}"
  spushd "${objdir}"

  RunWithLog llvm-pregcc-${LLVM_GCC_ARCH}.install \
       env -i PATH=/usr/bin/:/bin \
              CC="${CC}" \
              CXX="${CXX}" \
              CFLAGS="-Dinhibit_libc" \
              make ${MAKE_OPTS} install

  spopd
}

#########################################################################
#########################################################################
#                          < LIBSTDCPP >
#########################################################################
#########################################################################

#+ libstdcpp             - build and install libstdcpp in bitcode
libstdcpp() {
  StepBanner "LIBSTDCPP (BITCODE)"

  if libstdcpp-needs-configure; then
    libstdcpp-clean
    libstdcpp-configure
  else
    SkipBanner "LIBSTDCPP" "configure"
  fi

  if libstdcpp-needs-make; then
    libstdcpp-make
  else
    SkipBanner "LIBSTDCPP" "make"
  fi

  libstdcpp-install
}

#+ libstdcpp-clean - clean libstdcpp in bitcode
libstdcpp-clean() {
  StepBanner "LIBSTDCPP" "Clean"
  rm -rf "${TC_BUILD_LIBSTDCPP}"
}

libstdcpp-needs-configure() {
  speculative-check "llvm-gcc" && return 0
  ts-newer-than "${TC_BUILD_LLVM_GCC}-${CROSS_TARGET_ARM}" \
                "${TC_BUILD_LIBSTDCPP}" && return 0
  [ ! -f "${TC_BUILD_LIBSTDCPP}/config.status" ]
  return #?
}

#+ libstdcpp-configure - configure libstdcpp for bitcode
libstdcpp-configure() {
  StepBanner "LIBSTDCPP" "Configure"
  local srcdir="${TC_SRC_LIBSTDCPP}"
  local objdir="${TC_BUILD_LIBSTDCPP}"

  mkdir -p "${objdir}"
  spushd "${objdir}"

  local flags=""
  if ${LIBMODE_NEWLIB}; then
    flags+="--with-newlib --disable-shared"
  elif ${LIBMODE_GLIBC}; then
    # TODO(pdox): Fix right away.
    flags+="--disable-shared"
  else
    Fatal "Unknown library mode"
  fi

  RunWithLog llvm-gcc.configure_libstdcpp \
      env -i PATH=/usr/bin/:/bin \
        "${STD_ENV_FOR_LIBSTDCPP[@]}" \
        "${srcdir}"/configure \
          CC=${PNACL_GCC} \
          CXX=${PNACL_GPP} \
          --host="${CROSS_TARGET_ARM}" \
          --prefix="${LIBSTDCPP_INSTALL_DIR}" \
          --enable-llvm="${LLVM_INSTALL_DIR}" \
          ${flags} \
          --disable-libstdcxx-pch \
          --enable-languages=c,c++ \
          --target=${CROSS_TARGET_ARM} \
          --with-arch=${ARM_ARCH} \
          --srcdir="${srcdir}"
  spopd
}

libstdcpp-needs-make() {
  local srcdir="${TC_SRC_LIBSTDCPP}"
  local objdir="${TC_BUILD_LIBSTDCPP}"

  ts-modified "$srcdir" "$objdir"
  return $?
}

#+ libstdcpp-make - Make libstdcpp in bitcode
libstdcpp-make() {
  StepBanner "LIBSTDCPP" "Make"
  local srcdir="${TC_SRC_LIBSTDCPP}"
  local objdir="${TC_BUILD_LIBSTDCPP}"

  ts-touch-open "${objdir}"

  spushd "${objdir}"
  RunWithLog llvm-gcc.make_libstdcpp \
    env -i PATH=/usr/bin/:/bin \
        make \
        "${STD_ENV_FOR_LIBSTDCPP[@]}" \
        ${MAKE_OPTS}
  spopd

  ts-touch-commit "${objdir}"
}

#+ libstdcpp-install - Install libstdcpp in bitcode
libstdcpp-install() {
  StepBanner "LIBSTDCPP" "Install"
  local objdir="${TC_BUILD_LIBSTDCPP}"

  spushd "${objdir}"

  # install headers (=install-data)
  # for good measure make sure we do not keep any old headers
  rm -rf "${PNACL_ROOT}/include/c++"
  RunWithLog llvm-gcc.install_libstdcpp \
    make \
    "${STD_ENV_FOR_LIBSTDCPP[@]}" \
    ${MAKE_OPTS} install-data

  # Install bitcode library
  mkdir -p "${PNACL_BITCODE_ROOT}"
  cp "${objdir}/src/.libs/libstdc++.a" "${PNACL_BITCODE_ROOT}"

  spopd
}

#+ misc-tools            - Build and install sel_ldr and validator for ARM.
misc-tools() {
  if ${UTMAN_BUILD_ARM} ; then
    StepBanner "MISC-ARM" "Building sel_ldr (ARM)"

    # TODO(robertm): revisit some of these options
    RunWithLog arm_sel_ldr \
      ./scons MODE=opt-host \
      platform=arm \
      sdl=none \
      naclsdk_validate=0 \
      sysinfo=0 \
      sel_ldr
    rm -rf  "${PNACL_ROOT}/tools-arm"
    mkdir "${PNACL_ROOT}/tools-arm"
    local sconsdir="scons-out/opt-${SCONS_BUILD_PLATFORM}-arm"
    cp "${sconsdir}/obj/src/trusted/service_runtime/sel_ldr" \
       "${PNACL_ROOT}/tools-arm"
  else
    StepBanner "MISC-ARM" "Skipping ARM sel_ldr (No trusted ARM toolchain)"
  fi

  if ${BUILD_PLATFORM_LINUX} ; then
    StepBanner "MISC-ARM" "Building validator (ARM)"
    RunWithLog arm_ncval_core \
      ./scons MODE=opt-host \
      targetplatform=arm \
      sysinfo=0 \
      arm-ncval-core
    rm -rf  "${PNACL_ROOT}/tools-x86"
    mkdir "${PNACL_ROOT}/tools-x86"
    cp scons-out/opt-linux-x86-32-to-arm/obj/src/trusted/validator_arm/\
arm-ncval-core ${PNACL_ROOT}/tools-x86
  else
    StepBanner "MISC-ARM" "Skipping ARM validator (Not yet supported on Mac)"
  fi
}


#########################################################################
#     < BINUTILS >
#########################################################################

#+-------------------------------------------------------------------------
#+ binutils          - Build and install binutils for ARM
binutils() {
  StepBanner "BINUTILS (HOST)"

  local srcdir="${TC_SRC_BINUTILS}"

  assert-dir "${srcdir}" "You need to checkout binutils."

  if binutils-needs-configure; then
    binutils-clean
    binutils-configure
  else
    SkipBanner "BINUTILS" "configure"
  fi

  if binutils-needs-make; then
    binutils-make
  else
    SkipBanner "BINUTILS" "make"
  fi

  binutils-install
}

#+ binutils-clean    - Clean binutils
binutils-clean() {
  StepBanner "BINUTILS" "Clean"
  local objdir="${TC_BUILD_BINUTILS}"
  rm -rf ${objdir}
}

#+ binutils-configure- Configure binutils for ARM
binutils-configure() {
  StepBanner "BINUTILS" "Configure"

  local srcdir="${TC_SRC_BINUTILS}"
  local objdir="${TC_BUILD_BINUTILS}"

  # enable multiple targets so that we can use the same ar with all .o files
  local targ="arm-pc-nacl,i686-pc-nacl,x86_64-pc-nacl"
  mkdir -p "${objdir}"
  spushd "${objdir}"

  # --enable-checking is to avoid a build failure:
  #   tc-arm.c:2489: warning: empty body in an if-statement
  # The --enable-gold and --enable-plugins options are on so that we
  # can use gold's support for plugin to link PNaCl modules.

  # TODO(pdox): Building binutils for nacl/nacl64 target currently requires
  # providing NACL_ALIGN_* defines. This should really be defined inside
  # binutils instead.
  RunWithLog binutils.configure \
    env -i \
    PATH="/usr/bin:/bin" \
    CC="${CC}" \
    CXX="${CXX}" \
    CFLAGS="-DNACL_ALIGN_BYTES=32 -DNACL_ALIGN_POW2=5" \
    ${srcdir}/binutils-2.20/configure --prefix="${BINUTILS_INSTALL_DIR}" \
                                      --target=${BINUTILS_TARGET} \
                                      --enable-targets=${targ} \
                                      --enable-checking \
                                      --enable-gold=yes \
                                      --enable-ld=yes \
                                      --enable-plugins \
                                      --disable-werror \
                                      --with-sysroot="${NONEXISTENT_PATH}"
  # There's no point in setting the correct path as sysroot, because we
  # want the toolchain to be relocatable. The driver will use ld command-line
  # option --sysroot= to override this value and set it to the correct path.
  # However, we need to include --with-sysroot during configure to get this
  # option. So fill in a non-sense, non-existent path.

  spopd
}

binutils-needs-configure() {
  [ ! -f "${TC_BUILD_BINUTILS}/config.status" ]
  return $?
}

binutils-needs-make() {
  local srcdir="${TC_SRC_BINUTILS}"
  local objdir="${TC_BUILD_BINUTILS}"
  local ret=1
  binutils-mess-hide
  ts-modified "$srcdir" "$objdir" && ret=0
  binutils-mess-unhide
  return ${ret}
}

#+ binutils-make     - Make binutils for ARM
binutils-make() {
  StepBanner "BINUTILS" "Make"
  local srcdir="${TC_SRC_BINUTILS}"
  local objdir="${TC_BUILD_BINUTILS}"
  spushd "${objdir}"

  ts-touch-open "${objdir}"

  RunWithLog binutils.make \
    env -i PATH="/usr/bin:/bin" \
    make ${MAKE_OPTS}

  ts-touch-commit "${objdir}"

  spopd
}

#+ binutils-install  - Install binutils for ARM
binutils-install() {
  StepBanner "BINUTILS" "Install"
  local objdir="${TC_BUILD_BINUTILS}"
  spushd "${objdir}"

  RunWithLog binutils.install \
    env -i PATH="/usr/bin:/bin" \
    make \
      install ${MAKE_OPTS}

  spopd

  # Binutils builds readelf and size, but doesn't install it.
  mkdir -p "${PNACL_BIN}"
  cp -f "${objdir}"/binutils/readelf "${PNACL_READELF}"
  cp -f "${objdir}"/binutils/size "${PNACL_SIZE}"
}

#+-------------------------------------------------------------------------
#+ binutils-liberty      - Build native binutils libiberty
binutils-liberty() {
  local srcdir="${TC_SRC_BINUTILS}"

  assert-dir "${srcdir}" "You need to checkout binutils."

  if binutils-liberty-needs-configure; then
    binutils-liberty-clean
    binutils-liberty-configure
  else
    SkipBanner "BINUTILS-LIBERTY" "configure"
  fi

  if binutils-liberty-needs-make; then
    binutils-liberty-make
  else
    SkipBanner "BINUTILS-LIBERTY" "make"
  fi
}

binutils-liberty-needs-configure() {
  [ ! -f "${TC_BUILD_BINUTILS_LIBERTY}/config.status" ]
  return $?
}

#+ binutils-liberty-clean    - Clean binutils-liberty
binutils-liberty-clean() {
  StepBanner "BINUTILS-LIBERTY" "Clean"
  local objdir="${TC_BUILD_BINUTILS_LIBERTY}"
  rm -rf ${objdir}
}

#+ binutils-liberty-configure - Configure binutils-liberty
binutils-liberty-configure() {
  StepBanner "BINUTILS-LIBERTY" "Configure"

  local srcdir="${TC_SRC_BINUTILS}"
  local objdir="${TC_BUILD_BINUTILS_LIBERTY}"

  mkdir -p "${objdir}"
  spushd "${objdir}"
  RunWithLog binutils.liberty.configure \
      env -i \
      PATH="/usr/bin:/bin" \
      CC="${CC}" \
      CXX="${CXX}" \
      ${srcdir}/binutils-2.20/configure
  spopd
}

binutils-liberty-needs-make() {
  local srcdir="${TC_SRC_BINUTILS}"
  local objdir="${TC_BUILD_BINUTILS_LIBERTY}"

  ts-modified "$srcdir" "$objdir"
  return $?
}

#+ binutils-liberty-make - Make binutils-liberty
binutils-liberty-make() {
  StepBanner "BINUTILS-LIBERTY" "Make"
  local srcdir="${TC_SRC_BINUTILS}"
  local objdir="${TC_BUILD_BINUTILS_LIBERTY}"
  spushd "${objdir}"

  ts-touch-open "${objdir}"

  RunWithLog binutils.liberty.make \
      env -i \
      PATH="/usr/bin:/bin" \
      CC="${CC}" \
      CXX="${CXX}" \
      make ${MAKE_OPTS} all-libiberty

  ts-touch-commit "${objdir}"

  spopd
}

#########################################################################
#     CLIENT BINARIES (SANDBOXED)
#########################################################################

check-sb-mode() {
  local mode=$1
  if [ ${mode} != "srpc" ] && [ ${mode} != "nonsrpc" ]; then
    echo "ERROR: Unsupported mode. Choose one of: srpc, nonsrpc"
    exit -1
  fi
}

check-sb-arch() {
  local arch=$1
  for valid_arch in x8632 x8664 arm universal ; do
    if [ "${arch}" == "${valid_arch}" ] ; then
      return
    fi
  done

  Fatal "ERROR: Unsupported arch. Choose one of: x8632, x8664, arm, universal"
}

LLVM_SB_SETUP=false
llvm-sb-setup() {
  local bitsize
  local prefix
  local flags=""

  if ${LLVM_SB_SETUP} && [ $# -eq 0 ]; then
    return 0
  fi

  if [ $# -ne 2 ] ; then
    Fatal "Please specify arch and mode"
  fi

  LLVM_SB_SETUP=true

  LLVM_SB_ARCH=$1
  LLVM_SB_MODE=$2
  check-sb-arch ${LLVM_SB_ARCH}
  check-sb-mode ${LLVM_SB_MODE}

  LLVM_SB_LOG_PREFIX="llvm.sb.${LLVM_SB_ARCH}.${LLVM_SB_MODE}"
  LLVM_SB_OBJDIR="${TC_BUILD}/llvm-sb-${arch}-${mode}"
  if ${LIBMODE_NEWLIB}; then
    flags+=" -static"
  fi

  case ${mode} in
    srpc)    flags+=" -DNACL_SRPC" ;;
    nonsrpc) ;;
  esac

  # Speed things up by avoiding an intermediate step
  flags+=" --pnacl-skip-ll"

  LLVM_SB_CONFIGURE_ENV=(
    AR="${PNACL_AR}" \
    AS="${PNACL_AS}" \
    CC="${PNACL_GCC} ${flags}" \
    CXX="${PNACL_GPP} ${flags}" \
    LD="${PNACL_LD} ${flags}" \
    NM="${PNACL_NM}" \
    RANLIB="${PNACL_RANLIB}" \
    LDFLAGS="") # TODO(pdox): Support -s
}

#+-------------------------------------------------------------------------
#+ llvm-sb <arch> <mode> - Build and install llvm tools (sandboxed)
llvm-sb() {
  llvm-sb-setup "$@"
  local srcdir="${TC_SRC_LLVM}"
  assert-dir "${srcdir}" "You need to checkout llvm."

  if llvm-sb-needs-configure ; then
    llvm-sb-clean
    llvm-sb-configure
  else
    SkipBanner "LLVM-SB" "configure ${LLVM_SB_ARCH} ${LLVM_SB_MODE}"
  fi

  if llvm-sb-needs-make; then
    llvm-sb-make
  else
    SkipBanner "LLVM-SB" "make"
  fi

  llvm-sb-install
}

llvm-sb-needs-configure() {
  llvm-sb-setup "$@"
  [ ! -f "${LLVM_SB_OBJDIR}/config.status" ]
  return $?
}

# llvm-sb-clean          - Clean llvm tools (sandboxed)
llvm-sb-clean() {
  llvm-sb-setup "$@"
  StepBanner "LLVM-SB" "Clean ${LLVM_SB_ARCH} ${LLVM_SB_MODE}"
  local objdir="${LLVM_SB_OBJDIR}"

  rm -rf "${objdir}"
  mkdir -p "${objdir}"
}

# llvm-sb-configure - Configure llvm tools (sandboxed)
llvm-sb-configure() {
  llvm-sb-setup "$@"

  StepBanner "LLVM-SB" "Configure ${LLVM_SB_ARCH} ${LLVM_SB_MODE}"
  local srcdir="${TC_SRC_LLVM}"
  local objdir="${LLVM_SB_OBJDIR}"
  local installdir="${PNACL_SB_ROOT}/${arch}/${mode}"
  local targets=""
  case ${LLVM_SB_ARCH} in
    x8632) targets=x86 ;;
    x8664) targets=x86_64 ;;
    arm) targets=arm ;;
    universal) targets=x86,x86_64,arm ;;
  esac

  spushd "${objdir}"
  RunWithLog \
      ${LLVM_SB_LOG_PREFIX}.configure \
      env -i \
      PATH="/usr/bin:/bin" \
      ${srcdir}/llvm-trunk/configure \
        "${LLVM_SB_CONFIGURE_ENV[@]}" \
        --prefix=${installdir} \
        --host=nacl \
        --disable-jit \
        --enable-optimized \
        --target=${CROSS_TARGET_ARM} \
        --enable-targets=${targets} \
        --enable-pic=no \
        --enable-static \
        --enable-shared=no
  spopd
}

llvm-sb-needs-make() {
  llvm-sb-setup "$@"
  ts-modified "${TC_SRC_LLVM}" "${LLVM_SB_OBJDIR}"
  return $?
}

# llvm-sb-make - Make llvm tools (sandboxed)
llvm-sb-make() {
  llvm-sb-setup "$@"

  StepBanner "LLVM-SB" "Make ${LLVM_SB_ARCH} ${LLVM_SB_MODE}"
  local objdir="${LLVM_SB_OBJDIR}"

  spushd "${objdir}"
  ts-touch-open "${objdir}"

  local build_with_srpc=0
  if [ "${LLVM_SB_MODE}" == "srpc" ]; then
    build_with_srpc=1
  fi

  RunWithLog ${LLVM_SB_LOG_PREFIX}.make \
      env -i PATH="/usr/bin:/bin" \
      ONLY_TOOLS=llc \
      NACL_SANDBOX=1 \
      NACL_SRPC=${build_with_srpc} \
      KEEP_SYMBOLS=1 \
      VERBOSE=1 \
      make ENABLE_OPTIMIZED=1 OPTIMIZE_OPTION=-O3 \
           ${MAKE_OPTS} tools-only

  ts-touch-commit "${objdir}"

  spopd
}

# llvm-sb-install - Install llvm tools (sandboxed)
llvm-sb-install() {
  llvm-sb-setup "$@"

  StepBanner "LLVM-SB" "Install ${LLVM_SB_ARCH} ${LLVM_SB_MODE}"
  local objdir="${LLVM_SB_OBJDIR}"
  spushd ${objdir}

  RunWithLog ${LLVM_SB_LOG_PREFIX}.install \
      env -i PATH="/usr/bin:/bin" \
      ONLY_TOOLS=llc \
      NACL_SANDBOX=1 \
      KEEP_SYMBOLS=1 \
      make ${MAKE_OPTS} install

  spopd

  translate-and-install-sb-tool ${LLVM_SB_ARCH} ${LLVM_SB_MODE} llc
}

translate-and-install-sb-tool() {
  local arch=$1
  local mode=$2
  local name=$3

  # Translate bitcode program into an actual native executable.
  # If arch = universal, we need to translate and install multiple times.
  local bindir="${PNACL_SB_ROOT}/${arch}/${mode}/bin"
  local pexe="${bindir}/${name}.pexe"

  # Rename to .pexe
  mv "${bindir}/${name}" "${pexe}"

  local arches
  if [ "${arch}" == "universal" ]; then
    arches="${SBTC_BUILD_WITH_PNACL}"
  else
    arches="${arch}"
  fi

  local installer
  if [ "${arch}" == "universal" ]; then
    installer="cp -f"
  else
    installer="mv -f"
  fi

  # In universal/mode/bin directory, we'll end up with every translation:
  # e.g. llc.arm.nexe, llc.x8632.nexe, llc.x8664.nexe
  # In arch/mode/bin directories, we'll end up with just one copy
  local num_arches=$(wc -w <<< "${arches}")
  local extra=""
  if [ ${num_arches} -gt 1 ] && QueueConcurrent; then
    extra=" (background)"
  fi

  for tarch in ${arches}; do
    local nexe="${bindir}/${name}.${tarch}.nexe"
    StepBanner "TRANSLATE" "Translating ${name}.pexe to ${tarch}${extra}"
    "${PNACL_TRANSLATE}" -arch ${tarch} "${pexe}" -o "${nexe}" &
    QueueLastProcess
  done

  if [ ${num_arches} -gt 1 ] && ! QueueEmpty ; then
    StepBanner "TRANSLATE" "Waiting for processes to finish"
  fi
  QueueWait

  for tarch in ${arches}; do
    local nexe="${bindir}/${name}.${tarch}.nexe"
    local bindir_tarch="${PNACL_SB_ROOT}/${tarch}/${mode}/bin"
    mkdir -p "${bindir_tarch}"
    ${installer} "${nexe}" "${bindir_tarch}/${name}"
  done
}

#+-------------------------------------------------------------------------
#+ binutils-sb <arch> <mode> - Build and install binutils (sandboxed)
binutils-sb() {
  local srcdir="${TC_SRC_BINUTILS}"

  assert-dir "${srcdir}" "You need to checkout binutils."

  if [ $# -ne 2 ]; then
    echo "ERROR: Usage binutils-sb <arch> <mode>"
    exit -1
  fi

  local arch=$1
  local mode=$2
  check-sb-arch ${arch}
  check-sb-mode ${mode}

  if [ ! -d "${NNACL_ROOT}" ] ; then
    echo "ERROR: Install Native Client toolchain"
    exit -1
  fi

  if [ ! -f "${TC_BUILD_BINUTILS_LIBERTY}/libiberty/libiberty.a" ] ; then
    echo "ERROR: Missing lib. Run this script with binutils-liberty option"
    exit -1
  fi

  if binutils-sb-needs-configure "${arch}" "${mode}"; then
    binutils-sb-clean "${arch}" "${mode}"
    binutils-sb-configure "${arch}" "${mode}"
  else
    SkipBanner "BINUTILS-SB" "configure ${arch} ${mode}"
  fi

  if binutils-sb-needs-make "${arch}" "${mode}"; then
    binutils-sb-make "${arch}" "${mode}"
  else
    SkipBanner "BINUTILS-SB" "make ${arch} ${mode}"
  fi

  binutils-sb-install "${arch}" "${mode}"
}

binutils-sb-needs-configure() {
  local arch=$1
  local mode=$2
  [ ! -f "${TC_BUILD}/binutils-${arch}-${mode}-sandboxed/config.status" ]
  return $?
}

# binutils-sb-clean - Clean binutils (sandboxed)
binutils-sb-clean() {
  local arch=$1
  local mode=$2
  StepBanner "BINUTILS-SB" "Clean ${arch} ${mode}"
  local objdir="${TC_BUILD}/binutils-${arch}-${mode}-sandboxed"
  rm -rf "${objdir}"
  mkdir -p "${objdir}"
}

# binutils-sb-configure - Configure binutils (sandboxed)
binutils-sb-configure() {
  local arch=$1
  local mode=$2
  StepBanner "BINUTILS-SB" "Configure ${arch} ${mode}"
  local bitsize=${arch:3:2}
  local nacl="nacl${bitsize/"32"/}"
  local srcdir="${TC_SRC_BINUTILS}"
  local objdir="${TC_BUILD}/binutils-${arch}-${mode}-sandboxed"
  local installdir="${PNACL_SB_ROOT}/${arch}/${mode}"

  local flags="-DNACL_ALIGN_BYTES=32 -DNACL_ALIGN_POW2=5 -DNACL_TOOLCHAIN_PATCH"
  if [ ${mode} == "srpc" ]; then
    flags+=" -DNACL_SRPC"
  fi

  spushd ${objdir}
  mkdir -p liberty_tmp
  cp "${TC_BUILD_BINUTILS_LIBERTY}/libiberty/libiberty.a" liberty_tmp
  RunWithLog \
      binutils.${arch}.${mode}.sandboxed.configure \
      env -i \
      PATH="/usr/bin:/bin" \
      CC_FOR_BUILD="${CC}" \
      CXX_FOR_BUILD="${CXX}" \
      AR="${NNACL_ROOT}/bin/${nacl}-ar" \
      AS="${NNACL_ROOT}/bin/${nacl}-as" \
      CC="${NNACL_ROOT}/bin/${nacl}-gcc" \
      CXX="${NNACL_ROOT}/bin/${nacl}-g++" \
      LD="${NNACL_ROOT}/bin/${nacl}-ld" \
      RANLIB="${NNACL_ROOT}/bin/${nacl}-ranlib" \
      CFLAGS="-m${bitsize} -O3 ${flags} -I${NNACL_ROOT}/${nacl}/include" \
      LDFLAGS="-s" \
      LDFLAGS_FOR_BUILD="-L../liberty_tmp" \
      ${srcdir}/binutils-2.20/configure \
        --prefix=${installdir} \
        --host=${nacl} \
        --target=${nacl} \
        --disable-nls \
        --disable-werror \
        --enable-static \
        --enable-shared=no
  spopd
}

binutils-sb-needs-make() {
  local arch=$1
  local mode=$2
  local srcdir="${TC_SRC_BINUTILS}"
  local objdir="${TC_BUILD}/binutils-${arch}-${mode}-sandboxed"

  ts-modified "$srcdir" "$objdir"
  return $?
}

# binutils-sb-make - Make binutils (sandboxed)
binutils-sb-make() {
  local arch=$1
  local mode=$2
  StepBanner "BINUTILS-SB" "Make ${arch} ${mode}"
  local objdir="${TC_BUILD}/binutils-${arch}-${mode}-sandboxed"

  spushd ${objdir}

  ts-touch-open "${objdir}"

  local build_with_srpc=0
  if [ ${mode} == "srpc" ]; then
    build_with_srpc=1
  fi

  RunWithLog binutils.${arch}.sandboxed.make \
      env -i PATH="/usr/bin:/bin" \
      NACL_SRPC=${build_with_srpc} \
      make ${MAKE_OPTS} all-ld

  ts-touch-commit "${objdir}"

  spopd
}

# binutils-sb-install - Install binutils (sandboxed)
binutils-sb-install() {
  local arch=$1
  local mode=$2
  StepBanner "BINUTILS-SB" "Install ${arch} ${mode}"
  local objdir="${TC_BUILD}/binutils-${arch}-${mode}-sandboxed"

  spushd ${objdir}

  RunWithLog binutils.${arch}.${mode}.sandboxed.install \
      env -i PATH="/usr/bin:/bin" \
      make install-ld

  spopd
}


#+ tools-sb {arch} {mode} - Build all sandboxed tools for arch, mode
tools-sb() {
  local arch=$1
  local mode=$2

  StepBanner "${arch}"    "Sandboxing"
  StepBanner "----------" "--------------------------------------"
  llvm-sb ${arch} ${mode}

  # Use regular toolchain for building binutils.
  # This is a temporary hack because we can't build binutils with pnacl yet.
  # TODO(pdox): Make binutils buildable with pnacl.
  local arches
  if [[ "${arch}" == "universal" ]] ; then
    arches="${SBTC_BUILD_WITH_PNACL}"
  else
    arches="${arch}"
  fi
  for arch in ${arches} ; do
    if [[ "${arch}" == "arm" ]] ; then
      StepBanner "BINUTILS-SB" "Skipping ARM build (not yet sandboxed)"
    else
      binutils-sb ${arch} ${mode}
    fi
  done
}


#+--------------------------------------------------------------------------
#@ install-translators {srpc/nonsrpc} - Builds and installs sandboxed
#@                                      translator components
install-translators() {
  if [ $# -ne 1 ]; then
    echo "ERROR: Usage install-translators <srpc/nonsrpc>"
    exit -1
  fi

  local srpc_kind=$1
  check-sb-mode ${srpc_kind}

  StepBanner "INSTALL SANDBOXED TRANSLATORS (${srpc_kind})"

  binutils-liberty

  if ${SBTC_PRODUCTION}; then
    # Build each architecture separately.
    for arch in ${SBTC_BUILD_WITH_PNACL} ; do
      tools-sb ${arch} ${srpc_kind}
    done
  else
    # Using arch `universal` builds the sandboxed tools to a .pexe
    # which support all targets. This .pexe is then translated to
    # each architecture specified in ${SBTC_BUILD_WITH_PNACL}.
    tools-sb universal ${srpc_kind}
  fi

  echo "Done"
}

#+-------------------------------------------------------------------------
#@ prune-translator-install - Prunes translator install directories
prune-translator-install() {
  if [ $# -ne 1 ]; then
    echo "ERROR: Usage prune-translator-install <srpc/nonsrpc>"
    exit -1
  fi

  local srpc_kind=$1
  check-sb-mode ${srpc_kind}

  StepBanner "PRUNE" "Pruning translator installs (${srpc_kind})"

  spushd "${PNACL_SB_X8632}/${srpc_kind}"
  rm -rf include lib nacl share
  rm -rf bin/llvm-config bin/tblgen
  spopd

  spushd "${PNACL_SB_X8664}/${srpc_kind}"
  rm -rf include lib nacl64 share
  rm -rf bin/llvm-config bin/tblgen
  spopd

  if ! ${SBTC_PRODUCTION}; then
    spushd "${PNACL_SB_UNIVERSAL}/${srpc_kind}"
    rm -rf include lib share
    rm -f bin/llvm-config bin/tblgen
    # Delete intermediate files generated by the driver
    rm -f -- bin/llc*---llc.pexe---*
    spopd
  fi

  echo "remove driver log"
  rm -f "${PNACL_ROOT}"/driver.log

  echo "Done"
}

#########################################################################
#     < NEWLIB-BITCODE >
#########################################################################

#+ newlib                - Build and install newlib in bitcode.
newlib() {
  StepBanner "NEWLIB (BITCODE)"

  if newlib-needs-configure; then
    newlib-clean
    newlib-configure
  else
    SkipBanner "NEWLIB" "configure"
  fi

  if newlib-needs-make; then
    newlib-make
  else
    SkipBanner "NEWLIB" "make"
  fi

  newlib-install
}

#+ newlib-clean  - Clean bitcode newlib.
newlib-clean() {
  StepBanner "NEWLIB" "Clean"
  rm -rf "${TC_BUILD_NEWLIB}"
}

newlib-needs-configure() {
  speculative-check "llvm-gcc" && return 0
  ts-newer-than "${TC_BUILD_LLVM_GCC}-${CROSS_TARGET_ARM}" \
                   "${TC_BUILD_NEWLIB}" && return 0

  [ ! -f "${TC_BUILD_NEWLIB}/config.status" ]
  return #?
}

#+ newlib-configure - Configure bitcode Newlib
newlib-configure() {
  StepBanner "NEWLIB" "Configure"
  newlib-configure-common "${TC_BUILD_NEWLIB}"
}

newlib-configure-common() {
  local srcdir="${TC_SRC_NEWLIB}"
  local objdir="$1"
  mkdir -p "${objdir}"
  spushd "${objdir}"

  RunWithLog newlib.configure \
    env -i \
    PATH="/usr/bin:/bin" \
    "${STD_ENV_FOR_NEWLIB[@]}" \
    ${srcdir}/newlib-trunk/configure \
        --disable-multilib \
        --prefix="${NEWLIB_INSTALL_DIR}" \
        --disable-newlib-supplied-syscalls \
        --disable-texinfo \
        --disable-libgloss \
        --enable-newlib-iconv \
        --enable-newlib-io-long-long \
        --enable-newlib-io-long-double \
        --enable-newlib-io-c99-formats \
        --enable-newlib-io-mb \
        --target="${REAL_CROSS_TARGET}"
  spopd
}

newlib-needs-make() {
  local srcdir="${TC_SRC_NEWLIB}"
  local objdir="${TC_BUILD_NEWLIB}"

  ts-modified "$srcdir" "$objdir"
  return $?
}

#+ newlib-make   - Make bitcode Newlib
newlib-make() {
  StepBanner "NEWLIB" "Make"
  newlib-make-common "${TC_BUILD_NEWLIB}"
}

newlib-make-common() {
  local srcdir="${TC_SRC_NEWLIB}"
  local objdir="$1"

  ts-touch-open "${objdir}"

  spushd "${objdir}"
  RunWithLog newlib.make \
    env -i PATH="/usr/bin:/bin" \
    make \
      "${STD_ENV_FOR_NEWLIB[@]}" \
      ${MAKE_OPTS}
  spopd

  ts-touch-commit "${objdir}"

}

#+ newlib-bitcde-install    - Install Bitcode Newlib
newlib-install() {
  StepBanner "NEWLIB" "Install"

  local objdir="${TC_BUILD_NEWLIB}"

  spushd "${objdir}"

  # NOTE: we might be better off not using install, as we are already
  #       doing a bunch of copying of headers and libs further down
  RunWithLog newlib.install \
    env -i PATH="/usr/bin:/bin" \
      make \
      "${STD_ENV_FOR_NEWLIB[@]}" \
      install ${MAKE_OPTS}

  ###########################################################
  #                -- HACK HACK HACK --
  # newlib installs into ${REAL_CROSS_TARGET}
  # For now, move it back to the old ${CROSS_TARGET_ARM}
  # where everything expects it to be.
  rm -rf "${NEWLIB_INSTALL_DIR}/${CROSS_TARGET_ARM}"
  mv "${NEWLIB_INSTALL_DIR}/${REAL_CROSS_TARGET}" \
     "${NEWLIB_INSTALL_DIR}/${CROSS_TARGET_ARM}"
  ###########################################################

  StepBanner "NEWLIB" "Extra-install"
  local sys_include=${SYSROOT_DIR}/include
  # NOTE: we provide a new one via extra-sdk
  rm ${NEWLIB_INSTALL_DIR}/${CROSS_TARGET_ARM}/include/pthread.h

  cp ${NEWLIB_INSTALL_DIR}/${CROSS_TARGET_ARM}/include/machine/endian.h \
    ${sys_include}
  cp ${NEWLIB_INSTALL_DIR}/${CROSS_TARGET_ARM}/include/sys/param.h \
    ${sys_include}
  cp ${NEWLIB_INSTALL_DIR}/${CROSS_TARGET_ARM}/include/newlib.h \
    ${sys_include}

  # NOTE: we provide our own pthread.h via extra-sdk
  StepBanner "NEWLIB" "Removing old pthreads headers"
  rm -f "${NEWLIB_INSTALL_DIR}/${CROSS_TARGET_ARM}/usr/include/pthread.h"
  rm -f "${sys_include}/pthread.h"

  StepBanner "NEWLIB" "copying libraries"
  local destdir="${PNACL_BITCODE_ROOT}"
  # We only install libc/libg/libm
  mkdir -p "${destdir}"
  cp ${objdir}/${REAL_CROSS_TARGET}/newlib/lib[cgm].a "${destdir}"

  spopd
}


#########################################################################
#     < EXTRASDK >
#########################################################################
#+ extrasdk-clean  - Clean extra-sdk stuff

extrasdk-clean() {
  StepBanner "EXTRASDK" "Clean"
  rm -rf "${TC_BUILD_EXTRASDK}"

  StepBanner "EXTRASDK" "Clean bitcode lib"
  # TODO(robertm): consider having a dedicated dir for this so can
  #                delete this wholesale
  # Do not clean libc and libstdc++ but everything else
  rm -f "${PNACL_BITCODE_ROOT}"/*google*.a
  rm -f "${PNACL_BITCODE_ROOT}"/libsrpc.a
  rm -f "${PNACL_BITCODE_ROOT}"/libnpapi.a
  rm -f "${PNACL_BITCODE_ROOT}"/libppapi.a
  rm -f "${PNACL_BITCODE_ROOT}"/libnosys.a
  rm -f "${PNACL_BITCODE_ROOT}"/libav.a
  rm -f "${PNACL_BITCODE_ROOT}"/libgio.a

  if ${LIBMODE_NEWLIB}; then
    rm -f "${PNACL_BITCODE_ROOT}"/*nacl*
    rm -f "${PNACL_BITCODE_ROOT}"/libpthread.a
  fi

  if ${LIBMODE_NEWLIB}; then
    StepBanner "EXTRASDK" "Clean arm libs"
    # Do not clean libgcc but everything else
    rm -f "${PNACL_ARM_ROOT}"/*crt*

    StepBanner "EXTRASDK" "Clean x86-32 libs"
    # Do not clean libgcc but everything else
    rm -f "${PNACL_X8632_ROOT}"/*crt*

    StepBanner "EXTRASDK" "Clean x86-64 libs"
    # Do not clean libgcc but everything else
    rm -f "${PNACL_X8664_ROOT}"/*crt*
  fi

  StepBanner "EXTRASDK" "Cleaning libehsupport"
  rm -rf "${PNACL_ARM_ROOT}"/libehsupport*
  rm -rf "${PNACL_X8632_ROOT}"/libehsupport*
  rm -rf "${PNACL_X8664_ROOT}"/libehsupport*

  # clean scons obj dirs
  rm -rf scons-out/nacl_extra_sdk-*-pnacl*
}

libehsupport() {
  extrasdk-clean
  local headerdir
  local extra_flag=""
  if ${LIBMODE_NEWLIB}; then
    headerdir="${NEWLIB_INSTALL_DIR}/${CROSS_TARGET_ARM}/include"
  else
    extra_flag="--nacl_glibc"
    headerdir="${GLIBC_INSTALL_DIR}/include"
  fi

  if ${LIBMODE_NEWLIB}; then
    StepBanner "EXTRASDK" "Make/Install arm ehsupport"
    RunWithLog "extra_sdk.arm_ehsupport" \
        ./scons MODE=nacl_extra_sdk \
        extra_sdk_lib_destination="${PNACL_ARM_ROOT}" \
        extra_sdk_include_destination="${headerdir}" \
        bitcode=1 \
        ${extra_flag} \
        platform=arm \
        sdl=none \
        naclsdk_validate=0 \
        --verbose \
        install_libehsupport
  fi

  StepBanner "EXTRASDK" "Make/Install x86-32 ehsupport"
  RunWithLog "extra_sdk.x86_32_ehsupport" \
      ./scons MODE=nacl_extra_sdk \
      extra_sdk_lib_destination="${PNACL_X8632_ROOT}" \
      extra_sdk_include_destination="${headerdir}" \
      bitcode=1 \
      ${extra_flag} \
      platform=x86-32 \
      sdl=none \
      naclsdk_validate=0 \
      --verbose \
      install_libehsupport

  StepBanner "EXTRASDK" "Make/Install x86-64 ehsupport"
  RunWithLog "extra_sdk.x86_64_ehsupport" \
      ./scons MODE=nacl_extra_sdk \
      extra_sdk_lib_destination="${PNACL_X8664_ROOT}" \
      extra_sdk_include_destination="${headerdir}" \
      bitcode=1 \
      ${extra_flag} \
      platform=x86-64 \
      sdl=none \
      naclsdk_validate=0 \
      --verbose \
      install_libehsupport
}


#+ extrasdk               - build and install all extra sdk components
extrasdk() {
  StepBanner "EXTRASDK"

  extrasdk-clean

  local headerdir
  local extra_flag=""
  if ${LIBMODE_NEWLIB}; then
    headerdir="${NEWLIB_INSTALL_DIR}/${CROSS_TARGET_ARM}/include"
  else
    extra_flag="--nacl_glibc"
    headerdir="${GLIBC_INSTALL_DIR}/include"
  fi

  StepBanner "EXTRASDK" "Make/Install headers"
  RunWithLog "extra_sdk.headers" \
      ./scons MODE=nacl_extra_sdk \
      extra_sdk_lib_destination="${PNACL_BITCODE_ROOT}" \
      extra_sdk_include_destination="${headerdir}" \
      bitcode=1 \
      ${extra_flag} \
      platform=arm \
      sdl=none \
      naclsdk_validate=0 \
      extra_sdk_update_header

  if ${LIBMODE_NEWLIB}; then
    StepBanner "EXTRASDK" "Make/Install bitcode libpthread"
    RunWithLog "extra_sdk.bitcode_libpthread" \
        ./scons MODE=nacl_extra_sdk -j 8\
        extra_sdk_lib_destination="${PNACL_BITCODE_ROOT}" \
        extra_sdk_include_destination="${headerdir}" \
        bitcode=1 \
        ${extra_flag} \
        platform=arm \
        sdl=none \
        naclsdk_validate=0 \
        install_libpthread
  fi

  StepBanner "EXTRASDK" "Make/Install bitcode components (${LIBMODE})"
  RunWithLog "extra_sdk.bitcode_components" \
      ./scons MODE=nacl_extra_sdk -j 8 \
      extra_sdk_lib_destination="${PNACL_BITCODE_ROOT}" \
      extra_sdk_include_destination="${headerdir}" \
      disable_nosys_linker_warnings=1 \
      bitcode=1 \
      ${extra_flag} \
      platform=arm \
      sdl=none \
      naclsdk_validate=0 \
      --verbose \
      extra_sdk_libs

  if ${LIBMODE_NEWLIB} ; then
    StepBanner "EXTRASDK" "Make/Install arm components"
    RunWithLog "extra_sdk.arm_components" \
        ./scons MODE=nacl_extra_sdk \
        extra_sdk_lib_destination="${PNACL_ARM_ROOT}" \
        extra_sdk_include_destination="${headerdir}" \
        bitcode=1 \
        ${extra_flag} \
        platform=arm \
        sdl=none \
        naclsdk_validate=0 \
        --verbose \
        extra_sdk_libs_platform
  fi

  StepBanner "EXTRASDK" "Make/Install x86-32 components"
  RunWithLog "extra_sdk.libs_x8632" \
      ./scons MODE=nacl_extra_sdk \
      extra_sdk_lib_destination="${PNACL_X8632_ROOT}" \
      extra_sdk_include_destination="${headerdir}" \
      bitcode=1 \
      ${extra_flag} \
      platform=x86-32 \
      sdl=none \
      naclsdk_validate=0 \
      --verbose \
      extra_sdk_libs_platform

  StepBanner "EXTRASDK" "Make/Install x86-64 components"
  RunWithLog "extra_sdk.libs_x8664" \
      ./scons MODE=nacl_extra_sdk \
      extra_sdk_lib_destination="${PNACL_X8664_ROOT}" \
      extra_sdk_include_destination="${headerdir}" \
      bitcode=1 \
      ${extra_flag} \
      platform=x86-64 \
      sdl=none \
      naclsdk_validate=0 \
      --verbose \
      extra_sdk_libs_platform
}

newlib-nacl-headers-clean() {
  # Clean the include directory and revert it to its pure state
  if [ -d "${TC_SRC_NEWLIB}" ]; then
    rm -rf "${NEWLIB_INCLUDE_DIR}"
    # If the script is interrupted right here,
    # then NEWLIB_INCLUDE_DIR will not exist, and the repository
    # will be in a bad state. This will be fixed during the next
    # invocation by newlib-nacl-headers.

    # We jump into the parent directory and use a relative path so that
    # hg does not get confused by pathnames which contain a symlink.
    spushd "$(dirname "${NEWLIB_INCLUDE_DIR}")"
    RunWithLog "newlib-freshen" \
      hg-revert "$(basename "${NEWLIB_INCLUDE_DIR}")"
    spopd
  fi
}

#+ newlib-nacl-headers   - Install NaCl headers to newlib
newlib-nacl-headers() {
  StepBanner "newlib-nacl-headers" "Adding nacl headers to newlib"

  assert-dir "${TC_SRC_NEWLIB}" "Newlib is not checked out"

  # Make sure the headers directory has no local changes
  newlib-nacl-headers-check
  newlib-nacl-headers-clean

  # Install the headers
  "${EXPORT_HEADER_SCRIPT}" \
      "${NACL_SYS_HEADERS}" \
      "${NEWLIB_INCLUDE_DIR}"

  # Record the header install time
  ts-touch "${NACL_SYS_TS}"
}

#+ newlib-nacl-headers-check - Make sure the newlib nacl headers haven't
#+                             been modified since the last install.
newlib-nacl-headers-check() {
  # The condition where NEWLIB_INCLUDE_DIR does not exist may have been
  # caused by an incomplete call to newlib-nacl-headers-clean().
  # Let it pass this check so that the clean will be able to finish.
  # See the comment in newlib-nacl-headers-clean()
  if ! [ -d "${TC_SRC_NEWLIB}" ] ||
     ! [ -d "${NEWLIB_INCLUDE_DIR}" ]; then
    return 0
  fi

  # Already clean?
  if ! hg-has-changes "${NEWLIB_INCLUDE_DIR}" &&
     ! hg-has-untracked "${NEWLIB_INCLUDE_DIR}" ; then
    return 0
  fi

  if ts-dir-changed "${NACL_SYS_TS}" "${NEWLIB_INCLUDE_DIR}"; then
    echo ""
    echo "*******************************************************************"
    echo "*                            ERROR                                *"
    echo "*      The NewLib include directory has local modifications       *"
    echo "*******************************************************************"
    echo "* The NewLib include directory should not be modified directly.   *"
    echo "* Instead, modifications should be done from:                     *"
    echo "*   src/trusted/service_runtime/include                           *"
    echo "*                                                                 *"
    echo "* To destroy the local changes to newlib, run:                    *"
    echo "*  tools/llvm/utman.sh newlib-nacl-headers-clean                  *"
    echo "*******************************************************************"
    echo ""
    if ${UTMAN_BUILDBOT} ; then
      newlib-nacl-headers-clean
    else
      exit -1
    fi
  fi
}

#+-------------------------------------------------------------------------
#+ driver                - Install driver scripts.
driver() {
  StepBanner "DRIVER"
  # need to prep the dir just in case..
  prep-install-dir
  # otherwise linker-install will stomp it.
  linker-install
  driver-install
  driver-intrinsics
}

driver-intrinsics() {
  StepBanner "DRIVER" "Install LLVM intrinsics"
  "${LLVM_INSTALL_DIR}"/bin/llvm-as \
    tools/llvm/llvm-intrinsics.ll \
    -o "${PNACL_ROOT}/llvm-intrinsics.bc"
}

# Just in case we're calling this manually
prep-install-dir() {
  mkdir -p "${PNACL_ROOT}"
}

# We need to adjust the start address and aligment of nacl arm modules
linker-install() {
   StepBanner "DRIVER" "Installing untrusted ld scripts"
   mkdir -p "${LDSCRIPTS_DIR}"
   cp tools/llvm/ld_script_arm_untrusted "${LDSCRIPTS_DIR}"
   cp tools/llvm/ld_script_x8632_untrusted "${LDSCRIPTS_DIR}"
   cp tools/llvm/ld_script_x8664_untrusted "${LDSCRIPTS_DIR}"
}

# The driver is a simple python script which changes its behavior
# depending on the name it is invoked as.
driver-install() {
  StepBanner "DRIVER" "Installing driver adaptors to ${PNACL_BIN}"
  mkdir -p "${PNACL_BIN}"
  rm -f "${PNACL_BIN}"/pnacl-*
  cp "${DRIVER_DIR}"/driver_tools.py "${PNACL_BIN}"
  for t in "${DRIVER_DIR}"/pnacl-*; do
    local name=$(basename "$t")
    cp "${t}" "${PNACL_BIN}/${name/.py}"
  done

  # Tell the driver the library mode
  touch "${PNACL_BIN}"/${LIBMODE}.cfg
}

######################################################################
######################################################################
#
#                           HELPER FUNCTIONS
#
#             (These should not generally be used directly)
#
######################################################################
######################################################################

RecordRevisionInfo() {
  svn info > "${PNACL_ROOT}/REV"
}

######################################################################
######################################################################
#     < VERIFY >
######################################################################
######################################################################

readonly LLVM_DIS=${LLVM_INSTALL_DIR}/bin/llvm-dis
readonly LLVM_BCANALYZER=${LLVM_INSTALL_DIR}/bin/llvm-bcanalyzer
readonly LLVM_OPT=${LLVM_INSTALL_DIR}/bin/opt
readonly LLVM_AR=${CROSS_TARGET_AR}

# Note: we could replace this with a modified version of tools/elf_checker.py
#       if we do not want to depend on binutils
readonly NACL_OBJDUMP=${BINUTILS_INSTALL_DIR}/bin/${BINUTILS_TARGET}-objdump

# Usage: VerifyArchive <checker> <pattern> <filename>
ExtractAndCheck() {
  local checker="$1"
  local pattern="$2"
  local archive="$3"
  local tmp="/tmp/ar-verify-${RANDOM}"
  rm -rf ${tmp}
  mkdir -p ${tmp}
  cp "${archive}" "${tmp}"
  spushd ${tmp}
  ${LLVM_AR} x $(basename ${archive})
  # extract all the files
  local count=0
  for i in ${pattern} ; do
    if [ ! -e "$i" ]; then
      # we may also see the unexpanded pattern here if there is no match
      continue
    fi
    count=$((count+1))
    ${checker} $i
  done
  if [ "${count}" = "0" ] ; then
    echo "FAIL - archive empty or wrong contents: ${archive}"
    ls -l "${tmp}"
    exit -1
  fi
  echo "PASS  (${count} files)"
  rm -rf "${tmp}"
  spopd
}

# Usage: VerifyLinkerScript <filename>
VerifyLinkerScript() {
  local archive="$1"
  # Use cpp to strip the C-style comments.
  ${PNACL_GCC} -E -xc "${archive}" | awk -v archive="$(basename ${archive})" '
    BEGIN { status = 0 }
    NF == 0 || $1 == "#" { next }
    $1 == "INPUT" && $2 == "(" && $NF == ")" { next }
    {
      print "FAIL - unexpected linker script(?) contents:", archive
      status = 1
      exit(status)
    }
    END { if (status == 0) print "PASS  (trivial linker script)" }
' || exit -1
}

# Usage: VerifyArchive <checker> <pattern> <filename>
VerifyArchive() {
  local checker="$1"
  local pattern="$2"
  local archive="$3"
  echo -n "verify $(basename "${archive}"): "
  type="$(file --brief --mime-type "${archive}")"
  case "$type" in
    application/x-archive)
      ExtractAndCheck "$checker" "$pattern" "$archive"
      ;;
    text/x-c)
      # A linker script with C comments looks like C to "file".
      VerifyLinkerScript "$archive"
      ;;
    *)
      echo "FAIL - unknown file type ($type): ${archive}"
      exit -1
      ;;
  esac
}

#
# verify-object-llvm <obj>
#
#   Verifies that a given .o file is bitcode and free of ASMSs
verify-object-llvm() {
  local t=$(${LLVM_DIS} $1 -o -)

  if grep asm <<<$t ; then
    echo
    echo "ERROR asm in $1"
    echo
    exit -1
  fi
}



check-elf-abi() {
  local arch_info=$(${NACL_OBJDUMP} -f $1)
  if ! grep -q $2 <<< ${arch_info} ; then
    echo "ERROR $1 - bad file format: $2 vs ${arch_info}\n"
    echo ${arch_info}
    exit -1
  fi
}


# verify-object-arm <obj>
#
#   Ensure that the ARCH properties are what we expect, this is a little
#   fragile and needs to be updated when tools change
verify-object-arm() {
  check-elf-abi $1 "elf32-littlearm-nacl"
  arch_info=$("${PNACL_READELF}" -A $1)
  #TODO(robertm): some refactoring and cleanup needed
  if ! grep -q "Tag_FP_arch: VFPv2" <<< ${arch_info} ; then
    echo "ERROR $1 - bad Tag_FP_arch\n"
    #TODO(robertm): figure out what the right thing to do is here, c.f.
    # http://code.google.com/p/nativeclient/issues/detail?id=966
    "${PNACL_READELF}" -A $1 | grep  Tag_FP_arch
    exit -1
  fi

  if ! grep -q "Tag_CPU_arch: v7" <<< ${arch_info} ; then
    echo "FAIL bad $1 Tag_CPU_arch\n"
    "${PNACL_READELF}" -A $1 | grep Tag_CPU_arch
    exit -1
  fi
}


# verify-object-x86-32 <obj>
#
verify-object-x86-32() {
  check-elf-abi $1 "elf32-nacl"
}

# verify-object-x86-64 <obj>
#
verify-object-x86-64() {
  check-elf-abi $1 "elf64-nacl"
}


#
# verify-archive-llvm <archive>
# Verifies that a given archive is bitcode and free of ASMSs
#
verify-archive-llvm() {
  if ${LLVM_BCANALYZER} "$1" 2> /dev/null ; then
    # This fires only when we build in single-bitcode-lib mode
    echo -n "verify $(basename "$1"): "
    verify-object-llvm "$1"
    echo "PASS (single-bitcode)"
  else
    # Currently all the files are .o in the llvm archives.
    # Eventually more and more should be .bc.
    VerifyArchive verify-object-llvm '*.bc *.o' "$@"
  fi
}

#
# verify-archive-arm <archive>
# Verifies that a given archive is a proper arm achive
#
verify-archive-arm() {
  VerifyArchive verify-object-arm '*.o *.ons' "$@"
}

#
# verify-archive-x86-32 <archive>
# Verifies that a given archive is a proper x86-32 achive
#
verify-archive-x86-32() {
  VerifyArchive verify-object-x86-32 '*.o *.ons' "$@"
}

#
# verify-archive-x86-64 <archive>
# Verifies that a given archive is a proper x86-64 achive
#
verify-archive-x86-64() {
  VerifyArchive verify-object-x86-64 '*.o *.ons' "$@"
}
#@-------------------------------------------------------------------------
#+ verify                - Verifies that toolchain/pnacl-untrusted ELF files
#+                         are of the correct architecture.
verify() {
  StepBanner "VERIFY"

  if ${UTMAN_BUILD_ARM}; then
    SubBanner "VERIFY: ${PNACL_ARM_ROOT}"
    for i in ${PNACL_ARM_ROOT}/*.o ; do
      verify-object-arm "$i"
    done

    for i in ${PNACL_ARM_ROOT}/*.a ; do
      verify-archive-arm "$i"
    done
  fi

  SubBanner "VERIFY: ${PNACL_X8632_ROOT}"
  for i in ${PNACL_X8632_ROOT}/*.o ; do
     verify-object-x86-32  "$i"
  done

  for i in ${PNACL_X8632_ROOT}/*.a ; do
    verify-archive-x86-32 "$i"
  done

  SubBanner "VERIFY: ${PNACL_X8664_ROOT}"
  for i in ${PNACL_X8664_ROOT}/*.o ; do
     verify-object-x86-64  "$i"
  done

  for i in ${PNACL_X8664_ROOT}/*.a ; do
    verify-archive-x86-64 "$i"
  done

  SubBanner "VERIFY: ${PNACL_BITCODE_ROOT}"
  for i in ${PNACL_BITCODE_ROOT}/*.a ; do
    verify-archive-llvm "$i"
  done

  # we currently do not expect any .o files in this directory
  #for i in ${PNACL_BITCODE_ROOT}/*.o ; do
  #done
}

#@ verify-triple-build <arch> - Verify that the sandboxed translator produces
#@                              an identical translation of itself (llc.pexe)
#@                              as the unsandboxed translator.
verify-triple-build() {
  if [ $# -eq 0 ]; then
    local arch
    for arch in ${SBTC_BUILD_WITH_PNACL} ; do
      verify-triple-build ${arch}
    done
    return
  fi

  local arch=${1/-/}  # Get rid of dashes
  local mode=srpc

  check-sb-arch ${arch}
  check-sb-mode ${mode}

  StepBanner "VERIFY" "Verifying triple build for ${arch}"

  local archdir="${PNACL_SB_ROOT}/${arch}/${mode}"
  local archllc="${archdir}/bin/llc"
  local pexe

  if ${SBTC_PRODUCTION} ; then
    pexe="${archdir}/bin/llc.pexe"
  else
    pexe="${PNACL_SB_ROOT}/universal/${mode}/bin/llc.pexe"
  fi
  assert-file "${archllc}" "sandboxed llc for ${arch} does not exist"
  assert-file "${pexe}"    "llc.pexe does not exist"

  local flags="--pnacl-sb --pnacl-driver-verbose"
  if [ ${mode} == "srpc" ] ; then
    flags+=" --pnacl-driver-set-SRPC=1"
  else
    flags+=" --pnacl-driver-set-SRPC=0"
  fi

  if [ ${arch} == "arm" ] ; then
    # Use emulator if we are not on ARM
    local hostarch=$(uname -m)
    if ! [[ "${hostarch}" =~ arm ]]; then
      flags+=" --pnacl-use-emulator"
    fi
  fi

  local objdir="${TC_BUILD}/triple-build"
  local newllc="${objdir}/llc.${arch}.rebuild.nexe"
  mkdir -p "${objdir}"

  StepBanner "VERIFY" "Translating llc.pexe to ${arch} using sandboxed tools"
  RunWithLog "verify.triple.build" \
    "${PNACL_TRANSLATE}" ${flags} -arch ${arch} "${pexe}" -o "${newllc}"

  if ! cmp --silent "${archllc}" "${newllc}" ; then
    Banner "TRIPLE BUILD VERIFY FAILED"
    echo "Expected these files to be identical, but they are not:"
    echo "  ${archllc}"
    echo "  ${newllc}"
    exit -1
  fi
  StepBanner "VERIFY" "Verified ${arch} OK"
}

######################################################################
######################################################################
#
# UTILITIES
#
######################################################################
######################################################################

#@-------------------------------------------------------------------------
#@ show-config
show-config() {
  Banner "Config Settings:"
  echo "UTMAN_BUILDBOT:    ${UTMAN_BUILDBOT}"
  echo "UTMAN_CONCURRENCY: ${UTMAN_CONCURRENCY}"
  echo "UTMAN_DEBUG:       ${UTMAN_DEBUG}"

  Banner "Your Environment:"
  env | grep UTMAN
}

#@ help                  - Usage information.
help() {
  Usage
}

#@ help-full             - Usage information including internal functions.
help-full() {
  Usage2
}

has-trusted-toolchain() {
  if [ -f toolchain/linux_arm-trusted/ld_script_arm_trusted ]; then
    return 0
  else
    return 1
  fi
}

check-for-trusted() {
  if ! ${UTMAN_BUILD_ARM} ; then
    return
  fi

  if ! has-trusted-toolchain; then
    echo '*******************************************************************'
    echo '*   The ARM trusted toolchain does not appear to be installed yet *'
    echo '*   It is needed to run ARM tests.                                *'
    echo '*                                                                 *'
    echo '*   To download and install the trusted toolchain, run:           *'
    echo '*                                                                 *'
    echo '*       $ tools/llvm/utman.sh download-trusted                    *'
    echo '*                                                                 *'
    echo '*   To compile the trusted toolchain, use:                        *'
    echo '*                                                                 *'
    echo '*       $ tools/llvm/trusted-toolchain-creator.sh trusted_sdk     *'
    echo '*               (warning: this takes a while)                     *'
    echo '*******************************************************************'

    # If building on the bots, do not continue since it needs to run ARM tests.
    if ${UTMAN_BUILDBOT} ; then
      echo "Building on bots --> need ARM trusted toolchain to run tests!"
      exit -1
    elif trusted-tc-confirm ; then
      echo "Continuing without ARM trusted toolchain"
      UTMAN_BUILD_ARM=false
    else
      echo "Okay, stopping."
      exit -1
    fi
  fi
}

trusted-tc-confirm() {
  echo
  echo "Do you wish to continue without the ARM trusted TC (skip ARM testing)?"
  echo ""
  confirm-yes "Continue"
  return $?
}

DebugRun() {
  if ${UTMAN_DEBUG} || ${UTMAN_BUILDBOT}; then
    "$@"
  fi
}

######################################################################
######################################################################
#
#                           < TIME STAMPING >
#
######################################################################
######################################################################

ts-dir-changed() {
  local tsfile="$1"
  local dir="$2"

  if [ -f "${tsfile}" ]; then
    local MODIFIED=$(find "${dir}" -type f -newer "${tsfile}")
    [ ${#MODIFIED} -gt 0 ]
    ret=$?
  else
    true
    ret=$?
  fi
  return $ret
}

# Check if the source for a given build has been modified
ts-modified() {
  local srcdir="$1"
  local objdir="$2"
  local tsfile="${objdir}/${TIMESTAMP_FILENAME}"

  ts-dir-changed "${tsfile}" "${srcdir}"
  return $?
}

ts-touch() {
  local tsfile="$1"
  touch "${tsfile}"
}

# Record the time when make begins, but don't yet
# write that to the timestamp file.
# (Just in case make fails)

ts-touch-open() {
  local objdir="$1"
  local tsfile="${objdir}/${TIMESTAMP_FILENAME}"
  local tsfile_open="${objdir}/${TIMESTAMP_FILENAME}_OPEN"

  rm -f "${tsfile}"
  touch "${tsfile_open}"
}


# Write the timestamp. (i.e. make has succeeded)

ts-touch-commit() {
  local objdir="$1"
  local tsfile="${objdir}/${TIMESTAMP_FILENAME}"
  local tsfile_open="${objdir}/${TIMESTAMP_FILENAME}_OPEN"

  mv -f "${tsfile_open}" "${tsfile}"
}


# ts-newer-than dirA dirB
# Compare the make timestamps in both object directories.
# returns true (0) if dirA is newer than dirB
# returns false (1) otherwise.
#
# This functions errs on the side of returning 0, since
# that forces a rebuild anyway.

ts-newer-than() {
  local objdir1="$1"
  local objdir2="$2"

  local tsfile1="${objdir1}/${TIMESTAMP_FILENAME}"
  local tsfile2="${objdir2}/${TIMESTAMP_FILENAME}"

  if [ ! -d "${objdir1}" ]; then return 0; fi
  if [ ! -d "${objdir2}" ]; then return 0; fi

  if [ ! -f "${tsfile1}" ]; then return 0; fi
  if [ ! -f "${tsfile2}" ]; then return 0; fi

  local MODIFIED=$(find "${tsfile1}" -newer "${tsfile2}")
  if [ ${#MODIFIED} -gt 0 ]; then
    return 0
  fi
  return 1
}


# Don't define any functions after this or they won't show up in completions
function-completions() {
  if [ $# = 0 ]; then set -- ""; fi
  compgen -A function -- $1
  exit 0
}

######################################################################
######################################################################
#
#                               < MAIN >
#
######################################################################
######################################################################

mkdir -p "${PNACL_ROOT}"
PackageCheck

if [ $# = 0 ]; then set -- help; fi  # Avoid reference to undefined $1.

# Accept one -- argument for some compatibility with google3
if [ $1 = "--tab_completion_word" ]; then
  set -- function-completions $2
fi

if [ "$(type -t $1)" != "function" ]; then
  #Usage
  echo "ERROR: unknown function '$1'." >&2
  echo "For help, try:"
  echo "    $0 help"
  exit 1
fi

"$@"
