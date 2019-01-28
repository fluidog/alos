TARGAT = libcmos.a

OBJS = mktime.o

$(TARGAT) : $(OBJS)
	$(AR) $(ARFLAG) $(TARGAT) $(OBJS)

%.o : %.c
	$(CC) -c $(CFLAG)   -o $@ $<