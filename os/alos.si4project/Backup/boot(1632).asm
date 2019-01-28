%include "pm.inc"
;目前未完成
org 7c00H
__start:
	mov sp,0x100
	mov si,StrHello
	call Display
	;加载目标文件
	mov si,LoadFileName			
	mov di,0x400		;加载到0x400
	call Loadfile
	;加载GdtPr
	mov dword [GdtPtr+2],LABEL_GDT ;gdt基址
	lgdt [GdtPtr]
	;关中断
	cli
	;打开地址线A20
	in al,92h
	or al,2
	out 92h,al
	;准备切换到保护模式
	mov eax,cr0
	or eax,1
	mov cr0,eax
	;真正进入保护模式
	mov ax,SelectorData32
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov esp,0x400
	jmp dword SelectorCode32:0x400
	
	jmp $ ;讲道理应该到不了这儿

;--------------------------------------------------------------------
;功能描述：从磁盘加载指定文件到内存
;入口参数：		[DS:SI]=文件名
;		[ES:DI]=缓冲区
;出口参数：ZF=1——操作成功,ZF=0——失败
Loadfile:
	push ax
	push cx
	push si
	push di


	mov ax,4	;group
	mov cl,1	
	call ReadSector		

	mov ax,[di+8]	;第一个group的结点表的块号
	mov cl,1
	call ReadSector

	mov ax,[di+128+40]	;第二个结点的i_zone
	mov cl,1
	call ReadSector 

.amtch:
	


.match:
	add di,32					;跳过第一个目录项	;每个目录项32字节
	mov ax,[es:di]
	cmp ax,0
	jz	.error
	call Cmpare						
	jnz .match
	mov ax,[es:di+0x1a]			;首簇号
	
	sub ax,2
	mul byte [BPB_SecPerClus]		;暂时忽略ax高8位，有可能出错
		
	add ax,[BPB_SecPerFat]		;每FAT扇区数
	add ax,[BPB_SecPerFat]	
	add ax,[BPB_RsvdSecCnt]		;保留扇区数
	add ax,32					;目录项扇区数
	
	pop di
	push di
	
	mov cl,4
	call ReadSector				;读入load.bin文件	(暂时设为连续的2KB)
	
	
	jmp .ret
.error:
	mov si,[Error_findLoadFile]
	call Display
.ret:
	pop di
	pop si
	pop cx
	pop ax
	ret	
;---------------------------------------------------------
;功能描述：	读硬盘一个扇区
;入口参数:		[ES:DI]=缓冲区的地址
;		AX=LBA扇区号
;		CL=扇区数

ReadSector:	
	push cx
	push dx
	push di
	push ax

	mov al,cl
	mov dx,0x1f2
	out dx,al 	;sector counts
	
	pop ax		;recovery al
	push ax
	inc dx
	out dx,al	;lba 0~7

	mov al,ah	;lba 8~15
	inc dx
	out dx,al
	
	mov al,0	;lba 16~23 :0,so not support lba greater 0x10000
	inc dx
	out dx,al

	mov al,0xe0	;lba 24~27:0 ,master disk,LBA mode 
	inc dx
	out dx,al

	mov al,0x20	;read disk
	inc dx
	out dx,al

.loop:
	in al,dx	;read stat
	and al,0x88
	cmp al,0x08
	jne .loop

	;求字节数，要求扇区数<0x100
	mov ax,0x100
	mul cx
	mov cx,ax

	mov dx,0x1f0 ;读数据
	cld
	rep insw 
	
	pop ax
	pop di
	pop dx
	pop cx
	ret



;---------------------------------------------------------
;功能:显示字符串
;入口参数：	[DS:SI]=字符串地址

	;子功能int 10H (0eH)
	;功能描述：在Teletype模式下显示字符
	;入口参数：	AH＝0EH
	;			AL＝字符
	;			BH＝页码
	;			BL＝前景色(图形模式)
	;出口参数：无
Display:
	push ax
	push bx
	push si
	mov ah,0eh
	mov bh,0	
.loop:
	mov al,[si]
	cmp al,0
	jz .ret
	int 10h
	inc si
	jmp .loop
.ret:
	pop si
	pop bx
	pop ax
	ret

;--------------------------------------------------------
;功能：比较11bit文件名
;入口参数:		[DS:SI]=字符串地址
;		[ES:DI]=字符串地址
;出口参数：ZF=1——相同，ZF=0——不同
Cmpare:
	push ax
	push si
	push di
	push cx
	
	mov cx,3		;对比11次，每次1个字节
.cmp:
	mov al,[si]
	inc si
	inc di
	cmp al,[es:di-1]	
	jnz .ret
	loop .cmp
.ret:
	pop cx
	pop di
	pop si
	pop ax
	ret	


;---------------------------------------------------------	
;----------------数据区----------------------------------
;--------------------------------------------------------
;GDT
LABEL_GDT:Descriptor 0,0,0
LABEL_DESC_CODE32:Descriptor 0x0,0xfffff,DA_CODE
LABEL_DESC_DATA32:Descriptor 0x0,0xfffff,DA_DATA
LABEL_GDT_END:

;GDTPTR
GdtPtr 	DW	LABEL_GDT_END-LABEL_GDT		;GDT长度
		DD	0							;GDT段基址
;GDt selector
SelectorCode32 equ LABEL_DESC_CODE32-LABEL_GDT	
SelectorData32 equ LABEL_DESC_DATA32-LABEL_GDT

;字符串
LoadFileName db "load.bin"
StrHello db "hello word!",`\n`,`\r`,"read load.bin from ext2",`\n`,`\r`,0	;添加换行符和'\0'结束标志
Error_read_disk db "read disk error!",`\n`,`\r`,0 
Error_findLoadFile db "not find load.bin!",`\n`,`\r`,0


times 510-($-$$) db 0
dw 0xaa55
