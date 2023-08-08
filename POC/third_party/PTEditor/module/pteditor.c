#include <linux/mm_types.h>
#include <asm/tlbflush.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/fs.h>
#include <linux/kallsyms.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/ptrace.h>
#include <linux/proc_fs.h>
#include <linux/kprobes.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
#include <linux/mmap_lock.h>
#endif

#ifdef CONFIG_PAGE_TABLE_ISOLATION
pgd_t __attribute__((weak)) __pti_set_user_pgtbl(pgd_t *pgdp, pgd_t pgd);
#endif

static int real_page_size = 4096, real_page_shift = 12;

#include "pteditor.h"

MODULE_AUTHOR("Michael Schwarz");
MODULE_DESCRIPTION("Device to play around with paging structures");
MODULE_LICENSE("GPL");

#if defined(__aarch64__)
#include <linux/hugetlb.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
typedef pgdval_t p4dval_t;
#endif

void __attribute__((weak)) set_swapper_pgd(pgd_t* pgdp, pgd_t pgd) {}
pgd_t __attribute__((weak)) swapper_pg_dir[PTRS_PER_PGD];

static inline pte_t native_make_pte(pteval_t val)
{
  return __pte(val);
}

static inline pgd_t native_make_pgd(pgdval_t val)
{
  return __pgd(val);
}

static inline pmd_t native_make_pmd(pmdval_t val)
{
  return __pmd(val);
}

static inline pud_t native_make_pud(pudval_t val)
{
  return __pud(val);
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)

static inline p4d_t native_make_p4d(p4dval_t val)
{
  return __p4d(val);
}
#endif

static inline pteval_t native_pte_val(pte_t pte)
{
  return pte_val(pte);
}

static inline int pud_large(pud_t pud) {
#ifdef __PAGETABLE_PMD_FOLDED 
    return pud_val(pud) && !(pud_val(pud) & PUD_TABLE_BIT);
#else
    return 0;
#endif
}

static inline int pmd_large(pmd_t pmd) {
#ifdef __PAGETABLE_PMD_FOLDED
    return pmd_val(pmd) && !(pmd_val(pmd) & PMD_TABLE_BIT)
#else
    return 0;
#endif
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#define from_user raw_copy_from_user
#define to_user raw_copy_to_user
#else
#define from_user copy_from_user
#define to_user copy_to_user
#endif

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "[pteditor-module] " fmt

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
#define KPROBE_KALLSYMS_LOOKUP 1
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
kallsyms_lookup_name_t kallsyms_lookup_name_func;
#define kallsyms_lookup_name kallsyms_lookup_name_func

static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};
#endif

typedef struct {
    size_t pid;
    pgd_t *pgd;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    p4d_t *p4d;
#else
    size_t *p4d;
#endif
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    size_t valid;
} vm_t;

static bool device_busy = false;
static bool mm_is_locked = false;

void (*invalidate_tlb)(unsigned long);
void (*flush_tlb_mm_range_func)(struct mm_struct*, unsigned long, unsigned long, unsigned int, bool);
void (*native_write_cr4_func)(unsigned long);
static struct mm_struct* get_mm(size_t);

static int device_open(struct inode *inode, struct file *file) {
  /* Check if device is busy */
  if (device_busy == true) {
    return -EBUSY;
  }

  device_busy = true;

  return 0;
}

static int device_release(struct inode *inode, struct file *file) {
  /* Unlock module */
  device_busy = false;

  return 0;
}

static void
_invalidate_tlb(void *addr) {
#if defined(__i386__) || defined(__x86_64__)
  int pcid;
  unsigned long flags;
  unsigned long cr4;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 98)
#if defined(X86_FEATURE_INVPCID_SINGLE) && defined(INVPCID_TYPE_INDIV_ADDR)
  if (cpu_feature_enabled(X86_FEATURE_INVPCID_SINGLE)) {
    for(pcid = 0; pcid < 4096; pcid++) {
      invpcid_flush_one(pcid, (long unsigned int) addr);
    }
  } 
  else 
#endif
  {
    raw_local_irq_save(flags);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    cr4 = native_read_cr4();
#else
    cr4 = this_cpu_read(cpu_tlbstate.cr4);
#endif
#else
    cr4 = __read_cr4();
#endif
    native_write_cr4_func(cr4 & ~X86_CR4_PGE);
    native_write_cr4_func(cr4);
    raw_local_irq_restore(flags);
  }
#else
  asm volatile ("invlpg (%0)": : "r"(addr));
#endif
#elif defined(__aarch64__)
  asm volatile ("dsb ishst");
  asm volatile ("tlbi vmalle1is");
  asm volatile ("dsb ish");
  asm volatile ("isb");
