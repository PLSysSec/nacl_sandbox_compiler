# -*- python -*-
# Copyright 2010 The Native Client Authors.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

"""Build "SRPC" interfaces from specifications.

SRPC interfaces consist of one or more interface classes, typically defined
in a set of .srpc files.  The specifications are Python dictionaries, with a
top level 'name' element and an 'rpcs' element.  The rpcs element is a list
containing a number of rpc methods, each of which has a 'name', an 'inputs',
and an 'outputs' element.  These elements are lists of input or output
parameters, which are lists pairs containing a name and type.  The set of
types includes all the SRPC basic types.

These SRPC specifications are used to generate a header file and either a
server or client stub file, as determined by the command line flag -s or -c.
"""

import getopt
#import re
#import string
#import StringIO
import sys
import os

AUTOGEN_COMMENT = """\
// Copyright (c) 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// Automatically generated code.  See srpcgen.py
//
// NaCl Simple Remote Procedure Call interface abstractions.
"""

HEADER_START = """\
#ifndef xyz
#define xyz
#ifndef __native_client__
#include "native_client/src/include/portability.h"
#endif  // __native_client__
#include "native_client/src/shared/srpc/nacl_srpc.h"
"""

HEADER_END = """\
\n\n#endif  // xyz
"""

SOURCE_FILE_INCLUDES = """\
#include "xyz"
#ifdef __native_client__
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) do { (void) P; } while (0)
#endif  // UNREFERENCED_PARAMETER
#else
#include "native_client/src/include/portability.h"
#endif  // __native_client__
#include "native_client/src/shared/srpc/nacl_srpc.h"
"""

types = {'bool': ['b', 'bool', 'u.bval', ''],
         'char[]': ['C', 'char*', 'arrays.carr', 'u.count'],
         'double': ['d', 'double', 'u.dval', ''],
         'double[]': ['D', 'double*', 'arrays.darr', 'u.count'],
         'handle': ['h', 'NaClSrpcImcDescType', 'u.hval', ''],
         'int32_t': ['i', 'int32_t', 'u.ival', ''],
         'int32_t[]': ['I', 'int32_t*', 'arrays.iarr', 'u.count'],
         'int64_t': ['l', 'int64_t', 'u.lval', ''],
         'int64_t[]': ['L', 'int64_t', 'arrays.larr', 'u.count'],
         'PP_Instance': ['i', 'PP_Instance', 'u.ival', ''],
         'PP_Module': ['i', 'PP_Module', 'u.ival', ''],
         'PP_Resource': ['i', 'PP_Resource', 'u.ival', ''],
         'string': ['s', 'char*', 'arrays.str', ''],
        }


def AddHeader(name):
  """Adds a header to both the .cc and .h files."""
  global HEADER_START
  global SOURCE_FILE_INCLUDES

  HEADER_START += "#include \"%s\"\n" % name
  SOURCE_FILE_INCLUDES += "#include \"%s\"\n" % name


def CountName(name):
  """Returns the name of the auxiliary count member used for array typed."""
  return '%s_bytes' % name


def FormatRpcPrototype(is_server, class_name, indent, rpc):
  """Returns a string for the prototype of an individual RPC."""

  def FormatArgs(is_output, args):
    """Returns a string containing the formatted arguments for an RPC."""

    def FormatArg(is_output, arg):
      """Returns a string containing a formatted argument to an RPC."""
      if is_output:
        suffix = '* '
      else:
        suffix = ' '
      s = ''
      type_info = types[arg[1]]
      if type_info[3]:
        s += 'nacl_abi_size_t%s%s, %s %s' % (suffix,
                                     CountName(arg[0]),
                                     type_info[1],
                                     arg[0])
      else:
        s += '%s%s%s' % (type_info[1], suffix, arg[0])
      return s
    s = ''
    for arg in args:
      s += ',\n    %s%s' % (indent, FormatArg(is_output, arg))
    return s
  if is_server:
    ret_type = 'void'
  else:
    ret_type = 'NaClSrpcError'
  s = '%s %s%s(\n' % (ret_type, class_name, rpc['name'])
  # Until SRPC uses RPC/Closure on the client side, these must be different.
  if is_server:
    s += '    %sNaClSrpcRpc* rpc,\n' % indent
    s += '    %sNaClSrpcClosure* done' % indent
  else:
    s += '    %sNaClSrpcChannel* channel' % indent
  s += '%s' % FormatArgs(False, rpc['inputs'])
  s += '%s' % FormatArgs(True, rpc['outputs'])
  s += ')'
  return  s


