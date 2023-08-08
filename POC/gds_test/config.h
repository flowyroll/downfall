#define ADDRESS_NC 0x5678ff0000000000

// Prepare
#define P_PAGE_FAULT    0
#define P_CACHE_MISS    1

// Fault/assist mode
#define FAULT_KERNEL    0
#define FAULT_MPK       0
#define FAULT_PF        1
#define FAULT_NC        0
#define ASSIST_UC       0
#define ASSIST_A        0
#define ASSIST_D        0
#define ASSIST_WC       0
#define SPECULATIVE     0

#define LOAD_COUNT          1

#define BYTE_TRY            8
        

// Victim instruction
#define VICTIM_VMOVUPS      1
#define VICTIM_VMOVNT       0
#define VICTIM_REPMOV       0
#define VICTIM_REGISTERS    0
#define VICTIM_XRSTOR       0
#define VICTIM_FXRSTOR      0
#define VICTIM_CPUID        0
#define VICTIM_WRITEIO      0
#define VICTIM_READIO       0
#define VICTIM_VGATHER      0
#define VICTIM_TRY_NEW      0
#define VICTIM_VCOMPRESS    0
#define VICTIM_AESNI        0
#define VICTIM_MASKMOV      0

// CPU Configuration
#define CPU_VICTIM          1
#define CPU_ATTACKER        5
#define SINGLE_THREAD       0

// To avoid leaking printf related noise
#define SILENT_MODE         0

// AVX-512
#define AVX_512             1

