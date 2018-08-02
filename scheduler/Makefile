KERNEL_DIR ?= /lib/modules/`uname -r`/build

obj-m = scheduler.o
scheduler-objs = ex_module.o ex_dev.o ex_thread.o


ifeq ($(DEBUG),y)
$(info ciao)
KBUILD_CFLAGS   += -DDEBUG=1  # THE_REAL_DEAL now '1' in C files.
endif

all:
	gcc -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` clean
