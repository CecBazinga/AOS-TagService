echo "running command: sudo insmod tag_service_module.ko"
sudo insmod tag_service_module.ko
sudo mknod /dev/tag_system_device c 237 0 

