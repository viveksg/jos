/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


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


#define IDT_CREATOR(name)    \
   .globl name;   \
   .type name, @function;   \
   name: \
     push %ebp; \
     mov %esp, %ebp; \
     pushl %eax; \
     pushl %ebx; \
     pushl %ecx; \
     pushl %edx; \
     call _setup_idt; \
     popl %edx; \
     popl %ecx; \
     popl %ebx; \
     popl %eax; \
     mov %ebp, %esp; \
     pop %ebp; \
     ret; 

#define SET_IDT(vector_val, is_trap, sel, off, dpl)   \
   movl %esp, %edx; \
   pushl $(vector_val) ; \
   pushl $(is_trap) ; \
   pushl $(sel); \
   pushl $(off); \
   pushl $(dpl); \
   call _create_idt_entry; \
   movl %edx, %esp; 


.text
.extern idt
IDT_CREATOR(CREATE_IDT)
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

# ---------------External Interrupt handers---------------------------
TRAPHANDLER_NOEC(HANDLER_IRQ_0, IRQ_0)
TRAPHANDLER_NOEC(HANDLER_IRQ_1, IRQ_1)
TRAPHANDLER_NOEC(HANDLER_IRQ_2, IRQ_2)
TRAPHANDLER_NOEC(HANDLER_IRQ_3, IRQ_3)
TRAPHANDLER_NOEC(HANDLER_IRQ_4, IRQ_4)
TRAPHANDLER_NOEC(HANDLER_IRQ_5, IRQ_5)
TRAPHANDLER_NOEC(HANDLER_IRQ_6, IRQ_6)
TRAPHANDLER_NOEC(HANDLER_IRQ_7, IRQ_7)
TRAPHANDLER_NOEC(HANDLER_IRQ_8, IRQ_8)
TRAPHANDLER_NOEC(HANDLER_IRQ_9, IRQ_9)
TRAPHANDLER_NOEC(HANDLER_IRQ_10, IRQ_10)
TRAPHANDLER_NOEC(HANDLER_IRQ_11, IRQ_11)
TRAPHANDLER_NOEC(HANDLER_IRQ_12, IRQ_12)
TRAPHANDLER_NOEC(HANDLER_IRQ_13, IRQ_13)
TRAPHANDLER_NOEC(HANDLER_IRQ_14, IRQ_14)
TRAPHANDLER_NOEC(HANDLER_IRQ_19, IRQ_19)

_setup_idt:
  pushl %ebp
  mov %esp, %ebp 
  SET_IDT(T_DIVIDE, TYPE_INTR, GD_KT, HANDLER_DE_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_DEBUG, TYPE_INTR, GD_KT, HANDLER_DB_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_NMI, TYPE_INTR, GD_KT, HANDLER_NMI, PRIVILEGE_KERNEL)
  SET_IDT(T_BRKPT, TYPE_INTR, GD_KT, HANDLER_BP_TRAP, PRIVELEGE_USER)
  SET_IDT(T_OFLOW, TYPE_INTR, GD_KT, HANDLER_OF_TRAP, PRIVILEGE_KERNEL)
  SET_IDT(T_BOUND, TYPE_INTR, GD_KT, HANDLER_BR_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_ILLOP, TYPE_INTR, GD_KT, HANDLER_UD_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_DEVICE, TYPE_INTR, GD_KT, HANDLER_NM_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_DBLFLT, TYPE_ABORT, GD_KT, HANDLER_DF_ABORT, PRIVILEGE_KERNEL)
  SET_IDT(T_TSS, TYPE_INTR, GD_KT, HANDLER_TS_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_SEGNP, TYPE_INTR, GD_KT, HANDLER_NP_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_STACK, TYPE_INTR, GD_KT, HANDLER_SS_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_GPFLT, TYPE_INTR, GD_KT, HANDLER_GP_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_PGFLT, TYPE_INTR, GD_KT, HANDLER_PF, PRIVILEGE_KERNEL)
  SET_IDT(T_FPERR, TYPE_INTR, GD_KT, HANDLER_MF_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_ALIGN, TYPE_INTR, GD_KT, HANDLER_AC_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_MCHK, TYPE_ABORT, GD_KT, HANDLER_MC_ABORT, PRIVILEGE_KERNEL)
  SET_IDT(T_SIMDERR, TYPE_INTR, GD_KT, HANDLER_XF_ERROR, PRIVILEGE_KERNEL)
  SET_IDT(T_SYSCALL, TYPE_INTR, GD_KT, HANDLER_SYSCALL, PRIVELEGE_USER)
  # ---------------- EXTERNAL INTERRUPTS-----------------------------------
  SET_IDT(IRQ_0, TYPE_INTR, GD_KT, HANDLER_IRQ_0, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_1, TYPE_INTR, GD_KT, HANDLER_IRQ_1, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_2, TYPE_INTR, GD_KT, HANDLER_IRQ_2, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_3, TYPE_INTR, GD_KT, HANDLER_IRQ_3, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_4, TYPE_INTR, GD_KT, HANDLER_IRQ_4, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_5, TYPE_INTR, GD_KT, HANDLER_IRQ_5, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_6, TYPE_INTR, GD_KT, HANDLER_IRQ_6, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_7, TYPE_INTR, GD_KT, HANDLER_IRQ_7, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_8, TYPE_INTR, GD_KT, HANDLER_IRQ_8, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_9, TYPE_INTR, GD_KT, HANDLER_IRQ_9, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_10, TYPE_INTR, GD_KT, HANDLER_IRQ_10, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_11, TYPE_INTR, GD_KT, HANDLER_IRQ_11, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_12, TYPE_INTR, GD_KT, HANDLER_IRQ_12, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_13, TYPE_INTR, GD_KT, HANDLER_IRQ_13, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_14, TYPE_INTR, GD_KT, HANDLER_IRQ_14, PRIVILEGE_KERNEL)
  SET_IDT(IRQ_19, TYPE_INTR, GD_KT, HANDLER_IRQ_19, PRIVILEGE_KERNEL)

  movl %ebp, %esp
  popl %ebp
  ret

