KERNEL_DIR ?= /lib/modules/`uname -r`/build
NAME ?= -3

obj-m = misc-test$(NAME).o

all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` clean

