TARGAT = libext2.a

OBJS = inode.o super.o namei.o open.o read_write.o bitmap.o \
			ioctl.o stat.o fcntl.o pipe.o

$(TARGAT) : $(OBJS)
	$(AR) $(ARFLAG) $(TARGAT) $(OBJS)

%.o : %.c
	$(CC) -c $(CFLAG)   -o $@ $<