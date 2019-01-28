TARGAT = libmm.a

OBJS =  page.o segment.o memory.o

$(TARGAT) : $(OBJS)	
	$(AR) $(ARFLAG) $(TARGAT) $(OBJS)

%.o : %.c
	$(CC) -c $(CFLAG)   -o $@ $<

	