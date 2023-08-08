/* See LICENSE file for license and copyright information */

#ifndef PTEDITOR_MODULE_H
#define PTEDITOR_MODULE_H

#if defined(__linux__) || defined(__linux) || defined(__unix__) || defined(LINUX) || defined(UNIX)
#define LINUX
#endif
#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__CYGWIN__)
#define WINDOWS
#undef LINUX
#endif


#if defined(LINUX)
#define PTEDITOR_DEVICE_NAME "pteditor"
#define PTEDITOR_DEVICE_PATH "/dev/" PTEDITOR_DEVICE_NAME
#else
#define PTEDITOR_DEVICE_NAME L"PTEditorLink"
#define PTEDITOR_DEVICE_PATH L"\\\\.\\" PTEDITOR_DEVICE_NAME
#endif

/**
 * Structure containing the page-table entries of all levels.
 * The Linux names are aliased with the Intel names.
 */
typedef struct {
    /** Process ID */
    size_t pid;
    /** Virtual address */
    size_t vaddr;

    /** Page global directory / Page map level 5 */
    union {
        size_t pgd;
        size_t pml5;
    };
    /** Page directory 4 / Page map level 4 */
    union {
        size_t p4d;
        size_t pml4;
    };
    /** Page upper directory / Page directory pointer table */
    union {
        size_t pud;
        size_t pdpt;
    };
    /** Page middle directory / Page directory */
    union {
        size_t pmd;
        size_t pd;
    };
    /** Page table entry */
    size_t pte;
    /** Bitmask indicating which entries are valid/should be updated */
    size_t valid;
} ptedit_entry_t;

/**
 * Structure to read/write physical pages
 */
#if defined(LINUX)
typedef struct {
    /** Page-frame number */
    size_t pfn;
    /** Virtual address */
    size_t vaddr;
    /** Page size */
    size_t size;
    /** Page content */
    unsigned char* buffer;
} ptedit_page_t;
#else
__pragma(pack(push, 1))
typedef struct {
    char content[4096];
    size_t paddr;
} ptedit_page_t;
__pragma(pack(pop))
#endif


/**
 * Structure to get/set the root of paging
 */
typedef struct {
    /** Process id */
    size_t pid;
    /** Physical address of paging root */
    size_t root;
} ptedit_paging_t;

#define PTEDIT_VALID_MASK_PGD (1<<0)
#define PTEDIT_VALID_MASK_P4D (1<<1)
#define PTEDIT_VALID_MASK_PUD (1<<2)
#define PTEDIT_VALID_MASK_PMD (1<<3)
#define PTEDIT_VALID_MASK_PTE (1<<4)

#define PTEDITOR_TLB_INVALIDATION_KERNEL 0
#define PTEDITOR_TLB_INVALIDATION_CUSTOM 1

#if defined(LINUX)
#define PTEDITOR_IOCTL_MAGIC_NUMBER (long)0x3d17

#define PTEDITOR_IOCTL_CMD_VM_RESOLVE \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 1, size_t)

#define PTEDITOR_IOCTL_CMD_VM_UPDATE \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 2, size_t)

#define PTEDITOR_IOCTL_CMD_VM_LOCK \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 3, size_t)

#define PTEDITOR_IOCTL_CMD_VM_UNLOCK \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 4, size_t)

#define PTEDITOR_IOCTL_CMD_READ_PAGE \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 5, size_t)

#define PTEDITOR_IOCTL_CMD_WRITE_PAGE \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 6, size_t)

#define PTEDITOR_IOCTL_CMD_GET_ROOT \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 7, size_t)

#define PTEDITOR_IOCTL_CMD_SET_ROOT \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 8, size_t)

#define PTEDITOR_IOCTL_CMD_GET_PAGESIZE \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 9, size_t)

#define PTEDITOR_IOCTL_CMD_INVALIDATE_TLB \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 10, size_t)

#define PTEDITOR_IOCTL_CMD_GET_PAT \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 11, size_t)

#define PTEDITOR_IOCTL_CMD_SET_PAT \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 12, size_t)

#define PTEDITOR_IOCTL_CMD_SWITCH_TLB_INVALIDATION \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 13, size_t)
#else
#define PTEDITOR_READ_PAGE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_WRITE_PAGE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA)
#define PTEDITOR_GET_CR3 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_FLUSH_TLB CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_READ_PHYS_VAL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_WRITE_PHYS_VAL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_SET_CR3 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_SET_PAT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define PTEDITOR_GET_PAT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

#endif // PTEDITOR_MODULE_H