#endif
}

static void
invalidate_tlb_custom(unsigned long addr) {
  on_each_cpu(_invalidate_tlb, (void*) addr, 1);
}

#if defined(__aarch64__)
typedef struct tlb_page_s {
  struct vm_area_struct* vma;
  unsigned long addr;
} tlb_page_t;

void _flush_tlb_page_smp(void* info) {
  tlb_page_t* tlb_page = (tlb_page_t*) info;
  flush_tlb_page(tlb_page->vma, tlb_page->addr);
}
#endif

static void
invalidate_tlb_kernel(unsigned long addr) {
#if defined(__i386__) || defined(__x86_64__)
  flush_tlb_mm_range_func(get_mm(task_pid_nr(current)), addr, addr + real_page_size, real_page_shift, false);
#elif defined(__aarch64__)
  struct vm_area_struct *vma = find_vma(current->mm, addr);
  tlb_page_t tlb_page;
  if (unlikely(vma == NULL || addr < vma->vm_start)) {
    return;
  }
  tlb_page.vma = vma;
  tlb_page.addr = addr;
  on_each_cpu(_flush_tlb_page_smp, &tlb_page, 1);
#endif
}

static void _set_pat(void* _pat) {
#if defined(__i386__) || defined(__x86_64__)
    int low, high;
    size_t pat = (size_t)_pat;
    low = pat & 0xffffffff;
    high = (pat >> 32) & 0xffffffff;
    asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(0x277));
#elif defined(__aarch64__)
    size_t pat = (size_t)_pat;
    asm volatile ("msr mair_el1, %0\n" : : "r"(pat));
#endif
}

static void set_pat(size_t pat) {
    on_each_cpu(_set_pat, (void*) pat, 1);
}

static struct mm_struct* get_mm(size_t pid) {
  struct task_struct *task;
  struct pid* vpid;

  /* Find mm */
  task = current;
  if(pid != 0) {
    vpid = find_vpid(pid);
    if(!vpid) return NULL;
    task = pid_task(vpid, PIDTYPE_PID);
    if(!task) return NULL;
  }
  if(task->mm) {
      return task->mm;
  } else {
      return task->active_mm;
  }
  return NULL;
}

