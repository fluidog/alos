TARGAT = libblock.a

OBJS = ide.o ll_rw_blk.o

$(TARGAT) : $(OBJS)
	$(AR) $(ARFLAG) $(TARGAT) $(OBJS)

%.o : %.c
	$(CC) -c $(CFLAG)   -o $@ $<

	