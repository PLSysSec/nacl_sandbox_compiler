#include "../../include/nacl_asm.h"

#if defined(_M_X64) || defined(__x86_64__)

DEFINE_GLOBAL_HIDDEN_FUNCTION(exitFunctionWrapper):
    # Save the x64 uint64_t and float/double return
    mov    %rax,%rsi
    movq   %xmm0,%rcx
    # Move the top 32 bits of both values into separate registers
    mov    %rsi,%rdx
    shr    $0x20,%rdx
    mov    %rcx,%r8
    shr    $0x20,%r8
    # Call MakeNaClSysCall_exit_sandbox with
    # exitLocation (rdi) = 1
    # uint32_t register_ret_bottom (rsi)
    # uint32_t register_ret_top (rdx)
    # uint32_t register_float_ret_bottom (rcx)
    # uint32_t register_float_ret_top (r8)
    mov    $0x1,%edi
    call MakeNaClSysCall_exit_sandbox@PLT

#elif defined(_M_IX86) || defined(__i386__)

DEFINE_GLOBAL_HIDDEN_FUNCTION(exitFunctionWrapper):
    push   %esi
    sub    $0x28,%esp
    # Save the x32 uint64_t and float/double return
    mov    %eax,%eax
    mov    %edx,%ecx
    fstl   0x20(%esp)
    mov    0x20(%esp),%edx
    mov    0x24(%esp),%esi
    mov    %esi,0x10(%esp)
    mov    %edx,0xc(%esp)
    mov    %ecx,0x8(%esp)
    mov    %eax,0x4(%esp)
    movl   $0x1,(%esp)
    # Call MakeNaClSysCall_exit_sandbox with
    # exitLocation
    # uint32_t register_ret_bottom
    # uint32_t register_ret_top
    # uint32_t register_float_ret_bottom
    # uint32_t register_float_ret_top
    call MakeNaClSysCall_exit_sandbox@PLT


#else
    #error Unknown platform!
#endif