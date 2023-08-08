#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <memory.h>
#include <unistd.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"

#define TAG_OK COLOR_GREEN "[+]" COLOR_RESET " "
#define TAG_FAIL COLOR_RED "[-]" COLOR_RESET " "
#define TAG_PROGRESS COLOR_YELLOW "[~]" COLOR_RESET " "


#include "ptedit_header.h"


int main(int argc, char *argv[]) {
  size_t address_pfn, target_pfn;
  (void)argc;
  (void)argv;

  if(ptedit_init()) {
    printf(TAG_FAIL "Could not initialize ptedit (did you load the kernel module?)\n");
    return 1;
  }

//   ptedit_use_implementation(PTEDIT_IMPL_KERNEL);

  char page[ptedit_get_pagesize()];

  void *address = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  memset(address, 'A', 4096);

  void *target = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  memset(target, 'B', 4096);

  printf(TAG_OK "address @ " COLOR_YELLOW "%p" COLOR_RESET "\n", address);
  printf(TAG_OK "target @ " COLOR_YELLOW "%p" COLOR_RESET "\n", target);

  ptedit_entry_t vm = ptedit_resolve(address, 0);
  if(vm.pgd == 0) {
    printf(TAG_FAIL "Could not resolve PTs\n");
    goto error;
  }
  ptedit_print_entry_t(vm);
  printf(TAG_PROGRESS "PTE PFN %zx\n", (size_t)(ptedit_cast(vm.pte, ptedit_pte_t).pfn));

  printf(TAG_PROGRESS "address[0] = " COLOR_YELLOW "%c" COLOR_RESET"\n", *(volatile char*)address);
  if(*(volatile char*)address == 'A') {
      printf(TAG_OK "OK!\n");
  } else {
      printf(TAG_FAIL "Fail!\n");
  }

  printf(TAG_OK "Set PFN of address to PFN of target\n");


  // get current pfn of "access"
  address_pfn = ptedit_get_pfn(vm.pte);
  target_pfn = ptedit_pte_get_pfn(target, 0);
  // update to pfn of "target"
  vm.pte = ptedit_set_pfn(vm.pte, target_pfn);

  // update only PTE
  vm.valid = PTEDIT_VALID_MASK_PTE;
  ptedit_update(address, 0, &vm);

  printf(TAG_OK "address[0] = " COLOR_YELLOW "%c" COLOR_RESET "\n", *(volatile char*)address);
  if(*(volatile char*)address == 'B') {
      printf(TAG_OK "OK!\n");
  } else {
      printf(TAG_FAIL "Fail!\n");
  }

  printf(TAG_OK "Reading physical page of address\n");
  ptedit_read_physical_page(address_pfn, page);

  printf(TAG_OK "old address[0] (via physical page) = " COLOR_YELLOW "%c" COLOR_RESET "\n", page[0]);
  if(page[0] == 'A') {
      printf(TAG_OK "OK!\n");
  } else {
      printf(TAG_FAIL "Fail!\n");
  }

  printf(TAG_OK "Mapping physical address %zx to new virtual address\n", address_pfn * ptedit_get_pagesize());
  char* new_addr = ptedit_pmap(address_pfn * ptedit_get_pagesize(), ptedit_get_pagesize());
  printf(TAG_PROGRESS "mapped to virtual address %p\n", new_addr);

  printf(TAG_OK "old address[0] (via pmap) = " COLOR_YELLOW "%c" COLOR_RESET "\n", new_addr[0]);
  if(new_addr[0] == 'A') {
      printf(TAG_OK "OK!\n");
  } else {
      printf(TAG_FAIL "Fail!\n");
  }


  printf(TAG_OK "Overwriting physical page of target with " COLOR_YELLOW "C" COLOR_RESET "s\n");
  memset(page, 'C', ptedit_get_pagesize());
  ptedit_write_physical_page(target_pfn, page);

  ptedit_full_serializing_barrier();

  printf(TAG_OK "address[0] = " COLOR_YELLOW "%c" COLOR_RESET "\n", *(volatile char*)address);
  if(*(volatile char*)address == 'C') {
      printf(TAG_OK "OK!\n");
  } else {
      printf(TAG_FAIL "Fail!\n");
  }

  printf(TAG_OK "Resetting PFN of address\n");
  ptedit_pte_set_pfn(address, 0, address_pfn);

error:
  munmap(address, 4096);
  munmap(target, 4096);

  ptedit_cleanup();

  return 0;
}
