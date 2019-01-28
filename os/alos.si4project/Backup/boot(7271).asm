%include "pm.inc"
org 7c00H
__start:		;将load.bin加载到cs:0x400处,默认cs=0
	;mov ax,cs	;位于相同段
	;mov ss,ax
	;mov ds,ax
	;mov es,ax

	;显示问候语
	mov sp,0x7c00
	mov si,StrHello
	call Display
	
	;加载目标文件
	mov si,LoadFileName			
	mov di,[load_addr]		;加载到0x400
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
;
;出口参数：ZF=1——操作成功,ZF=0——失败
Loadfile:
	mov di,[load_addr]
	mov ax,2	;group块号	
	call ReadSector		

	mov ax,[di+8]	;第一个group的结点表的块号
	mov [bg_inode_table],ax	;保存ax
	
	call ReadSector	;读结点表到es:di

	;根结点的i_zone[0] 
	;注意：只读一个block目录，load.bin目录项靠后可能读不到
	mov ax,[di+128+40]
	call ReadSector	

;;--------------在/下找到load.bin目录项------ax=inode_num------------
	mov bp,sp
.match:		
	mov ax,[di]	;索引结点号
	cmp ax,0
	jz .error

	push si
	push [di+8]
	push [di+6]
	add di,[di+4]	;指向下一个目录项
	call Compare
	mov sp,bp
	jnz .match

;;----------读入load.bin结点表信息,保存文件占用块号-----------
	sub ax,1	;(ax-1)/4=al...ah
	mov bl,4
	div bl	
	
	mov bl,ah
	mov ah,0
	add ax,[bg_inode_table]
	mov di,[load_addr]
	call ReadSector	;读入含目标结点的block
	
	mov al,128	;ax=(bl*128)+40
	mul bl	
	add ax,40
	add di,ax

	mov cx,12
	mov bx,0
.loop0:
	mov ax,[di+bx]
	mov [i_zone+bx],ax
	add bx,4
	loop .loop0

;;-------------读入load.bin文件内容-----------------------	
	mov bx,0
	mov cx,12
	mov di,[load_addr]
.loop1:
	mov ax,[i_zone+bx]
	cmp ax,0
	jz .ret
	add bx,4
	call ReadSector
	add di,1024
	loop .loop1
.ret:
	ret

.error:
	mov si,Error_findLoadFile
	call Display
	jmp $
	
;---------------------------------------------------------
;功能描述：	读硬盘一个逻辑扇区
;入口参数:		[ES:DI]=缓冲区的地址
;		AX=LBA扇区号
;注意：	为了和ext2文件系统保持一致,将硬盘假设为block size=1K,
;	但真实物理block size=512B,so LBA会发生转变

ReadSector:
	push ax
	push cx
	push dx
	push di
	
	mov dx,2
	mul dx 		;转换LBA，ax=ax*2  溢出位舍弃
	
	push ax		
	mov ax,2	;read 2 物理扇区，即一个逻辑扇区
	mov dx,0x1f2
	out dx,al 	;sector counts:2
	pop ax		;recovery al

	
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
	
	mov cx,512	;求字数 1K
	mov dx,0x1f0 	;数据端口
	cld
	rep insw

	pop di
	pop dx
	pop cx
	pop ax
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
;功能：比较文件名
;
;compare(u16 size,u16 *str1,u16 *str2)
;		
;出口参数：ZF=1——相同，ZF=0——不同
Compare:
	push bp
	mov bp,sp
	push si
	push di
	push bx
	push cx
	mov si,[bp+2]
	mov di,[bp+4]
	mov cx,[bp+6]
	mov bx,0
.loop:	
	mov al,[si+bx]
	cmp al,[di+bx]	
	jnz .ret
	inc bx
	loop .loop
	
.ret:
	pop cx
	pop bx
	pop di
	pop si
	pop bp
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
bg_inode_table db 0
load_addr	db 0,0x4
i_zone  times 48 db 0 	;i_zone[0]~i_zone[11] 最多表示12block

times 510-($-$$) db 0
dw 0xaa55
