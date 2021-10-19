obj-m += the_usctm.o
the_usctm-objs += ./SyscallTableDiscoveryAndManagment/usctm.o 
the_usctm-objs += ./SyscallTableDiscoveryAndManagment/lib/vtpmo.o
the_usctm-objs += ./SysCalls/syscall.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

