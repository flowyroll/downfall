#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/filter.h>
#include "gds_helper_lkm.h"
#include "../config.h"

MODULE_AUTHOR("Daniel Moghimi");
MODULE_DESCRIPTION("Device to help with data sampling tests");
MODULE_LICENSE("GPL");

#ifdef asm_inline
#undef asm_inline
#define asm_inline asm
#endif

char __attribute__((aligned(PAGE_SIZE))) dest[PAGE_SIZE];
char __attribute__((aligned(PAGE_SIZE))) secret[PAGE_SIZE];
char __attribute__((aligned(PAGE_SIZE))) source[PAGE_SIZE];

static bool device_busy = false;

static int device_release(struct inode *inode, struct file *file)
{
  /* Unlock module */
  device_busy = false;

  return 0;
}

static int device_open(struct inode *inode, struct file *file)
{
  /* Check if device is busy */
  if (device_busy == true)
  {
    return -EBUSY;
  }

  device_busy = true;

  return 0;
}

static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
  long err = 0;
  unsigned long x;
  struct gds_helper_lkm_params *user_param = (struct gds_helper_lkm_params *)ioctl_param;
  struct gds_helper_lkm_params local_param;

  if (copy_from_user(&local_param, user_param, sizeof(struct gds_helper_lkm_params)))
  {
    return -EFAULT;
  }

  switch (ioctl_num)
  {
  case GDS_HELPER_LKM_IOCTL_OOB_GADGET:

#if KERNEL_GADGET_SAFE
    // Gadget 1: Safe
    if (local_param.ulong2 < PAGE_SIZE &&
        local_param.ulong2 + local_param.ulong1 < PAGE_SIZE)
    {
      memcpy(dest, source + local_param.ulong1, local_param.ulong2);
    }
#elif KERNEL_GADGET_SAFEZ
    // Gadget 2: Safe zero
    x = local_param.ulong2;
    if (local_param.ulong2 >= PAGE_SIZE ||
        local_param.ulong2 + local_param.ulong1 >= PAGE_SIZE)
      x = 0;
    memcpy(dest, source + local_param.ulong1, x);
#elif KERNEL_GADGET_BUG
    // Gadget 3: Buggy, unexploitable
    if (local_param.ulong2 < PAGE_SIZE)
      memcpy(dest, source + local_param.ulong1, local_param.ulong2);
#endif

    break;

  default:
    err = EFAULT;
  }

  if (copy_to_user(user_param, &local_param, sizeof(local_param)))
    return -EFAULT;

  return err;
}

static const struct file_operations gds_helper_lkm_fops =
    {.owner = THIS_MODULE,
     .unlocked_ioctl = device_ioctl,
     .open = device_open,
     .release = device_release};

static struct miscdevice gds_helper_lkm_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = GDS_HELPER_LKM_DEVICE_NAME,
    .fops = &gds_helper_lkm_fops,
    .mode = S_IRWXUGO,
};

static int gds_helper_lkm_init(void)
{
  int ret;
  char *message = "Linus Torvalds; born 28 December 1969 is a Finnish-American soft"
                  "ware engineer who is the creator and, historically, the main dev"
                  "eloper of the Linux kernel, used by Linux distributions and othe"
                  "r operating systems such as Android. He also created the distrib"
                  "uted version control system Git.";

  printk(KERN_INFO "gds_helper_lkm: HELLO\n");
  memset(source, '*', PAGE_SIZE);

  memset(secret, 'X', PAGE_SIZE);
  memcpy(secret, message, strlen(message));

  ret = misc_register(&gds_helper_lkm_miscdev);
  if (ret)
  {
    printk(KERN_ERR "cannot register miscdev(err=%d)\n", ret);
    return ret;
  }

  return 0;
}

static void gds_helper_lkm_exit(void)
{

  // Remove the misc device
  misc_deregister(&gds_helper_lkm_miscdev);

  printk(KERN_INFO "gds_helper_lkm: BYE\n");
}

module_init(gds_helper_lkm_init);
module_exit(gds_helper_lkm_exit);
