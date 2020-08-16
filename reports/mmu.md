#### Memory Allocation and Page management:  
<pre>
1. boot_alloc: This method allocates number of bytes (n), provided   
               In function argument and returns the address of next  
               Free byte, stored in nextfree, which is a **static** variable.  
               Initially nextfree points to end of kernel's bss section.   
               Since nextfree is a static variable, so its value is preserved between calls.  
               If number of bytes passed in boot_alloc argument is 0 then, boot_alloc  
               simply returns current value of nextfree. Otherwise it saves the old value  
               of nextfree in temp variable, And adds number of bytes to nextfree    
               And return old value of nextfree through temp variable.  

2. mem_init: Initializes the memory blocks obtianed by boot_alloc  to 0.
             And initializes page datastructures. Once the pages and page directory, page tables
             Are initialized, mem_init creates range based bootmappings.  
             This function at very beginning allocates 4KB bytes using boot_alloc to create   
             Page directory and initialize it to 0. It then allocates enough bytes to hold
             page Array, again using boot_alloc. After this page and page table initialization  
             is carried out and range based mappings are created.  

3. page_init: This function initializes all the pages of pages array and add the free ones  
              in **page_free_list** linked list. All the pages are inserted at the beginning of
              The linked list. Idea is to iterate through whole address space at 
              **page level granularity**, that is each iteration involves single page.
              And initialize the page and add it to page_free_list linked list.
              First page and pages between IOPHYSMEM and Kernel limit ( which includes kernel   Page directory and pages array also) are not supposed to be added in page_free_list.  

4. page_alloc: This function removes the first page, which is the first node in    
               page_free_list linked list. And then obtains the kernel address   
               Of first page and initialize all bits to 0 depending upon value of   
               Allocation flags.  Once this is done, head pointer is adjusted to  
               Point to next free page in linked list. And next pointer of removed  
               First page is set to NULL

5. page_free: This function  decrements the reference count of the  
              Page and once reference count reaches to zero,  
              The page reinserted to beginning of page free list   
              Linked list.  

</pre>
#### Pointer Exercise    
```C
        mystery_t x;
	char* value = return_a_pointer();  
	*value = 10;  
	x = (mystery_t) value;  
```
<pre>
Each memory reference has to be processed through mmu   
For dereferencing, and mmu input is virtual address  
So type of **x** should be ** uintptr_t **    
</pre>
#### Page Table Management
<pre>
Index calculation logic:  
JOS virtual memory system follows 2-level paging architecture.  
The first level consists of page directory which can hold reference to 1024 pages.
Each entry in page directory is a 32 bit unsigned integer, such that First 20 bits
MSBs are base address of a page table, and last 12 bits are permission bits.  
So size of page directory is 4KB.

Each page table entry points to base address of Page.  
First 20 MSBs are base address of page, and last 12 bits are permission bits.
Page table is capable of storing 1024 page addresses,  
So size of page table is 4KB.

Default page size is 4KB, so page directory, and page tables are stored as a page.

A 32 bit virual adddress consists of three parts
a. Page directory offset: First 10 bits from MSB of virtual address(va), 
                          Usually obtained by right shifting va by 22 bits (PDX macro).
                          And then taking out last 10 bits (& 0x3ff)
b. Page Table offset: Bits [21 - 12], left shifted by 12 bits (PTX macro),  
                      And then taking out last 10 bits (& 0x3ff)
c. Page Offset: Bits [11 - 0]         

1. pgdir_walk(): This function returns entry in page table entry,     
                 Corresponding to virtual address provided in parameter of pdgir_walk function.  
                 For the given virtual address first page directory offset and page table   offsets are calculated using logic mentioned points a, b, c.  
                 Page directory offset is combined with page directory base, and checked if  
                 There is any entry (page table) on this address using PTE_P (0) bit,  
                 If not a new page is taken out from free page list, and used for the new page   Table. Kernel virtual address is returned for page table entry.


2. boot_map_region(): Maps virtual memory range to physical memory range, in page by page way.    
                      This function first checks if the mapping involves extended (4MB) pages  
                      If so, thn each iteration involves, just inserting base of page address in  
                      Page directory entry. Otherwise, call to pgdir_walk is made which sets the  
                      Page table base in page directory and creates corresponding page table  page, If required. In both cases permission bits and present bits are  
                      Added in their corresponding locations in page table or page directory entry.  

