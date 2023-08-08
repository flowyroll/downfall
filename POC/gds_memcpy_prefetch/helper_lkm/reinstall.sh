make clean && make && sudo rmmod gds_helper_lkm && sudo insmod gds_helper_lkm.ko
objdump -D gds_helper_lkm.ko > gds_helper_lkm.dis
