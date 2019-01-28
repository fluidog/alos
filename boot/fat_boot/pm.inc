;描述符
;usage:	Descriptor Base,Limit,Attr
;		Base:	dd
;		Limit:	dd(low 20 bits available)
;		Attr:	dw(lower 4 bits of higher byte are always 0)

;共八字节

%macro Descriptor 3
	dw %2 & 0ffffH
	dw %1 & 0ffffH
	db (%1 >> 16) & 0ffH
	
	dw ((%2 >> 8) & 0f00H) | (%3 & 0f0ffH)
	db (%1 >> 24) & 0ffH
%endmacro

%define DA_DRW	0100000010010010B
%define DA_LDT  0100000010000010B

%define DA_CODE		0x409a
%define DA_DATA 	0x4092
%define	DA_STACK	0x4096