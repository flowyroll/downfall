#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include "../ptedit_header.h"

#define NOP16                                                                  \
  asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nn" \
               "op\nnop\nnop\nnop\n");
#define NOP256                                                                 \
  NOP16 NOP16 NOP16 NOP16 NOP16 NOP16 NOP16 NOP16 NOP16 NOP16 NOP16 NOP16      \
      NOP16 NOP16 NOP16 NOP16
#define NOP4K                                                                  \
  NOP256 NOP256 NOP256 NOP256 NOP256 NOP256 NOP256 NOP256 NOP256 NOP256 NOP256 \
      NOP256 NOP256 NOP256 NOP256 NOP256

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RESET "\x1b[0m"

#define TAG_OK COLOR_GREEN "[+]" COLOR_RESET " "
#define TAG_FAIL COLOR_RED "[-]" COLOR_RESET " "
#define TAG_PROGRESS COLOR_YELLOW "[~]" COLOR_RESET " "

void nx_function() {
  NOP4K
  printf("Hello\n");
  NOP4K
}

#if defined(__i386__) || defined(__x86_64__)
#define NX_BIT PTEDIT_PAGE_BIT_NX
#elif defined(__aarch64__)
#define NX_BIT PTEDIT_PAGE_BIT_XN
#endif



int main(int argc, char *argv[]) {
  /* Get 4kb-aligned pointer to function */
  void *nx_function_aligned = (void *)((((size_t)nx_function) + 4096) & ~0xfff);

  printf(TAG_PROGRESS "Expect 'Hello': ");
  nx_function();

  /* Make function non-executable (calling it now leads to crash) */
  mprotect(nx_function_aligned, 4096, PROT_READ);

  pid_t pid = fork();

  if (pid) {
    /* parent process */
    if (ptedit_init()) {
      printf(TAG_FAIL "Error: Could not initalize PTEditor, did you load the kernel module?\n");
      return 1;
    }

    /* wait for child to access the function s.t. it has its own copy (-> copy
     * on write) */
    sleep(1);

    /* verify that child's copy is non-executable */
    printf(TAG_PROGRESS "Child entry should have NX bit set\n");

    if (ptedit_pte_get_bit(nx_function_aligned, pid, NX_BIT)) {
      printf(TAG_OK "Child mapping is non-executable\n");
    } else {
      printf(TAG_FAIL "Child mapping is executable\n");
    }

    /* clear the non-executable (NX) bit and update child's page-table entry */
    printf(TAG_PROGRESS "Clearing child's NX bit...\n");
    ptedit_pte_clear_bit(nx_function_aligned, pid, NX_BIT);

    printf(TAG_PROGRESS "Check NX bit of child\n");

    if (ptedit_pte_get_bit(nx_function_aligned, pid, NX_BIT)) {
      printf(TAG_FAIL "Child mapping is still non-executable\n");
    } else {
      printf(TAG_OK "Child mapping is executable\n");
    }

    /* verify that own page-tabel entry is still non-executable */
    printf(TAG_OK "Own entry should have NX bit set\n");

    if (ptedit_pte_get_bit(nx_function_aligned, 0, NX_BIT)) {
      printf(TAG_OK "Own mapping is non-executable\n");
    } else {
      printf(TAG_FAIL "Own mapping is executable\n");
    }

    ptedit_cleanup();

    /* wait for child */
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      printf(TAG_OK "Success\n");
    } else {
      printf(TAG_FAIL "Fail\n");
    }
    printf(TAG_OK "Done\n");
  } else {
    // child
    if (*(unsigned char *)nx_function_aligned) {
      printf(TAG_OK "Function is mapped\n");
    } else {
      printf(TAG_FAIL "Function is not mapped in child!\n");
    }
    sleep(2);
    printf(TAG_PROGRESS "Executing previously non-executable function\n");
    nx_function();
    printf(TAG_OK "Child exited normally!\n");
  }
}
