#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../ptedit_header.h"

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RESET "\x1b[0m"

#define TAG_OK COLOR_GREEN "[+]" COLOR_RESET " "
#define TAG_FAIL COLOR_RED "[-]" COLOR_RESET " "
#define TAG_PROGRESS COLOR_YELLOW "[~]" COLOR_RESET " "

int main(int argc, char *argv[]) {
  char *pt = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  char *target = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  char *secret = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  memset(pt, 'A', 4096);
  memset(target, 'B', 4096);
  memset(secret, 'S', 4096);

  if (ptedit_init()) {
    printf(TAG_FAIL "Error: Could not initalize PTEditor, did you load the kernel module?\n");
    return 1;
  }

  ptedit_entry_t secret_entry = ptedit_resolve(secret, 0);

  /* "target" uses the manipulated page-table entry */
  ptedit_entry_t target_entry = ptedit_resolve(target, 0);
  ptedit_print_entry(target_entry.pte);

  /* "pt" should map the page table corresponding to "target" */
  ptedit_entry_t pt_entry = ptedit_resolve(pt, 0);
  size_t old_pte = pt_entry.pte;

  /* -> set page-table PFN of "pt" to page-directory PFN of "target" */
  pt_entry.pte = ptedit_set_pfn(pt_entry.pte, ptedit_get_pfn(target_entry.pmd));

  pt_entry.valid = PTEDIT_VALID_MASK_PTE;
  ptedit_update(pt, 0, &pt_entry);

  /* "target" entry is bits 12 to 20 of "target" virtual address */
  size_t entry = (((size_t)target) >> 12) & 0x1ff;
  printf(TAG_PROGRESS "Entry: %zd\n", entry);
  size_t *mapped_entry = ((size_t *)pt) + entry;

  /* "mapped_entry" is a user-space-accessible pointer to the PTE of "target" */
  if (*mapped_entry != target_entry.pte) {
    printf(TAG_FAIL "Something went wrong...\n");
  } else {
    printf(TAG_OK "Worked!\n");
  }

  /* let "target" point to "secret" */
  *mapped_entry = ptedit_set_pfn(*mapped_entry, ptedit_get_pfn(secret_entry.pte));
  ptedit_invalidate_tlb(target);

  printf(TAG_PROGRESS "Target[0]: %c\n", *target);
  if (*target == 'S') {
    printf(TAG_OK "Success!\n");
  } else {
    printf(TAG_FAIL "Fail!\n");
  }

  /* reset "target" entry */
  *mapped_entry = target_entry.pte;

  /* reset mapping */
  pt_entry.pte = old_pte;
  pt_entry.valid = PTEDIT_VALID_MASK_PTE;
  ptedit_update(pt, 0, &pt_entry);

  ptedit_cleanup();

  printf(TAG_OK "Done\n");
}
