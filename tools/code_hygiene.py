#!/usr/bin/python
# Copyright 2009, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


"""Simple Code Hygiene tool meant to be run as a presubmit test or standalone

Usage:
./code_hygiene.py <file1> <file2> ...

For the styleguides see:
http://code.google.com/p/soc/wiki/PythonStyleGuide
http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml
"""

import os
import re
import signal
import subprocess
import sys
import time

# ======================================================================
VERBOSE = 0
LIMIT = 5
TIMEOUT = 10

# These programs are likely not available/working on windows.

# http://tidy.sourceforge.net/
HTML_CHECKER = ['tidy', '-errors']

# From depot_tools:
# To see a list of all filters run: 'depot_tools/cpplint.py --filter='
CPP_CHECKER = ['cpplint.py', '--filter=-build/header_guard']

# From depot_tools (currently not used -- too many false positives).
# To see a list of all filters run: 'depot_tools/cpplint.py --filter='
C_CHECKER = ['cpplint.py', '--filter=-build/header_guard']

# http://pychecker.sourceforge.net/
PYTHON_CHECKER = ['pychecker']

RE_IGNORE = re.compile(r'@IGNORE_LINES_FOR_CODE_HYGIENE\[([0-9]+)\]')

# ======================================================================
def Debug(s):
  if VERBOSE:
    print s


def RunCommand(cmd):
  """Run a shell command given an argv style vector."""
  Debug(str(cmd))

  start = time.time()
  p = subprocess.Popen(cmd,
                       bufsize=1000*1000,
                       stderr=subprocess.PIPE,
                       stdout=subprocess.PIPE)
  while p.poll() is None:
    time.sleep(1)
    now = time.time()
    if now - start > TIMEOUT:
      Debug('Error: timeout')
      os.kill(p.pid, signal.SIGKILL)
      os.waitpid(-1, os.WNOHANG)
      return -666, "", ""
  stdout = p.stdout.read()
  stderr = p.stderr.read()
  retcode = p.wait()

  return retcode, stdout, stderr


# ======================================================================
class ExternalChecker(object):
  """Base Class"""
  def __init__(self, commandline, use_stderr=False):
    self._commandline = commandline
    self._use_stderr = use_stderr

  def FindProblems(self, props, unused_data):
    filename = props['name']
    try:
      retcode, stdout, stderr = RunCommand(self._commandline + [filename])
    except Exception, err:
      print 'Error processing %s:' % filename
      print '  Cannot execute command %s. failed reason: %s ' % (
          str(self._commandline), str(err))
      return []
    if retcode == 0:
      return []
    if retcode == 666:
      print 'command %s timed out' % str(self._commandline)
      return []
    if self._use_stderr:
      return [stderr]
    else:
      return [stdout]

  def FileFilter(self, unused_props):
    raise NotImplementedError()


class TidyChecker(ExternalChecker):
  """Invokes tidy tool on html files."""
  def __init__(self):
    ExternalChecker.__init__(self, HTML_CHECKER, use_stderr=True)
    return

  def FileFilter(self, props):
    return '.html' in props


class PyChecker(ExternalChecker):
  """Invokes pychecker tool on python files."""
  def __init__(self):
    ExternalChecker.__init__(self, PYTHON_CHECKER)
    return

  def FileFilter(self, props):
    return '.py' in props


class CppLintChecker(ExternalChecker):
  """Invokes Google's cpplint tool on c++ files."""
  def __init__(self):
    ExternalChecker.__init__(self, CPP_CHECKER, use_stderr=True)
    return

  def FileFilter(self, props):
    return '.cc' in props or '.h' in props


