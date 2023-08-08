/* See LICENSE file for license and copyright information */

#ifndef GDS_HELPER_LKM_MODULE_H
#define GDS_HELPER_LKM_MODULE_H

#define GDS_HELPER_LKM_DEVICE_NAME "gds_helper_lkm"
#define GDS_HELPER_LKM_DEVICE_PATH "/dev/" GDS_HELPER_LKM_DEVICE_NAME

#define GDS_HELPER_LKM_IOCTL_MAGIC_NUMBER (long) 'B'

#define GDS_HELPER_LKM_IOCTL_OOB_GADGET _IOR(GDS_HELPER_LKM_IOCTL_MAGIC_NUMBER, 1, struct gds_helper_lkm_params)

struct gds_helper_lkm_params
{
    unsigned long ulong1;
    unsigned long ulong2;     
};

#endif // GDS_HELPER_LKM_MODULE_H