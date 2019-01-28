#include<linux/sys.h>

#include<linux/kernel.h>
unsigned int nr_system_calls = 72;
void system_call()
{
	
	unsigned char call_num;
	asm("mov %%eax,%0"
		:"=m"(call_num):);
	if(call_num>nr_system_calls)return;


	//���沢���öμĴ�����ѹ�����
	asm("push %ds\n"
		"push %es\n"
		"push %fs\n"
		"push %edx\n"
		"push %ecx\n"
		"push %ebx\n"
		"mov $0x10,%edx\n"
		"mov %edx,%ds\n"
		"mov %edx,%es\n"
		"mov $0x23,%edx\n"
		"mov %edx,%fs\n");

	sys_call_table[call_num]();
	
	asm("pop %ebx\n"
		"pop %ecx\n"
		"pop %edx\n"
		"pop %fs\n"
		"pop %es\n"
		"pop %ds\n"
		"leave\n"
		"iret\n");
}



#if 0

asm("system_call:\n"
		"cmp eax,nr_system_calls-1\n"
		"ja77")


bad_sys_call:
	mov eax,-1 ;// eax ����-1���˳��жϡ�
	iretd
;// ����ִ�е��ȳ�����ڡ����ȳ���schedule ��(kernel/sched.c,104)��
align 4
reschedule:
	push ret_from_sys_call ;// ��ret_from_sys_call �ĵ�ַ��ջ��101 �У���
	jmp _schedule
;//// int 0x80 --linux ϵͳ������ڵ�(�����ж�int 0x80��eax ���ǵ��ú�)��
align 4
_system_call:
	cmp eax,nr_system_calls-1 ;// ���ú����������Χ�Ļ�����eax ����-1 ���˳���
	ja bad_sys_call
	push ds ;// ����ԭ�μĴ���ֵ��
	push es
	push fs
	push edx ;// ebx,ecx,edx �з���ϵͳ������Ӧ��C ���Ժ����ĵ��ò�����
	push ecx ;// push %ebx,%ecx,%edx as parameters
	push ebx ;// to the system call
	mov edx,10h ;// set up ds,es to kernel space
	mov ds,dx ;// ds,es ָ���ں����ݶ�(ȫ���������������ݶ�������)��
	mov es,dx
	mov edx,17h ;// fs points to local data space
	mov fs,dx ;// fs ָ��ֲ����ݶ�(�ֲ��������������ݶ�������)��
;// �������������ĺ����ǣ����õ�ַ = _sys_call_table + %eax * 4���μ��б���˵����
;// ��Ӧ��C �����е�sys_call_table ��include/linux/sys.h �У����ж�����һ������72 ��
;// ϵͳ����C �������ĵ�ַ�����
	call [_sys_call_table+eax*4]
	push eax ;// ��ϵͳ���ú���ջ��
	mov eax,_current ;// ȡ��ǰ���񣨽��̣����ݽṹ��ַ??eax��
;// ����97-100 �в鿴��ǰ���������״̬��������ھ���״̬(state ������0)��ȥִ�е��ȳ���
;// ����������ھ���״̬��counter[??]ֵ����0����Ҳȥִ�е��ȳ���
	cmp dword ptr [state+eax],0 ;// state
	jne reschedule
	cmp dword ptr [counter+eax],0 ;// counter
	je reschedule
;// ������δ���ִ�д�ϵͳ����C �������غ󣬶��ź�������ʶ����
ret_from_sys_call:
;// �����б�ǰ�����Ƿ��ǳ�ʼ����task0��������򲻱ض�������ź�������Ĵ���ֱ�ӷ��ء�
;// 103 ���ϵ�_task ��ӦC �����е�task[]���飬ֱ������task �൱������task[0]��
	mov eax,_current ;// task[0] cannot have signals
	cmp eax,_task
	je l1 ;// ��ǰ(forward)��ת�����l1��
;// ͨ����ԭ���ó������ѡ����ļ�����жϵ��ó����Ƿ��ǳ����û�������ǳ����û���ֱ��
;// �˳��жϣ�����������ź����Ĵ�������Ƚ�ѡ����Ƿ�Ϊ��ͨ�û�����ε�ѡ���0x000f
;// (RPL=3���ֲ�����1 ����(�����))�������������ת�˳��жϳ���
	cmp word ptr [R_CS+esp],0fh ;// was old code segment supervisor ?
	jne l1
;// ���ԭ��ջ��ѡ�����Ϊ0x17��Ҳ��ԭ��ջ�����û����ݶ��У�����Ҳ�˳���
	cmp word ptr [OLR_DSS+esp],17h ;// was stack segment = 0x17 ?
	jne l1
;// ������δ��루109-120������;������ȡ��ǰ����ṹ�е��ź�λͼ(32 λ��ÿλ����1 ���ź�)��
;// Ȼ��������ṹ�е��ź����������Σ��룬������������ź�λ��ȡ����ֵ��С���ź�ֵ���ٰ�
;// ԭ�ź�λͼ�и��źŶ�Ӧ��λ��λ����0������󽫸��ź�ֵ��Ϊ����֮һ����do_signal()��
;// do_signal()�ڣ�kernel/signal.c,82���У����������13 ����ջ����Ϣ��
	mov ebx,[signal+eax] ;// ȡ�ź�λͼ??ebx��ÿ1 λ����1 ���źţ���32 ���źš�
	mov ecx,[blocked+eax] ;// ȡ���������Σ��ź�λͼ??ecx��
	not ecx ;// ÿλȡ����
	and ecx,ebx ;// �����ɵ��ź�λͼ��
	bsf ecx,ecx ;// �ӵ�λ��λ0����ʼɨ��λͼ�����Ƿ���1 ��λ��
;// ���У���ecx ������λ��ƫ��ֵ�����ڼ�λ0-31����
	je l1 ;// ���û���ź�����ǰ��ת�˳���
	btr ebx,ecx ;// ��λ���źţ�ebx ����ԭsignal λͼ����
	mov dword ptr [signal+eax],ebx ;// ���±���signal λͼ��Ϣ??current->signal��
	inc ecx ;// ���źŵ���Ϊ��1 ��ʼ����(1-32)��
	push ecx ;// �ź�ֵ��ջ��Ϊ����do_signal �Ĳ���֮һ��
	call _do_signal ;// ����C �����źŴ������(kernel/signal.c,82)
	pop eax ;// �����ź�ֵ��
l1: pop eax
	pop ebx
	pop ecx
	pop edx
	pop fs
	pop es
	pop ds
	iretd
#endif