# ======================================================================
class GenericRegexChecker(object):
  """Base Class"""
  def __init__(self, content_regex, line_regex=None, analyze_match=False):
    self._content_regex = re.compile(content_regex, re.MULTILINE)
    if line_regex:
      self._line_regex = re.compile(line_regex)
    else:
      self._line_regex = re.compile(content_regex)
    self._analyze_match = analyze_match
    return

  def FindProblems(self, unused_props, data):
    """Looks for presubmit issues in the data from a file."""
    # Speedy early out.
    if not self._content_regex.search(data):
      return []

    lines = data.split('\n')
    problem = []
    ignore_line_count = 0
    for no, line in enumerate(lines):
      if ignore_line_count > 0:
        ignore_line_count -= 1
        continue
      match = RE_IGNORE.search(line)
      if match:
        ignore_line_count = int(match.group(1))
        continue
      match = self._line_regex.search(line)
      if not match:
        continue
      if self._analyze_match:
        if not self.IsProblemMatch(match):
          continue
      problem.append(self._RenderItem(no, line))
    return problem

  def _RenderItem(self, no, line):
    """Returns a formatted message."""
    return 'line %d: [%s]' % (no, repr(line))

  def FileFilter(self, unused_props):
    """Returns true if the file should be checked by the checker."""
    # NOTE: All subclasses need to implement this and return True if the
    # input Properties show that this file should be checked.
    raise NotImplementedError()

  def IsProblemMatch(self, unused_match):
    """Returns true if the file contains a presubmit problem."""
    # NOTE: Not all subclasses need to implement this.
    pass


class OpenCurlyChecker(GenericRegexChecker):
  """Checks for orphan opening curly braces."""
  def __init__(self):
    GenericRegexChecker.__init__(self, r'^ *{ *$')
    return

  def FileFilter(self, props):
    return '.h' in props or '.cc' in props or '.c' in props


class TrailingWhiteSpaceChecker(GenericRegexChecker):
  """No trailing whitespaces in code we control."""
  def __init__(self):
    GenericRegexChecker.__init__(self, r' $')
    return

  def FileFilter(self, props):
    return '.patch' not in props


class UntrustedIfDefChecker(GenericRegexChecker):
  """No #ifdef in untrusted code (except stubs)."""
  def __init__(self):
    GenericRegexChecker.__init__(self, r'^#if')
    return

  def FileFilter(self, props):
    if 'is_untrusted' not in props: return False
    if 'is_untrusted_stubs' in props: return False
    return '.c' in props or '.cc' in props


class UntrustedAsmChecker(GenericRegexChecker):
  """No inline assembler in untrusted code."""
  def __init__(self):
    # TODO(robertm): also cope with asm
    GenericRegexChecker.__init__(self, r'__asm__')
    return

  def FileFilter(self, props):
    if 'is_untrusted' not in props: return False
    return '.c' in props or '.cc' in props or '.h' in props


class CarriageReturnChecker(GenericRegexChecker):
  """Abolish windows style line terminators."""
  def __init__(self):
    GenericRegexChecker.__init__(self, r'\r')
    return

  def FileFilter(self, props):
    return '.h' in props or '.cc' in props or '.c' in props or '.py' in props


class CppCommentChecker(GenericRegexChecker):
  """No cpp comments in regular c files."""
  def __init__(self):
    # We tolerate http:// etc in comments
    GenericRegexChecker.__init__(self, r'(^|[^:])//')
    return

  def FileFilter(self, props):
    return '.c' in props


class TabsChecker(GenericRegexChecker):
  """No tabs except for makefiles."""
  def __init__(self):
    GenericRegexChecker.__init__(self, r'\t')
    return

  def FileFilter(self, props):
    return ('is_makefile' not in props and
            '.patch' not in props and
            '.valerr' not in props and
            '.dis' not in props)


class FixmeChecker(GenericRegexChecker):
  """No FIXMEs in code we control."""
  def __init__(self):
    GenericRegexChecker.__init__(self, r'\bFIXME\b')
    return

  def FileFilter(self, props):
    return '.patch' not in props


class ExternChecker(GenericRegexChecker):
  """Only c/c++ headers may have externs."""
  def __init__(self):
    GenericRegexChecker.__init__(self, r'^ *extern\s+(.*)$', analyze_match=True)
    return

  def IsProblemMatch(self, match):
    extra = match.group(1).strip()
    # Allow 'extern "C" {'
    # but do not allow plain 'extern "C"'
    if extra == '"C" {':
      return False
    return True

  def FileFilter(self, props):
    return '.cc' in props or '.c' in props


