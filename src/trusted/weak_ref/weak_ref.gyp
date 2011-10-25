# -*- python -*-
# Copyright 2011 (c) The Native Client Authors. All rights reserved.  Use
# of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'includes': [
    '../../../build/common.gypi',
  ],
  'target_defaults': {
    'variables':{
      'target_base': 'none',
    },
    'target_conditions': [
      ['target_base=="weak_ref"', {
        'sources': [
          'weak_ref.cc',
          'weak_ref.h',
          'call_on_main_thread.h',
        ],
        'xcode_settings': {
          'WARNING_CFLAGS': [
            '-Wno-missing-field-initializers'
          ]
        },
      },
    ]],
  },
  'conditions': [
    ['disable_untrusted==0 and target_arch!="arm"', {
      'targets': [
        {
          'target_name': 'weak_ref_lib',
          'type': 'none',
          'variables': {
            'nlib_target': 'libweak_ref.a',
            'build_glibc': 1,
            'build_newlib': 1,
            'sources': ['weak_ref.cc'],
          },
          'dependencies': [
            '<(DEPTH)/native_client/tools.gyp:prep_toolchain',
          ],
        },
      ],
    }],
    ['OS=="linux"', {
      'defines': [
        'XP_UNIX',
      ],
    }],
    ['OS=="mac"', {
      'defines': [
        'XP_MACOSX',
        'XP_UNIX',
        'TARGET_API_MAC_CARBON=1',
        'NO_X11',
        'USE_SYSTEM_CONSOLE',
      ],
    }],
    ['OS=="win"', {
      'defines': [
        'XP_WIN',
        'WIN32',
        '_WINDOWS'
      ],
      'targets': [
        {
          'target_name': 'weak_ref64',
          'type': 'static_library',
          'variables': {
            'target_base': 'weak_ref',
            'win_target': 'x64',
          },
          'dependencies': [
            '<(DEPTH)/native_client/src/shared/platform/platform.gyp:platform64',
          ],
        },
      ],
    }],
  ],
  'targets': [
    {
      'target_name': 'weak_ref',
      'type': 'static_library', # 'dynamic_library', ?!?
      'variables': {
        'target_base': 'weak_ref',
      },
      'dependencies': [
        '<(DEPTH)/native_client/src/shared/platform/platform.gyp:platform',
      ],
    },
  ],
}
