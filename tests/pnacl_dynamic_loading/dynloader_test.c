/*
 * Copyright (c) 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdio.h>

#include "native_client/src/include/nacl_assert.h"
#include "native_client/src/untrusted/pnacl_dynloader/dynloader.h"
#include "native_client/tests/pnacl_dynamic_loading/test_pso.h"


int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: dynloader_test <ELF file>...\n");
    return 1;
  }
  const char *test_dso_file = argv[1];
  const char *data_only_dso_file = argv[2];

  void *pso_root;
  int err;

  printf("Testing %s...\n", test_dso_file);
  err = pnacl_load_elf_file(test_dso_file, &pso_root);
  ASSERT_EQ(err, 0);
  struct test_pso_root *root = pso_root;
  int val = 1000000;
  int result = root->example_func(&val);
  ASSERT_EQ(result, 1001234);
  ASSERT_EQ(*root->get_var(), 2345);
  /* Test that a variable in the BSS is properly zero-initialized. */
  int i;
  for (i = 0; i < BSS_VAR_SIZE; i++)
    ASSERT_EQ(root->bss_var[i], 0);

  /*
   * Each call to pnacl_load_elf_file() should create a fresh instantiation
   * of the PSO/DSO in memory.
   */
  printf("Testing loading DSO a second time...\n");
  void *pso_root2;
  err = pnacl_load_elf_file(test_dso_file, &pso_root2);
  ASSERT_EQ(err, 0);
  struct test_pso_root *root2 = pso_root2;
  ASSERT_NE(pso_root2, pso_root);
  ASSERT_NE(root2->get_var, root->get_var);
  ASSERT_NE(root2->get_var(), root->get_var());
  ASSERT_EQ(*root2->get_var(), 2345);

  printf("Testing %s...\n", data_only_dso_file);
  err = pnacl_load_elf_file(data_only_dso_file, &pso_root);
  ASSERT_EQ(err, 0);
  uint32_t *ptr = pso_root;
  ASSERT_EQ(ptr[0], 0x44332211);
  ASSERT_EQ(ptr[1], 0xDDCCBBAA);

  return 0;
}