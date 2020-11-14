obj-m +=Lab1.o
obj-m +=Lab2.o
obj-m +=Assignment1.o
obj-m +=Assignment2.o
PWD :=$(shell pwd)
KERNEL_PATH :=/usr/src/linux-headers-$(shell uname -r)

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean