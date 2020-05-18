/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps


#define SET_IDT(vector_val, is_trap, sel, off, dpl)   \
   pushl %eax; \
   pushl %ebx; \
   pushl %ecx; \
   pushl %edx; \
   movl %esp, %edx; \
   pushl $(vector_val) ; \
   pushl $(is_trap) ; \
   pushl $(sel); \
   pushl $(off); \
   pushl $(dpl); \
   call _create_idt_entry; \
   movl %edx, %esp; \
   popl %edx; \
   popl %ecx; \
   popl %ebx; \
   popl %eax; 


.text

TRAPHANDLER_NOEC(HANDLER_DE_ERROR, T_DIVIDE)          /* Divide Error */
TRAPHANDLER_NOEC(HANDLER_DB_ERROR, T_DEBUG)           /* Reserved */
TRAPHANDLER_NOEC(HANDLER_NMI, T_NMI)                  /* Non maskable Interrupt */
TRAPHANDLER_NOEC(HANDLER_BP_TRAP, T_BRKPT)            /* Breakpoint */
TRAPHANDLER_NOEC(HANDLER_OF_TRAP, T_OFLOW)            /* Overflow */
TRAPHANDLER_NOEC(HANDLER_BR_ERROR, T_BOUND)           /* Bound Range Exceeded */
TRAPHANDLER_NOEC(HANDLER_UD_ERROR, T_ILLOP)           /* Invalid Opcode */ 
TRAPHANDLER_NOEC(HANDLER_NM_ERROR, T_DEVICE)          /* Device not available, no math processor */
TRAPHANDLER(HANDLER_DF_ABORT, T_DBLFLT)               /* Double Fault */
TRAPHANDLER(HANDLER_TS_ERROR, T_TSS)                  /* Invalid TSS*/
TRAPHANDLER(HANDLER_NP_ERROR, T_SEGNP)                /* Segment Not Present */
TRAPHANDLER(HANDLER_SS_ERROR, T_STACK)                /* Stack Segment fault */
TRAPHANDLER(HANDLER_GP_ERROR, T_GPFLT)                /* General Protection */
TRAPHANDLER(HANDLER_PF, T_PGFLT)                      /* Page Fault */
TRAPHANDLER_NOEC(HANDLER_MF_ERROR, T_FPERR)           /* Floating point Error Math Fault */
TRAPHANDLER(HANDLER_AC_ERROR, T_ALIGN)                /* Alignment Check */
TRAPHANDLER_NOEC(HANDLER_MC_ABORT, T_MCHK)            /* Machine Check */
TRAPHANDLER_NOEC(HANDLER_XF_ERROR, T_SIMDERR)         /* SIMD Floating point exception */
TRAPHANDLER_NOEC(HANDLER_SYSCALL, T_SYSCALL)          /* SYSCALL */

SET_IDT(T_DIVIDE, TYPE_EXCEPTION, GD_KT, HANDLER_DE_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_DEBUG, TYPE_EXCEPTION, GD_KT, HANDLER_DB_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_NMI, TYPE_INTERRUPT, GD_KT, HANDLER_NMI, PRIVILEGE_KERNEL)
SET_IDT(T_BRKPT, TYPE_EXCEPTION, GD_KT, HANDLER_BP_TRAP, PRIVELEGE_USER)
SET_IDT(T_OFLOW, TYPE_EXCEPTION, GD_KT, HANDLER_OF_TRAP, PRIVILEGE_KERNEL)
SET_IDT(T_BOUND, TYPE_EXCEPTION, GD_KT, HANDLER_BR_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_ILLOP, TYPE_EXCEPTION, GD_KT, HANDLER_UD_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_DEVICE, TYPE_EXCEPTION, GD_KT, HANDLER_NM_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_DBLFLT, TYPE_EXCEPTION, GD_KT, HANDLER_DF_ABORT, PRIVILEGE_KERNEL)
SET_IDT(T_TSS, TYPE_EXCEPTION, GD_KT, HANDLER_TS_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_SEGNP, TYPE_EXCEPTION, GD_KT, HANDLER_NP_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_STACK, TYPE_EXCEPTION, GD_KT, HANDLER_SS_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_GPFLT, TYPE_EXCEPTION, GD_KT, HANDLER_GP_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_PGFLT, TYPE_EXCEPTION, GD_KT, HANDLER_PF, PRIVILEGE_KERNEL)
SET_IDT(T_FPERR, TYPE_EXCEPTION, GD_KT, HANDLER_MF_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_ALIGN, TYPE_EXCEPTION, GD_KT, HANDLER_AC_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_MCHK, TYPE_EXCEPTION, GD_KT, HANDLER_MC_ABORT, PRIVILEGE_KERNEL)
SET_IDT(T_SIMDERR, TYPE_EXCEPTION, GD_KT, HANDLER_XF_ERROR, PRIVILEGE_KERNEL)
SET_IDT(T_SYSCALL, TYPE_INTERRUPT, GD_KT, HANDLER_SYSCALL, PRIVELEGE_USER)

_create_idt_entry:
   pushl %ebp
   mov %ebp, %esp

   xor %eax, %eax 
   mov %eax, %ebx 
   mov %eax, %ecx

   mov PARAM_OFFSET_HANDLER(%ebp), %ecx 
   mov PARAM_OFFSET_VECTOR(%ebp), %ebx
   imul $0x8, %ebx 
   lea idt, %eax 
   add %ebx, %eax 
   xor %ebx, %ebx 
   mov %ecx, %ebx 
   and 0xFFFF, %ebx 
   mov %ebx, (%eax)   /* setting offset value (First two bytes) */ 
   mov PARAM_OFFSET_SEL(%ebp), %ebx
   movw %bx, 16(%eax)  /* setting selector value (third and fouth byte) */ 
   movb $0x0, 32(%eax)  /* set unsused and reserved bits (bits 0 -4 and bits 5-7 of fifth byte) */ 
   xor %ebx, %ebx 
   mov PARAM_OFFSET_ISTRAP(%ebp), %ebx
   cmp %ebx, 0x0  /*check if vector belongs to an interrupt of trap*/ 
   jne .is_trap  
   movb 0xe, %bl    /* set type = interrupt (0xe) */ 
   jmp .type_filling_complete   
   .is_trap:  
        movb 0xf, %bl   /* set type = exception/trap (0xf) */ 
   .type_filling_complete:  
        nop
   pushl %ebx   
   xor %ebx , %ebx
   mov PARAM_OFFSET_DPL(%ebp), %ebx
   shl $0x1, %ebx
   or $0x8, %ebx
   shl $0x4, %ebx
   or %ebx, (%esp)
   popl %ebx
   movb %bl, 40(%eax)
   xor %ebx, %ebx 
   mov %ecx, %ebx 
   shr $0x10, %ebx
   movl %ebx, 48(%eax) 
   movl %ebp, %esp
   pop %ebp
   ret 

_alltraps:
   pushl %ds 
   pushl %es 
   pushal
   movw $GD_KD, %ax
   movw %ax, %ds
   movw %ax, %es
   pushl %esp 
   call trap 

.data
PARAM_OFFSET_VECTOR:
                    .long 0x24
PARAM_OFFSET_ISTRAP:
                    .long 0x14
PARAM_OFFSET_SEL:
                    .long 0x10
PARAM_OFFSET_HANDLER:
                    .long 0xc
PARAM_OFFSET_DPL:
                    .long 0x8