
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

#include "lib.h"
#include "config.h"
#include "./cacheutils.h"
#include "../third_party/PTEditor/ptedit_header.h"

#define PAGE_SIZE 4096
#define CACHE_SIZE 64


uint8_t  __attribute__((aligned(PAGE_SIZE))) throttle[8 * 4096];

/* see asm.S */

extern uint64_t CACHE_THRESHOLD;
extern uint8_t * address_normal;
extern uint8_t * address_UC;
extern uint8_t * address_WC;
extern uint8_t * address_US;
extern uint8_t * address_A;
extern uint8_t * address_D;
extern uint8_t * address_MPK;
extern uint8_t * oracles;
extern uint8_t * source;
extern uint8_t * dest;

char * str =
    "ABCDEFGHIJKLMNOP"
    "QRSTUVWXYZabcdef"
    "ghijklmnopqrstuv"
    "wxyz0123456789#!"
    "XXXXXXXXXXXXXXXX"
    "XXXXXXXXXXXXXXXX"
    "XXXXXXXXXXXXXXXX"
    "XXXXXXXXXXXXXXXX"
    "ZZZZZZZZZZZZZZZZ"
    "ZZZZZZZZZZZZZZZZ"
    "ZZZZZZZZZZZZZZZZ"
    "ZZZZZZZZZZZZZZZZ"
;
extern void s_load_encode(uint32_t * perm, uint32_t * index);
extern void victim_asm();



void victim_file() {
  int fd = open("dummy.txt", O_CREAT | O_RDWR, S_IRUSR);
#if VICTIM_WRITEIO 
  while(1){
    lseek(fd, 0, SEEK_SET);
    int r = write(fd, str, strlen(str));
  }
  #else
  uint8_t buffer[CACHE_SIZE];
  while(1){
    int r = read(fd, buffer, CACHE_SIZE);
  }
  #endif
}

void setup_throttle()
{
    for (int i = 0; i < 8; i++) {
        throttle[i * 4096] = 1;
    }
}


#if VICTIM_WRITEIO || VICTIM_READIO
    #define VICTIM_ROUTINE() victim_file();
#else
    #define VICTIM_ROUTINE() victim_asm();
#endif


void setup_oracle(){
    // printf("[+] Flush+Reload Threshold: ");
    CACHE_MISS = detect_flush_reload_threshold();
    // printf("%lu\n", CACHE_MISS);
    for(int i = 0; i < 256; i++){
        flush((uint8_t *)&oracles + i * PAGE_SIZE);
    }
}

void setup_pages(){
    ptedit_init();

    int mt_type[] = {PTEDIT_MT_UC, PTEDIT_MT_WC,PTEDIT_MT_WT};
    int mt[sizeof(mt_type)/sizeof(int)];
    for(int i = 0; i < sizeof(mt)/sizeof(int); i++)
    {
        mt[i] = ptedit_find_first_mt(mt_type[i]);
        if (mt[i] == -1) {
            printf("MT not available!\n");
            exit(1);
        }
    }

    uint8_t * ptr = NULL;
    ptedit_entry_t entry;
    
    ptr = (uint8_t *)&address_normal;
    ptr[0] = ptr[0] + 1 - 1;

    ptr = (uint8_t *)&address_US;
    ptr[0] = ptr[0] + 1 - 1;
    ptedit_pte_clear_bit(ptr, 0, PTEDIT_PAGE_BIT_USER);


    ptr = (uint8_t *)&address_MPK;
    ptr[0] = ptr[0] + 1 - 1;
    ptedit_pte_set_bit(ptr, 0, PTEDIT_PAGE_BIT_PKEY_BIT0);


    ptr = (uint8_t *)&address_A;
    ptr[0] = ptr[0] + 1 - 1;
    ptedit_pte_set_bit(ptr, 1, PTEDIT_PAGE_BIT_ACCESSED);


    ptr = (uint8_t *)&address_D;
    ptr[0] = ptr[0] + 1 - 1;
    ptedit_pte_set_bit(ptr, 1, PTEDIT_PAGE_BIT_DIRTY);



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

void setup_fh(){
    signal(SIGSEGV, trycatch_segfault_handler);
    signal(SIGFPE, trycatch_segfault_handler);
}
