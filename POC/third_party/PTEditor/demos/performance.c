#include <stdio.h>
#include <stdint.h>
#include <memory.h>

#include "../ptedit_header.h"

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RESET "\x1b[0m"

#define TAG_OK COLOR_GREEN "[+]" COLOR_RESET " "
#define TAG_FAIL COLOR_RED "[-]" COLOR_RESET " "
#define TAG_PROGRESS COLOR_YELLOW "[~]" COLOR_RESET " "

unsigned long target;


uint64_t rdtsc() {
#if defined(__i386__) || defined(__x86_64__)
  uint64_t a, d;
  asm volatile("mfence");
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
  a = (d << 32) | a;
  asm volatile("mfence");
  return a;
#elif defined(__aarch64__)
#include <time.h>
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  return t1.tv_sec * 1000 * 1000 * 1000ULL + t1.tv_nsec;
#endif
}

#define REPEAT 10000

int is_same(ptedit_entry_t* e1, ptedit_entry_t* e2) {
    int diff = 0;
    if((e1->valid & PTEDIT_VALID_MASK_PGD) && (e2->valid & PTEDIT_VALID_MASK_PGD)) {
        diff |= e1->pgd ^ e2->pgd;
    }
    if((e1->valid & PTEDIT_VALID_MASK_P4D) && (e2->valid & PTEDIT_VALID_MASK_P4D)) {
        diff |= e1->p4d ^ e2->p4d;
    }    
    if((e1->valid & PTEDIT_VALID_MASK_PUD) && (e2->valid & PTEDIT_VALID_MASK_PUD)) {
        diff |= e1->pud ^ e2->pud;
    }
    if((e1->valid & PTEDIT_VALID_MASK_PMD) && (e2->valid & PTEDIT_VALID_MASK_PMD)) {
        diff |= e1->pmd ^ e2->pmd;
    }
    if((e1->valid & PTEDIT_VALID_MASK_PTE) && (e2->valid & PTEDIT_VALID_MASK_PTE)) {
        diff |= e1->pte ^ e2->pte;
    }
    return !diff;
}

int main(int argc, char *argv[]) {
    if (ptedit_init()) {
      printf(TAG_FAIL "Error: Could not initalize PTEditor, did you load the kernel module?\n");
      return 1;
    }

    target = 'X';
    size_t phys = 0;

    int i;
    ptedit_entry_t entry, entry_us;
    uint64_t start, stop;
    
    ptedit_use_implementation(PTEDIT_IMPL_KERNEL);
    start = rdtsc();
    for(i = 0; i < REPEAT; i++) {
        entry = ptedit_resolve(&target, 0);
    }
    stop = rdtsc();
    printf(TAG_OK "Kernel implementation takes " COLOR_YELLOW "%d" COLOR_RESET " cycles/resolve\n", (int)((stop - start) / REPEAT));
    
    ptedit_use_implementation(PTEDIT_IMPL_USER);
    start = rdtsc();
    for(i = 0; i < REPEAT; i++) {
        entry_us = ptedit_resolve(&target, 0);
    }
    stop = rdtsc();
    printf(TAG_OK "User implementation takes " COLOR_YELLOW "%d" COLOR_RESET " cycles/resolve\n", (int)((stop - start) / REPEAT));

    if(!is_same(&entry, &entry_us)) {
        printf(TAG_FAIL "Kernel and user-space resolver do not agree!\n");
        ptedit_print_entry_t(entry);
        ptedit_print_entry_t(entry_us);
    }
    
    ptedit_use_implementation(PTEDIT_IMPL_USER_PREAD);
    start = rdtsc();
    for(i = 0; i < REPEAT; i++) {
        entry_us = ptedit_resolve(&target, 0);
    }
    stop = rdtsc();
    printf(TAG_OK "User (pread) implementation takes " COLOR_YELLOW "%d" COLOR_RESET " cycles/resolve\n", (int)((stop - start) / REPEAT));

    if(!is_same(&entry, &entry_us)) {
        printf(TAG_FAIL "Kernel and user-space resolver do not agree!\n");
        ptedit_print_entry_t(entry);
        ptedit_print_entry_t(entry_us);
    }
    
    
    ptedit_cleanup();

    printf(TAG_OK "Done\n");
}
