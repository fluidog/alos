OBJS:=load.o ext2.o fat.o memory.o  printf.o vsprintf.o ctype.o

CFALG:=-I include -fno-asynchronous-unwind-tables  -fno-builtin -fno-stack-protector
TEXT_BASE:=0x10000
LDFLAG:=-Ttext $(TEXT_BASE) -T load.lds
#DEV := /dev/sdb
DEV := ../1vmdisk/1-flat.vmdk
DIR := include/
	
load.bin:$(OBJS) 
	ld $(LDFLAG) -nostartfiles -e main -o load.elf $^
	objcopy -O binary -S load.elf $@
	mount $(DEV)  $(DIR)
	cp $@ $(DIR)
	umount $(DIR)

%.o:%.c
	$(CC) $(CFALG) -c $< 


.PHONY :clean load.bin
clean :
	rm -f *.o load.bin load.elf 
