###Makefile to Compile Kernel Module
###Author : Alok Prasad
###Usage  : make module=<module_filename(without.c)>
#usage make module=<file>
obj-m += $(module).o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
