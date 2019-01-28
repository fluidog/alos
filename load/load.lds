SECTIONS { 
. = 0x00000000 ;

.text  : {*(.text)}
.rodata ALIGN(4) : {*(.rodata)} 
.data  ALIGN(4): { *(.data) }

__bss_start = . ;
.bss : {*(.bss)}
_end = . ;

} 