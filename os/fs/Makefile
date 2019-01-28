TARGAT = libfs.a

OBJS = buffer.o block_dev.o file_dev.o char_dev.o exec.o

$(TARGAT) : $(OBJS)
	$(AR) $(ARFLAG) $(TARGAT) $(OBJS)

%.o : %.c
	$(CC) -c $(CFLAG)   -o $@ $<