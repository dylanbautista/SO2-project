#include <asm.h>

/* void switch_stack(void ** save_sp, void * new_sp) NOT REALLY A FUNCTION!!! */
ENTRY(switch_stack)
	mov 4(%esp), %eax
	mov %ebp, (%eax)
	mov 8(%esp), %esp
	popl %ebp
	ret

/* void task_switch(union task_union *new) */
ENTRY(task_switch)
	pushl %ebp
	movl %esp, %ebp
	pushl %esi
	pushl %edi
	pushl %ebx
	pushl 8(%ebp)
	call inner_task_switch
	addl $4, %esp
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

/* void prepare_user_stack(int esp, (void*)function, (void*)parameter); */
ENTRY(prepare_user_stack)
	push %ebp
	movl %esp, %ebp

	movl 0x8(%ebp), %eax //esp
	movl 0xc(%ebp), %edx //finish function
	movl 0x10(%ebp), %ecx //parameter

	movl $777, (%eax) //parameter
	movl $777, -4(%eax) //finish function

	movl %ebp, %esp
	pop %ebp
	ret

/* void * get_ebp(void) */
ENTRY(get_ebp)
	mov %ebp, %eax
	ret

/* void setMSR(unsigned long msr_number, unsigned long high, unsigned long low)*/
ENTRY(setMSR)
	push %ebp
	movl %esp, %ebp
	movl 0x10(%ebp), %eax
	movl 0xc(%ebp), %edx
	movl 0x8(%esp), %ecx
	wrmsr
	pop %ebp
	ret

