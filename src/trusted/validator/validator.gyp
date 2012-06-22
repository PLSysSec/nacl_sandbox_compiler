# -*- gyp -*-
# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../../../build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'validators',
      'type': 'static_library',
      'sources' : [
        'validator_init.c',
      ],
    },
  ],
  'conditions': [
    ['OS=="win"', {
      'targets': [
        {
          'target_name': 'validators64',
          'type': 'static_library',
          'sources' : [
            'validator_init.c',
          ],
          'variables': {
            'win_target': 'x64',
          },
        },
      ],
    }],
  ],
}
