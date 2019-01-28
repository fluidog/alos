#include<time.h>
#include<sys/types.h>
#include<asm/io.h>
#include<linux/kernel.h>
#include<string.h>
#include<linux/config.h>
extern long startup_time;
static long kernel_mktime (struct tm *tm);
static void time_init(void);

struct
{
	//addr:0
	u8	sec;
	u8	alarm_sec;
	u8	min;
	u8 	alarm_min;
	u8	hour;
	u8	alarm_hour;
	u8	wday;//week day
	u8	mday;//month day
	u8	mon;
	u8	year;
	u8	stateA;
	u8	stateB;
	u8	stateC;
	u8	stateD;//0:电池无效 80:电池有效
	u8	diagnose;//诊断
	u8	shut_stat;//关机状态
	
	//addr:0x10
	u8	floppy;
	u8	reserve0;
	u8	hd;//high 4 bit:c driver low 4 bits:d driver  0--无 f--用户盘 其余系统盘
	u8	reserve1;
	u8	dev_stat;
	u16	mem;	//以KB为单位
	u16	ext_mem;
	u8	type_c;	//c盘类型
	u8	type_d;	//d盘类型
	u16	reserve2;

	//addr:0x1d
	u16	cyl_c;	// 柱面数
	u8	head_c;	// 磁头数
	u16	wpcom_c;// 写前预补偿柱面号
	u16	lzone_c;//磁头着陆区柱面号
	u8	sect_c;	//每磁道扇区数
	
	u16	cyl_d;	// 柱面数
	u8	head_d;	// 磁头数
	u16	wpcom_d;// 写前预补偿柱面号
	u16	lzone_d;//磁头着陆区柱面号
	u8	sect_d;	//每磁道扇区数
	
	//addr:0x2d
	u8	reserve3;
	u16	check;	//校验和  10~2D和，2E为高位
	u16	extend;	//扩展内存
	u8	century;//世纪
	u8	flag;
	u8	res[12];
} cmos;

static inline unsigned char CMOS_READ(unsigned char addr)
{
	outb_p(addr,0x70);
	return inb_p(0x71);
}

void cmos_init(void)
{
	memset((void *)&cmos,0,sizeof(cmos));
	//addr:0
	cmos.sec=CMOS_READ(0);
	cmos.alarm_sec=CMOS_READ(1);
	cmos.min=CMOS_READ(2);
	cmos.alarm_min=CMOS_READ(3);
	cmos.hour=CMOS_READ(4);
	cmos.alarm_hour=CMOS_READ(5);
	cmos.wday=CMOS_READ(6);//week day
	cmos.mday=CMOS_READ(7);//month day
	cmos.mon=CMOS_READ(8);
	cmos.year=CMOS_READ(9);
	cmos.stateA=CMOS_READ(0xa);
	cmos.stateB=CMOS_READ(0xb);
	cmos.stateC=CMOS_READ(0xc);
	cmos.stateD=CMOS_READ(0xd);//0:电池无效 80:电池有效
	cmos.diagnose=CMOS_READ(0xe);//诊断
	cmos.shut_stat=CMOS_READ(0xf);//关机状态
	
	//addr:0x10
	cmos.floppy=CMOS_READ(0x10);
	//cmos.reserve0=CMOS_READ(0x11);
	cmos.hd=CMOS_READ(0x12);//high 4 bit:c driver low 4 bits:d driver  0--无 f--用户盘 其余系统盘
	cmos.reserve1=CMOS_READ(0x13);
	cmos.dev_stat=CMOS_READ(0x14);
	cmos.mem=CMOS_READ(0x15);	//内存
	*(char *)((int)(&cmos.mem)+1)=CMOS_READ(0x16);
	cmos.ext_mem=CMOS_READ(0x17);	//扩展内存
	*(char *)((int)(&cmos.ext_mem)+1)=CMOS_READ(0x18);
	cmos.type_c=CMOS_READ(0x19);	//c盘类型
	cmos.type_d=CMOS_READ(0x1a);	//d盘类型
	//cmos.reserve2=CMOS_READ(0x1b);

	//addr:0x1d
	cmos.cyl_c=(u8)CMOS_READ(0x1d);	// 柱面数
	*(unsigned char *)((int)(&cmos.cyl_c)+1)=CMOS_READ(0x1e);
	cmos.head_c=CMOS_READ(0x1f);	// 磁头数
	cmos.wpcom_c=CMOS_READ(0x20);// 写前预补偿柱面号
	*(char *)((int)(&cmos.wpcom_c)+1)=CMOS_READ(0x21);
	cmos.lzone_c=CMOS_READ(0x22);//磁头着陆区柱面号
	*(char *)((int)(&cmos.lzone_c)+1)=CMOS_READ(0x23);
	cmos.sect_c=CMOS_READ(0x24);	//每磁道扇区数
	
	cmos.cyl_d=CMOS_READ(0x25);	// 柱面数
	*(unsigned char *)((int)(&cmos.cyl_d)+1)=CMOS_READ(0x26);
	cmos.head_d=CMOS_READ(0x27);	// 磁头数
	cmos.wpcom_d=CMOS_READ(0x28);// 写前预补偿柱面号
	*(unsigned char *)((int)(&cmos.wpcom_d)+1)=CMOS_READ(0x29);
	cmos.lzone_d=CMOS_READ(0x2a);//磁头着陆区柱面号
	*(unsigned char *)((int)(&cmos.lzone_d)+1)=CMOS_READ(0x2b);
	cmos.sect_d=CMOS_READ(0x2c);	//每磁道扇区数
	
	//addr:0x2d
	//cmos.reserve3=CMOS_READ(0x2d);
	cmos.check=CMOS_READ(0x2e);	//校验和  10~2D和，2E为高位
	*(unsigned char *)((int)(&cmos.check)+1)=CMOS_READ(0x2f);
	cmos.extend=CMOS_READ(0x30);	//扩展内存
	*(unsigned char *)((int)(&cmos.extend)+1)=CMOS_READ(0x31);
	cmos.century=CMOS_READ(0x32);//世纪
	cmos.flag=CMOS_READ(0x33);
	//cmos.res[12];
	

	/*			根据cmos参数设置系统参数					*/
	//内存
	HIGH_MEMORY=(cmos.ext_mem +1024)* 1024;//将KB转换为B，再加上低端1MB内存
	DEBUG("memory:%dMB\n",HIGH_MEMORY/1024/1024);

	//开机时间(startup_time)
	time_init();

	//磁盘参数(目前稍有问题)
		
}