_create_idt_entry:
   pushl %ebp
   mov %esp, %ebp
   .clear_regs:
       xor %eax, %eax 
       mov %eax, %ebx 
       mov %eax, %ecx
       mov PARAM_OFFSET_HANDLER(%ebp), %ecx 
   .calculate_gate_address:
       mov PARAM_OFFSET_VECTOR(%ebp), %ebx
       imul $0x8, %ebx 
       lea idt, %eax 
       add %ebx, %eax 
   .set_handler_offset_bits_0_15:   
       xor %ebx, %ebx 
       mov %ecx, %ebx 
       and $0xFFFF, %ebx 
       mov %ebx, (%eax)      /* setting handler offset value (First two bytes) */ 
   .set_selector_and_reserved_bytes:
       mov PARAM_OFFSET_SEL(%ebp), %ebx
       movw %bx, 0x02(%eax)  /* setting selector value (third and fouth byte) */ 
       movb $0x0,0x04(%eax)  /* set unsused and reserved bits (bits 0 -4 and bits 5-7 of fifth byte) */ 
   .set_gate_type:           /*interrupt or exception */
       xor %ebx, %ebx 
       mov PARAM_OFFSET_ISTRAP(%ebp), %ebx
       cmp  $0x0, %ebx  /*check if vector belongs to an interrupt of trap*/ 
       jne .is_trap  
       movb $0xe, %bl    /* set type = interrupt (0xe) */ 
       jmp .set_dpl_and_p_bits  
       .is_trap:  
             movb $0xf, %bl   /* set type = exception/trap (0xf) */  
   .set_dpl_and_p_bits:  
      pushl %ebx   
      xor %ebx , %ebx
      mov PARAM_OFFSET_DPL(%ebp), %ebx
      shl $0x1, %ebx
      or $0x8, %ebx  /* 1xx0 combines p bit and dpl bit and s bit, p =1, dpl in ebx, s = 0 */
      shl $0x4, %ebx 
      or %ebx, (%esp) /* combine p,dpl,s bits with type bits */
      popl %ebx     
      movb %bl, 0x5(%eax) /*sets type,s, dpl, p bits */
   .set_handler_offset_bits_16_31:   
      xor %ebx, %ebx 
      mov %ecx, %ebx 
      shr $0x10, %ebx
      movl %ebx, 0x6(%eax) /* setting handler offset value (last two bytes) */ 
   .end_function:   
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
.equ PARAM_OFFSET_VECTOR, 0x18
.equ PARAM_OFFSET_ISTRAP, 0x14
.equ PARAM_OFFSET_SEL, 0x10
.equ PARAM_OFFSET_HANDLER, 0xc
.equ PARAM_OFFSET_DPL, 0x8