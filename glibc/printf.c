#include<unistd.h>
#include<stdarg.h>
#include<string.h>
static char buf[1024];
extern int vsprintf(char * buf, const char * fmt, va_list args);
int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsprintf(buf,fmt,args);
	va_end(args);

	i=write(1,buf,i);
	return i;
}
int puts( char *s)
{
	return write(1,s,strlen(s));
}
int putc(char c)
{
	return write(1,&c,1);
}
