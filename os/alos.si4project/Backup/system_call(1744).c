#include<linux/sys.h>

#include<linux/kernel.h>

	fn_ptr sys_call_table[] = { sys_setup, sys_exit, sys_fork, sys_read,
	sys_write, sys_open, sys_close, sys_waitpid, sys_creat, sys_link,
	sys_unlink, sys_execve, sys_chdir, sys_time, sys_mknod, sys_chmod,
	sys_chown, sys_break, sys_stat, sys_lseek, sys_getpid, sys_mount,
	sys_umount, sys_setuid, sys_getuid, sys_stime, sys_ptrace, sys_alarm,
	sys_fstat, sys_pause, sys_utime, sys_stty, sys_gtty, sys_access,
	sys_nice, sys_ftime, sys_sync, sys_kill, sys_rename, sys_mkdir,
	sys_rmdir, sys_dup, sys_pipe, sys_times, sys_prof, sys_brk, sys_setgid,
	sys_getgid, sys_signal, sys_geteuid, sys_getegid, sys_acct, sys_phys,
	sys_lock, sys_ioctl, sys_fcntl, sys_mpx, sys_setpgid, sys_ulimit,
	sys_uname, sys_umask, sys_chroot, sys_ustat, sys_dup2, sys_getppid,
	sys_getpgrp, sys_setsid, sys_sigaction, sys_sgetmask, sys_ssetmask,
	sys_setreuid,sys_setregid,sys_sigreturn};




#if 0

asm("system_call:\n"
		"cmp eax,nr_system_calls-1\n"
		"ja77")


bad_sys_call:
	mov eax,-1 ;// eax 中置-1，退出中断。
	iretd
;// 重新执行调度程序入口。调度程序schedule 在(kernel/sched.c,104)。
align 4
reschedule:
	push ret_from_sys_call ;// 将ret_from_sys_call 的地址入栈（101 行）。
	jmp _schedule
;//// int 0x80 --linux 系统调用入口点(调用中断int 0x80，eax 中是调用号)。
align 4
_system_call:
	cmp eax,nr_system_calls-1 ;// 调用号如果超出范围的话就在eax 中置-1 并退出。
	ja bad_sys_call
	push ds ;// 保存原段寄存器值。
	push es
	push fs
	push edx ;// ebx,ecx,edx 中放着系统调用相应的C 语言函数的调用参数。
	push ecx ;// push %ebx,%ecx,%edx as parameters
	push ebx ;// to the system call
	mov edx,10h ;// set up ds,es to kernel space
	mov ds,dx ;// ds,es 指向内核数据段(全局描述符表中数据段描述符)。
	mov es,dx
	mov edx,17h ;// fs points to local data space
	mov fs,dx ;// fs 指向局部数据段(局部描述符表中数据段描述符)。
;// 下面这句操作数的含义是：调用地址 = _sys_call_table + %eax * 4。参见列表后的说明。
;// 对应的C 程序中的sys_call_table 在include/linux/sys.h 中，其中定义了一个包括72 个
;// 系统调用C 处理函数的地址数组表。
	call [_sys_call_table+eax*4]
	push eax ;// 把系统调用号入栈。
	mov eax,_current ;// 取当前任务（进程）数据结构地址??eax。
;// 下面97-100 行查看当前任务的运行状态。如果不在就绪状态(state 不等于0)就去执行调度程序。
;// 如果该任务在就绪状态但counter[??]值等于0，则也去执行调度程序。
	cmp dword ptr [state+eax],0 ;// state
	jne reschedule
	cmp dword ptr [counter+eax],0 ;// counter
	je reschedule
;// 以下这段代码执行从系统调用C 函数返回后，对信号量进行识别处理。
ret_from_sys_call:
;// 首先判别当前任务是否是初始任务task0，如果是则不必对其进行信号量方面的处理，直接返回。
;// 103 行上的_task 对应C 程序中的task[]数组，直接引用task 相当于引用task[0]。
	mov eax,_current ;// task[0] cannot have signals
	cmp eax,_task
	je l1 ;// 向前(forward)跳转到标号l1。
;// 通过对原调用程序代码选择符的检查来判断调用程序是否是超级用户。如果是超级用户就直接
;// 退出中断，否则需进行信号量的处理。这里比较选择符是否为普通用户代码段的选择符0x000f
;// (RPL=3，局部表，第1 个段(代码段))，如果不是则跳转退出中断程序。
	cmp word ptr [R_CS+esp],0fh ;// was old code segment supervisor ?
	jne l1
;// 如果原堆栈段选择符不为0x17（也即原堆栈不在用户数据段中），则也退出。
	cmp word ptr [OLR_DSS+esp],17h ;// was stack segment = 0x17 ?
	jne l1
;// 下面这段代码（109-120）的用途是首先取当前任务结构中的信号位图(32 位，每位代表1 种信号)，
;// 然后用任务结构中的信号阻塞（屏蔽）码，阻塞不允许的信号位，取得数值最小的信号值，再把
;// 原信号位图中该信号对应的位复位（置0），最后将该信号值作为参数之一调用do_signal()。
;// do_signal()在（kernel/signal.c,82）中，其参数包括13 个入栈的信息。
	mov ebx,[signal+eax] ;// 取信号位图??ebx，每1 位代表1 种信号，共32 个信号。
	mov ecx,[blocked+eax] ;// 取阻塞（屏蔽）信号位图??ecx。
	not ecx ;// 每位取反。
	and ecx,ebx ;// 获得许可的信号位图。
	bsf ecx,ecx ;// 从低位（位0）开始扫描位图，看是否有1 的位，
;// 若有，则ecx 保留该位的偏移值（即第几位0-31）。
	je l1 ;// 如果没有信号则向前跳转退出。
	btr ebx,ecx ;// 复位该信号（ebx 含有原signal 位图）。
	mov dword ptr [signal+eax],ebx ;// 重新保存signal 位图信息??current->signal。
	inc ecx ;// 将信号调整为从1 开始的数(1-32)。
	push ecx ;// 信号值入栈作为调用do_signal 的参数之一。
	call _do_signal ;// 调用C 函数信号处理程序(kernel/signal.c,82)
	pop eax ;// 弹出信号值。
l1: pop eax
	pop ebx
	pop ecx
	pop edx
	pop fs
	pop es
	pop ds
	iretd
#endif
