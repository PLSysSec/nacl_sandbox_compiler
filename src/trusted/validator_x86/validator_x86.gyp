# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# TODO(bradchen): eliminate need for the warning flag removals below
{
  'includes': [
    '../../../build/common.gypi',
  ],
  'target_defaults': {
    'variables': {
      'target_base': 'none',
    },
    'target_conditions': [
      ['target_base=="nccopy"', {
        'sources': [
          'nccopycode.c',
          'nccopycode_stores.S',
        ],
        'cflags!': [
          '-Wextra',
          '-Wswitch-enum',
          '-Wsign-compare'
        ],
        'xcode_settings': {
          'WARNING_CFLAGS!': [
            '-Wextra',
            '-Wswitch-enum',
            '-Wsign-compare'
          ],
        },
      }],
      ['target_base=="ncval_reg_sfi"', {
        # we depend on ncvalidate build to generate the headers
        'sources': [ 'ncvalidate_iter.c',
                     'ncvalidator_registry.c',
                     'nc_cpu_checks.c',
                     'nc_illegal.c',
                     'nc_jumps.c',
                     'nc_opcode_histogram.c',
                     'nc_protect_base.c',
                     'nc_memory_protect.c',
                     'ncvalidate_utils.c',
         ],
        'cflags!': [
          '-Wextra',
          '-Wswitch-enum',
          '-Wsign-compare'
        ],
        'xcode_settings': {
          'WARNING_CFLAGS!': [
            '-Wextra',
            '-Wswitch-enum',
            '-Wsign-compare'
          ]
        },
      }],
      ['target_base=="ncval_reg_sfi_verbose"', {
        'sources': ['ncvalidate_iter_verbose.c'],
        'cflags!': [
          '-Wextra',
          '-Wswitch-enum',
          '-Wsign-compare'
        ],
        'xcode_settings': {
          'WARNING_CFLAGS!': [
            '-Wextra',
            '-Wswitch-enum',
            '-Wsign-compare'
          ]
        },
      }],
    ],
  },
  # ----------------------------------------------------------------------
  'conditions': [
    ['target_arch=="ia32"', {
      'targets': [
        {
          'target_name': 'nccopy_x86_32',
          'type': 'static_library',
          'hard_dependency': 1,
          'variables': {
            'target_base': 'nccopy',
          },
        }, {
          'target_name': 'ncval_reg_sfi_x86_32',
          'type': 'static_library',
          'dependencies': [
            'nccopy_x86_32',
            '<(DEPTH)/native_client/src/trusted/validator/x86/validate_x86.gyp:ncval_base_x86_32',
            '<(DEPTH)/native_client/src/trusted/validator/x86/decoder/ncval_x86_decoder.gyp:nc_decoder_x86_32',
           ],
          'variables': {
            'target_base': 'ncval_reg_sfi',
          },
        }, {
          'target_name': 'ncval_reg_sfi_verbose_x86_32',
          'type': 'static_library',
          'dependencies': [
            'ncval_reg_sfi_x86_32',
           ],
          'variables': {
            'target_base': 'ncval_reg_sfi_verbose',
          },
        }],
    }],
    ['OS=="win"', {
      'targets': [
        {
          'target_name': 'nccopy_x86_64',
          'type': 'static_library',
          'hard_dependency': 1,
          'variables': {
            'target_base': 'nccopy',
            'win_target': 'x64',
          },
        }, {
          'target_name': 'ncval_reg_sfi_x86_64',
          'type': 'static_library',
          'dependencies': [
            'nccopy_x86_64',
            '<(DEPTH)/native_client/src/trusted/validator/x86/validate_x86.gyp:ncval_base_x86_64',
            '<(DEPTH)/native_client/src/trusted/validator/x86/decoder/ncval_x86_decoder.gyp:nc_decoder_x86_64',
           ],
          'variables': {
            'target_base': 'ncval_reg_sfi',
            'win_target': 'x64',
          },
        }, {
          'target_name': 'ncval_reg_sfi_verbose_x86_64',
          'type': 'static_library',
          'dependencies': [
            'ncval_reg_sfi_x86_64',
           ],
          'variables': {
            'target_base': 'ncval_reg_sfi_verbose',
            'win_target': 'x64',
          },
        }],
    }],
    ['OS!="win" and target_arch=="x64"', {
      'targets': [
        {
          'target_name': 'nccopy_x86_64',
          'type': 'static_library',
          'hard_dependency': 1,
          'variables': {
            'target_base': 'nccopy',
          },
        }, {
          'target_name': 'ncval_reg_sfi_x86_64',
          'type': 'static_library',
          'dependencies': [
            'nccopy_x86_64',
            '<(DEPTH)/native_client/src/trusted/validator/x86/validate_x86.gyp:ncval_base_x86_64',
            '<(DEPTH)/native_client/src/trusted/validator/x86/decoder/ncval_x86_decoder.gyp:nc_decoder_x86_64',
           ],
          'variables': {
            'target_base': 'ncval_reg_sfi',
          },
        }, {
          'target_name': 'ncval_reg_sfi_verbose_x86_64',
          'type': 'static_library',
          'dependencies': [
            'ncval_reg_sfi_x86_64',
           ],
          'variables': {
            'target_base': 'ncval_reg_sfi_verbose',
          },
        }],
    }],
    [ 'target_arch=="arm"', {
      'targets': []
    }],
  ],
}
