#include <malloc.h>
#include <asm/io.h>
#include <drivers/i8259.h>
#include<linux/kernel.h>
#include<asm/interrupt.h>
#include<string.h>

#include<asm/system.h>


asm(".text\n.global timer_interrupt\n"
	"timer_interrupt:\n"
	"push %eax\n"
	"push %ebx\n"
	"push %ecx\n"
	"push %edx\n"
	"push %ds\n"
	"push %es\n"
	"push %fs\n"
	"mov $0x10,%ax\n"
	"mov %ax,%ds\n"
	"mov %ax,%es\n"
	"mov $0x23,%ax\n"
	"mov %ax,%fs\n"
	"mov $0x20,%al\n"	//发送非特指EOI(0x20)到0x20端口,结束硬件中断
	"out %al,$0x20\n"
	"mov 32(%esp),%eax\n" //求中断发生前的特权级 
	"and $3,%eax\n"
	"push %eax\n"
	"call do_timer\n"	//do_timer(int cpl)
	"add $4,%esp\n"
	"pop %fs\n"
	"pop %es\n"
	"pop %ds\n"
	"pop %edx\n"
	"pop %ecx\n"
	"pop %ebx\n"
	"pop %eax\n"
	"iret\n");


static idt_entry idt[0x88];//0x20个异常 0x10个中断
static idtr_t idtr={0x88*8,(u32)idt};

#define MAX_IRQ 16


#define IRQ_DISABLED   1

typedef struct {
	irq_func_t *isr_func;
	void *isr_data;
	unsigned long status;
} irq_desc_t;

static irq_desc_t irq_table[MAX_IRQ];



