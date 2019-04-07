# Copyright (c) 2014 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function

from conditions import *

"""Nodes for scons to gn

Nodes class provides a means to store, explore and write out a tree
describing the table generated by interating over the scons file.  The
tree is created in the form of:

  TopNode
    Condition
      Object
        Condition
          Property

"""


REMAP = {
  'arm' : 'arm',
  'AND' : 'android',
  'CHR' : 'chrome',
  'LIN' : 'linux',
  'MAC' : 'mac',
  'IOS' : 'ios',
  'WIN' : 'win',
  'x86' : 'x86',
  'x64' : 'x64',
  'glibc' : 'glibc',
  'newlib' : 'newlib',
  'pnacl' : 'pnacl'
}

def Express(use, avail, tag):
  use = sorted([REMAP[x] for x in use])
  avail = sorted([REMAP[x] for x in avail])
  if use == avail:
    return None

  if len(use) == 1:
    return '(%s == "%s")' % (tag, use[0])

  luse = len(use)
  lavail = len(avail)
  if luse > (lavail - luse):
    items = ['%s != "%s"' % (tag, i) for i in avail if i not in use]
    return '(' + ' || '.join(items) + ')'

  items = ['%s == "%s"' % (tag, i) for i in use]
  return '(' + ' || '.join(items) + ')'


def Condition(os_use, os_avail, cpu_use, cpu_avail):
  # We can not hash a list, so we sometimes use  a space sperated string.
  if isinstance(cpu_use, str):
    cpu_use = cpu_use.split(' ')
  if isinstance(cpu_avail, str):
    cpu_avail = cpu_avail.split(' ')

  o_cond = Express(os_use, os_avail, 'os')
  c_cond = Express(cpu_use, cpu_avail, 'cpu_arch')

  if not o_cond and not c_cond:
    return None

  if o_cond and c_cond:
    return 'if (%s && %s) {' % (o_cond, c_cond)

  if o_cond:
    return 'if %s {' % o_cond
  return 'if %s {' % c_cond

#
# TopNode
#   Condition
#     Object
#       Condition
#         Property
#
class Node(object):
  def __init__(self, name=''):
    self.children = []
    self.name = name
    self.parent = None

  def DumpInfo(self, depth=0):
    print('%s%s(%s)' % ('  ' * depth, str(type(self)), self.name))
    for child in self.children:
      child.DumpInfo(depth+1)

  def Write(self, fileobj, depth, text):
    for line in text.split('\n'):
      string = '  ' * depth + line + '\n'
      fileobj.write(string)

  def Dump(self, fileobj, depth):
    adjust = self.DumpStart(fileobj, depth)
    for idx, child in enumerate(self.children):
      self.DumpChild(fileobj, child, adjust)
      if idx != len(self.children) - 1:
        fileobj.write('\n')
    self.DumpEnd(fileobj, depth)

  def DumpStart(self, fileobj, depth):
    self.Write(fileobj, depth, self.name)
    return depth

  def DumpEnd(self, fileobj, depth):
    pass

  def DumpChild(self, fileobj, child, depth):
    child.Dump(fileobj, depth)

  def AddChild(self, child):
    self.children.append(child)
    child.parent = self

  def Examine(self, obj):
    obj.Enter(self)
    for child in self.children:
      child.Examine(obj)
    obj.Exit(self)


class TopNode(Node):
  def __init__(self, name):
    Node.__init__(self, name)

  def DumpStart(self, fileobj, depth):
    self.Write(fileobj, depth, "#  Autogenerated from %s.\n\n" % self.name)
    return depth


class ConditionNode(Node):
  def __init__(self, os_use, os_avail, cpu_use, cpu_avail):
    name = Condition(os_use, os_avail, cpu_use, cpu_avail)
    Node.__init__(self, name)

  def Dump(self, fileobj, depth):
    if self.name:
      self.Write(fileobj, depth, self.name)
      depth += 1
    for child in self.children:
      child.Dump(fileobj, depth)
    if self.name:
      self.Write(fileobj, depth - 1, '}')


class ObjectNode(Node):
  def __init__(self, name, obj_type):
    Node.__init__(self, name)
    self.obj_type = obj_type
    self.conditional = set()
    self.unconditional = set()

  def DumpStart(self, fileobj, depth):
    self.Write(fileobj, depth, '%s("%s") {' % (self.obj_type, self.name))
    depth += 1

    # For every conditional only property, set and empty array
    for cond in self.conditional:
      if cond not in self.unconditional:
        self.Write(fileobj, depth, '%s = []' % cond)
    return depth

  def DumpEnd(self, fileobj, depth):
    self.Write(fileobj, depth, '}')


class PropertyNode(Node):
  def __init__(self, name):
    Node.__init__(self, name)

  def Dump(self, fileobj, depth):
    if self.parent.name:
      self.Write(fileobj, depth, '%s += [' % self.name)
    else:
      self.Write(fileobj, depth, '%s = [' % self.name)

    for child in self.children:
      child.Dump(fileobj, depth + 1)

    self.Write(fileobj, depth, ']')


class ValueNode(Node):
  def __init__(self, name):
    Node.__init__(self,  name)

  def Dump(self, fileobj, depth):
    self.Write(fileobj, depth, '"%s",' % self.name)


class OrganizeProperties(object):
  def __init__(self):
    self.cond = None
    self.obj = None
    pass

  def Enter(self, node):
    if isinstance(node, ObjectNode):
      self.obj = node

    if isinstance(node, ConditionNode):
      self.cond = node.name

    if isinstance(node, PropertyNode):
      if self.cond == None:
        self.obj.unconditional |= set([node.name])
      else:
        self.obj.conditional |= set([node.name])
      node.children = sorted(node.children, key=lambda x: x.name)

  def Exit(self, node):
    pass