3. page_lookup(): Makes a call to pgdir_walk with create flag set to 0, to obtain the   
                  corresponding page table entry PTE.  First 20 bits from MSB are taken  
                  (page address) and passed to pa2page function which returns  
                  A struct PageInfo pointer, for the page.  

4. page_remove(): Obtains the page info for the page using page lookup, and calls page_decref  
                  to decrement and check the reference count after decrement,  
                  If reference count is 0, then corresponding page is added to free page list.  
                  Back In page remove, the page table entry is set to 0 and tlb is invalidated.  


5. page_insert(): Inserts the address of first byte of the page (along with permissions)  
                  At the corresponding page table offset, given for a virtual address.
                  The page info and permissions is provided in function paramerter  
                  A call to pdgir_walk provides the corresponding page table entry.
                  The reference count for page is incremented,  
                  If there is some other page is already mapped to given virtual address,  
                  Then that page is removed by making call to page_remove().  
                  Once this is done, the 20 bit page is physical address of the page is computed  
                  And inserted in page table entry along with corresponding permissions.

</pre>

#### Mapped region info:

1. **UPAGE:**
<pre>
Start address           = KERNBASE(or KSTACKTOP) - 4xPTSIZE  
                        = 0xF0000000 - 0x1000000   
                        = 0xEF000000  
          
Page Directory Index    = 0xEF000000 >> 22 = 0x3BC          

End Address             = 0xEF000000 + 0x400000 - 0x1  
                        = 0xEF3FFFFF  
Page Directory Index    = 0xEF3FFFFF >> 22 = 0x3BD  
</pre>
2. **KSTACK:**
<pre>
Start address           = KSTACKTOP - KSTKSIZE  
                        = 0xF0000000 - 0x8000  
                        = 0xEFFF8000  
          
Page Directory Index    = 0xEFFF8000 >> 22 = 0x3BF          

End Address             = 0xEFFF8000 + 0x8000 - 0x1  
                        = 0xEFFFFFFF  
Page Directory Index    = 0xEFFFFFFF >> 22 = 0x3BF  
</pre>
3. **KERNEL:** 
<pre>
Start address           = KERNBASE   
                        = 0xF0000000  
                        = 0xF0000000  
          
Page Directory Index    = 0xF0000000 >> 22 = 0x3C0          

End Address             = 0xFFFFFFFF - 0x1000 - 0x1  
                        = 0xFFFEEEEE  
Page Directory Index    = 0xFFFEEEEE >> 22 = 0x3FF    

</pre>
#### Page directory mappings  

<pre>
0 to 955 (0x3bb): No mappings all entries 0x00000000
956 - 957 (0x3bc - 0x3bd): Page tables for UPAGES
959 (0x3bf): Page tables for Kernel Stack
960 (0x3c0) onwards: Page tables for kernel 
                     Incase of extended page size, 0x3c0 onwards entries will contain  
                     Address of extended pages mapped to kernel
 
 0x000003bc: 0x003fd007 0x0011d005 0x00000000 0x003fe007   
 0x000003c0: 0x003ff027 0x003fc027 0x003fb027 0x003fa027  
 0x000003c4: 0x003f9027 0x003f8027 0x003f7027 0x003f6027  
 0x000003c8: 0x003f5027 0x003f4027 0x003f3027 0x003f2027  
 0x000003cc: 0x003f1027 0x003f0027 0x003ef027 0x003ee027  
 0x000003d0: 0x003ed027 0x003ec027 0x003eb027 0x003ea027  
 0x000003d4: 0x003e9027 0x003e8027 0x003e7027 0x003e6027  
 0x000003d8: 0x003e5027 0x003e4027 0x003e3027 0x003e2027   
 0x000003dc: 0x003e1027 0x003e0027 0x003df027 0x003de027  
 0x000003e0: 0x003dd007 0x003dc007 0x003db007 0x003da007  
 0x000003e4: 0x003d9007 0x003d8007 0x003d7007 0x003d6007  
 0x000003e8: 0x003d5007 0x003d4007 0x003d3007 0x003d2007  
 0x000003ec: 0x003d1007 0x003d0007 0x003cf007 0x003ce007  
 0x000003f0: 0x003cd007 0x003cc007 0x003cb007 0x003ca007  
 0x000003f4: 0x003c9007 0x003c8007 0x003c7007 0x003c6007  
 0x000003f8: 0x003c5007 0x003c4007 0x003c3007 0x003c2007  
 0x000003fc: 0x003c1007 0x003c0007 0x003bf007 0x003be007  

