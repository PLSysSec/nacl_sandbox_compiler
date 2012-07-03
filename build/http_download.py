#!/usr/bin/python
# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Download a file from a URL to a file on disk.

This module supports username and password with basic authentication.
"""

import base64
import os.path
import sys
import urllib2

import download_utils


def _CreateDirectory(path):
  """Create a directory tree, ignore if it's already there."""
  try:
    os.makedirs(path)
    return True
  except os.error:
    return False


def HttpDownload(url, target, username=None, password=None, verbose=True):
  """Download a file from a remote server.

  Args:
    url: A URL to download from.
    target: Filename to write download to.
    username: Optional username for download.
    password: Optional password for download (ignored if no username).
  """

  headers = [('Accept', '*/*')]
  if username:
    if password:
      auth_code = base64.b64encode(username + ':' + password)
    else:
      auth_code = base64.b64encode(username)
    headers.append(('Authorization', 'Basic ' + auth_code))
  opener = urllib2.build_opener()
  opener.addheaders = headers
  urllib2.install_opener(opener)
  _CreateDirectory(os.path.split(target)[0])
  # Retry up to 10 times (appengine logger is flaky).
  for i in xrange(10):
    if i:
      sys.stdout.write('Download failed on %s, retrying... (%d)\n' % (url, i))
    try:
      # 30 second timeout to ensure we fail and retry on stalled connections.
      src = urllib2.urlopen(url, timeout=30)
      try:
        download_utils.WriteDataFromStream(target, src, chunk_size=2**20,
                                           verbose=verbose)
        content_len = src.headers.get('Content-Length')
        if content_len:
          content_len = int(content_len)
          file_size = os.path.getsize(target)
          if content_len != file_size:
            sys.stdout.write('Filesize:%d does not match Content-Length:%d' % (
                file_size, content_len))
            continue
      finally:
        src.close()
      break
    except urllib2.HTTPError:
      pass
  else:
    sys.stdout.write('Download failed on %s, giving up.\n' % url)
    raise
