/*
 * 'kernel.h' contains some often-used function prototypes etc
 */
void verify_area(void * addr,int count);
void panic(const char * str);
//int printk(const char * fmt, ...);
//#define printf(format,args...)	printk(format,##args)
int printk(const char * fmt, ...);
int tty_write(unsigned ch,char * buf,int count);
int tty_read(unsigned ch,char * buf,int count);
void * malloc(unsigned int size);
void *realloc(void *addr, unsigned int newSize);
signed char free(void *addr);
//void free_s(void * obj, int size);
//#define free(x) free_s((x), 0)

void prb(unsigned char *addr);

void display_vmalloc_area(void);

#define DEBUG(format, args...)
//printk(format,##args)

/*
 * This is defined as a macro, but at some point this might become a
 * real subroutine that sets a flag if it returns true (to do
 * BSD-style accounting where the process is flagged if it uses root
 * privs).  The implication of this is that you should do normal
 * permissions checks first, and check suser() last.
 */
#define suser() (current->euid == 0)

