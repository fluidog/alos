TARGAT = libchar.a

OBJS = vga.o  keyboard.o

$(TARGAT) : $(OBJS)
	$(AR) $(ARFLAG) $(TARGAT) $(OBJS)

%.o : %.c
	$(CC) -c $(CFLAG)   -o $@ $<

	