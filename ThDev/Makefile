KERNEL_DIR ?= /lib/modules/`uname -r`/build

obj-m = example.o
example-objs = ex_module.o ex_dev.o ex_thread.o

all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` clean
