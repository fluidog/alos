/*
	�˳����������ģ�飬fat16�ļ�ϵͳ��fat.c������ҳ�ڴ�ϵͳ��memory.c����
	�����ǣ����ز���ϵͳ����0x1000��
	��ʹ��ҳ��0xc0000000~0xc0400000 -- 0~4M �� 0~4M -- 0~4M��ӳ���ϵ��
		����ת������ϵͳ��ڣ�0xc0001000��ִ�С�
*/
asm("jmp main");

#include<fat.h>
#include<ext2.h>
#include<memory.h>

#define KERNEL_VIR_ADDR		0xc0001000
#define KERNEL_LOAD_ADDR	0x1000
#define KERNEL_FILE_NAME_FAT	"ALOS       "
#define KERNEL_FILE_NAME		"alos"
char buffer0[1024];//2K������ BSS�Σ���ռ���ļ���С
char buffer1[1024];
void main()
{
	init_gdt();
	init_page();
	//��Ĭ�ϲ���ϵͳ��ext2�ļ��У�ʧ�ܱ㳢��fat�ļ�ϵͳ
	if(ext2_load_file((void *)KERNEL_LOAD_ADDR,KERNEL_FILE_NAME)<0)
		fat_load_file((void *)KERNEL_LOAD_ADDR,KERNEL_FILE_NAME_FAT);
	EN_PAGE;
	EN_GDT;	//enable gdt and long jmp to CS:KERNEL_VIR_ADDR
}