KERNEL ?= $(shell uname -r)
obj-m += gds_helper_lkm.o
ccflags-y += -Wno-unused-result -Os
all:
	make -C /lib/modules/${KERNEL}/build M=$(PWD) modules
clean:
	make -C /lib/modules/${KERNEL}/build M=$(PWD) clean