def PrintHeaderFile(output, is_server, guard_name, interface_name, specs):
  """Prints out the header file containing the prototypes for the RPCs."""
  print >>output, AUTOGEN_COMMENT
  s = HEADER_START.replace('xyz', guard_name)
  # iterate over all the specified interfaces
  if is_server:
    suffix = 'Server'
  else:
    suffix = 'Client'
  for spec in specs:
    class_name = spec['name'] + suffix
    rpcs = spec['rpcs']
    s += 'class %s {\n public:\n' % class_name
    for rpc in rpcs:
      s += '  static %s;\n' % FormatRpcPrototype(is_server, '', '  ', rpc)
    s += '\n private:\n  %s();\n' % class_name
    s += '  %s(const %s&);\n' % (class_name, class_name)
    s += '  void operator=(const %s);\n' % class_name
    s += '};  // class %s\n\n' % class_name
  if is_server:
    s += 'class %s {\n' % interface_name
    s += ' public:\n'
    s += '  static NaClSrpcHandlerDesc srpc_methods[];\n'
    s += '};  // class %s' % interface_name
  s += HEADER_END.replace('xyz', guard_name)
  print >>output, s


def PrintServerFile(output, include_name, interface_name, specs):
  """Print the server (stub) .cc file."""

  def FormatDispatchPrototype(indent, rpc):
    """Format the prototype of a dispatcher method."""
    s = '%sstatic void %sDispatcher(\n' % (indent, rpc['name'])
    s += '%s    NaClSrpcRpc* rpc,\n' % indent
    s += '%s    NaClSrpcArg** inputs,\n' % indent
    s += '%s    NaClSrpcArg** outputs,\n' % indent
    s += '%s    NaClSrpcClosure* done\n' % indent
    s += '%s)' % indent
    return s

  def FormatMethodString(rpc):
    """Format the SRPC text string for a single rpc method."""

    def FormatTypes(args):
      s = ''
      for arg in args:
        s += types[arg[1]][0]
      return s
    s = '  { "%s:%s:%s", %sDispatcher },\n' % (rpc['name'],
                                               FormatTypes(rpc['inputs']),
                                               FormatTypes(rpc['outputs']),
                                               rpc['name'])
    return s

  def FormatCall(class_name, indent, rpc):
    """Format a call from a dispatcher method to its stub."""

    def FormatArgs(is_output, args):
      """Format the arguments passed to the stub."""

      def FormatArg(is_output, num, arg):
        """Format an argument passed to a stub."""
        if is_output:
          prefix = 'outputs[' + str(num) + ']->'
          addr_prefix = '&('
          addr_suffix = ')'
        else:
          prefix = 'inputs[' + str(num) + ']->'
          addr_prefix = ''
          addr_suffix = ''
        type_info = types[arg[1]]
        if type_info[3]:
          s = '%s%s%s%s, %s%s' % (addr_prefix,
                                  prefix,
                                  type_info[3],
                                  addr_suffix,
                                  prefix,
                                  type_info[2])
        else:
          s = '%s%s%s%s' % (addr_prefix, prefix, type_info[2], addr_suffix)
        return s
      s = ''
      num = 0
      for arg in args:
        s += ',\n%s    %s' % (indent, FormatArg(is_output, num, arg))
        num += 1
      return s
    s = '%s::%s(\n%s    rpc,\n' % (class_name, rpc['name'], indent)
    s += '%s    done' % indent
    s += FormatArgs(False, rpc['inputs'])
    s += FormatArgs(True, rpc['outputs'])
    s += '\n%s)' % indent
    return s
  print >>output, AUTOGEN_COMMENT
  s = 'namespace {\n\n'
  for spec in specs:
    class_name = spec['name'] + 'Server'
    rpcs = spec['rpcs']
    for rpc in rpcs:
      s += '%s {\n' % FormatDispatchPrototype('', rpc)
      if rpc['inputs'] == []:
        s += '  UNREFERENCED_PARAMETER(inputs);\n'
      if rpc['outputs'] == []:
        s += '  UNREFERENCED_PARAMETER(outputs);\n'
      s += '  %s;\n' % FormatCall(class_name, '  ', rpc)
      s += '}\n\n'
  s += '}  // namespace\n\n'
  s += 'NaClSrpcHandlerDesc %s::srpc_methods[] = {\n' % interface_name
  for spec in specs:
    class_name = spec['name'] + 'Server'
    rpcs = spec['rpcs']
    for rpc in rpcs:
      s += FormatMethodString(rpc)
  s += '  { NULL, NULL }\n};\n'
  print >>output, SOURCE_FILE_INCLUDES.replace('xyz', include_name)
  print >>output, s


