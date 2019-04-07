#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function

import os
import sys
import coverage_helper

"""
Coverage for Linux

On the Linux Coverage Bots, coverage information is stored in
   scons-out/coverage-<os>-<arch>/coverage.lcov

The data is in the form of:
  SF:<file_name>
  DA:<line,hits>
  end_of_record

Where SF specifices the full path to the soruce file.  DA specifies a line
number in that source and how many hits happened on that line. 'end_of_record'
specifies the end of a source file.

The lcov data is generated by updating hit counts for any source file contained
in any instrumented executable.   The lcov data will only list lines which
were compiled into the executables.

The data is transformed into a dictionary containing per file coverage data
which is also mapped into per 'group' coverage data where a group is list
of paths of interest.
"""


#
# SortedOutput
#
# SortedOutput is an object for collecting lines of output which can be
# sorted in sections.  Lines are added via 'append' and sections are
# declared via the 'section' function, which can prepend a title and
# optionally sort the current section.
#
class SortedOutput(object):
  def __init__(self):
    self.out_lines = []
    self.q_lines = []

  def append(self, line):
    self.q_lines.append(line)

  def section(self, sort_it = True, title = None):
    # Insert title if one is provided
    if title: self.out_lines.append(title)

    # Sort the current lines if requested
    if sort_it: self.q_lines.sort()

    # Append the current set of lines to the output
    self.out_lines.extend(self.q_lines)
    self.q_lines = []

  def output(self, out = sys.stdout):
    # Append any lines in the outstanding unsorted list
    self.out_lines.extend(self.q_lines)
    for line in self.out_lines:
      out.write('%s\n' % line)

    # Clear the output
    self.out_lines = []
    self.q_lines = []

#
# CoverageParse
#
# Parse the 'lcov' data generated by the scons coverage target into a
# dictionary mapping file to a touple of lines hits and total lines.
def CoverageParse(lines, ignore_set):
  # Starting with an empty dictionary
  out = {}
  filename = None

  # Scan each input line
  for line in lines:
    line = line.strip()
    words = line.split(':')

    # Ignore malformed lines
    if len(words) < 1 or len(words) > 2: continue

    # If we are starting a new file, clear running totals
    if words[0] == 'SF':
      filename = words[1]
      if sys.platform == 'win32':
        if filename.startswith('/cygdrive/c/'):
          filename = 'C:/' + filename[len('/cygdrive/c/'):]
        if filename.startswith('/cygdrive/e/'):
          filename = 'E:/' + filename[len('/cygdrive/e/'):]
        filename = os.path.normpath(filename)
      total = 0
      used  = 0
      continue

    # If we have line data, append it to the list of line hits
    if words[0] == 'DA':
      # Split the data into line number/hits
      info = words[1].split(',')

      # Check if there are any hits on this line
      if int(info[1]): used += 1
      total += 1
      continue

    # If this is the end of a file, store the running totals
    if line == 'end_of_record':
      # If this file is not ignored, add it to the dictionary
      path, name = os.path.split(filename)
      if not name in ignore_set:
        out[filename] = (used, total)

      # Reset filename to None to force an exception of something goes wrong
      # in the parsing (such as missing a start of file marker)
      filename = None
      continue
  return out


def CoveragePercent(used, total, name):
  if total:
    percent = float(used) * float(100) / float(total)
    return '%6.2f  %d/%d %s' % (percent, used, total, name)
  else:
    return '%6.2f  > Unused < %s' % (0.0, name)


def ShortGroup(path):
  parts = path.rsplit('/native_client/', 1)
  if len(parts) > 1:
    name = parts[1]
  else:
    name = path
  return name.replace('/', '_')


def CoverageResult(used, total, name):
  # Perf dashboard format results.
  return [
      'RESULT coverage_percent_%s: coverage_percent_%s= %f coverage%%' % (
          name, name, used * 100.0 / total),
      'RESULT lines_%s: lines_%s= %d lines' % (
          name, name, total),
      'RESULT covered_%s: covered_%s= %d lines' % (
          name, name, used),
  ]


def CoverageProcess(platform, verbose = False):
  helper = coverage_helper.CoverageHelper()
  groups  = helper.groups
  filters = helper.path_filter

  # Load and parse the 'lcov' file
  covfile = 'scons-out/coverage-%s/coverage/coverage.lcov' % platform
  covdata = open(covfile, 'r').readlines()
  cov = CoverageParse(covdata, helper.ignore_set)

  # Get a sorted list of sources
  filelist = sorted(cov.keys())

  # Construct output objects
  filtered = SortedOutput()
  perfile = SortedOutput()
  pergroup = SortedOutput()
  summary = SortedOutput()
  results = []

  # Build the group dictionary with an empty list for each key
  group_data = {}
  for g in groups: group_data[g] = []

  for f in filelist:
    # Ignore test files and standard includes
    if f.find('test') >= 0: continue
    if f.find('/usr/include') >= 0: continue

    # If the first group this path matches
    group = None
    for g in groups:
      if f.find(g) >= 0:
        group = g
        break

    # Retrieved the used and total line information for this file
    used = cov[f][0]
    total = cov[f][1]

    # If this file matched a group, add it to that group's data
    if group: group_data[group].append((used, total))

    # In either case, add it to the per file data
    perfile.append(CoveragePercent(used, total, f))

  # Calculate the over all percentage by interating through non-filtered groups
  global_total = 0
  global_used = 0
  for group in groups:
    used = 0
    total = 0
    for u, t in group_data[group]:
      used += u
      total += t

    sgroup = ShortGroup(group)

    # If this is a filtered group, add it to the filtered output set
    if group in filters:
      filtered.append(CoveragePercent(used, total, sgroup))
    else:
      # Otherwise add it to the per group only if the group has hits
      if total:
        pergroup.append(CoveragePercent(used, total, sgroup))
        results.extend(CoverageResult(used, total, sgroup))
        global_total += total
        global_used += used

  # Add titles and sort each output group
  perfile.section(title = '\nPer file:')
  pergroup.section(title = '\nGroups:')
  filtered.section(title = '\nFiltered files:')

  summary.append(CoveragePercent(global_used, global_total, 'Overall'))
  summary.section(title = '\nSummary')
  results.extend(CoverageResult(global_used, global_total, 'Overall'))

  summary.output()
  pergroup.output()
  filtered.output()
  if verbose: perfile.output()

  # Emit perf dashboard results.
  print('')
  print('Dashboard Results')
  print('-----------------')
  for line in results:
    print(line)
  print('')

  return (global_used * 100) / global_total

def main(argv):
  platform = argv[0]

  # TOODO Raise the coverage requirement once we get coverage to a reasonable
  # number.
  if platform.startswith('linux-'):
    coverage_target = 45
  else:
    coverage_target = 12

  if CoverageProcess(platform) >= coverage_target:
    return 0
  print('Coverage bellow %d%%, failed.' % coverage_target)
  return -1

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
