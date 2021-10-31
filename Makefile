obj-m += tag_service_module.o
tag_service_module-objs += ./SyscallTableDiscoveryAndManagment/lib/usctm.o 
tag_service_module-objs += ./SyscallTableDiscoveryAndManagment/lib/vtpmo.o
tag_service_module-objs += ./SyscallTableDiscoveryAndManagment/lib/syscall.o
tag_service_module-objs += ./SyscallTableDiscoveryAndManagment/lib/device_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


