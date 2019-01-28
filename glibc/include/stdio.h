#ifndef _STDIO_H_
#define _STDIO_H_

//标准格式输出
int printf(const char * fmt, ...);
int puts( char *s);
int putc(char c);


//获得最多count个字符，遇到'\n'提前结束，返回获得字符数
int scanf(char *buf,int count);

#endif

