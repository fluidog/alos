#ifndef _UTIME_H
#define _UTIME_H

#include <sys/types.h>		/* 我知道 - 不应该这样做，但是.. */

typedef struct utimbuf
{
  time_t actime;		// 文件访问时间。从1970.1.1:0:0:0 开始的秒数。
  time_t modtime;		// 文件修改时间。从1970.1.1:0:0:0 开始的秒数。
}utimbuf;


#endif
