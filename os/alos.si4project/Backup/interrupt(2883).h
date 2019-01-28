
typedef struct {
	u16	base_low;
	u16	selector;
	u8	res;
	u8	access;
	u16	base_high;
} __attribute__ ((packed))idt_entry;

typedef struct{
	u16 limit;
	u32 gdt_baseAddr;
}__attribute__((packed))idtr_t;



typedef void irq_func_t(void *);

int interrupt_init(void);
void irq_install_handler(int ino, irq_func_t *func, void *pdata);


