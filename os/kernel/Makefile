TARGAT = libkernel.a

OBJS = system_call.o printk.o vsprintf.o interrupts.o \
		chr_drv/tty_io.o chr_drv/console.o \
		panic.o sched.o fork.o \
		sys.o exit.o signal.o

$(TARGAT) : $(OBJS)
	$(AR) $(ARFLAG) $(TARGAT) $(OBJS)

%.o : %.c
	$(CC) -c $(CFLAG)   -o $@ $<

	