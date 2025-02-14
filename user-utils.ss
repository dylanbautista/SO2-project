#include <asm.h>

ENTRY(syscall_sysenter)
	push %ecx
	push %edx
	push $SYSENTER_RETURN
	push %ebp
	mov %esp, %ebp
	sysenter
ENTRY(SYSENTER_RETURN)
	pop %ebp
	pop %edx
	pop %edx
	pop %ecx
	ret

/* int write(int fd, char *buffer, int size) */
ENTRY(write)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $4, %eax
	movl 0x8(%ebp), %ebx;	//fd
	movl 0xC(%ebp), %ecx;	//buffer
	movl 0x10(%ebp), %edx;	//size
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* int gotoXY(int x, int y) */
ENTRY(gotoXY)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $16, %eax
	movl 0x8(%ebp), %ebx;	//X
	movl 0xC(%ebp), %ecx;	//Y
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* strcut semafor * sys_sem_create(struct semafor *s, int value) */
ENTRY(sem_create)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $25, %eax
	movl 0x8(%ebp), %ebx;	//s
	movl 0xC(%ebp), %ecx;	//value
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* int sys_semWait(struct semafor *s) */
ENTRY(semWait)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $26, %eax
	movl 0x8(%ebp), %ebx;	//s
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* int sys_semSignal(struct semafor *s) */
ENTRY(semSignal)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $27, %eax
	movl 0x8(%ebp), %ebx;	//s
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* char* sys_memRegGet(int num_pages) */
ENTRY(memRegGet)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $30, %eax
	movl 0x8(%ebp), %ebx;	//num_pages
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* int sys_memRegDel(char* m) */
ENTRY(memRegDel)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $31, %eax
	movl 0x8(%ebp), %ebx;	//m
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* int changeColor(color_t fg, color_t bg) */
ENTRY(changeColor)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $17, %eax
	movl 0x8(%ebp), %ebx;	//fg
	movl 0xC(%ebp), %ecx;	//bg
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* int clrscr(Word* b) */
ENTRY(clrscr)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $15, %eax
	movl 0x8(%ebp), %ebx;	//Word*
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* int getKey(char* b, int timeout) */
ENTRY(getKey)
	pushl %ebp
	mov %esp, %ebp
	pushl %ebx
	movl $6, %eax
	movl 0x8(%ebp), %ebx; //b pointer
	movl 0xC(%ebp), %ecx; //timeout
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok
	popl %ebp
	ret

/* Common code for negative return */
nok:
	neg %eax
	mov %eax, errno
	mov $-1, %eax
	popl %ebp
	ret

/* int gettime() */
ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp
	movl $10, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int getpid() */
ENTRY(getpid)
	pushl %ebp
	movl %esp, %ebp
	movl $20, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int fork() */
ENTRY(fork)
	pushl %ebp
	movl %esp, %ebp
	movl $2, %eax
	call syscall_sysenter
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* int threadCreateWithStack( void (*function)(void* arg), int N, void* parameter, void (*ext)) */
ENTRY(threadCreateWithStack)
	pushl %ebp
	mov %esp, %ebp
	pushl %ebx
	pushl %esi
	movl $3, %eax
	movl 0x8(%ebp), %ebx; //function pointer
	movl 0xC(%ebp), %ecx; //N
	movl 0x10(%ebp), %edx; //parameter
	movl 0x14(%ebp), %esi; //exit
	call syscall_sysenter
	popl %esi
	popl %ebx
	test %eax, %eax
	js nok
	popl %ebp
	ret

/* char* memRegGet(int num_pages) */
ENTRY(memRegGet)
	pushl %ebp
	mov %esp, %ebp
	pushl %ebx
	movl $21, %eax
	movl 0x8(%ebp), %ebx;
	call syscall_sysenter
	test %eax, %eax
	js nok
	popl %ebp
	ret

/* int memRegDel(char* m) */
ENTRY(memRegDel)
	pushl %ebp
	mov %esp, %ebp
	pushl %ebx
	movl $22, %eax
	movl 0x8(%ebp), %ebx;
	call syscall_sysenter
	test %eax, %eax
	js nok
	popl %ebp
	ret

/* void block(void) */
ENTRY(block)
	pushl %ebp
	movl %esp, %ebp
	movl $7, %eax
	call syscall_sysenter
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret


/* void exit() */
ENTRY(exit)
	pushl %ebp
	movl %esp, %ebp
	movl $1, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int yield() */
ENTRY(yield)
	pushl %ebp
	movl %esp, %ebp
	movl $13, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int get_stats(int pid, struct stats *st) */
ENTRY(get_stats)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $35, %eax
	movl 0x8(%ebp), %ebx;	//pid
	movl 0xC(%ebp), %ecx;	//st
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

ENTRY(SAVE_REGS)
      pushl %eax
      movl %eax, REGS    //SAVE EAX
      lea REGS, %eax
      movl %ebp, 4(%eax)
      movl %edi, 8(%eax)
      movl %esi, 12(%eax)
      movl %edx, 16(%eax)
      movl %ecx, 20(%eax)
      movl %ebx, 24(%eax)
      popl %eax
      ret

ENTRY(RESTORE_REGS)
      lea REGS  , %eax
      movl 4(%eax) , %ebp 
      movl 8(%eax) , %edi 
      movl 12(%eax), %esi 
      movl 16(%eax), %edx 
      movl 20(%eax), %ecx 
      movl 24(%eax), %ebx 
      movl (%eax), %eax
      ret
