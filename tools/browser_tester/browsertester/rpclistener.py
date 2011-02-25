#!/usr/bin/python
# Copyright 2010 The Native Client Authors.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.


import sys


class RPCListener(object):

  def __init__(self, shutdown_callback):
    self.shutdown_callback = shutdown_callback
    self.prefix = '**** '
    self.ever_failed = False

  def Log(self, message):
    lines = [line.rstrip() for line in message.split('\n')]
    text = ''.join(['%s%s\n' % (self.prefix, line) for line in lines])
    sys.stdout.write(text)

  def TestLog(self, message):
    self.Log(message)
    return 'OK'

  # Something went very wrong on the server side, everything is horked?
  # Only called locally.
  def ServerError(self, message):
    self.Log('\n[SERVER_ERROR] %s' % (message,))
    self.ever_failed = True
    self._TestingDone()
    return 'OK'

  def Shutdown(self, message, passed):
    self.Log(message)
    # This check looks slightly backwards, but this is intentional.
    # Everything but passed.lower() == 'true' is considered a failure.  This
    # means that if the test runner sends garbage, it will be a failure.
    # NOTE in interactive mode this function may be called multiple times.
    # ever_failed is designed to be set and never reset - if any of the runs
    # fail, the an error code will be returned to the command line.
    # In summary, the tester is biased towards failure - it should scream "FAIL"
    # if things are not 100% correct.  False positives must be avoided.
    if passed.lower() != 'true':
      self.ever_failed = True
    self._TestingDone()
    return 'OK'

  def _TestingDone(self):
    self.shutdown_callback()