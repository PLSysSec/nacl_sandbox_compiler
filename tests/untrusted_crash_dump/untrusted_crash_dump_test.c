/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "native_client/src/untrusted/crash_dump/untrusted_crash_dump.h"


void CallMe(void (*func)(int), int);


/*
 * Calling through several layers of functions, varying arguments to yield
 * differently sized stack frames.
 */

void layer5(int x, int y) {
  *(volatile int *) x = y;
}

void layer4(int x) {
  layer5(x, 1);
}

void layer3(int a, int b, int c) {
  CallMe(layer4, a + b + c);
}

void layer2(int i, int j) {
  layer3(i, j, 7);
}

void layer1(int s, int t) {
  int *junk = (int*)alloca(sizeof(int)* 1234);
  junk[0] = s + 5;
  layer2(junk[0], t + 1);
}

int main() {
  NaClCrashDumpInit();

  layer1(2, 9);

  return 1;
}