#define DECLARE_EXCEPTION(x, f) 	\
	asm(".text\n.global exp_"#x"\n"			\
		    "exp_"#x":\n"			\
		    "pusha\n"				\
			"pushl		36(%esp)\n"	\
			"pushl		36(%esp)\n"	\
		    "pushl		$"#x"\n"	\
		    "pushl		$exp_return\n"\
		    "jmp "#f"\n");\
	void __attribute__ ((regparm(0))) exp_##x(void)
	
#define DECLARE_EXCEPTION_ERRORCODE(x, f) 	\
		asm(".text\n.global exp_"#x"\n" 		\
				"exp_"#x":\n"			\
				"pusha\n"	\
		 /*seg*/"pushl		40(%esp)\n" \
		  /*ip*/"pushl		40(%esp)\n" \
   /*errorcode*/"pushl		40(%esp)\n"	\
				"pushl		$exp_return_errorcode\n"\
				"jmp "#f"\n");\
		void __attribute__ ((regparm(0))) exp_##x(void)

asm ("exp_return:\n"
     	"addl  $12, %esp\n"
     	"popa\n"
     	"iret\n");
asm ("exp_return_errorcode:\n"
     	"addl  $12, %esp\n"
     	"popa\n"
     	"add $4,%esp\n"//errror code
     	"iret\n");



#define DECLARE_INTERRUPT(x)	\
	asm(".text\n.global irq_"#x"\n" 	\
			"irq_"#x":\n"		\
		    "pusha \n"			\
		    "pushl $"#x"\n"		\
		    "pushl $irq_return\n" \
		    "jmp   do_irq\n"); \
	void __attribute__ ((regparm(0))) irq_##x(void)

asm ("irq_return:\n"
     	"addl  $4, %esp\n"
     	"popa\n"
     	"iret\n");



	
DECLARE_EXCEPTION(0, divide_exception_entry);      /* Divide exception */
DECLARE_EXCEPTION(1, debug_exception_entry);       /* Debug exception */
DECLARE_EXCEPTION(2, nmi_entry);                   /* NMI */
DECLARE_EXCEPTION(3, unknown_exception_entry);     /* Breakpoint/Coprocessor Error */
DECLARE_EXCEPTION(4, unknown_exception_entry);     /* Overflow */
DECLARE_EXCEPTION(5, unknown_exception_entry);     /* Bounds */
DECLARE_EXCEPTION(6, invalid_instruction_entry);   /* Invalid instruction */
DECLARE_EXCEPTION(7, unknown_exception_entry);     /* Device not present */
DECLARE_EXCEPTION(8, double_fault_entry);          /* Double fault */
DECLARE_EXCEPTION(9, unknown_exception_entry);     /* Co-processor segment overrun */
DECLARE_EXCEPTION(10, invalid_tss_exception_entry);/* Invalid TSS */
DECLARE_EXCEPTION(11, seg_fault_entry);            /* Segment not present */
DECLARE_EXCEPTION(12, stack_fault_entry);          /* Stack overflow */
DECLARE_EXCEPTION_ERRORCODE(13, gpf_entry);                  /* GPF */

extern void page_fault(int errorcode,u32 ip,u16 seg);
DECLARE_EXCEPTION_ERRORCODE(14, page_fault);           /* PF */
DECLARE_EXCEPTION(15, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(16, fp_exception_entry);         /* Floating point */
DECLARE_EXCEPTION(17, alignment_check_entry);      /* alignment check */
DECLARE_EXCEPTION(18, machine_check_entry);        /* machine check */
DECLARE_EXCEPTION(19, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(20, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(21, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(22, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(23, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(24, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(25, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(26, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(27, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(28, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(29, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(30, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(31, unknown_exception_entry);    /* Reserved */

DECLARE_INTERRUPT(0); //Timer & Counter
DECLARE_INTERRUPT(1);
DECLARE_INTERRUPT(3);
DECLARE_INTERRUPT(4);
DECLARE_INTERRUPT(5);
DECLARE_INTERRUPT(6);
DECLARE_INTERRUPT(7);
DECLARE_INTERRUPT(8);
DECLARE_INTERRUPT(9);
DECLARE_INTERRUPT(10);
DECLARE_INTERRUPT(11);
DECLARE_INTERRUPT(12);
DECLARE_INTERRUPT(13);
DECLARE_INTERRUPT(14);
DECLARE_INTERRUPT(15);

void __attribute__ ((regparm(0))) default_isr(void);
asm ("default_isr: iret\n");

void disable_irq(int irq)
{
	if (irq >= MAX_IRQ) {
		return;
	}
	irq_table[irq].status |= IRQ_DISABLED;
}
void enable_irq(int irq)
{
	if (irq >= MAX_IRQ) {
		return;
	}
	irq_table[irq].status &= ~IRQ_DISABLED;
}

static void specific_eoi(int irq)
{
	/* If it is on the slave PIC this have to be performed on
	 * both the master and the slave PICs */
	if (irq > 7) {
		outb(OCW2_SEOI|(irq&7), SLAVE_PIC + OCW2);
		irq = SEOI_IR2;               /* also do IR2 on master */
	}
	outb(OCW2_SEOI|irq, MASTER_PIC + OCW2);
}


/* masks one specific IRQ in the PIC */
static void unmask_irq(int irq)
{
	int imr_port;

	if (irq >= MAX_IRQ) {
		return;
	}
	if (irq > 7) {
		imr_port = SLAVE_PIC + IMR;
	} else {
		imr_port = MASTER_PIC + IMR;
	}
	outb(inb(imr_port)&~(1<<(irq&7)), imr_port);
}

/* unmasks one specific IRQ in the PIC */
static void mask_irq(int irq)
{
	int imr_port;

	if (irq >= MAX_IRQ) {
		return;
	}
	if (irq > 7) {
		imr_port = SLAVE_PIC + IMR;
	} else {
		imr_port = MASTER_PIC + IMR;
	}

	outb(inb(imr_port)|(1<<(irq&7)), imr_port);
}
void  enable_interrupts(void)
{
	asm("sti\n");
}

int disable_interrupts(void)
{
	long flags;

	asm volatile ("pushfl ; popl %0 ; cli\n" : "=g" (flags) : );

	return (flags&0x200); /* IE flags is bit 9 */
}


/* issue a Specific End Of Interrupt instruciton */

void __attribute__ ((regparm(0))) do_irq(int irq)
{
	specific_eoi(irq);
	//mask_irq(irq);	
	if (irq_table[irq].status & IRQ_DISABLED) {
		unmask_irq(irq);
		specific_eoi(irq);
		return;
	}
	

	if (NULL != irq_table[irq].isr_func) {        
		irq_table[irq].isr_func(irq_table[irq].isr_data);
	} else {
		printf("Spurious irq %d\n", irq);
	}	
	//unmask_irq(irq);
	
}


void __attribute__ ((regparm(0))) unknown_exception_entry(int cause, int ip, int seg)
{
	printf("Unknown Exception %d at %04x:%08x\n", cause, seg, ip);
}

void __attribute__ ((regparm(0))) divide_exception_entry(int cause, int ip, int seg)
{
	printf("Divide Error (Division by zero) at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) debug_exception_entry(int cause, int ip, int seg)
{
	printf("Debug Interrupt (Single step) at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) nmi_entry(int cause, int ip, int seg)
{
	printf("NMI Interrupt at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) invalid_instruction_entry(int cause, int ip, int seg)
{
	printf("Invalid Instruction at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) double_fault_entry(int cause, int ip, int seg)
{
	printf("Double fault at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) invalid_tss_exception_entry(int cause, int ip, int seg)
{
	printf("Invalid TSS at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) seg_fault_entry(int cause, int ip, int seg)
{
	printf("Segmentation fault at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) stack_fault_entry(int cause, int ip, int seg)
{
	printf("Stack fault at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) gpf_entry(int cause, int ip, int seg)
{
	printf("General protection fault at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) page_fault_entry(int cause, int ip, int seg)
{
	printf("Page fault at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) fp_exception_entry(int cause, int ip, int seg)
{
	printf("Floating point exception at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) alignment_check_entry(int cause, int ip, int seg)
{
	printf("Alignment check at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) machine_check_entry(int cause, int ip, int seg)
{
	printf("Machine check exception at %04x:%08x\n", seg, ip);
}


void irq_install_handler(int ino, irq_func_t *func, void *pdata)
{
	int status;	
	
	if (ino>MAX_IRQ) {
		return;
	}

	if (NULL != irq_table[ino].isr_func) {
		return;
	}

	status = disable_interrupts();
	irq_table[ino].isr_func=func;
	irq_table[ino].isr_data=pdata;
	enable_irq(ino);
	
	if (status) {
		enable_interrupts();
	}
	
	unmask_irq(ino);
	return;
}

void irq_free_handler(int ino)
{
	int status;
	if (ino>MAX_IRQ) {
		return;
	}

	status = disable_interrupts();
	mask_irq(ino);
	
	disable_irq(ino);
	irq_table[ino].isr_func=NULL;
	irq_table[ino].isr_data=NULL;
	

	if (status) {
		enable_interrupts();
	}
	return;
}

extern void system_call();
extern void timer_interrupt(void);

int interrupt_init(void)
{
	int i;

	/* Just in case... */
	disable_interrupts();

	/* Initialize the IDT and stuff */
	memset(idt,0,sizeof(idt));
	memset(irq_table, 0, sizeof(irq_table));
	
	/* Setup exceptions */
	set_trap_gate(0x00, exp_0);
	set_trap_gate(0x01, exp_1);
	set_trap_gate(0x02, exp_2);
	set_trap_gate(0x03, exp_3);
	set_trap_gate(0x04, exp_4);
	set_trap_gate(0x05, exp_5);
	set_trap_gate(0x06, exp_6);
	set_trap_gate(0x07, exp_7);
	set_trap_gate(0x08, exp_8);
	set_trap_gate(0x09, exp_9);
	set_trap_gate(0x0a, exp_10);
	set_trap_gate(0x0b, exp_11);
	set_trap_gate(0x0c, exp_12);
	set_trap_gate(0x0d, exp_13);
	set_trap_gate(0x0e, exp_14);
	set_trap_gate(0x0f, exp_15);
	set_trap_gate(0x10, exp_16);
	set_trap_gate(0x11, exp_17);
	set_trap_gate(0x12, exp_18);
	set_trap_gate(0x13, exp_19);
	set_trap_gate(0x14, exp_20);
	set_trap_gate(0x15, exp_21);
	set_trap_gate(0x16, exp_22);
	set_trap_gate(0x17, exp_23);
	set_trap_gate(0x18, exp_24);
	set_trap_gate(0x19, exp_25);
	set_trap_gate(0x1a, exp_26);
	set_trap_gate(0x1b, exp_27);
	set_trap_gate(0x1c, exp_28);
	set_trap_gate(0x1d, exp_29);
	set_trap_gate(0x1e, exp_30);
	set_trap_gate(0x1f, exp_31);

	/* Setup interrupts */
	set_intr_gate(0x20, timer_interrupt);
	set_intr_gate(0x21, irq_1);
	set_intr_gate(0x23, irq_3);
	set_intr_gate(0x24, irq_4);
	set_intr_gate(0x25, irq_5);
	set_intr_gate(0x26, irq_6);
	set_intr_gate(0x27, irq_7);
	set_intr_gate(0x28, irq_8);
	set_intr_gate(0x29, irq_9);
	set_intr_gate(0x2a, irq_10);
	set_intr_gate(0x2b, irq_11);
	set_intr_gate(0x2c, irq_12);
	set_intr_gate(0x2d, irq_13);
	set_intr_gate(0x2e, irq_14);
	set_intr_gate(0x2f, irq_15);
	/* vectors 0x30-0x3f are reserved for irq 16-31 */
	set_system_gate(0x80, system_call);
	
	asm ("lidt idtr\n");

	
	/* Mask all interrupts */
	outb(0xff, MASTER_PIC + IMR);
	outb(0xff, SLAVE_PIC + IMR);

	/* Master PIC */
	outb(ICW1_SEL|ICW1_EICW4, MASTER_PIC + ICW1);
	outb(0x20, MASTER_PIC + ICW2);          /* Place master PIC interrupts at INT20 */
	outb(IR2, MASTER_PIC + ICW3);		/* ICW3, One slevc PIC is present */
	outb(ICW4_PM, MASTER_PIC + ICW4);

	for (i=0;i<8;i++) {
		outb(OCW2_SEOI|i, MASTER_PIC + OCW2);
	}

	/* Slave PIC */
	outb(ICW1_SEL|ICW1_EICW4, SLAVE_PIC + ICW1);
	outb(0x28, SLAVE_PIC + ICW2);	        /* Place slave PIC interrupts at INT28 */
	outb(0x02, SLAVE_PIC + ICW3);		/* Slave ID */
	outb(ICW4_PM, SLAVE_PIC + ICW4);

	for (i=0;i<8;i++) {
		outb(OCW2_SEOI|i, SLAVE_PIC + OCW2);
	}


	/* enable cascade interrerupt */
	outb(0xfb, MASTER_PIC + IMR);
	outb(0xff, SLAVE_PIC + IMR);

	/* It is now safe to enable interrupts */
	enable_interrupts();
	unmask_irq(0);
	return 0;
}




#ifdef CFG_RESET_GENERIC

void __attribute__ ((regparm(0))) generate_gpf(void);
asm(".globl generate_gpf\n"
    "generate_gpf:\n"
    "ljmp   $0x70, $0x47114711\n"); /* segment 0x70 is an arbitrary segment which does not
				    * exist */
void reset_cpu(ulong addr)
{
	set_vector(13, generate_gpf);  /* general protection fault handler */
	set_vector(8, generate_gpf);   /* double fault handler */
	generate_gpf();                /* start the show */
}
#endif