static int resolve_vm(size_t addr, vm_t* entry, int lock) {
  struct mm_struct *mm;

  if(!entry) return 1;
  entry->pud = NULL;
  entry->pmd = NULL;
  entry->pgd = NULL;
  entry->pte = NULL;
  entry->p4d = NULL;
  entry->valid = 0;

  mm = get_mm(entry->pid);
  if(!mm) {
      return 1;
  }

  /* Lock mm */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
  if(lock) mmap_read_lock(mm);
#else
  if(lock) down_read(&mm->mmap_sem);
#endif

  /* Return PGD (page global directory) entry */
  entry->pgd = pgd_offset(mm, addr);
  if (pgd_none(*(entry->pgd)) || pgd_bad(*(entry->pgd))) {
      entry->pgd = NULL;
      goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_PGD;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
  /* Return p4d offset */
  entry->p4d = p4d_offset(entry->pgd, addr);
  if (p4d_none(*(entry->p4d)) || p4d_bad(*(entry->p4d))) {
    entry->p4d = NULL;
    goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_P4D;

  /* Get offset of PUD (page upper directory) */
  entry->pud = pud_offset(entry->p4d, addr);
  if (pud_none(*(entry->pud))) {
    entry->pud = NULL;
    goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_PUD;
#else
  /* Get offset of PUD (page upper directory) */
  entry->pud = pud_offset(entry->pgd, addr);
  if (pud_none(*(entry->pud))) {
    entry->pud = NULL;
    goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_PUD;
#endif


  /* Get offset of PMD (page middle directory) */
  entry->pmd = pmd_offset(entry->pud, addr);
  if (pmd_none(*(entry->pmd)) || pud_large(*(entry->pud))) {
    entry->pmd = NULL;
    goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_PMD;

  /* Map PTE (page table entry) */
  entry->pte = pte_offset_map(entry->pmd, addr);
  if (entry->pte == NULL || pmd_large(*(entry->pmd))) {
    entry->pte = NULL;
    goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_PTE;

  /* Unmap PTE, fine on x86 and ARM64 -> unmap is NOP */
  pte_unmap(entry->pte);

  /* Unlock mm */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
  if(lock) mmap_read_unlock(mm);
#else
  if(lock) up_read(&mm->mmap_sem);
#endif

  return 0;

error_out:

  /* Unlock mm */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
  if(lock) mmap_read_unlock(mm);
#else
  if(lock) up_read(&mm->mmap_sem);
#endif

  return 1;
}


static int update_vm(ptedit_entry_t* new_entry, int lock) {
  vm_t old_entry;
  size_t addr = new_entry->vaddr;
  struct mm_struct *mm = get_mm(new_entry->pid);
  if(!mm) return 1;

  old_entry.pid = new_entry->pid;

  /* Lock mm */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
  if(lock) mmap_write_lock(mm);
#else
  if(lock) down_write(&mm->mmap_sem);
#endif

  resolve_vm(addr, &old_entry, 0);

  /* Update entries */
  if((old_entry.valid & PTEDIT_VALID_MASK_PGD) && (new_entry->valid & PTEDIT_VALID_MASK_PGD)) {
      pr_warn("Updating PGD\n");
      set_pgd(old_entry.pgd, native_make_pgd(new_entry->pgd));
  }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
  if((old_entry.valid & PTEDIT_VALID_MASK_P4D) && (new_entry->valid & PTEDIT_VALID_MASK_P4D)) {
      pr_warn("Updating P4D\n");
      set_p4d(old_entry.p4d, native_make_p4d(new_entry->p4d));
  }
#endif

  if((old_entry.valid & PTEDIT_VALID_MASK_PUD) && (new_entry->valid & PTEDIT_VALID_MASK_PUD)) {
      pr_warn("Updating PUD\n");
      set_pud(old_entry.pud, native_make_pud(new_entry->pud));
  }

  if((old_entry.valid & PTEDIT_VALID_MASK_PMD) && (new_entry->valid & PTEDIT_VALID_MASK_PMD)) {
      pr_warn("Updating PMD\n");
      set_pmd(old_entry.pmd, native_make_pmd(new_entry->pmd));
  }

  if((old_entry.valid & PTEDIT_VALID_MASK_PTE) && (new_entry->valid & PTEDIT_VALID_MASK_PTE)) {
      pr_warn("Updating PTE\n");
      set_pte(old_entry.pte, native_make_pte(new_entry->pte));
  }

  invalidate_tlb(addr);

  /* Unlock mm */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
  if(lock) mmap_write_unlock(mm);
#else
  if(lock) up_write(&mm->mmap_sem);
#endif

  return 0;
}


static void vm_to_user(ptedit_entry_t* user, vm_t* vm) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#if CONFIG_PGTABLE_LEVELS > 4
    if(vm->p4d) user->p4d = (vm->p4d)->p4d;
#else
#if !defined(__ARCH_HAS_5LEVEL_HACK)
    if(vm->p4d) user->p4d = (vm->p4d)->pgd.pgd;
#else
    if(vm->p4d) user->p4d = (vm->p4d)->pgd;    
#endif
#endif
#endif
#if defined(__i386__) || defined(__x86_64__)
    if(vm->pgd) user->pgd = (vm->pgd)->pgd;
    if(vm->pmd) user->pmd = (vm->pmd)->pmd;
    if(vm->pud) user->pud = (vm->pud)->pud;
    if(vm->pte) user->pte = (vm->pte)->pte;
#elif defined(__aarch64__)
    if(vm->pgd) user->pgd = pgd_val(*(vm->pgd));
    if(vm->pmd) user->pmd = pmd_val(*(vm->pmd));
    if(vm->pud) user->pud = pud_val(*(vm->pud));
    if(vm->pte) user->pte = pte_val(*(vm->pte));
#endif
    user->valid = vm->valid;
}


static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
  switch (ioctl_num) {
    case PTEDITOR_IOCTL_CMD_VM_RESOLVE:
    {
        ptedit_entry_t vm_user;
        vm_t vm;
        (void)from_user(&vm_user, (void*)ioctl_param, sizeof(vm_user));
        vm.pid = vm_user.pid;
        resolve_vm(vm_user.vaddr, &vm, !mm_is_locked);
        vm_to_user(&vm_user, &vm);
        (void)to_user((void*)ioctl_param, &vm_user, sizeof(vm_user));
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_VM_UPDATE:
    {
        ptedit_entry_t vm_user;
        (void)from_user(&vm_user, (void*)ioctl_param, sizeof(vm_user));
        update_vm(&vm_user, !mm_is_locked);
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_VM_LOCK:
    {
        struct mm_struct *mm = current->active_mm;
        if(mm_is_locked) {
            pr_warn("VM is already locked\n");
            return -1;
        }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
        mmap_write_lock(mm);
        mmap_read_lock(mm);
#else
        down_write(&mm->mmap_sem);
        down_read(&mm->mmap_sem);
#endif
        mm_is_locked = true;
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_VM_UNLOCK:
    {
        struct mm_struct *mm = current->active_mm;
        if(!mm_is_locked) {
            pr_warn("VM is not locked\n");
            return -1;
        }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
        mmap_write_unlock(mm);
        mmap_read_unlock(mm);
#else
        up_write(&mm->mmap_sem);
        up_read(&mm->mmap_sem);
#endif
        mm_is_locked = false;
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_READ_PAGE:
    {
        ptedit_page_t page;
        (void)from_user(&page, (void*)ioctl_param, sizeof(page));
        to_user(page.buffer, phys_to_virt(page.pfn * real_page_size), real_page_size);
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_WRITE_PAGE:
    {
        ptedit_page_t page;
        (void)from_user(&page, (void*)ioctl_param, sizeof(page));
        (void)from_user(phys_to_virt(page.pfn * real_page_size), page.buffer, real_page_size);
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_GET_ROOT:
    {
        struct mm_struct *mm;
        ptedit_paging_t paging;

        (void)from_user(&paging, (void*)ioctl_param, sizeof(paging));
        mm = get_mm(paging.pid);

#if defined(__aarch64__)
        if(!mm || (mm && !mm->pgd)) {
            // M1 Asahi Linux workaround with the limitation that it only works for the current process
            asm volatile("mrs %0, ttbr0_el1" : "=r" (paging.root));
            paging.root &= ~1;
            (void)to_user((void*)ioctl_param, &paging, sizeof(paging));
            return 0;
        }
#endif

        if(!mm) return 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
        if(!mm_is_locked) mmap_read_lock(mm);
#else
        if(!mm_is_locked) down_read(&mm->mmap_sem);
#endif
        paging.root = virt_to_phys(mm->pgd);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
        if(!mm_is_locked) mmap_read_unlock(mm);
#else
        if(!mm_is_locked) up_read(&mm->mmap_sem);
#endif
        (void)to_user((void*)ioctl_param, &paging, sizeof(paging));
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_SET_ROOT:
    {
        struct mm_struct *mm;
        ptedit_paging_t paging = {0};

        (void)from_user(&paging, (void*)ioctl_param, sizeof(paging));
        mm = get_mm(paging.pid);
        if(!mm) return 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
        if(!mm_is_locked) mmap_write_lock(mm);
#else
        if(!mm_is_locked) down_write(&mm->mmap_sem);
#endif
        mm->pgd = (pgd_t*)phys_to_virt(paging.root);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
        if(!mm_is_locked) mmap_write_unlock(mm);
#else
        if(!mm_is_locked) up_write(&mm->mmap_sem);
#endif
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_GET_PAGESIZE:
        return real_page_size;
    case PTEDITOR_IOCTL_CMD_INVALIDATE_TLB:
        invalidate_tlb(ioctl_param);
        return 0;
    case PTEDITOR_IOCTL_CMD_GET_PAT:
    {
#if defined(__i386__) || defined(__x86_64__)
        int low, high;
        size_t pat;
        asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(0x277));
        pat = low | (((size_t)high) << 32);
        (void)to_user((void*)ioctl_param, &pat, sizeof(pat));
        return 0;
#elif defined(__aarch64__)
        uint64_t value;
        asm volatile ("mrs %0, mair_el1\n" : "=r"(value));
        (void)to_user((void*)ioctl_param, &value, sizeof(value));
        return 0;
#endif
    }
    case PTEDITOR_IOCTL_CMD_SET_PAT:
    {
        set_pat(ioctl_param);
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_SWITCH_TLB_INVALIDATION:
    {
      if((int)ioctl_param != PTEDITOR_TLB_INVALIDATION_KERNEL && (int)ioctl_param != PTEDITOR_TLB_INVALIDATION_CUSTOM)
        return -1;
      invalidate_tlb = ((int)ioctl_param == PTEDITOR_TLB_INVALIDATION_KERNEL) ? invalidate_tlb_kernel : invalidate_tlb_custom;
      return 0;
    }

    default:
        return -1;
  }

  return 0;
}

static struct file_operations f_ops = {.owner = THIS_MODULE,
                                       .unlocked_ioctl = device_ioctl,
                                       .open = device_open,
                                       .release = device_release};

static struct miscdevice misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = PTEDITOR_DEVICE_NAME,
    .fops = &f_ops,
    .mode = S_IRWXUGO,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static struct proc_ops umem_ops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
  .proc_flags = 0,
#endif
  .proc_open = NULL,
  .proc_read = NULL,
  .proc_write = NULL,
  .proc_lseek = NULL,
  .proc_release = NULL,
  .proc_poll = NULL,
  .proc_ioctl = NULL,
#ifdef CONFIG_COMPAT
  .proc_compat_ioctl = NULL,
#endif
  .proc_mmap = NULL,
  .proc_get_unmapped_area = NULL,
};
#define OP_lseek lseek
#define OPCAT(a, b) a ## b
#define OPS(o) OPCAT(umem_ops.proc_, o)
#else
static struct file_operations umem_ops = {.owner = THIS_MODULE};
#define OP_lseek llseek
#define OPS(o) umem_ops.o
#endif

static int open_umem(struct inode *inode, struct file *filp) { return 0; }
static int has_umem = 0;

static const char *devmem_hook = "devmem_is_allowed";


static int devmem_bypass(struct kretprobe_instance *p, struct pt_regs *regs) {
#if defined(__aarch64__)
  if (regs->regs[0] == 0) {
    regs->regs[0] = 1;
  }
#else
  if (regs->ax == 0) {
    regs->ax = 1;
  }
#endif
  return 0;
}

static struct kretprobe probe_devmem = {.handler = devmem_bypass, .maxactive = 20};

static int __init pteditor_init(void) {
  int r;
#if defined(__aarch64__)
  uint64_t tcr_el1;
#endif

#ifdef KPROBE_KALLSYMS_LOOKUP
    register_kprobe(&kp);
    kallsyms_lookup_name = (kallsyms_lookup_name_t) kp.addr;
    unregister_kprobe(&kp);

    if(!unlikely(kallsyms_lookup_name)) {
      pr_alert("Could not retrieve kallsyms_lookup_name address\n");
      return -ENXIO;
    }
#endif

  /* Register device */
  r = misc_register(&misc_dev);
  if (r != 0) {
    pr_alert("Failed registering device with %d\n", r);
    return -ENXIO;
  }

#if defined(__i386__) || defined(__x86_64__)
  flush_tlb_mm_range_func = (void *) kallsyms_lookup_name("flush_tlb_mm_range");
  if(!flush_tlb_mm_range_func) {
    pr_alert("Could not retrieve flush_tlb_mm_range function\n");
    return -ENXIO;
  }
#endif
  invalidate_tlb = invalidate_tlb_kernel;
  
#if defined(__i386__) || defined(__x86_64__)
  if (!cpu_feature_enabled(X86_FEATURE_INVPCID_SINGLE)) {
    native_write_cr4_func = (void *) kallsyms_lookup_name("native_write_cr4");
    if(!native_write_cr4_func) {
        pr_alert("Could not retrieve native_write_cr4 function\n");
        return -ENXIO;
    }
  }
#endif

#if defined(__aarch64__)
  asm volatile("mrs %0, tcr_el1" : "=r" (tcr_el1));
  switch((tcr_el1 >> 14) & 3) {
      case 1:
          // 64k pages
          real_page_size = 64 * 1024;
          real_page_shift = 16;
          break;
      case 2:
          // 16k pages
          real_page_size = 16 * 1024;
          real_page_shift = 14;
          break;
      default:
          break;
  }
#endif

  probe_devmem.kp.symbol_name = devmem_hook;

  if (register_kretprobe(&probe_devmem) < 0) {
    pr_alert("Could not bypass /dev/mem restriction\n");
  } else {
    pr_info("/dev/mem is now superuser read-/writable\n");
  }

  OPS(OP_lseek) = (void*)kallsyms_lookup_name("memory_lseek");
  OPS(read) = (void*)kallsyms_lookup_name("read_mem");
  OPS(write) = (void*)kallsyms_lookup_name("write_mem");
  OPS(mmap) = (void*)kallsyms_lookup_name("mmap_mem");
  OPS(open) = open_umem;

  if (!OPS(OP_lseek) || !OPS(read) || !OPS(write) ||
      !OPS(mmap) || !OPS(open)) {
    pr_alert("Could not create unprivileged memory access\n");
  } else {
    proc_create("umem", 0666, NULL, &umem_ops);
    pr_info("Unprivileged memory access via /proc/umem set up\n");
    has_umem = 1;
  }
  pr_info("Loaded.\n");

  return 0;
}

static void __exit pteditor_exit(void) {
  misc_deregister(&misc_dev);
  
  unregister_kretprobe(&probe_devmem);

  if (has_umem) {
    pr_info("Remove unprivileged memory access\n");
    remove_proc_entry("umem", NULL);
  }
  pr_info("Removed.\n");
}

module_init(pteditor_init);
module_exit(pteditor_exit);
