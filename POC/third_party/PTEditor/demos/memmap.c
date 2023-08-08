#include <stdio.h>

#include "../ptedit_header.h"

int is_present(size_t entry) {
#if defined(__i386__) || defined(__x86_64__)
  return entry & (1ull << PTEDIT_PAGE_BIT_PRESENT);
#elif defined(__aarch64__)
  return (entry & 3) == 3;
#endif
}

int is_normal_page(size_t entry) {
#if defined(__i386__) || defined(__x86_64__)
  return !(entry & (1ull << PTEDIT_PAGE_BIT_PSE));
#elif defined(__aarch64__)
  return 1;
#endif
}

#if defined(__i386__) || defined(__x86_64__)
#define FIRST_LEVEL_ENTRIES 256 // only 256, because upper half is kernel
#elif defined(__aarch64__)
#define FIRST_LEVEL_ENTRIES 512
#endif

void dump(int do_dump, size_t entry, char *type) {
  if (do_dump) {
    for (int i = 0; i < 4; i++) {
      printf("%s", type);
      ptedit_print_entry_line(entry, i);
    }
  }
}

int main(int argc, char *argv[]) {
  if (ptedit_init()) {
    printf("Error: Could not initalize PTEditor, did you load the kernel module?\n");
    return 1;
  }

  int dump_entry = 1;
  size_t pid = 0;
  if (argc >= 2) {
    pid = atoi(argv[1]);
  }

  printf("Dumping PID %zd\n", pid);

  size_t root = ptedit_get_paging_root(pid);
  size_t pagesize = ptedit_get_pagesize();
  size_t pml4[pagesize / sizeof(size_t)], pdpt[pagesize / sizeof(size_t)],
      pd[pagesize / sizeof(size_t)], pt[pagesize / sizeof(size_t)];

  ptedit_read_physical_page(root / pagesize, (char *)pml4);

  int pml4i, pdpti, pdi, pti;
  size_t mem_usage = 0;

  /* Iterate through PML4 entries */
  for (pml4i = 0; pml4i < FIRST_LEVEL_ENTRIES; pml4i++) {
    size_t pml4_entry = pml4[pml4i];
    if (!is_present(pml4_entry))
      continue;
    dump(dump_entry, pml4_entry, "");

#if defined(__i386__) || defined(__x86_64__)
    /* Iterate through PDPT entries */
    ptedit_read_physical_page(ptedit_get_pfn(pml4_entry), (char *)pdpt);
    for (pdpti = 0; pdpti < 512; pdpti++) {
      size_t pdpt_entry = pdpt[pdpti];
      if (!is_present(pdpt_entry))
        continue;
      dump(dump_entry, pdpt_entry, "PDPT");
#elif defined(__aarch64__)
      size_t pdpt_entry = pml4_entry;
#endif

      /* Iterate through PD entries */
      ptedit_read_physical_page(ptedit_get_pfn(pdpt_entry), (char *)pd);
      for (pdi = 0; pdi < 512; pdi++) {
        size_t pd_entry = pd[pdi];
        if (!is_present(pd_entry))
          continue;
        dump(dump_entry, pd_entry, "    PD  ");

        /* Normal 4kb page */
        if (is_normal_page(pd_entry)) {
          /* Iterate through PT entries */
          ptedit_read_physical_page(ptedit_get_pfn(pd_entry), (char *)pt);
          for (pti = 0; pti < 512; pti++) {
            size_t pt_entry = pt[pti];
            if (!is_present(pt_entry))
              continue;
            dump(dump_entry, pt_entry, "        PT  ");
#if defined(__i386__) || defined(__x86_64__)
            printf("            -> %zx\n", ((size_t)pti << 12) | ((size_t)pdi << 21) | ((size_t)pdpti << 30) | ((size_t)pml4i << 39));
#elif defined(__aarch64__)
            printf("            -> %zx\n", ((size_t)pti << 12) | ((size_t)pdi << 21) | ((size_t)pml4i << 30));
#endif
            mem_usage += 4096;
          }
        } else {
          /* Large 2MB page (no PT) */
          printf("        -> %zx\n", ((size_t)pdi << 21) | ((size_t)pdpti << 30) | ((size_t)pml4i << 39));
          mem_usage += 2 * 1024 * 1024;
        }
      }
#if defined(__i386__) || defined(__x86_64__)
    }
#endif
  }

  printf("Used memory: %zd KB\n", mem_usage / 1024);

  ptedit_cleanup();
}
