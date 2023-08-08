KERNEL ?= $(shell uname -r)
obj-m += pteditor.o
ccflags-y += -Wno-unused-result
all:
	make -C /lib/modules/${KERNEL}/build M=$(PWD) modules
clean:
	make -C /lib/modules/${KERNEL}/build M=$(PWD) clean
