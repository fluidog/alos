DEV :=1vmdisk/1-flat.vmdk
#DEV :=/dev/sdb
target:
	mkfs.ext2 -b 1k -I 128 $(DEV)
	mount $(DEV) os
	mkdir os/bin
	sync
	umount os 
	make -C os/
	make -C boot/ext_boot/
	make -C load
	make -C bin/shell
	make -C bin/ls