class RewriteChecker(GenericRegexChecker):
  """No rewrite markers allowed (probaly not an issue anymore)."""
  def __init__(self):
    GenericRegexChecker.__init__(self, r'[@][@]REWRITE')
    return

  def FileFilter(self, unused_props):
    return 1


VALID_INCLUDE_PREFIX = [
    'native_client/',
    'third_party/',
    'gtest/',
    'gen/native_client/',
    'base/',
    'chrome/common',
    ]


class IncludeChecker(object):
  """Enforce some base 'include' rules. Cpplint does more of these."""
  def __init__(self):
    pass

  def FindProblems(self, unused_props, data):
    lines = data.split('\n')
    problem = []
    seen_code = False
    for no, line in enumerate(lines):
      if line.startswith('}'):
        # This test is our hacked indicator signaling that no includes
        # directives should follow after we have seen actual code.
        seen_code = True
        continue

      if not line.startswith('#include'):
        continue
      if seen_code:
        problem.append('line %d: [%s]' % (no, repr(line)))
      token = line.split()
      path = token[1][1:]
      if '..' in path:
        problem.append('line %d: [%s]' % (no, repr(line)))
      if not token[1].startswith('<'):
        for prefix in VALID_INCLUDE_PREFIX:
          if path.startswith(prefix):
            break
        else:
          # No prefix matched.
          problem.append('line %d: [%s]' % (no, repr(line)))

    return problem

  def FileFilter(self, props):
    return '.cc' in props or '.c' in props


class LineLengthChecker(object):
  def __init__(self):
    self._dummy = None   # Shut up pylint.

  def FindProblems(self, unused_props, data):
    lines = data.split('\n')
    problem = []
    for no, line in enumerate(lines):
      if len(line) <= 80:
        continue
      line = line.strip()
      # We allow comments (and as it so happens, preprocessor directives) to
      # be longer
      if line.startswith('//') or line.startswith('/*') or line.startswith('#'):
        continue
      problem.append('line %d: [%s]' % (no + 1, repr(line)))
    return problem

  def FileFilter(self, props):
    return ('.patch' not in props and
            '.val' not in props and
            '.valerr' not in props)


# ======================================================================
# 'props' (short for properties) are tags associated with checkable files.
#
# Except for the 'name', props do not carry any associated value.
#
# Props are used to make decisions on whether (and how) a certain check is
# applicable.
#
# Props are usually derived from the pathname but can also be forced using
# the magic string '@PROPS[prop1,prop2,...]' anywhere in the first 2k of
# data.

VALID_PROPS = {
    'name': True,                # Filename, only prop with a value.
    'is_makefile': True,
    'is_trusted': True,          # Is trusted code.
    'is_untrusted': True,        # Is untrusted code.
    'is_untrusted_stubs': True,  # Is untrusted/stubs code.
    'is_shared': True,           # Is shared code.
    'is_scons': True,            # Is scons file (not one generated by gyp).
}


def IsValidProp(prop):
  """Returns true if the property name is valid.

  Valid property names start with '.' or are listed in VALID_PROPS.

  Args:
    prop: a property name.
  Returns:
    True if the property name is valid.
  """
  if prop.startswith('.'):
    return True
  return prop in VALID_PROPS


def FindProperties(data):
  """Finds extra properties for a file.

  Finds strings of the form '@PROPS[name name name]'.

  Args:
    data: The contents of the file being checked.
  Returns:
    A list of properties from the PROPS line in the file.
  """
  match = re.search(r'@PROPS\[([^\]]*)\]', data)
  if match:
    return match.group(1).split(',')
  return []


