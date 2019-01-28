TARGAT	= test
OBJS	= test.o
LDFLAG	= -Ttext 0x2000 

LIBS	= ../../glibc/libalosc.a 
CFLAG	= -fno-builtin -fno-asynchronous-unwind-tables -fno-stack-protector  -I ../../glibc/include/
DEV		= ../../1vmdisk/1-flat.vmdk
#DEV = /dev/sdb
DIR	= ../test/

$(TARGAT):$(OBJS) $(LIBS) 
	ld $(LDFLAG) -nostartfiles -e test \
		-o $@.elf $^
	objcopy -O binary  -S $@.elf $@
	mount $(DEV) $(DIR)
	cp $(TARGAT) $(DIR)bin/
	umount $(DIR)

	
%.o : %.c
	$(CC) -c $(CFLAG)   -o $@ $<
	
.PHONY : clean $(TARGAT)
clean:
	rm -f *.o *.elf *.bin test

