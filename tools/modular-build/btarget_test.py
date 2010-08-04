
# Copyright 2010 The Native Client Authors.  All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

import StringIO
import os
import shutil
import subprocess
import unittest

from dirtree_test import TempDirTestCase
import dirtree
import dirtree_test
import btarget


def PlanToString(targets):
  stream = StringIO.StringIO()
  btarget.PrintPlan(targets, stream)
  return stream.getvalue()


def MapSnapshotToContents(tree):
  if isinstance(tree, dirtree.FileSnapshot):
    return tree.GetContents()
  else:
    return dict((key, MapSnapshotToContents(value))
                 for key, value in tree.iteritems())


class BuildTargetTests(TempDirTestCase):

  def test_src(self):
    tempdir = self.MakeTempDir()
    src = btarget.SourceTarget("src", os.path.join(tempdir, "src"),
                              dirtree.CopyTree("example"))
    self.assertEquals(PlanToString([src]), "src: yes\n")
    btarget.Rebuild([src], open(os.devnull, "w"))
    self.assertEquals(PlanToString([src]), "src: no\n")

  def test_build(self):
    tempdir = self.MakeTempDir()
    src = btarget.SourceTarget("src", os.path.join(tempdir, "src"),
                              dirtree.CopyTree("example"))
    input_prefix = btarget.UnionDir("input",
                                    os.path.join(tempdir, "prefix"),
                                    [])
    install = btarget.AutoconfModule("bld",
                                     os.path.join(tempdir, "install"),
                                     os.path.join(tempdir, "build"),
                                     input_prefix,
                                     src)
    self.assertEquals(PlanToString([install]),
                      "input: yes\nsrc: yes\nbld: yes\n")
    btarget.Rebuild([install], open(os.devnull, "w"))
    self.assertEquals(PlanToString([install]),
                      "input: no\nsrc: no\nbld: no\n")

    proc = subprocess.Popen(
        [os.path.join(tempdir, "install/bin/hellow")],
        stdout=subprocess.PIPE)
    stdout = proc.communicate()[0]
    self.assertEquals(proc.wait(), 0)
    self.assertEquals(stdout, "Hello world\n")

    # Check that input/prefix gets rebuilt if it is manually
    # deleted.
    shutil.rmtree(os.path.join(tempdir, "prefix"))
    os.unlink(os.path.join(tempdir, "prefix.state"))
    os.unlink(os.path.join(tempdir, "prefix.state.log"))
    self.assertEquals(PlanToString([install]),
                      "input: yes\nsrc: no\nbld: maybe\n")
    btarget.Rebuild([install], open(os.devnull, "w"))
    # Try again to test idempotence of install step.
    install.DoBuild()

  def test_building_specific_targets(self):
    tempdir = self.MakeTempDir()
    src = btarget.SourceTarget("src", os.path.join(tempdir, "src"),
                              dirtree.CopyTree("example"))
    input_prefix = btarget.UnionDir("input",
                                    os.path.join(tempdir, "prefix"),
                                    [])
    install = btarget.AutoconfModule("bld",
                                     os.path.join(tempdir, "install"),
                                     os.path.join(tempdir, "build"),
                                     input_prefix,
                                     src)
    root_targets = [install]

    self.assertEquals(btarget.SubsetTargets(root_targets, ["input"]),
                      [input_prefix])
    self.assertEquals(btarget.SubsetTargets(root_targets, ["src"]),
                      [src])
    # It should work if there are multiple paths to a target.
    self.assertEquals(btarget.SubsetTargets(root_targets * 2, ["src"]),
                      [src])

    # The default is to build all targets.
    stream = StringIO.StringIO()
    btarget.BuildMain(root_targets, ["-b"], stream)
    self.assertEquals(stream.getvalue(),
                      "input: yes\nsrc: yes\nbld: yes\n"
                      "** building input\n** building src\n** building bld\n")
    # But we can also list target names to build.
    stream = StringIO.StringIO()
    btarget.BuildMain(root_targets, ["-b", "input"], stream)
    self.assertEquals(stream.getvalue(),
                      "input: no\n"
                      "** skipping input\n")

  def test_install_root(self):
    # Test package that supports "make install install_root=DIR"
    # but not the usual "make install DESTDIR=DIR".
    tempdir = self.MakeTempDir()
    src = btarget.SourceTarget("src", os.path.join(tempdir, "src"),
                              dirtree.CopyTree("example"))
    input_prefix = btarget.UnionDir("input",
                                    os.path.join(tempdir, "prefix"),
                                    [])
    install = btarget.AutoconfModule("bld",
                                     os.path.join(tempdir, "install"),
                                     os.path.join(tempdir, "build"),
                                     input_prefix,
                                     src, use_install_root=True)
    btarget.Rebuild([src], open(os.devnull, "w"))
    # Override a file.
    dirtree.WriteFile(
        os.path.join(tempdir, "src", "Makefile.in"),
        dirtree.ReadFile(
            os.path.join(tempdir, "src", "Makefile.in_install_root")))
    btarget.Rebuild([install], open(os.devnull, "w"))
    assert os.path.exists(os.path.join(tempdir, "install/bin/hellow"))
    # Try again to test idempotence of install step.
    install.DoBuild()
    assert os.path.exists(os.path.join(tempdir, "install/bin/hellow"))

  def test_union_dirs(self):
    Dir = dirtree_test.Dir
    File = dirtree_test.File

    tree1 = Dir([("subdir_foo", Dir([("myfile", File("my file"))]))])
    tree2 = Dir([("subdir_bar", Dir([("urfile", File("another file"))]))])

    tempdir = self.MakeTempDir()
    dir1 = btarget.SourceTarget("dir1", os.path.join(tempdir, "dir1"), tree1)
    dir2 = btarget.SourceTarget("dir2", os.path.join(tempdir, "dir2"), tree2)
    dir3 = btarget.UnionDir("dir3", os.path.join(tempdir, "dir3"),
                            [dir1, dir2])
    btarget.Rebuild([dir3], open(os.devnull, "w"))
    assert os.path.exists(os.path.join(dir3.dest_path,
                                       "subdir_foo", "myfile"))
    assert os.path.exists(os.path.join(dir3.dest_path,
                                       "subdir_bar", "urfile"))

  def test_tree_hash_function(self):
    # Check that the hash uses file and directory contents, not inode
    # numbers and timestamps.
    def GetHash():
      tempdir = self.MakeTempDir()
      os.mkdir(os.path.join(tempdir, "subdir"))
      dirtree.WriteFile(os.path.join(tempdir, "subdir", "myfile"), "Contents")
      dirtree.WriteFile(os.path.join(tempdir, "subdir", "script"), "echo foo")
      subprocess.check_call(["chmod", "+x",
                             os.path.join(tempdir, "subdir", "script")])
      os.symlink("symlink_dest", os.path.join(tempdir, "symlink"))
      self.assertEquals(
          list(btarget.ListFiles(tempdir)),
          [('subdir', 'dir'),
           ('subdir/myfile', 'file',
            'f5cbdf6bfb51439be085b5c6b7460a7c91eabc3c', False),
           ('subdir/script', 'file',
            '9f168d2f8df57c83626cf6026658c6adba47c759', True),
           ('symlink', 'symlink', 'symlink_dest')])
      return btarget.HashTree(tempdir)

    self.assertEquals(GetHash(), GetHash())

  def test_tree_mapper(self):
    Dir = dirtree_test.Dir
    File = dirtree_test.File

    tree1 = Dir([("subdir_foo", Dir([("myfile", File("my file"))]))])
    tree2 = Dir([("subdir_bar", Dir([("urfile", File("another file"))]))])

    tempdir = self.MakeTempDir()
    dir_in1 = btarget.SourceTarget(
        "dir_in1", os.path.join(tempdir, "dir1"), tree1)
    dir_in2 = btarget.SourceTarget(
        "dir_in2", os.path.join(tempdir, "dir2"), tree2)

    def MapperFunc(input1, input2):
      return {"new_subdir": input1,
              "urfile": input2["subdir_bar"]["urfile"]}
    MapperFunc.function_identity = "12345"

    dir_out = btarget.TreeMapper("dir_out", os.path.join(tempdir, "dir_out"),
                                 MapperFunc, [dir_in1, dir_in2])
    btarget.Rebuild([dir_out], open(os.devnull, "w"))
    self.assertEquals(
        MapSnapshotToContents(dirtree.MakeSnapshotFromPath(dir_out.dest_path)),
        {"new_subdir": {"subdir_foo": {"myfile": "my file"}},
         "urfile": "another file"})

  def test_tree_mapper_changes(self):
    def MapperFunc1():
      return {"some_file": dirtree.FileSnapshotInMemory("Contents 1")}
    MapperFunc1.function_identity = "AAAA"

    def MapperFunc2():
      return {"some_file": dirtree.FileSnapshotInMemory("Contents 2")}
    MapperFunc2.function_identity = "BBBB"

    tempdir = self.MakeTempDir()
    target1 = btarget.TreeMapper("dir_out", os.path.join(tempdir, "dir_out"),
                                 MapperFunc1, [])
    btarget.Rebuild([target1], open(os.devnull, "w"))

    target2 = btarget.TreeMapper("dir_out", os.path.join(tempdir, "dir_out"),
                                 MapperFunc2, [])
    self.assertEquals(target2.NeedsBuild(), True)


if __name__ == "__main__":
  unittest.main()