def FileProperties(filename, data):
  """Finds all properties associated with this file.

  Valid properties are based on the file's name and on the '@PROPS' line in
  the file.

  Args:
    filename: the filename.
    data: The contents of the file being checked.
  Returns:
    A map of property to value for each property associated with this file.
  """
  d = {}
  d['name'] = filename
  _, ext = os.path.splitext(filename)
  if ext:
    d[ext] = True

  if 'akefile' in filename:
    d['is_makefile'] = True

  if 'src/trusted/' in filename:
    d['is_trusted'] = True
  if 'src/untrusted/stubs/' in filename:
    d['is_untrusted_stubs'] = True
  if 'src/untrusted/' in filename:
    d['is_untrusted'] = True
  if 'src/shared/' in filename:
    d['is_shared'] = True

  if (filename.endswith('nacl.scons') or
      filename.endswith('build.scons') or
      filename.endswith('SConstruct')):
      d['is_scons'] = True

  # NOTE: Look in a somewhat arbitrary region at the file beginning for
  #       a special marker advertising addtional properties.
  for p in FindProperties(data[:2048]):
    d[p] = True

  for p in d:
    assert IsValidProp(p)
  return d


# ======================================================================
def IsBinary(data):
  """Checks for a 0x00 byte as a proxy for 'binary-ness'."""
  return '\x00' in data


# ======================================================================
# CHECK entry is: is_fatal, name, check_function.
CHECKS = [# fatal checks
          (True, 'tabs', TabsChecker()),
          (True, 'trailing_space', TrailingWhiteSpaceChecker()),
          (True, 'cpp_comment', CppCommentChecker()),
          (True, 'fixme', FixmeChecker()),
          (True, 'include', IncludeChecker()),
          (True, 'extern', ExternChecker()),
          (True, 'untrusted_ifdef', UntrustedIfDefChecker()),
          (True, 'untrusted_asm', UntrustedAsmChecker()),
          # Non fatal checks
          (False, 'open_curly', OpenCurlyChecker()),
          (False, 'line_length', LineLengthChecker()),
          (False, 'carriage_return', CarriageReturnChecker()),
          (False, 'rewrite', RewriteChecker()),
          (False, 'tidy', TidyChecker()),
          (False, 'pychecker', PyChecker()),
          (False, 'cpplint', CppLintChecker()),
          ]


def EmitStatus(filename, status, details=[]):
  """Prints the status and all supporting details for a file."""
  print '%s: %s' % (filename, status)
  for no, line in details[:LIMIT]:
    print '  line %d: [%s]' % (no, repr(line))
  return


def FindExemptions(data):
  """Looks for and returns EXEMPTION items from the data.

  Finds strings of the form '@EXEMPTION[name name name]' where
  'name' is one of the names in the CHECKS table.

  Args:
    data: The contents of the file being checked.
  """
  match = re.search(r'@EXEMPTION\[([^\]]*)\]', data)
  if match:
    return match.group(1).split(',')
  return []


def CheckFile(filename, report):
  """Runs all possible presubmit checks against the filename."""
  Debug('\n' + filename)
  errors = {}
  warnings = {}

  data = open(filename).read()

  if IsBinary(data):
    EmitStatus(filename, 'binary')
    return errors, warnings

  exemptions = FindExemptions(data)
  props = FileProperties(filename, data)
  Debug('PROPS: ' + repr(props.keys()))

  for fatal, name, check in CHECKS:
    Debug('trying %s' % name)

    if not check.FileFilter(props):
      Debug('skipping')
      continue

    if name in exemptions:
      Debug('exempt')
      continue

    problems = check.FindProblems(props, data)
    if problems:
      info = '%s: %s' % (filename, name)
      items = problems[:LIMIT]
      if report:
        print info
        for i in items:
          print i
      if fatal:
        errors[info] = items
        break
      else:
        warnings[info] = items
  return errors, warnings


def main(argv):
  """Runs all required presubmit checks for all listed files."""
  num_error = 0
  for filename in argv:
    if os.path.isdir(filename):
      EmitStatus(filename, 'directory')
      continue

    if '-' in os.path.basename(filename):
      print 'hyphen in:', filename
      num_error += 1
      continue

    errors, warnings = CheckFile(filename, True)
    if errors:
      num_error += 1
  return num_error

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
