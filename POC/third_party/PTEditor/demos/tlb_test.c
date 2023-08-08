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

#define REPEAT 10000

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

void maccess(void *p) { asm volatile("movq (%0), %%rax\n" : : "c"(p) : "rax"); }

int main(int argc, char *argv[]) {
    unsigned long target = 'X';
    if (ptedit_init()) {
      printf(TAG_FAIL "Error: Could not initalize PTEditor, did you load the kernel module?\n");
      return 1;
    }

    printf(TAG_OK "Setting TLB invalidation method to kernel version\n");
    ptedit_switch_tlb_invalidation(PTEDITOR_TLB_INVALIDATION_KERNEL);

    size_t total = 0;
    for(int i=0; i<REPEAT; i++) {
        maccess(&target);
        size_t start = rdtsc();
        ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_INVALIDATE_TLB, (size_t)&target);
        total += rdtsc() - start;
    }
    printf(TAG_OK "TLB invalidation: %f\n", ((float)total)/REPEAT);

    printf(TAG_OK "Setting TLB invalidation method to kernel version\n");
    ptedit_switch_tlb_invalidation(PTEDITOR_TLB_INVALIDATION_CUSTOM);

    for(int i=0; i<REPEAT; i++) {
        maccess(&target);
        size_t start = rdtsc();
        ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_INVALIDATE_TLB, (size_t)&target);
        total += rdtsc() - start;
    }
    printf(TAG_OK "TLB invalidation: %f\n", ((float)total)/REPEAT);

    ptedit_cleanup();

    printf(TAG_OK "Done\n");
}
