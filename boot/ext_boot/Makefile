IMG:=../../1vmdisk/1-flat.vmdk
#IMG:=/dev/sdb
boot.bin : boot.asm pm.inc
	nasm $< -o $@ -l boot.list
	dd if=$@ of=$(IMG) conv=notrunc

.PHONY : clean boot.bin
clean :
	rm -f boot.bin boot.list 