def PrintClientFile(output, include_name, specs):
  """Prints the client (proxy) .cc file."""

  def FormatCall(rpc):
    """Format a call to the generic dispatcher, NaClSrpcInvokeBySignature."""

    def FormatTypes(args):
      """Format a the type signature string for either inputs or outputs."""
      s = ''
      for arg in args:
        s += types[arg[1]][0]
      return s
    def FormatArgs(args):
      """Format the arguments for the call to the generic dispatcher."""

      def FormatArg(arg):
        """Format a single argument for the call to the generic dispatcher."""
        s = ''
        type_info = types[arg[1]]
        if type_info[3]:
          s += '%s, ' % CountName(arg[0])
        s += arg[0]
        return s
      s = ''
      for arg in args:
        s += ',\n      %s' % FormatArg(arg)
      return s
    s = '(\n      channel,\n      "%s:%s:%s"' % (rpc['name'],
                                                 FormatTypes(rpc['inputs']),
                                                 FormatTypes(rpc['outputs']))
    s += FormatArgs(rpc['inputs'])
    s += FormatArgs(rpc['outputs']) + '\n  )'
    return s
  print >>output, AUTOGEN_COMMENT
  print >>output, SOURCE_FILE_INCLUDES.replace('xyz', include_name)
  s = ''
  for spec in specs:
    class_name = spec['name'] + 'Client'
    rpcs = spec['rpcs']
    for rpc in rpcs:
      s += '%s  {\n' % FormatRpcPrototype('', class_name + '::', '', rpc)
      s += '  NaClSrpcError retval;\n'
      s += '  retval = NaClSrpcInvokeBySignature%s;\n' % FormatCall(rpc)
      s += '  return retval;\n'
      s += '}\n\n'
  print >>output, s

def MakePath(name):
  paths = name.split(os.sep)
  path = os.sep.join(paths[:-1])
  try:
    os.makedirs(path)
  except OSError:
    return


def main(argv):
  usage = 'Usage: srpcgen.py <-c | -s> [--include=<name>] [--ppapi]'
  usage = usage + ' <iname> <gname> <.h> <.cc> <specs>'

  mode = None
  ppapi = False
  try:
    long_opts = ['include=', 'ppapi']
    opts, pargs = getopt.getopt(argv[1:], 'cs', long_opts)
  except getopt.error, e:
    print >>sys.stderr, 'Illegal option:', str(e)
    print >>sys.stderr, usage
    return 1

  # Get the class name for the interface.
  interface_name = pargs[0]
  # Get the name for the token used as a multiple inclusion guard in the header.
  include_guard_name = pargs[1]
  # Get the name of the header file to be generated.
  h_file_name = pargs[2]
  MakePath(h_file_name)
  h_file = open(h_file_name, 'w')
  # Get the name of the source file to be generated.  Depending upon whether
  # -c or -s is generated, this file contains either client or server methods.
  cc_file_name = pargs[3]
  MakePath(cc_file_name)
  cc_file = open(cc_file_name, 'w')
  # The remaining arguments are the spec files to be compiled.
  spec_files = pargs[4:]

  for opt, val in opts:
    if opt == '-c':
      mode = 'client'
    elif opt == '-s':
      mode = 'server'
    elif opt == '--include':
      h_file_name = val
    elif opt == '--ppapi':
      ppapi = True

  if ppapi:
    AddHeader("ppapi/c/pp_instance.h")
    AddHeader("ppapi/c/pp_module.h")
    AddHeader("ppapi/c/pp_resource.h")

  # Convert to forward slash paths if needed
  h_file_name = "/".join(h_file_name.split("\\"))

  # Verify we picked server or client mode
  if not mode:
    print >>sys.stderr, 'Neither -c nor -s specified'
    usage()
    return 1

  # Combine the rpc specs from spec_files into rpcs.
  specs = []
  for spec_file in spec_files:
    code_obj = compile(open(spec_file, 'r').read(), 'file', 'eval')
    specs.append(eval(code_obj))
  # Print out the requested files.
  if mode == 'client':
    PrintHeaderFile(h_file, False, include_guard_name, interface_name, specs)
    PrintClientFile(cc_file, h_file_name, specs)
  elif mode == 'server':
    PrintHeaderFile(h_file, True, include_guard_name, interface_name, specs)
    PrintServerFile(cc_file, h_file_name, interface_name, specs)

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv))
