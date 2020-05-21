#### Page Size Extension:  
<pre>
Starting from Pentium processors, intel allowed to have page size to be 4 MB  
in addition to choice of default 4KB pages. To enable page size extension  
4th bit of CR4 register is to be set. 4 MB pages are not linked to a page table,  
4MB pages are directly linked via page directory entry. 

A 4MB pointer page directory entry has first 10 MSBs of the page's physical address
last 13 bits as permission bits and remaining bits as reserved.  

The page directory can have default page adirectory entries pointing to page 
Tables and entries pointing to 4MB pages. The extended page pointing page directory
Entries have 7th (0-indexed) LSB set.  

JOS kernel is from 0xF0000000 to 0xFFFFFFFF. Under default Paging mechanis  
This requires 256/4 = 64MB space just to maintain kernel page tables,
While with extended paging enabled, 
This mapping can be done in 64 extended page directory entries.

To enable extended page size, we first need to ensure that processor architecture
Supports page size extension. And then 4th bit of CR4 register is enabled.
In this kernel, when bootloader transits to 32 bit protected mode.  
CR4 bit was introduced in Pentium processors, so if we can detect the processor  
Model is greater than or equal to pentium model, then we can proceed with  
Page size extenstion logic.
</pre>
   
**CPUID (detecting CPUID support and setting CR4)**  
<pre>
To detect type/model of processor, "CPUID" instruction is used on Intel processors.  
CPUID actually provides whole lot of information.  
But some old processors doesn't support this instruction.  
To detect if processor supports CPUID instruction, the software should attempt to  
Modify ID bit (21st bit) of EFLAGS register. If that is possible then processor  
Can execute CPUID instruction.

Bootloader is modified to obtain the current ELFAG value using "pushf" instruction  
Moving the value of EFLAGS from stack to eax, and ebx. And then inverting 21st bit in ebx.
And storing the value of ebx, back in EFLAGs using "popf" instruction. And then reading  
EFLAGs again to check if modificaiton was successful. On successful modification of EFLAGs,
Bootloader restores the initial value of EFLAGs and move to processor model detection.  

CPUID delivers the results in eax, ebx, ecx, edx registers based on value set in eax register  
Before execution CPUID instruction. For model detection EAX is set to 0x1 and CPUID is executed.
Bits 3-7 contains model bits, Bits 8-11 contains processor family code bits.
If processor family code is greater than 0x4 and processor model is greater than 0  
Then page size extension support is available in processor. And 4th bit of CR4 register is set  
To enable page size extension.
</pre>
```asm

.set CR4_PSE_ON,     0x10        # page size extension flag 
.set PROCESSOR_MODEL, 0xF0       # Processor model is stored in bit 4-7 in eax
.set PROCESSOR_FAMILY, 0xF00     # Processor family is stored in bits 8-11 in eax
.set PENTIUM_IDENTIFIER, 0x01    # Identifier for pentium processor
.set PENTIUM_FAMILY_ID, 0x05     # COde for pentium family processor
.set IDENTIFICATION_TYPE, 0x01   # Value to be set in eax, for model info operation
.set ID_BIT_SET, 0x200000 # Value to be set in eax, for CPUID support detection
.set ZERO, 0x0
.
.
.
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
```

Modifications in pmap.c:
<pre>
In boot_map_region mapping of kernel involves checking first about page size extension support  
Through "is_pgsize_extension_supported()". Which checks if CR4 bit is set.  
On true result, boot_map_region is called with PTE_PS permission bit set in permission variable.  
The mapping loop in boot_map_region involves checking PTE_PS bit in every iteration  
And if this bit is set, then extended page related entry is created in page directory,  
Otherwise entry for corresponding to page table is created in page directory.  
</pre>

```C
static void  
boot_map_region(pde_t *pgdir, uintptr_t va, size_t size, physaddr_t pa, int perm)  
{  
	// Fill this function in  
	bool requires_extended_pgsize = (perm & PTE_PS);  
	uint32_t num_pages = size / PGSIZE ;  
	uint32_t i = 0, create = 1, vaddr_limit = va + size;  
	pte_t * pte;	  
	for(i = 0; i < num_pages && va < vaddr_limit; i++, va = va + PGSIZE, pa =pa + PGSIZE)  
	{  
           if(requires_extended_pgsize)  
		   {
              pgdir[PDX(va)] = (pa & 0xFFC00000) | perm | PTE_P;  
		   }  
		   else  
		   {  
		      pte = pgdir_walk(pgdir, (char *)va , create);  
		      if(pte == NULL)  
		           panic("page allocation error");  
              *pte = pa|perm|PTE_P;	     
		   }  
		   
	}  
}  
```


