#define ADDRESS_NC 0x5678ff0000000000

// Prepare
#define P_PAGE_FAULT 1
#define P_CACHE_MISS 1

// Fault/assist mode
#define FAULT_KERNEL 0
#define FAULT_PF 0
#define FAULT_NC 0
#define ASSIST_UC 1
#define ASSIST_WC 0

// CPU Configuration
#define CPU_VICTIM 1
#define CPU_ATTACKER 5

// AVX-512
#define AVX_512 1

#define KERNEL_GADGET_SAFE 1
#define KERNEL_GADGET_SAFEZ 0
#define KERNEL_GADGET_BUG 0

// 8 or 4
#define BYTES_TO_TRY 4
