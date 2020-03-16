#include <inc/mmu.h>

# Start the CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

.set PROT_MODE_CSEG, 0x8         # kernel code segment selector
.set PROT_MODE_DSEG, 0x10        # kernel data segment selector
.set CR0_PE_ON,      0x1         # protected mode enable flag
.set CR4_PSE_ON,     0x10        # page size extension flag 
.set PROCESSOR_MODEL, 0xF0       # Processor model is stored in bit 4-7 in eax
.set PROCESSOR_FAMILY, 0xF00     # Processor family is stored in bits 8-11 in eax
.set PENTIUM_IDENTIFIER, 0x01    # Identifier for pentium processor
.set PENTIUM_FAMILY_ID, 0x05     # COde for pentium family processor
.set IDENTIFICATION_TYPE, 0x01   # Value to be set in eax, for model info operation
.set ID_BIT_SET, 0x200000 # Value to be set in eax, for CPUID support detection
.set ZERO, 0x0
.globl start
start:
  .code16                     # Assemble for 16-bit mode
  cli                         # Disable interrupts
  cld                         # String operations increment

  # Set up the important data segment registers (DS, ES, SS).
  xorw    %ax,%ax             # Segment number zero
  movw    %ax,%ds             # -> Data Segment
  movw    %ax,%es             # -> Extra Segment
  movw    %ax,%ss             # -> Stack Segment

  # Enable A20:
  #   For backwards compatibility with the earliest PCs, physical
  #   address line 20 is tied low, so that addresses higher than
  #   1MB wrap around to zero by default.  This code undoes this.
seta20.1:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.1

  movb    $0xd1,%al               # 0xd1 -> port 0x64
  outb    %al,$0x64

seta20.2:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.2

  movb    $0xdf,%al               # 0xdf -> port 0x60
  outb    %al,$0x60

  # Switch from real to protected mode, using a bootstrap GDT
  # and segment translation that makes virtual addresses 
  # identical to their physical addresses, so that the 
  # effective memory map does not change during the switch.
  lgdt    gdtdesc
  movl    %cr0, %eax
  orl     $CR0_PE_ON, %eax
  movl    %eax, %cr0
  
  # Jump to next instruction, but in 32-bit code segment.
  # Switches processor into 32-bit mode.
  ljmp    $PROT_MODE_CSEG, $protcseg

  .code32                     # Assemble for 32-bit mode
protcseg:
  # Set up the protected-mode data segment registers
  movw    $PROT_MODE_DSEG, %ax    # Our data segment selector
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %fs                # -> FS
  movw    %ax, %gs                # -> GS
  movw    %ax, %ss                # -> SS: Stack Segment

cpuid_support_check:
  pushfw 
  movl (%esp), %eax;
  movl %eax, %ebx
  xor $ID_BIT_SET, %ebx
  push %ebx
  popfw
  pushfw
  pop %ebx
  popfw  # set old value of eflag

compare_eflag_values: 
  cmp %eax, %ebx
  je bootmain_call

extract_processor_model:
  movl $IDENTIFICATION_TYPE, %eax
  cpuid
  push %eax
  
processor_family_check:
  andl $PROCESSOR_FAMILY, %eax
  cmp $PENTIUM_FAMILY_ID, %eax
  jl bootmain_call

processor_model_check:
  pop %eax
  andl $PROCESSOR_MODEL, %eax
  cmp $PENTIUM_IDENTIFIER, %eax 
  jge pagesizeext
  jmp bootmain_call

pagesizeext:
  movl  %cr4, %eax
  orl   $CR4_PSE_ON, %eax
  movl  %eax, %cr4

bootmain_call:
  # Set up the stack pointer and call into C.
  movl    $start, %esp
  call bootmain

  # If bootmain returns (it shouldn't), loop.
spin:
  jmp spin

# Bootstrap GDT
.p2align 2                                # force 4 byte alignment
gdt:
  SEG_NULL				# null seg
  SEG(STA_X|STA_R, 0x0, 0xffffffff)	# code seg
  SEG(STA_W, 0x0, 0xffffffff)	        # data seg

gdtdesc:
  .word   0x17                            # sizeof(gdt) - 1
  .long   gdt                             # address gdt

