TARGAT = libalosc.a

OBJS = printf.o vsprintf.o scanf.o ctype.o file.o process.o errno.o
CFLAG = -I ./include/  -fno-builtin -fno-asynchronous-unwind-tables -fno-stack-protector
$(TARGAT) : $(OBJS)
	ar rcs $(TARGAT) $(OBJS)

%.o : %.c
	gcc -c $(CFLAG)   -o $@ $<

clean:
	rm *.o *.a
