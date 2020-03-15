#### Page Size Extension:  
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
In this kernel, when bootloader transits to 32 bit protected mode

<code>
pagesizeext:  
  movl  %cr4, %eax  
  orl   $CR4_PSE_ON, %eax  
  movl  %eax, %cr4  
</code>

Modifications in pmap.c:

In boot_map_region mapping of kernel involves checking first about page size extension support  
Through "is_pgsize_extension_supported()". Which checks if CR4 bit is set.  
On true result, boot_map_region is called with PTE_PS permission bit set in permission variable.  
The mapping loop in boot_map_region involves checking PTE_PS bit in every iteration  
And if this bit is set, then extended page related entry is created in page directory,  
Otherwise entry for corresponding to page table is created in page directory.  
<code>
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
</code>


