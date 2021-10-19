echo "running command: sudo rmmod the_usctm"
sudo rmmod the_usctm 
echo "cleaning environment..."
sudo find . -type f \( -iname \*.cmd -o -iname \*.symvers -o -iname \*.order -o -iname \*.ko -o -iname \*.o -o -iname \*.mod -o -iname \*.mod.c \) -delete 
echo "done !"