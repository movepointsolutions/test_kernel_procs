include Kbuild

all: test_kernel_procs.ko

mknod:
	sudo mknod tkp c 100 0

modprobe: all
	sudo rmmod test_kernel_procs || true
	sudo insmod test_kernel_procs.ko

test_kernel_procs.ko: test_kernel_procs.c test_kernel_procs.h
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

.PHONY: modprobe mknod
