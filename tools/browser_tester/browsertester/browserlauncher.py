#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os.path
import re
import shutil
import subprocess
import sys
import tempfile
import time

# mozrunner is needed as long as we are supporting versions of Python
# before 2.6.
import mozrunner


def SelectRunCommand():
  # The subprocess module added support for .kill in Python 2.6
  if sys.version_info[0] < 2 or (sys.version_info[0] == 2 and
                                 sys.version_info[1] < 6):
    return mozrunner.run_command
  else:
    return subprocess.Popen


RunCommand = SelectRunCommand()

def RemoveDirectory(path):
  retry = 4
  while True:
    try:
      shutil.rmtree(path)
    except Exception:
      # Windows processes sometime hang onto files too long
      if retry > 0:
        retry -= 1
        time.sleep(0.125)
      else:
        # No luck - don't mask the error
        raise
    else:
      # succeeded
      break


class LaunchFailure(Exception):
  pass


def GetPlatform():
  if sys.platform == 'darwin':
    platform = 'mac'
  elif sys.platform == 'linux2':
    platform = 'linux'
  elif sys.platform in ('cygwin', 'win32'):
    platform = 'windows'
  else:
    raise LaunchFailure('Unknown platform: %s' % sys.platform)
  return platform


PLATFORM = GetPlatform()


# In Windows, subprocess seems to have an issue with file names that
# contain spaces.
def EscapeSpaces(path):
  if PLATFORM == 'windows' and ' ' in path:
    return '"%s"' % path
  return path


def MakeEnv(debug):
  env = dict(os.environ)
  if debug:
    env['PPAPI_BROWSER_DEBUG'] = '1'
    env['NACL_PLUGIN_DEBUG'] = '1'
    env['NACL_PPAPI_PROXY_DEBUG'] = '1'
    # env['NACL_SRPC_DEBUG'] = '1'
  return env


class BrowserLauncher(object):

  WAIT_TIME = 20
  WAIT_STEPS = 80
  SLEEP_TIME = float(WAIT_TIME) / WAIT_STEPS

  def __init__(self, options):
    self.options = options
    self.profile = None
    self.binary = None
    self.tool_log_dir = None

  def KnownPath(self):
    raise NotImplementedError

  def BinaryName(self):
    raise NotImplementedError

  def CreateProfile(self):
    raise NotImplementedError

  def MakeCmd(self, url):
    raise NotImplementedError

  def CreateToolLogDir(self):
    self.tool_log_dir = tempfile.mkdtemp(prefix='vglogs_')
    return self.tool_log_dir

  def FindBinary(self):
    if self.options.browser_path:
      return self.options.browser_path
    else:
      path = self.KnownPath()
      if path is None or not os.path.exists(path):
        raise LaunchFailure('Cannot find the browser directory')
      binary = os.path.join(path, self.BinaryName())
      if not os.path.exists(binary):
        raise LaunchFailure('Cannot find the browser binary')
      return binary

  # subprocess.wait() doesn't have a timeout, unfortunately.
  def WaitForProcessDeath(self):
    i = 0
    while self.handle.poll() is None and i < self.WAIT_STEPS:
      time.sleep(self.SLEEP_TIME)
      i += 1

  def Cleanup(self):
    if self.handle.poll() is None:
      print 'KILLING the browser'
      self.handle.kill()
      # If is doesn't die, we hang.  Oh well.
      self.handle.wait()

    RemoveDirectory(self.profile)
    if self.tool_log_dir is not None:
      RemoveDirectory(self.tool_log_dir)

  def MakeProfileDirectory(self):
    self.profile = tempfile.mkdtemp(prefix='browserprofile_')
    return self.profile

  def Launch(self, cmd, env):
    browser_path = cmd[0]
    if not os.path.exists(browser_path):
      raise LaunchFailure('Browser does not exist %r'% browser_path)
    if not os.access(browser_path, os.X_OK):
      raise LaunchFailure('Browser cannot be executed %r (Is this binary on an '
                          'NFS volume?)' % browser_path)
    if self.options.sel_ldr:
      env['NACL_SEL_LDR'] = self.options.sel_ldr
    if self.options.irt_library:
      env['NACL_IRT_LIBRARY'] = self.options.irt_library
    if self.options.enable_experimental_js:
      env['NACL_ENABLE_EXPERIMENTAL_JAVASCRIPT_APIS'] = '1'
    print 'ENV:', ' '.join(['='.join(pair) for pair in env.iteritems()])
    print 'LAUNCHING: %s' % ' '.join(cmd)
    sys.stdout.flush()
    self.handle = RunCommand(cmd, env=env)
    print 'PID', self.handle.pid

  def IsRunning(self):
    return self.handle.poll() is None

  def Run(self, url, port):
    self.binary = EscapeSpaces(self.FindBinary())
    self.profile = self.CreateProfile()
    if self.options.tool is not None:
      self.tool_log_dir = self.CreateToolLogDir()
    cmd = self.MakeCmd(url, port)
    self.Launch(cmd, MakeEnv(self.options.debug))


