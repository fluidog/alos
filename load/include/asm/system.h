#include<types.h>

	
#define _set_gate(gate_addr,type,dpl,addr) do{ \
	*(u32 *)(gate_addr)=((addr)&0xffff) | 0x00080000;	\
	*((u32 *)(gate_addr)+1)=0x8000 | ((addr)&0xffff0000) | \
			(type)<<8 | (dpl)<<13;	\
	}while(0)


#define _set_seg_desc(gate_addr,type,dpl,base,limit) do{ \
	*(u32 *)(gate_addr)=((base) << 16) | ((limit) & 0xffff); \
	*((u32 *)(gate_addr)+1)=((base) & 0xff000000) | \
		(((base) >> 16) & 0xff) | \
		((dpl)<<13) | ((type)<<8) | ((limit) & 0x0f0000) | \
		(0x00c09000); }while(0)	