------------------------------------------------------------  
When extended page size is allowed, [E] denotes extended pages


 0x000003bc: 0x003fd007 0x0011d005 0x00000000 0x003fe007  
 0x000003c0: 0x000000e3 [E] 0x004000e3 [E] 0x008000e3 [E] 0x00c000e3 [E]  
 0x000003c4: 0x010000e3 [E] 0x014000e3 [E] 0x018000e3 [E] 0x01c000e3 [E]  
 0x000003c8: 0x020000e3 [E] 0x024000e3 [E] 0x028000e3 [E] 0x02c000e3 [E]  
 0x000003cc: 0x030000e3 [E] 0x034000e3 [E] 0x038000e3 [E] 0x03c000e3 [E]  
 0x000003d0: 0x040000e3 [E] 0x044000e3 [E] 0x048000e3 [E] 0x04c000e3 [E]  
 0x000003d4: 0x050000e3 [E] 0x054000e3 [E] 0x058000e3 [E] 0x05c000e3 [E]  
 0x000003d8: 0x060000e3 [E] 0x064000e3 [E] 0x068000e3 [E] 0x06c000e3 [E]  
 0x000003dc: 0x070000e3 [E] 0x074000e3 [E] 0x078000e3 [E] 0x07c000e3 [E]  
 0x000003e0: 0x08000083 [E] 0x08400083 [E] 0x08800083 [E] 0x08c00083 [E]  
 0x000003e4: 0x09000083 [E] 0x09400083 [E] 0x09800083 [E] 0x09c00083 [E]  
 0x000003e8: 0x0a000083 [E] 0x0a400083 [E] 0x0a800083 [E] 0x0ac00083 [E]  
 0x000003ec: 0x0b000083 [E] 0x0b400083 [E] 0x0b800083 [E] 0x0bc00083 [E]  
 0x000003f0: 0x0c000083 [E] 0x0c400083 [E] 0x0c800083 [E] 0x0cc00083 [E]  
 0x000003f4: 0x0d000083 [E] 0x0d400083 [E] 0x0d800083 [E] 0x0dc00083 [E]  
 0x000003f8: 0x0e000083 [E] 0x0e400083 [E] 0x0e800083 [E] 0x0ec00083 [E]  
 0x000003fc: 0x0f000083 [E] 0x0f400083 [E] 0x0f800083 [E] 0x0fc00083 [E] 
</pre>
#### Permission mechanism
<pre>
Last 12 bits of page directory or page table entry provides permissions.
Bit 1 is read/write bit, if not set then page is read only, otherwise 
Page is writable. Bit 2 is user/supervisor bit, if not set then page  
Is assigned supervisor priveleges, otherwise page is assigned user priveleges.
When kernel and user enironments are in same address space, combination of  
These two bits are used to maintain desired privelege levels.
</pre>

#### Total Physical memory supported by JOS:  
<pre>
KERNBASE = 0xF0000000  
MAX ADDRESS = 0xFFFFFFFF  
Total virtual address space which is available for mapping = 0xFFFFFFFF - 0xF0000000 + 0x1    
                                                           = 0x0FFFFFFF + 0x1
                                                           = 0x10000000  
                                                           = 256 MB    
So 256MB of virtual address space can be mapped to physical memory  
Therefore JOS supports 256MB of physical memory
</pre>
#### Space Overhead
<pre>
Page directory = 1024 x 4 = 4KB
Page Tables = 1024 x 1024 x 4 = 4MB
Total space overhead for memory management = 4100 KB
</pre>
#### EIP High Address (above kernbase) execution  
<pre>
As per kern/entry.S eip transition above kernbase occurs at "relocated" label.  
File entrypgdir.c creates page directory with mapping of two regions
[0, 4MB) and [KERNBASE, KERNBASE + 4MB). Both of these virtual address space  
Regions are mapped to physican region [0, 4MB). This allows execution of eip  
At low and high(above KERNBASE) addresses.  This transition is necessary
Because low address mapping is just created to support initial bootloading,
While rest of the kernel is supposed to be loaded above KERNBASE + 4MB.
</pre>



