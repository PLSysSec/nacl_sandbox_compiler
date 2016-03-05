# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../../../build/common.gypi',
  ],
  'variables': {
    'sources_for_standard_interfaces': [
      '_exit.c',
      'abort.c',
      'access.c',
      'chdir.c',
      'chmod.c',
      'clock.c',
      'clock_getres.c',
      'clock_gettime.c',
      'close.c',
      'dup.c',
      'eaccess.c',
      'fchdir.c',
      'fchmod.c',
      'fdatasync.c',
      'fstat.c',
      'fsync.c',
      'ftruncate.c',
      'getcwd.c',
      'getcwd_without_malloc.c',
      'getdents.c',
      'gethostname.c',
      'getpagesize.c',
      'getpid.c',
      'gettimeofday.c',
      'htonl.c',
      'htons.c',
      'isatty.c',
      'link.c',
      'lock.c',
      'lseek.c',
      'lstat.c',
      'malloc.c',
      'mkdir.c',
      'mmap.c',
      'mprotect.c',
      'munmap.c',
      'nacl_add_tp.c',
      'nacl_ext_supply.c',
      'nacl_interface_query.c',
      'nacl_irt_fdio.c',
      'nacl_irt_filename.c',
      'nacl_read_tp.c',
      'nanosleep.c',
      'ntohl.c',
      'ntohs.c',
      'open.c',
      'pread.c',
      'pwrite.c',
      'pthread_initialize_minimal.c',
      'pthread_stubs.c',
      'random.c',
      'read.c',
      'readlink.c',
      'rename.c',
      'rmdir.c',
      'sbrk.c',
      'sched_yield.c',
      'sigblock.c',
      'siggetmask.c',
      'sigmask.c',
      'sigprocmask.c',
      'sigsetmask.c',
      'srandom.c',
      'stacktrace.c',
      'start.c',
      'stat.c',
      'symlink.c',
      'sysconf.c',
      'tls.c',
      'truncate.c',
      'uname.c',
      'unlink.c',
      'utime.c',
      'utimes.c',
      'write.c',
    ],
    'sources_for_stubs': [
      'stubs/_execve.c',
      'stubs/accept.c',
      'stubs/addmntent.c',
      'stubs/bind.c',
      'stubs/chown.c',
      'stubs/closelog.c',
      'stubs/connect.c',
      'stubs/endgrent.c',
      'stubs/endmntent.c',
      'stubs/endpwent.c',
      'stubs/environ.c',
      'stubs/execvpe.c',
      'stubs/fchown.c',
      'stubs/fcntl.c',
      'stubs/fork.c',
      'stubs/freeaddrinfo.c',
      'stubs/fstatvfs.c',
      'stubs/gai_strerror.c',
      'stubs/get_current_dir_name.c',
      'stubs/getaddrinfo.c',
      'stubs/getdtablesize.c',
      'stubs/getegid.c',
      'stubs/geteuid.c',
      'stubs/getgid.c',
      'stubs/getgrent.c',
      'stubs/getgrgid.c',
      'stubs/getgrgid_r.c',
      'stubs/getgrnam.c',
      'stubs/getgroups.c',
      'stubs/gethostbyaddr.c',
      'stubs/gethostbyname.c',
      'stubs/getlogin.c',
      'stubs/getmntent.c',
      'stubs/getnameinfo.c',
      'stubs/getpeername.c',
      'stubs/getpgrp.c',
      'stubs/getppid.c',
      'stubs/getpwent.c',
      'stubs/getpwnam.c',
      'stubs/getpwnam_r.c',
      'stubs/getpwuid.c',
      'stubs/getpwuid_r.c',
      'stubs/getrlimit.c',
      'stubs/getrusage.c',
      'stubs/getservbyname.c',
      'stubs/getservbyport.c',
      'stubs/getsockname.c',
      'stubs/getsockopt.c',
      'stubs/getuid.c',
      'stubs/getwd.c',
      'stubs/hasmntopt.c',
      'stubs/if_freenameindex.c',
      'stubs/if_indextoname.c',
      'stubs/if_nameindex.c',
      'stubs/if_nametoindex.c',
      'stubs/inet_ntoa.c',
      'stubs/inet_ntop.c',
      'stubs/initgroups.c',
      'stubs/ioctl.c',
      'stubs/issetugid.c',
      'stubs/kill.c',
      'stubs/lchown.c',
      'stubs/listen.c',
      'stubs/llseek.c',
      'stubs/major.c',
      'stubs/makedev.c',
      'stubs/minor.c',
      'stubs/mkfifo.c',
      'stubs/mknod.c',
      'stubs/msync.c',
      'stubs/openlog.c',
      'stubs/pipe.c',
      'stubs/poll.c',
      'stubs/pselect.c',
      'stubs/pthread_sigmask.c',
      'stubs/readv.c',
      'stubs/recv.c',
      'stubs/recvfrom.c',
      'stubs/recvmsg.c',
      'stubs/sched_get_priority_max.c',
      'stubs/sched_get_priority_min.c',
      'stubs/sched_setparam.c',
      'stubs/sched_setscheduler.c',
      'stubs/select.c',
      'stubs/send.c',
      'stubs/sendmsg.c',
      'stubs/sendto.c',
      'stubs/setegid.c',
      'stubs/seteuid.c',
      'stubs/setgid.c',
      'stubs/setgrent.c',
      'stubs/setgroups.c',
      'stubs/setmntent.c',
      'stubs/setpgid.c',
      'stubs/setpwent.c',
      'stubs/setrlimit.c',
      'stubs/setsid.c',
      'stubs/setsockopt.c',
      'stubs/settimeofday.c',
      'stubs/setuid.c',
      'stubs/shutdown.c',
      'stubs/sigaction.c',
      'stubs/signal.c',
      'stubs/sigsuspend.c',
      'stubs/sigvec.c',
      'stubs/socket.c',
      'stubs/socketpair.c',
      'stubs/statvfs.c',
      'stubs/syslog.c',
      'stubs/tcdrain.c',
      'stubs/tcflow.c',
      'stubs/tcflush.c',
      'stubs/tcgetattr.c',
      'stubs/tcsendbreak.c',
      'stubs/tcsetattr.c',
      'stubs/times.c',
      'stubs/ttyname.c',
      'stubs/ttyname_r.c',
      'stubs/umask.c',
      'stubs/vfork.c',
      'stubs/wait.c',
      'stubs/waitpid.c',
    ],
    'sources_for_nacl_extensions': [
      'gc_hooks.c',
      'nacl_irt.c',
      'nacl_irt_init.c',
      'nacl_random.c',
      'nacl_tls_get.c',
      'nacl_tls_init.c',
    ],
    'imc_syscalls': [
      'imc_accept.c',
      'imc_connect.c',
      'imc_makeboundsock.c',
      'imc_mem_obj_create.c',
      'imc_recvmsg.c',
      'imc_sendmsg.c',
      'imc_socketpair.c',
    ],
  },

  'targets' : [
    {
      'target_name': 'nacl_lib',
      'type': 'none',
      'dependencies': [
        'nacl_lib_newlib',
        'nacl_lib_glibc'
      ],
    },
    {
      'target_name': 'nacl_lib_glibc',
      'type': 'none',
      'variables': {
        'nlib_target': 'libnacl.a',
        'build_glibc': 1,
        'build_newlib': 0,
      },
      'sources': ['<@(sources_for_nacl_extensions)'],
    },
    {
      'target_name': 'nacl_lib_newlib',
      'type': 'none',
      'variables': {
        'nlib_target': 'libnacl.a',
        'build_glibc': 0,
        'build_newlib': 1,
        'build_irt': 1,
        'build_pnacl_newlib': 1,
        'build_nonsfi_helper': 1,
      },
      'sources': [
        '<@(sources_for_nacl_extensions)',
        '<@(sources_for_standard_interfaces)',
        '<@(sources_for_stubs)',
      ],
      'conditions': [
        ['target_arch=="arm"', {
          'variables': {
            'native_sources': [
              'aeabi_read_tp.S'
            ]
          }
        }],
      ],
    },
    {
      'target_name': 'nacl_dynacode_lib',
      'type': 'none',
      'variables': {
        'nlib_target': 'libnacl_dyncode.a',
        'nso_target': 'libnacl_dyncode.so',
        'build_glibc': 1,
        'build_newlib': 1,
        'build_pnacl_newlib': 1,
      },
      'sources': ['dyncode.c'],
    },
    {
      'target_name': 'nacl_dyncode_private_lib',
      'type': 'none',
      'variables': {
        'nlib_target': 'libnacl_dyncode_private.a',
        'build_glibc': 0,
        'build_newlib': 1,
        'build_pnacl_newlib': 1,
      },
      'sources': ['dyncode_private.c'],
    },
    {
      'target_name': 'nacl_exception_lib',
      'type': 'none',
      'variables': {
        'nlib_target': 'libnacl_exception.a',
        'nso_target': 'libnacl_exception.so',
        'build_glibc': 1,
        'build_newlib': 1,
        'build_pnacl_newlib': 1,
      },
      'sources': ['nacl_exception.c'],
    },
    {
      'target_name': 'nacl_exception_private_lib',
      'type': 'none',
      'variables': {
        'nlib_target': 'libnacl_exception_private.a',
        'build_glibc': 1,
        'build_newlib': 1,
        'build_pnacl_newlib': 1,
      },
      'sources': ['nacl_exception_private.c'],
    },
    {
      'target_name': 'nacl_list_mappings_lib',
      'type': 'none',
      'variables': {
        'nlib_target': 'libnacl_list_mappings.a',
        'nso_target': 'libnacl_list_mappings.so',
        'build_glibc': 1,
        'build_newlib': 1,
        'build_pnacl_newlib': 1,
      },
      'sources': ['list_mappings.c'],
    },
    {
      'target_name': 'nacl_list_mappings_private_lib',
      'type': 'none',
      'variables': {
        'nlib_target': 'libnacl_list_mappings_private.a',
        'build_glibc': 0,
        'build_newlib': 1,
      },
      'sources': ['list_mappings_private.c'],
    },
    {
      'target_name': 'imc_syscalls_lib',
      'type': 'none',
      'variables': {
        'nlib_target': 'libimc_syscalls.a',
        'nso_target': 'libimc_syscalls.so',
        'build_glibc': 1,
        'build_newlib': 1,
        'build_pnacl_newlib': 1,
        'build_irt': 1,
      },
      'sources': ['<@(imc_syscalls)'],
    },
  ],
}
