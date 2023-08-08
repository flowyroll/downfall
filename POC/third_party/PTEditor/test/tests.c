#include "utest.h"
#include "../ptedit_header.h"
#include <time.h>
#include <stdlib.h>

UTEST_STATE();

#if defined(LINUX)
#define PAGE_ALIGN_CHAR char __attribute__((aligned(4096)))
#else
#define PAGE_ALIGN_CHAR __declspec(align(4096)) char
#endif

#if defined(__i386__) || defined(__x86_64__)
void flush(void *p) { asm volatile("clflush 0(%0)\n" : : "c"(p) : "rax"); }
#elif defined(__aarch64__)
void flush(void *p) {
  asm volatile("DC CIVAC, %0" ::"r"(p));
  asm volatile("DSB ISH");
  asm volatile("ISB");
}
#endif

#ifndef MAP_HUGE_2MB
#if defined(LINUX)
#include <linux/mman.h>
#endif
#ifndef MAP_HUGE_2MB
#define MAP_HUGE_2MB (21 << 26)
#endif
#endif

PAGE_ALIGN_CHAR page1[4096];
PAGE_ALIGN_CHAR page2[4096];
PAGE_ALIGN_CHAR scratch[4096];
PAGE_ALIGN_CHAR accessor[4096];

// =========================================================================
//                             Helper functions
// =========================================================================

#if defined(LINUX)
size_t hrtime() {
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  return t1.tv_sec * 1000 * 1000 * 1000ULL + t1.tv_nsec;
}
#else
size_t hrtime() {
    __int64 wintime; 
    GetSystemTimePreciseAsFileTime((FILETIME*)&wintime);
    return wintime;
}
#endif

typedef void (*access_time_callback_t)(void*);

size_t access_time_ext(void *ptr, size_t MEASUREMENTS, access_time_callback_t cb) {
  size_t start = 0, end = 0, sum = 0;

  for (int i = 0; i < MEASUREMENTS; i++) {
    start = hrtime();
    *((volatile size_t*)ptr);
    end = hrtime();
    sum += (end - start);
    if(cb) cb(ptr);
  }

  return (size_t) (10 * sum / MEASUREMENTS);
}

size_t access_time(void *ptr) {
  return access_time_ext(ptr, 1000000, NULL);
}

