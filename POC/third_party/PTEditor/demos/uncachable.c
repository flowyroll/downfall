#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#include "../ptedit_header.h"

#ifndef MAP_HUGE_2MB
#if defined(LINUX)
#include <linux/mman.h>
#endif
#ifndef MAP_HUGE_2MB
#define MAP_HUGE_2MB (21 << 26)
#endif
#endif

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RESET "\x1b[0m"

#define TAG_OK COLOR_GREEN "[+]" COLOR_RESET " "
#define TAG_FAIL COLOR_RED "[-]" COLOR_RESET " "
#define TAG_PROGRESS COLOR_YELLOW "[~]" COLOR_RESET " "

#define MEASUREMENTS 1000000

// ---------------------------------------------------------------------------
void dump_mts() {
  size_t mts = ptedit_get_mts();
  printf("MTs (raw): %zx\n", mts);
  int i;
  for (i = 0; i < 8; i++) {
    printf("MT%d: %d -> %s\n", i, ptedit_get_mt(i),
           ptedit_mt_to_string(ptedit_get_mt(i)));
  }
}

#if defined(__i386__) || defined(__x86_64__)
// ---------------------------------------------------------------------------
uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile("mfence");
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
  a = (d << 32) | a;
  asm volatile("mfence");
  return a;
}

// ---------------------------------------------------------------------------
void flush(void *p) { asm volatile("clflush 0(%0)\n" : : "c"(p) : "rax"); }

// ---------------------------------------------------------------------------
void maccess(void *p) { asm volatile("movq (%0), %%rax\n" : : "c"(p) : "rax"); }

// ---------------------------------------------------------------------------
void mfence() { asm volatile("mfence"); }
#elif defined(__aarch64__)
#include <time.h>
// ---------------------------------------------------------------------------
uint64_t rdtsc() {
#if 1
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  return t1.tv_sec * 1000 * 1000 * 1000ULL + t1.tv_nsec;
#else
  uint64_t result = 0;

  asm volatile("DSB SY");
  asm volatile("ISB");
  asm volatile("MRS %0, PMCCNTR_EL0" : "=r"(result));
  asm volatile("DSB SY");
  asm volatile("ISB");

  return result;
#endif
}

// ---------------------------------------------------------------------------
void flush(void *p) {
  asm volatile("DC CIVAC, %0" ::"r"(p));
  asm volatile("DSB ISH");
  asm volatile("ISB");
}

// ---------------------------------------------------------------------------
void maccess(void *p) {
  volatile uint32_t value;
  asm volatile("LDR %0, [%1]\n\t" : "=r"(value) : "r"(p));
  asm volatile("DSB ISH");
}

// ---------------------------------------------------------------------------
void mfence() { asm volatile("DSB ISH"); }
#endif

// ---------------------------------------------------------------------------
int access_time(void *ptr) {
  uint64_t start = 0, end = 0, sum = 0;

  for (int i = 0; i < MEASUREMENTS; i++) {
    start = rdtsc();
    maccess(ptr);
    end = rdtsc();
    sum += end - start;
  }

  return (int)(sum / MEASUREMENTS);
}

// ---------------------------------------------------------------------------
int main() {
  char *pt = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  memset(pt, 'A', 4096);

  if (ptedit_init()) {
    printf(TAG_FAIL "Error: Could not initalize PTEditor, did you load the kernel module?\n");
    return 1;
  }

  ptedit_entry_t entry = ptedit_resolve(pt, 0);
  ptedit_print_entry(entry.pte);
  printf(TAG_OK "Mapping is %s\n", ptedit_mt_to_string(ptedit_get_mt(ptedit_extract_mt(entry.pte))));

  flush(pt);
  printf(TAG_PROGRESS "Average access time: " COLOR_YELLOW "%d" COLOR_RESET " cycles\n", access_time(pt));

  dump_mts();

  int uc_mt = ptedit_find_first_mt(PTEDIT_MT_UC);
  if (uc_mt != -1) {
    printf(TAG_OK "%d MTs for UC (first is MT%d)\n", __builtin_popcount(ptedit_find_mt(PTEDIT_MT_UC)), uc_mt);
  } else {
    printf(TAG_FAIL "No UC MT available!\n");
    goto error;
  }

  entry.pte = ptedit_apply_mt(entry.pte, uc_mt);
  entry.valid = PTEDIT_VALID_MASK_PTE;
  ptedit_update(pt, 0, &entry);

  printf(TAG_OK "Mapping should now be uncachable\n");

  flush(pt);
  printf(TAG_PROGRESS "Average access time: " COLOR_YELLOW "%d" COLOR_RESET " cycles\n", access_time(pt));

  int wb_mt = ptedit_find_first_mt(PTEDIT_MT_WB);
  if (wb_mt != -1) {
    printf(TAG_OK "%d MTs for WB (first is MT%d)\n", __builtin_popcount(ptedit_find_mt(PTEDIT_MT_WB)), wb_mt);
  } else {
    printf(TAG_FAIL "No WB MT available!\n");
    goto error;
  }

  entry.pte = ptedit_apply_mt(entry.pte, wb_mt);
  entry.valid = PTEDIT_VALID_MASK_PTE;
  ptedit_update(pt, 0, &entry);

  printf(TAG_OK "Mapping should now be cachable again\n");

  flush(pt);
  printf(TAG_PROGRESS "Average access time: " COLOR_YELLOW "%d" COLOR_RESET " cycles\n", access_time(pt));

  char* huge_page = mmap(0, (2*1024*1024), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS|MAP_POPULATE|MAP_HUGETLB|MAP_HUGE_2MB, -1, 0);
  if (huge_page != MAP_FAILED) {
    printf(TAG_OK "Allocated huge page\n");

    ptedit_entry_t huge_entry = ptedit_resolve(huge_page, 0);
    ptedit_print_entry_t(huge_entry);
    unsigned char mt = ptedit_extract_mt_huge(huge_entry.pmd);
    printf(TAG_OK "Mapping is %s\n", ptedit_mt_to_string(ptedit_get_mt(mt)));
    size_t original_pmd = huge_entry.pmd;

    flush(huge_page);
    printf(TAG_PROGRESS "Average access time: " COLOR_YELLOW "%d" COLOR_RESET " cycles\n", access_time(huge_page));

    huge_entry.pmd = ptedit_apply_mt_huge(huge_entry.pmd, uc_mt);
    huge_entry.valid = PTEDIT_VALID_MASK_PMD;
    ptedit_update(huge_page, 0, &huge_entry);

    printf(TAG_OK "Mapping should now be uncachable\n");

    flush(huge_page);
    printf(TAG_PROGRESS "Average access time: " COLOR_YELLOW "%d" COLOR_RESET " cycles\n", access_time(huge_page));

    huge_entry.pmd = original_pmd;
    huge_entry.valid = PTEDIT_VALID_MASK_PMD;
    ptedit_update(huge_page, 0, &huge_entry);

    printf(TAG_OK "Mapping should now be cachable again\n");

    flush(huge_page);
    printf(TAG_PROGRESS "Average access time: " COLOR_YELLOW "%d" COLOR_RESET " cycles\n", access_time(huge_page));

    munmap(huge_page, (2*1024*1024));
  } else {
    printf(TAG_FAIL "Note: Could not allocate huge page.\n");
  }

error:
  ptedit_cleanup();
  printf(TAG_OK "Done\n");
}
