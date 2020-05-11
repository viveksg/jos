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

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

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




/*
 * Lab 3: Your code here for _alltraps
 */

_alltraps:
   pushl %ds 
   pushl %es 
   pushal
   movw $GD_KD, %ax
   movw %ax, %ds
   movw %ax, %es
   pushl %esp 
   call trap 