int entry_equal(ptedit_entry_t* e1, ptedit_entry_t* e2) {
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

// =========================================================================
//                             Resolving address
// =========================================================================


UTEST(resolve, resolve_basic) {
    ptedit_entry_t vm = ptedit_resolve(page1, 0);
    ASSERT_TRUE(vm.pgd);
    ASSERT_TRUE(vm.pte);
    ASSERT_TRUE(vm.valid & PTEDIT_VALID_MASK_PTE);
    ASSERT_TRUE(vm.valid & PTEDIT_VALID_MASK_PGD);    
}

UTEST(resolve, resolve_valid_mask) {
    ptedit_entry_t vm = ptedit_resolve(page1, 0);
    if(vm.valid & PTEDIT_VALID_MASK_PGD) ASSERT_TRUE(vm.pgd);
    if(vm.valid & PTEDIT_VALID_MASK_P4D) ASSERT_TRUE(vm.p4d);
    if(vm.valid & PTEDIT_VALID_MASK_PMD) ASSERT_TRUE(vm.pmd);
    if(vm.valid & PTEDIT_VALID_MASK_PUD) ASSERT_TRUE(vm.pud);
    if(vm.valid & PTEDIT_VALID_MASK_PTE) ASSERT_TRUE(vm.pte);
}

UTEST(resolve, resolve_deterministic) {
    ptedit_entry_t vm1 = ptedit_resolve(page1, 0);
    ptedit_entry_t vm2 = ptedit_resolve(page1, 0);
    ASSERT_TRUE(entry_equal(&vm1, &vm2));
}

UTEST(resolve, resolve_different) {
    ptedit_entry_t vm1 = ptedit_resolve(page1, 0);
    ptedit_entry_t vm2 = ptedit_resolve(page2, 0);
    ASSERT_FALSE(entry_equal(&vm1, &vm2));
}

UTEST(resolve, resolve_invalid) {
    ptedit_entry_t vm1 = ptedit_resolve(0, 0);
    ASSERT_FALSE(vm1.valid & PTEDIT_VALID_MASK_PTE);
}

UTEST(resolve, resolve_invalid_pid) {
    ptedit_entry_t vm1 = ptedit_resolve(page1, -1);
    ASSERT_FALSE(vm1.valid);
}

UTEST(resolve, resolve_page_offset) {
    ptedit_entry_t vm1 = ptedit_resolve(page1, 0);
    ptedit_entry_t vm2 = ptedit_resolve(page1 + 1, 0);
    vm1.vaddr = vm2.vaddr = 0;
    ASSERT_TRUE(entry_equal(&vm1, &vm2));
    ptedit_entry_t vm3 = ptedit_resolve(page1 + 1024, 0);
    vm1.vaddr = vm3.vaddr = 0;
    ASSERT_TRUE(entry_equal(&vm1, &vm3));
    ptedit_entry_t vm4 = ptedit_resolve(page1 + 4095, 0);
    vm1.vaddr = vm4.vaddr = 0;
    ASSERT_TRUE(entry_equal(&vm1, &vm4));
}


// =========================================================================
//                             Updating address
// =========================================================================

UTEST(update, nop) {
    ptedit_entry_t vm1 = ptedit_resolve(scratch, 0);
    ASSERT_TRUE(vm1.valid);
    size_t valid = vm1.valid;
    vm1.valid = 0;
    ptedit_update(scratch, 0, &vm1);
    vm1.valid = valid;
    ptedit_entry_t vm2 = ptedit_resolve(scratch, 0);
    ASSERT_TRUE(entry_equal(&vm1, &vm2));
}

UTEST(update, pte_nop) {
    ptedit_entry_t vm1 = ptedit_resolve(scratch, 0);
    ASSERT_TRUE(vm1.valid);
    size_t valid = vm1.valid;
    vm1.valid = PTEDIT_VALID_MASK_PTE;
    ptedit_update(scratch, 0, &vm1);
    vm1.valid = valid;
    ptedit_entry_t vm2 = ptedit_resolve(scratch, 0);
    ASSERT_TRUE(entry_equal(&vm1, &vm2));
}

UTEST(update, new_pte) {
    ptedit_entry_t vm = ptedit_resolve(scratch, 0);
    ptedit_entry_t vm1 = ptedit_resolve(scratch, 0);
    ASSERT_TRUE(vm1.valid);
    size_t pte = vm1.pte;
    vm1.pte = ptedit_set_pfn(vm1.pte, 0x1234);
    vm1.valid = PTEDIT_VALID_MASK_PTE;
    ptedit_update(scratch, 0, &vm1);
    
    ptedit_entry_t check = ptedit_resolve(scratch, 0);
    ASSERT_NE((size_t)ptedit_cast(check.pte, ptedit_pte_t).pfn, ptedit_get_pfn(pte));
    ASSERT_EQ((size_t)ptedit_cast(check.pte, ptedit_pte_t).pfn, 0x1234);
    
    vm1.valid = PTEDIT_VALID_MASK_PTE;
    vm1.pte = pte;
    ptedit_update(scratch, 0, &vm1);
    
    ptedit_entry_t vm2 = ptedit_resolve(scratch, 0);
    ASSERT_TRUE(entry_equal(&vm, &vm2));
}

// =========================================================================
//                                  PTEs
// =========================================================================

UTEST(pte, get_pfn) {
    ptedit_entry_t vm = ptedit_resolve(page1, 0);
    ASSERT_EQ(ptedit_get_pfn(vm.pte), (size_t)ptedit_cast(vm.pte, ptedit_pte_t).pfn);
}

UTEST(pte, get_pte_pfn) {
    ptedit_entry_t vm = ptedit_resolve(page1, 0);
    ASSERT_EQ(ptedit_pte_get_pfn(page1, 0), (size_t)ptedit_cast(vm.pte, ptedit_pte_t).pfn);
}

UTEST(pte, get_pte_pfn_invalid) {
    ASSERT_FALSE(ptedit_pte_get_pfn(0, 0));
}

UTEST(pte, pte_present) {
    ptedit_entry_t vm = ptedit_resolve(page1, 0);
    ASSERT_EQ((size_t)ptedit_cast(vm.pte, ptedit_pte_t).present, PTEDIT_PAGE_PRESENT);
}

UTEST(pte, pte_set_pfn_basic) {
    size_t entry = 0;
    ASSERT_EQ(entry, ptedit_set_pfn(entry, 0));
    ASSERT_NE(entry, ptedit_set_pfn(entry, 1));
    ASSERT_EQ(entry, ptedit_set_pfn(ptedit_set_pfn(entry, 1234), 0));
    ASSERT_GT(ptedit_set_pfn(entry, 2), ptedit_set_pfn(entry, 1));
    entry = (size_t)-1;
    ASSERT_NE(0, ptedit_set_pfn(entry, 0));
}

UTEST(pte, pte_set_pfn) {
    ASSERT_TRUE(accessor[0] == 2);
    size_t accessor_pfn = ptedit_pte_get_pfn(accessor, 0);
    ASSERT_TRUE(accessor_pfn);
    size_t page1_pfn = ptedit_pte_get_pfn(page1, 0);
    ASSERT_TRUE(page1_pfn);
    size_t page2_pfn = ptedit_pte_get_pfn(page2, 0);
    ASSERT_TRUE(page2_pfn);
    ptedit_pte_set_pfn(accessor, 0, page1_pfn);
    ASSERT_TRUE(accessor[0] == 0);
    ptedit_pte_set_pfn(accessor, 0, page2_pfn);
    ASSERT_TRUE(accessor[0] == 1);
    ptedit_pte_set_pfn(accessor, 0, accessor_pfn);
    ASSERT_TRUE(accessor[0] == 2);
}

// =========================================================================
//                             Bit Modifications in Paging
// =========================================================================
UTEST(pte, ptedit_clear_bit) {
    memset(page1,0x42,4096);
    ptedit_clear_bit(page1, 0, PTEDIT_PAGE_BIT_ACCESSED,PTEDIT_VALID_MASK_PUD|PTEDIT_VALID_MASK_PMD|PTEDIT_VALID_MASK_PTE);
    ptedit_entry_t entry = ptedit_resolve(page1, 0);
    ASSERT_TRUE(PTEDIT_B(entry.pte, PTEDIT_PAGE_BIT_ACCESSED) == 0);
    ASSERT_TRUE(PTEDIT_B(entry.pud, PTEDIT_PAGE_BIT_ACCESSED) == 0);
    ASSERT_TRUE(PTEDIT_B(entry.pd, PTEDIT_PAGE_BIT_ACCESSED) == 0);
}

UTEST(pte, ptedit_set_bit) {
    memset(page1,0x42,4096);
    ptedit_clear_bit(page1, 0, PTEDIT_PAGE_BIT_ACCESSED,PTEDIT_VALID_MASK_PUD|PTEDIT_VALID_MASK_PMD|PTEDIT_VALID_MASK_PTE);
    ptedit_entry_t entry = ptedit_resolve(page1, 0);
    ASSERT_TRUE(PTEDIT_B(entry.pte, PTEDIT_PAGE_BIT_ACCESSED) == 0);
    ASSERT_TRUE(PTEDIT_B(entry.pud, PTEDIT_PAGE_BIT_ACCESSED) == 0);
    ASSERT_TRUE(PTEDIT_B(entry.pd, PTEDIT_PAGE_BIT_ACCESSED) == 0);
    ptedit_set_bit(page1, 0, PTEDIT_PAGE_BIT_ACCESSED,PTEDIT_VALID_MASK_PUD|PTEDIT_VALID_MASK_PMD|PTEDIT_VALID_MASK_PTE);
    entry = ptedit_resolve(page1, 0);
    ASSERT_TRUE(PTEDIT_B(entry.pte, PTEDIT_PAGE_BIT_ACCESSED) == 1);
    ASSERT_TRUE(PTEDIT_B(entry.pud, PTEDIT_PAGE_BIT_ACCESSED) == 1);
    ASSERT_TRUE(PTEDIT_B(entry.pd, PTEDIT_PAGE_BIT_ACCESSED) == 1);
}

// =========================================================================
//                             Physical Pages
// =========================================================================

UTEST(page, read) {
    char buffer[4096];
    size_t pfn = ptedit_pte_get_pfn(page1, 0);
    ASSERT_TRUE(pfn);
    ptedit_read_physical_page(pfn, buffer);
    ASSERT_TRUE(!memcmp(buffer, page1, sizeof(buffer)));
    pfn = ptedit_pte_get_pfn(page2, 0);
    ASSERT_TRUE(pfn);
    ptedit_read_physical_page(pfn, buffer);
    ASSERT_TRUE(!memcmp(buffer, page2, sizeof(buffer)));
}

UTEST(page, write) {
    char buffer[4096];
    size_t pfn = ptedit_pte_get_pfn(scratch, 0);
    ASSERT_TRUE(pfn);
    ptedit_write_physical_page(pfn, page1);
    ptedit_read_physical_page(pfn, buffer);
    ASSERT_TRUE(!memcmp(page1, buffer, sizeof(buffer)));
    ptedit_write_physical_page(pfn, page2);
    ptedit_read_physical_page(pfn, buffer);
    ASSERT_TRUE(!memcmp(page2, buffer, sizeof(buffer)));
}

// =========================================================================
//                                Paging
// =========================================================================

UTEST(paging, get_root) {
    size_t root = ptedit_get_paging_root(0);
    ASSERT_TRUE(root);
}

UTEST(paging, get_root_deterministic) {
    size_t root = ptedit_get_paging_root(0);
    ASSERT_TRUE(root);
    size_t root_check = ptedit_get_paging_root(0);
    ASSERT_EQ(root, root_check);   
}

UTEST(paging, get_root_invalid_pid) {
    size_t root = ptedit_get_paging_root(-1);
    ASSERT_FALSE(root);
}

UTEST(paging, root_page_aligned) {
    size_t root = ptedit_get_paging_root(0);
    ASSERT_TRUE(root);
    ASSERT_FALSE(root % ptedit_get_pagesize());
}

UTEST(paging, correct_root) {
    size_t buffer[4096 / sizeof(size_t)];
    size_t root = ptedit_get_paging_root(0);
    ptedit_read_physical_page(root / ptedit_get_pagesize(), (char*)buffer);
    ptedit_entry_t vm = ptedit_resolve(0, 0);
    ASSERT_EQ(vm.pgd, buffer[0]);
}

// =========================================================================
//                               Memory Types
// =========================================================================

UTEST(memtype, get) {
    ASSERT_TRUE(ptedit_get_mts());
}

UTEST(memtype, get_deterministic) {
    ASSERT_EQ(ptedit_get_mts(), ptedit_get_mts());
}

UTEST(memtype, uncachable) {
    ASSERT_NE(ptedit_find_first_mt(PTEDIT_MT_UC), -1);
}

UTEST(memtype, writeback) {
    ASSERT_NE(ptedit_find_first_mt(PTEDIT_MT_WB), -1);
}

UTEST(memtype, apply) {
    size_t entry = 0;
    ASSERT_NE(ptedit_apply_mt(entry, 1), entry);
    ASSERT_EQ(ptedit_apply_mt(entry, 0), entry);
}

UTEST(memtype, apply_huge) {
    size_t entry = 0;
    ASSERT_NE(ptedit_apply_mt_huge(entry, 1), entry);
    ASSERT_EQ(ptedit_apply_mt_huge(entry, 0), entry);
}

UTEST(memtype, extract) {
    ASSERT_TRUE(ptedit_extract_mt(ptedit_apply_mt(0, 5)) == 5);
    ASSERT_TRUE(ptedit_extract_mt(ptedit_apply_mt((size_t)-1, 2)) == 2);
}

UTEST(memtype, extract_huge) {
    ASSERT_TRUE(ptedit_extract_mt_huge(ptedit_apply_mt_huge(0, 5)) == 5);
    ASSERT_TRUE(ptedit_extract_mt_huge(ptedit_apply_mt_huge((size_t)-1, 2)) == 2);
}

UTEST(memtype, uncachable_access_time) {
    if(getenv("TRAVISCI")) {
        ASSERT_TRUE(1);
    } else {
        int uc_mt = ptedit_find_first_mt(PTEDIT_MT_UC);
        ASSERT_NE(uc_mt, -1);
        int wb_mt = ptedit_find_first_mt(PTEDIT_MT_WB);
        ASSERT_NE(wb_mt, -1);
        
        flush(scratch);
        size_t before = access_time(scratch);
        
        ptedit_entry_t entry = ptedit_resolve(scratch, 0);
        size_t pte = entry.pte;
        ASSERT_TRUE(entry.valid);
        ASSERT_TRUE(entry.pte);
        entry.pte = ptedit_apply_mt(entry.pte, uc_mt);
        entry.valid = PTEDIT_VALID_MASK_PTE;
        ptedit_update(scratch, 0, &entry);   
        
        flush(scratch);
        size_t uc = access_time(scratch);
        
        entry.pte = pte;
        entry.valid = PTEDIT_VALID_MASK_PTE;
        ptedit_update(scratch, 0, &entry);   
        
        size_t after = access_time(scratch);

        ASSERT_LT(after + 5, uc);
        ASSERT_LT(before + 5, uc);
    }
}

UTEST(memtype, uncachable_huge_page_access_time) {
    if(getenv("TRAVISCI")) {
        ASSERT_TRUE(1);
    } else {
        char* huge_page = mmap(0, (2*1024*1024), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS|MAP_POPULATE|MAP_HUGETLB|MAP_HUGE_2MB, -1, 0);
        if (huge_page != MAP_FAILED) {
          int uc_mt = ptedit_find_first_mt(PTEDIT_MT_UC);
          ASSERT_NE(uc_mt, -1);
          int wb_mt = ptedit_find_first_mt(PTEDIT_MT_WB);
          ASSERT_NE(wb_mt, -1);

          flush(huge_page);
          size_t before = access_time(huge_page);
          
          ptedit_entry_t entry = ptedit_resolve(huge_page, 0);
          size_t pmd = entry.pmd;
          ASSERT_TRUE(entry.valid);
          ASSERT_TRUE(entry.pmd);

          entry.pmd = ptedit_apply_mt_huge(entry.pmd, uc_mt);
          entry.valid = PTEDIT_VALID_MASK_PMD;
          ptedit_update(huge_page, 0, &entry);   
          
          flush(huge_page);
          size_t uc = access_time(huge_page);
          
          entry.pmd = pmd;
          entry.valid = PTEDIT_VALID_MASK_PMD;
          ptedit_update(huge_page, 0, &entry);   
          
          size_t after = access_time(huge_page);

          munmap(huge_page, (2*1024*1024));

          ASSERT_LT(after + 5, uc);
          ASSERT_LT(before + 5, uc);
        } else {
          fprintf(stdout, "Note: Could not allocate huge page.\n");
        }
    }
}

// =========================================================================
//                               TLB
// =========================================================================

UTEST(tlb, invalid_tlb_invalidate_method) {
    int ret = ptedit_switch_tlb_invalidation(3);
    ASSERT_TRUE(ret);
}

UTEST(tlb, valid_tlb_invalidate_method) {
    int ret = ptedit_switch_tlb_invalidation(PTEDITOR_TLB_INVALIDATION_KERNEL);
    ASSERT_FALSE(ret);
}

UTEST(tlb, access_time_kernel_tlb_flush) {
    ptedit_switch_tlb_invalidation(PTEDITOR_TLB_INVALIDATION_KERNEL);
    int flushed = access_time_ext(scratch, 100, ptedit_invalidate_tlb);
    int normal = access_time_ext(scratch, 100, NULL);
    ASSERT_GT(flushed, normal);
}

UTEST(tlb, access_time_custom_tlb_flush) {
    ptedit_switch_tlb_invalidation(PTEDITOR_TLB_INVALIDATION_CUSTOM);
    int flushed = access_time_ext(scratch, 100, ptedit_invalidate_tlb);
    int normal = access_time_ext(scratch, 100, NULL);
    ASSERT_GT(flushed, normal);
}

int main(int argc, const char *const argv[]) {
    if(ptedit_init()) {
        printf("Could not initialize PTEditor, did you load the kernel module?\n");
        return 1;
    }
    memset(scratch, 0, sizeof(scratch));
    memset(page1, 0, sizeof(page1));
    memset(page2, 1, sizeof(page2));
    memset(accessor, 2, sizeof(accessor));
    
//     ptedit_use_implementation(PTEDIT_IMPL_USER_PREAD);
    
    int result = utest_main(argc, argv);
    
    ptedit_cleanup();
    return result;
}
