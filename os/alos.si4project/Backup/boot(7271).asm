%include "pm.inc"
org 7c00H
__start:		;��load.bin���ص�cs:0x400��,Ĭ��cs=0
	;mov ax,cs	;λ����ͬ��
	;mov ss,ax
	;mov ds,ax
	;mov es,ax

	;��ʾ�ʺ���
	mov sp,0x7c00
	mov si,StrHello
	call Display
	
	;����Ŀ���ļ�
	mov si,LoadFileName			
	mov di,[load_addr]		;���ص�0x400
	call Loadfile
	;����GdtPr
	mov dword [GdtPtr+2],LABEL_GDT ;gdt��ַ
	lgdt [GdtPtr]
	;���ж�
	cli
	;�򿪵�ַ��A20
	in al,92h
	or al,2
	out 92h,al
	;׼���л�������ģʽ
	mov eax,cr0
	or eax,1
	mov cr0,eax
	;�������뱣��ģʽ
	mov ax,SelectorData32
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov esp,0x400
	jmp dword SelectorCode32:0x400
	
	jmp $ ;������Ӧ�õ��������

;--------------------------------------------------------------------
;�����������Ӵ��̼���ָ���ļ����ڴ�
;��ڲ�����		[DS:SI]=�ļ���
;
;���ڲ�����ZF=1���������ɹ�,ZF=0����ʧ��
Loadfile:
	mov di,[load_addr]
	mov ax,2	;group���	
	call ReadSector		

	mov ax,[di+8]	;��һ��group�Ľ���Ŀ��
	mov [bg_inode_table],ax	;����ax
	
	call ReadSector	;������es:di

	;������i_zone[0] 
	;ע�⣺ֻ��һ��blockĿ¼��load.binĿ¼�����ܶ�����
	mov ax,[di+128+40]
	call ReadSector	

;;--------------��/���ҵ�load.binĿ¼��------ax=inode_num------------
	mov bp,sp
.match:		
	mov ax,[di]	;��������
	cmp ax,0
	jz .error

	push si
	push [di+8]
	push [di+6]
	add di,[di+4]	;ָ����һ��Ŀ¼��
	call Compare
	mov sp,bp
	jnz .match

;;----------����load.bin������Ϣ,�����ļ�ռ�ÿ��-----------
	sub ax,1	;(ax-1)/4=al...ah
	mov bl,4
	div bl	
	
	mov bl,ah
	mov ah,0
	add ax,[bg_inode_table]
	mov di,[load_addr]
	call ReadSector	;���뺬Ŀ�����block
	
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

;;-------------����load.bin�ļ�����-----------------------	
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
;����������	��Ӳ��һ���߼�����
;��ڲ���:		[ES:DI]=�������ĵ�ַ
;		AX=LBA������
;ע�⣺	Ϊ�˺�ext2�ļ�ϵͳ����һ��,��Ӳ�̼���Ϊblock size=1K,
;	����ʵ����block size=512B,so LBA�ᷢ��ת��

ReadSector:
	push ax
	push cx
	push dx
	push di
	
	mov dx,2
	mul dx 		;ת��LBA��ax=ax*2  ���λ����
	
	push ax		
	mov ax,2	;read 2 ������������һ���߼�����
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
	
	mov cx,512	;������ 1K
	mov dx,0x1f0 	;���ݶ˿�
	cld
	rep insw

	pop di
	pop dx
	pop cx
	pop ax
	ret


;---------------------------------------------------------
;����:��ʾ�ַ���
;��ڲ�����	[DS:SI]=�ַ�����ַ

	;�ӹ���int 10H (0eH)
	;������������Teletypeģʽ����ʾ�ַ�
	;��ڲ�����	AH��0EH
	;			AL���ַ�
	;			BH��ҳ��
	;			BL��ǰ��ɫ(ͼ��ģʽ)
	;���ڲ�������
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
;���ܣ��Ƚ��ļ���
;
;compare(u16 size,u16 *str1,u16 *str2)
;		
;���ڲ�����ZF=1������ͬ��ZF=0������ͬ
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
;----------------������----------------------------------
;--------------------------------------------------------
;GDT
LABEL_GDT:Descriptor 0,0,0
LABEL_DESC_CODE32:Descriptor 0x0,0xfffff,DA_CODE
LABEL_DESC_DATA32:Descriptor 0x0,0xfffff,DA_DATA
LABEL_GDT_END:

;GDTPTR
GdtPtr 	DW	LABEL_GDT_END-LABEL_GDT		;GDT����
		DD	0							;GDT�λ�ַ
;GDt selector
SelectorCode32 equ LABEL_DESC_CODE32-LABEL_GDT	
SelectorData32 equ LABEL_DESC_DATA32-LABEL_GDT

;�ַ���
LoadFileName db "load.bin"
StrHello db "hello word!",`\n`,`\r`,"read load.bin from ext2",`\n`,`\r`,0	;��ӻ��з���'\0'������־
Error_read_disk db "read disk error!",`\n`,`\r`,0 
Error_findLoadFile db "not find load.bin!",`\n`,`\r`,0
bg_inode_table db 0
load_addr	db 0,0x4
i_zone  times 48 db 0 	;i_zone[0]~i_zone[11] ����ʾ12block

times 510-($-$$) db 0
dw 0xaa55
