TARGAT = libblock.a

OBJS = ide.o

TARGAT : $(OBJS)
	$(AR) $(ARFLAGS) $(TARGAT) $(OBJS)

%.o : %.c
	$(CC) -c $(CFLAGS)   -o $@ $<

	