// 将BCD 码转换成数字。
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
// 该子程序取CMOS 时钟，并设置开机时间 startup_time(为从1970-1-1-0 时起到开机时的秒数)。
static void time_init(void)
{	
	struct tm time;
	time.tm_sec = cmos.sec;
	time.tm_min = cmos.min;
	time.tm_hour = cmos.hour;
	time.tm_mday = cmos.mday;
	time.tm_mon = cmos.mon;
	time.tm_year = cmos.year;
	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_min);
	BCD_TO_BIN(time.tm_hour);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	
	DEBUG("20%d-%d-%d  %d:%d:%d\n",time.tm_year,time.tm_mon,time.tm_mday,\
				time.tm_hour,time.tm_min,time.tm_sec);

	time.tm_mon--;
	startup_time = kernel_mktime(&time);
	
}

#define MINUTE 60		// 1 分钟的秒数。
#define HOUR (60*MINUTE)	// 1 小时的秒数。
#define DAY (24*HOUR)		// 1 天的秒数。
#define YEAR (365*DAY)		// 1 年的秒数。

static int month[12] = {
  0,
  DAY * (31),
  DAY * (31 + 29),
  DAY * (31 + 29 + 31),
  DAY * (31 + 29 + 31 + 30),
  DAY * (31 + 29 + 31 + 30 + 31),
  DAY * (31 + 29 + 31 + 30 + 31 + 30),
  DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31),
  DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),
  DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
  DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
  DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)
};

// 该函数计算从1970 年1 月1 日0 时起到开机当日经过的秒数，作为开机时间。
static long kernel_mktime (struct tm *tm)
{
  long res;
  int year;

  //year = tm->tm_year - 70;	// 从70 年到现在经过的年数(2 位表示方式)，
  // 因此会有2000 年问题。
  year=tm->tm_year + 30;	// 从70 年到现在经过的年数(2 位表示方式)
  /* magic offsets (y+1) needed to get leapyears right. */
  /* 为了获得正确的闰年数，这里需要这样一个魔幻偏值(y+1) */
  res = YEAR * year + DAY * ((year + 1) / 4);	// 这些年经过的秒数时间 + 每个闰年时多1 天
  res += month[tm->tm_mon];	// 的秒数时间，在加上当年到当月时的秒数。
  /* and (y+2) here. If it wasn't a leap-year, we have to adjust */
  /* 以及(y+2)。如果(y+2)不是闰年，那么我们就必须进行调整(减去一天的秒数时间)。 */
  if (tm->tm_mon > 1 && ((year + 2) % 4))
    res -= DAY;
  res += DAY * (tm->tm_mday - 1);	// 再加上本月过去的天数的秒数时间。
  res += HOUR * tm->tm_hour;	// 再加上当天过去的小时数的秒数时间。
  res += MINUTE * tm->tm_min;	// 再加上1 小时内过去的分钟数的秒数时间。
  res += tm->tm_sec;		// 再加上1 分钟内已过的秒数。
  return res;			// 即等于从1970 年以来经过的秒数时间。
}




