#!/bin/bash
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -o nounset
set -o errexit

# Turn on/off debugging mode
readonly UTMAN_DEBUG=${UTMAN_DEBUG:-false}

# True if the scripts are running on the build bots.
readonly UTMAN_BUILDBOT=${UTMAN_BUILDBOT:-false}

# Mercurial Retry settings
HG_MAX_RETRIES=${HG_MAX_RETRIES:-3}
if ${UTMAN_BUILDBOT} ; then
  HG_RETRY_DELAY_SEC=${HG_RETRY_DELAY_SEC:-60}
else
  HG_RETRY_DELAY_SEC=${HG_RETRY_DELAY_SEC:-1}
fi

readonly TIME_AT_STARTUP=$(date '+%s')

SetScriptPath() {
  SCRIPT_PATH="$1"
}

SetLogDirectory() {
  TC_LOG="$1"
  TC_LOG_ALL="${TC_LOG}/ALL"
}

######################################################################
# Detect system type
######################################################################

readonly BUILD_PLATFORM=$(uname | tr '[A-Z]' '[a-z]')
if [ "${BUILD_PLATFORM}" == "linux" ] ; then
  readonly BUILD_PLATFORM_LINUX=true
  readonly BUILD_PLATFORM_MAC=false
  readonly SCONS_BUILD_PLATFORM=linux
elif [ "${BUILD_PLATFORM}" == "darwin" ] ; then
  readonly BUILD_PLATFORM_LINUX=false
  readonly BUILD_PLATFORM_MAC=true
  readonly SCONS_BUILD_PLATFORM=mac
else
  echo "Unknown system '${BUILD_PLATFORM}'"
  exit -1
fi

readonly BUILD_ARCH=$(uname -m)
if [ "${BUILD_ARCH}" == "i386" ] ||
   [ "${BUILD_ARCH}" == "i686" ] ; then
  readonly BUILD_ARCH_X8632=true
  readonly BUILD_ARCH_X8664=false
elif [ "${BUILD_ARCH}" == "x86_64" ] ; then
  readonly BUILD_ARCH_X8632=false
  readonly BUILD_ARCH_X8664=true
else
  echo "Unknown arch '${BUILD_ARCH}'"
  exit -1
fi


######################################################################
# Mercurial repository tools
######################################################################

#+ hg-pull <dir>
hg-pull() {
  local dir="$1"

  assert-dir "$dir" \
    "Repository $(basename "${dir}") doesn't exist. First do 'hg-checkout'"

  RunWithRetry ${HG_MAX_RETRIES} ${HG_RETRY_DELAY_SEC} \
    hg-pull-try "${dir}"
}

hg-pull-try() {
  local dir="$1"
  local retcode=0

  spushd "${dir}"
  RunWithLog "hg-pull" hg pull || retcode=$?
  spopd
  return ${retcode}
}

#+ hg-revert <dir>
hg-revert() {
  local dir="$1"
  hg revert "$(GetAbsolutePath "${dir}")"
}

#+ hg-clone <url> <dir>
hg-clone() {
  local url="$1"
  local dir="$2"
  RunWithRetry ${HG_MAX_RETRIES} ${HG_RETRY_DELAY_SEC} \
    hg-clone-try "${url}" "${dir}"
}

hg-clone-try() {
  local url="$1"
  local dir="$2"
  local retcode=0

  rm -rf "${dir}"
  hg clone "${url}" "${dir}" || retcode=$?

  if [ ${retcode} -ne 0 ] ; then
   # Clean up directory after failure
   rm -rf "${dir}"
  fi
  return ${retcode}
}

hg-checkout() {
  local repo=$1
  local dest=$2
  local rev=$3
  mkdir -p "${TC_SRC}"
  if [ ! -d ${dest} ] ; then
    StepBanner "HG-CHECKOUT" "Checking out new repository for ${repo} @ ${rev}"
    # Use a temporary directory just in case HG has problems
    # with long filenames during checkout.
    local TMPDIR="/tmp/hg-${rev}-$RANDOM"
    hg-clone "https://${repo}.googlecode.com/hg/" "${TMPDIR}"
    hg-update "${TMPDIR}" -C ${rev}
    mv "${TMPDIR}" "${dest}"
  else
    StepBanner "HG-CHECKOUT" "Using existing source for ${repo} in ${dest}"
  fi
}

#+ hg-update <dir> [extra_flags] [rev]
hg-update() {
  local dir="$1"
  shift 1

  assert-dir "${dir}" \
    "HG repository $(basename "${dir}") doesn't exist. First do 'hg-checkout'"

  RunWithRetry ${HG_MAX_RETRIES} ${HG_RETRY_DELAY_SEC} \
    hg-update-try "${dir}" "$@"
}

