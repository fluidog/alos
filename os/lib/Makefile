TARGAT = libsys.a

OBJS = ctype.o malloc.o
$(TARGAT) : $(OBJS) 
	$(AR) $(ARFLAG) $(TARGAT) $(OBJS)

%.o : %.c
	$(CC) -c $(CFLAG)   -o $@ $<