def EnsureDirectory(path):
  if not os.path.exists(path):
    os.makedirs(path)


def EnsureDirectoryForFile(path):
  EnsureDirectory(os.path.dirname(path))


class ChromeLauncher(BrowserLauncher):

  def KnownPath(self):
    if PLATFORM == 'linux':
      # TODO(ncbray): look in path?
      return '/opt/google/chrome'
    elif PLATFORM == 'mac':
      return '/Applications/Google Chrome.app/Contents/MacOS'
    else:
      homedir = os.path.expanduser('~')
      path = os.path.join(homedir, r'AppData\Local\Google\Chrome\Application')
      return path

  def BinaryName(self):
    if PLATFORM == 'mac':
      return 'Google Chrome'
    elif PLATFORM == 'windows':
      return 'chrome.exe'
    else:
      return 'chrome'

  def MakeEmptyJSONFile(self, path):
    EnsureDirectoryForFile(path)
    f = open(path, 'w')
    f.write('{}')
    f.close()

  def CreateProfile(self):
    profile = self.MakeProfileDirectory()

    # Squelch warnings by creating bogus files.
    self.MakeEmptyJSONFile(os.path.join(profile, 'Default', 'Preferences'))
    self.MakeEmptyJSONFile(os.path.join(profile, 'Local State'))

    return profile

  def MakeCmd(self, url, port):
    cmd = [self.binary,
            '--disable-web-resources',
            '--disable-preconnect',
            '--no-first-run',
            '--no-default-browser-check',
            '--enable-logging',
            '--log-level=1',
            '--safebrowsing-disable-auto-update',
            # Chrome explicitly blacklists some ports as "unsafe" because
            # certain protocols use them.  Chrome gives an error like this:
            # Error 312 (net::ERR_UNSAFE_PORT): Unknown error
            # Unfortunately, the browser tester can randomly choose a
            # blacklisted port.  To work around this, the tester whitelists
            # whatever port it is using.
            '--explicitly-allowed-ports=%d' % port,
            '--user-data-dir=%s' % self.profile]
    if self.options.ppapi_plugin is None:
      cmd.append('--enable-nacl')
    else:
      cmd.append('--register-pepper-plugins=%s;application/x-nacl'
                 % self.options.ppapi_plugin)
      cmd.append('--no-sandbox')
    if self.options.browser_extensions:
      for extension in self.options.browser_extensions:
        cmd.append('--load-extension=%s' % extension)
      cmd.append('--enable-experimental-extension-apis')
    if self.options.tool == 'memcheck':
      cmd = ['src/third_party/valgrind/memcheck.sh',
             '-v',
             '--xml=yes',
             '--leak-check=no',
             '--gen-suppressions=all',
             '--num-callers=30',
             '--trace-children=yes',
             '--nacl-file=%s' % (self.options.files[0],),
             '--suppressions=' +
             '../tools/valgrind/memcheck/suppressions.txt',
             '--xml-file=%s/xml.%%p' % (self.tool_log_dir,),
             '--log-file=%s/log.%%p' % (self.tool_log_dir,)] + cmd
    elif self.options.tool == 'tsan':
      cmd = ['src/third_party/valgrind/tsan.sh',
             '-v',
             '--num-callers=30',
             '--trace-children=yes',
             '--nacl-file=%s' % (self.options.files[0],),
             '--ignore=../tools/valgrind/tsan/ignores.txt',
             '--suppressions=../tools/valgrind/tsan/suppressions.txt',
             '--log-file=%s/log.%%p' % (self.tool_log_dir,)] + cmd
    elif self.options.tool != None:
      raise LaunchFailure('Invalid tool name "%s"' % (self.options.tool,))
    cmd.append(url)
    return cmd