hg-update-try() {
  local dir="$1"
  shift 1
  local retcode=0
  spushd "${dir}"
  RunWithLog "hg-update" hg update "$@" || retcode=$?
  spopd
  return ${retcode}
}

#+ hg-push <dir>
hg-push() {
  local dir="$1"
  spushd "${dir}"
  hg push
  spopd
}

hg-info() {
  local dir="$1"
  local rev="$2"

  spushd "$dir"
  local hg_status=$(hg status -mard)
  if [ ${#hg_status} -gt 0 ]; then
    LOCAL_CHANGES="YES"
  else
    LOCAL_CHANGES="NO"
  fi

  echo ""
  echo "Directory: hg/$(basename ${dir})"
  echo "  Branch         : $(hg branch)"
  echo "  Revision       : $(hg identify)"
  echo "  Local changes  : ${LOCAL_CHANGES}"
  echo "  Stable Revision: ${rev}"
  echo ""
  spopd
}

hg-at-revision() {
  local dir="$1"
  local rev="$2"

  spushd "${dir}"
  local HGREV=($(hg identify | tr -d '+'))
  spopd
  [ "${HGREV[0]}" == "${rev}" ]
  return $?
}

#+ hg-assert-is-merge <dir> - Assert an working directory contains a merge
hg-assert-is-merge() {
  local dir=$1
  spushd "${dir}"

  # When the working directory is a merge, hg identify -i
  # emits "changesetid1+changesetid2+"
  if hg identify -i | egrep -q '^[0-9a-f]+\+[0-9a-f]+\+$'; then
    spopd
    return
  fi
  local REPONAME=$(basename $(pwd))
  spopd
  Banner "ERROR: Working directory of '${REPONAME}' does not have a merge."
  exit -1
}

#+ hg-assert-branch <dir> <branch> - Assert hg repo in <dir> is on <branch>
hg-assert-branch() {
  local dir=$1
  local branch=$2
  spushd "${dir}"
  if hg branch | grep -q "^${branch}\$"; then
    spopd
    return 0
  fi
  local CURBRANCH=$(hg branch)
  local REPONAME=$(basename $(pwd))
  spopd
  Banner "ERROR: ${REPONAME} is on branch ${CURBRANCH} instead of ${branch}."
  return 1
}

#+ hg-assert-no-changes <dir> - Assert an hg repo has no local changes
hg-assert-no-changes() {
  local dir=$1
  spushd "${dir}"
  local PLUS=$(hg identify | tr -d -c '+')
  local STATUS=$(hg status)
  local REPONAME=$(basename $(pwd))
  spopd

  if [ "${PLUS}" != "" ]; then
    Banner "ERROR: Repository ${REPONAME} has local changes"
    return 1
  fi

  if [ "${STATUS}" != "" ]; then
    Banner "ERROR: Repository ${REPONAME} has untracked files"
    return 2
  fi

  return 0
}

hg-assert-no-outgoing() {
  local dir=$1
  spushd "${dir}"
  if hg outgoing | grep -q "^no changes found$" ; then
    spopd
    return
  fi
  local REPONAME=$(basename $(pwd))
  spopd
  msg="ERROR: Repository ${REPONAME} has outgoing commits. Clean first."
  Banner "${msg}"
  exit -1
}

#+ hg-commit <dir> <msg>
hg-commit() {
  local dir="$1"
  local msg="$2"
  spushd "${dir}"
  hg commit -m "${msg}"
  spopd
}

######################################################################
# Subversion repository tools
######################################################################

#+ svn-get-revision <dir>
svn-get-revision() {
  local dir="$1"
  spushd "${dir}"
  local rev=$(svn info | grep 'Revision: ' | cut -b 11-)
  if ! [[ "${rev}" =~ ^[0-9]+$ ]]; then
    echo "Invalid revision number '${rev}' or invalid repository '${dir}'" 1>&2
    exit -1
  fi
  spopd
  echo "${rev}"
}

#+ svn-checkout <url> <repodir> - Checkout an SVN repository
svn-checkout() {
  local url="$1"
  local dir="$2"

  if [ ! -d "${dir}" ]; then
    StepBanner "SVN-CHECKOUT" "Checking out ${url}"
    RunWithLog "svn-checkout" svn co "${url}" "${dir}"
  else
    SkipBanner "SVN-CHECKOUT" "Using existing SVN repository for ${url}"
  fi
}

#+ svn-update <dir> <rev> - Update an SVN repository
svn-update() {
  local dir="$1"
  local rev="$2"

  assert-dir "$dir" \
    "SVN repository $(basename "${dir}") doesn't exist."

  spushd "${dir}"
  if [[ "$rev" == "tip" ]]; then
    RunWithLog "svn-update" svn update
  else
    RunWithLog "svn-update" svn update -r ${rev}
  fi
  spopd
}

#+ svn-assert-no-changes <dir> - Assert an svn repo has no local changes
svn-assert-no-changes() {
  local dir=$1
  spushd "${dir}"
  local STATUS=$(svn status)
  local REPONAME=$(basename $(pwd))
  spopd

  if [ "${STATUS}" != "" ]; then
    Banner "ERROR: Repository ${REPONAME} has local changes"
    exit -1
  fi
}

######################################################################
# Logging tools
######################################################################

#@ progress              - Show build progress (open in another window)
progress() {
  StepBanner "PROGRESS WINDOW"

  while true; do
    if [ -f "${TC_LOG_ALL}" ]; then
      if tail --version > /dev/null; then
        # GNU coreutils tail
        tail -s 0.05 --max-unchanged-stats=20 --follow=name "${TC_LOG_ALL}"
      else
        # MacOS tail
        tail -F "${TC_LOG_ALL}"
      fi
    fi
    sleep 1
  done
}

#+ clean-logs            - Clean all logs
clean-logs() {
  rm -rf "${TC_LOG}"
}

# Logged pushd
spushd() {
  LogEcho "-------------------------------------------------------------------"
  LogEcho "ENTERING: $1"
  pushd "$1" > /dev/null
}

# Logged popd
spopd() {
  LogEcho "LEAVING: $(pwd)"
  popd > /dev/null
  LogEcho "-------------------------------------------------------------------"
  LogEcho "ENTERING: $(pwd)"
}

LogEcho() {
  mkdir -p "${TC_LOG}"
  echo "$*" >> "${TC_LOG_ALL}"
}

RunWithLog() {
  local log="${TC_LOG}/$1"

  mkdir -p "${TC_LOG}"

  shift 1
  echo "RUNNING: " "$@" | tee "${log}" >> "${TC_LOG_ALL}"
  "$@" 2>&1 | tee "${log}" >> "${TC_LOG_ALL}"
  if [ ${PIPESTATUS[0]} -ne 0 ]; then
    echo
    Banner "ERROR"
    echo -n "COMMAND:"
    PrettyPrint "$@"
    echo
    echo "LOGFILE: ${log}"
    echo
    echo "PWD: $(pwd)"
    echo
    if ${UTMAN_BUILDBOT}; then
      echo "BEGIN LOGFILE Contents."
      cat "${log}"
      echo "END LOGFILE Contents."
    fi
    return 1
  fi
  return 0
}

PrettyPrint() {
  # Pretty print, respecting parameter grouping
  for I in "$@"; do
    local has_space=$(echo "$I" | grep " ")
    if [ ${#has_space} -gt 0 ]; then
      echo -n ' "'
      echo -n "$I"
      echo -n '"'
    else
      echo -n " $I"
    fi
  done
  echo
}

assert-dir() {
  local dir="$1"
  local msg="$2"

  if [ ! -d "${dir}" ]; then
    Banner "ERROR: ${msg}"
    exit -1
  fi
}

assert-file() {
  local fn="$1"
  local msg="$2"

  if [ ! -f "${fn}" ]; then
    Banner "ERROR: ${fn} does not exist. ${msg}"
    exit -1
  fi
}

Usage() {
  egrep "^#@" "${SCRIPT_PATH}" | cut -b 3-
}

Usage2() {
  egrep "^#(@|\+)" "${SCRIPT_PATH}" | cut -b 3-
}

Banner() {
  echo ""
  echo " *********************************************************************"
  echo " | "
  for arg in "$@" ; do
    echo " | ${arg}"
  done
  echo " | "
  echo " *********************************************************************"
}

StepBanner() {
  local module="$1"
  if [ $# -eq 1 ]; then
    echo ""
    echo "-------------------------------------------------------------------"
    local padding=$(RepeatStr ' ' 28)
    echo "${padding}${module}"
    echo "-------------------------------------------------------------------"
  else
    shift 1
    local padding=$(RepeatStr ' ' $((20-${#module})) )
    echo "[$(TimeStamp)] ${module}${padding}" "$@"
  fi
}

TimeStamp() {
  if date --version &> /dev/null ; then
    # GNU 'date'
    date -d "now - ${TIME_AT_STARTUP}sec" '+%M:%S'
  else
    # Other 'date' (assuming BSD for now)
    local time_now=$(date '+%s')
    local time_delta=$[ ${time_now} - ${TIME_AT_STARTUP} ]
    date -j -f "%s" "${time_delta}" "+%M:%S"
  fi
}


SubBanner() {
  echo "----------------------------------------------------------------------"
  echo " $@"
  echo "----------------------------------------------------------------------"
}

SkipBanner() {
    StepBanner "$1" "Skipping $2, already up to date."
}

RepeatStr() {
  local str="$1"
  local count=$2
  local ret=""

  while [ $count -gt 0 ]; do
    ret="${ret}${str}"
    count=$((count-1))
  done
  echo "$ret"
}


Abort() {
  local module="$1"

  shift 1
  local padding=$(RepeatStr ' ' $((20-${#module})) )
  echo
  echo "${module}${padding} ERROR:" "$@"
  echo
  exit -1
}

PackageCheck() {
  assert-bin "makeinfo" "makeinfo not found. Please install 'texinfo' package."
  assert-bin "bison"    "bison not found. Please install 'bison' package."
  assert-bin "flex"     "flex not found. Please install 'flex' package."
}

assert-bin() {
  local exe="$1"
  local msg="$2"

  if ! which "$exe" > /dev/null; then
    Banner "ERROR: $msg"
    exit -1
  fi
}

is-ELF() {
  local F=$(file -b "$1")
  [[ "${F}" =~ "ELF" ]]
}

is-Mach() {
  local F=$(file -b "$1")
  [[ "${F}" =~ "Mach-O" ]]
}

is-shell-script() {
  local F=$(file -b "$1")
  [[ "${F}" =~ "shell" ]]
}

get_dir_size_in_mb() {
  du -msc "$1" | tail -1 | egrep -o "[0-9]+"
}

confirm-yes() {
  local msg="$1"
  while true; do
    echo -n "${msg} [Y/N]? "
    local YESNO
    read YESNO
    if [ "${YESNO}" == "N" ] || [ "${YESNO}" == "n" ]; then
      return 1
    fi
    if [ "${YESNO}" == "Y" ] || [ "${YESNO}" == "y" ]; then
      return 0
    fi
  done
}

# On Linux with GNU readlink, "readlink -f" would be a quick way
# of getting an absolute path, but MacOS has BSD readlink.
GetAbsolutePath() {
  local relpath=$1
  local reldir
  local relname
  if [ -d "${relpath}" ]; then
    reldir="${relpath}"
    relname=""
  else
    reldir="$(dirname "${relpath}")"
    relname="/$(basename "${relpath}")"
  fi

  local absdir="$(cd "${reldir}" && pwd)"
  echo "${absdir}${relname}"
}


# The Queue* functions provide a simple way of keeping track
# of multiple background processes in order to ensure that:
# 1) they all finish successfully (with return value 0), and
# 2) they terminate if the master process is killed/interrupted.
#    (This is done using a bash "trap" handler)
#
# Example Usage:
#     command1 &
#     QueueLastProcess
#     command2 &
#     QueueLastProcess
#     command3 &
#     QueueLastProcess
#     echo "Waiting for commands finish..."
#     QueueWait
#
# TODO(pdox): Right now, this abstraction is only used for
# paralellizing translations in the self-build. If we're going
# to use this for anything more complex, then throttling the
# number of active processes to exactly UTMAN_CONCURRENCY would
# be a useful feature.
CT_WAIT_QUEUE=""
QueueLastProcess() {
  local pid=$!
  CT_WAIT_QUEUE+=" ${pid}"
  if ! QueueConcurrent ; then
    QueueWait
  fi
}

QueueConcurrent() {
  [ ${UTMAN_CONCURRENCY} -gt 1 ]
}

QueueWait() {
  for pid in ${CT_WAIT_QUEUE} ; do
    wait ${pid}
  done
  CT_WAIT_QUEUE=""
}

# Add a trap so that if the user Ctrl-C's or kills
# this script while background processes are running,
# the background processes don't keep going.
trap 'QueueKill' SIGINT SIGTERM SIGHUP
QueueKill() {
  echo
  echo "Killing queued processes: ${CT_WAIT_QUEUE}"
  for pid in ${CT_WAIT_QUEUE} ; do
    kill ${pid} &> /dev/null || true
  done
  echo
  CT_WAIT_QUEUE=""
}

QueueEmpty() {
  [ "${CT_WAIT_QUEUE}" == "" ]
}

#+ RunWithRetry <max_retries> <delay_sec> cmd [args]
RunWithRetry() {
  local max_retries="$1"
  local delay_sec="$2"
  local cmdname=$(basename "$3")
  shift 2

  local retries=0
  while true; do
    if "$@" ; then
      return 0
    fi

    StepBanner "RUN-WITH-RETRY" "Execution of '${cmdname}' failed."
    retries=$[retries+1]
    if [ ${retries} -lt ${max_retries} ]; then
      StepBanner "RUN-WITH-RETRY" "Retrying in ${delay_sec} seconds."
      sleep ${delay_sec}
    else
      StepBanner "RUN-WITH-RETRY" \
        "'${cmdname}' failed ${max_retries} times. Aborting."
      return 1
    fi
  done

}
