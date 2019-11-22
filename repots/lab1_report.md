> #### Exercise 2. 
> Use GDB's si (Step Instruction) command to trace into the ROM BIOS for a few more 
> instructions, and try to guess what it might be doing. You might want to look at Phil Storrs I/O  
> Ports Description, as well as other materials on the 6.828 reference materials page. No need to  
> figure out all the details - just the general idea of what the BIOS is doing first.

> #### Solution:
> The bootloader starts with real mode, first few instructions initializes to registers ax,ds,ss,ex
> To 0. After that line A20 is enabled. Line A20 is diabled by default due to historical reasons.
> Due to this any addresss from 1 MB wraps around zero. Line A20 is enabled by setting ouput port 
> Of keyboard controller. Bit 0 of keyboard contoller resets CPU while bit 1 enables A20.

> Ox64 is the command line of the keyboard controller, while 0x60 is the data line of the keyboard 
> contoller. The command is sent to port 0x64, while the constant value 0xdf is loaded to 
> al register and then written to output port via data line0x60.  

> After this processor is switched from realmode to protected mode by setting first bit of CR0  
> register. And code switches to protected mode. finally after this bootmain is called.   
> Function bootmain loads the jos kernel elf from disk to memory and then jumps to first intruction 
> Of kernel




> #### Excersie 3:
> Trace into bootmain() in boot/main.c, and then into readsect(). Identify the exact assembly 
> instructions that correspond to each of the statements in readsect(). Trace through the rest of 
> readsect() and back out into bootmain(), and identify the begin and end of the for loop that 
> reads the remaining sectors of the kernel from the disk. Find out what code will run when the 
> loop is finished, set a breakpoint there, and continue to that breakpoint. Then step through the 
> remainder of the boot loader.

> At what point does the processor start executing 32-bit code? What exactly causes the switch from  >  16- to 32-bit mode?  

> What is the last instruction of the boot loader executed, and what is the first instruction of   > the kernel it just loaded?  

> Where is the first instruction of the kernel?  
> How does the boot loader decide how many sectors it must read in order to fetch the entire kernel  > from disk? Where does it find this information?

> #### Solution:
> From instructions at addresses 0x7c24 to 0x7c2a, the bootloader enables first bit of CR0 register.  

> which cause the code to switch from 16 - to 32 bit mode.  
> (Note that virtual and physical address mappings are not yet created at this point)  

> Code executes in 32 bit mode form line 0x7c32


> Last instruction is executed by bootloader involves jumping to kernel's entry address.  
> This entry address is obtained by reading kernel's elf header's entry value.  
> Last Instruction is **ELFHDR->e_entry()** address of this instruction is 0x7d6b  

> First instruction kernel instruction is at physical address **0x0010000c: movw $0x1234, 0x472**  

> Bootloader reads program header from kernel's elf and checks physical address of program segment, 
> Segment Size and total number of headers. Segment Size is passed as offset variable as the  
> Argument of "readsect" function where number of sectors required are calculated as  
> numnber of sectors = offset/sectorSize;



> #### Exercise 4: reading exercis

> #### Exercise 5.   
>  Trace through the first few instructions of the boot loader again and identify the   
>  first instruction that would "break" or otherwise do the wrong thing if you were to get the   
> boot loader's link address wrong. Then change the link address in boot/Makefrag to something  
> wrong, run make clean, recompile the lab with make, and trace into the boot loader again to see  
> what happens. 

> #### Solution
>  First instruction that breaks or produce unexpected results, after linking kernel to wrong  
>  address is **ljmp $PROT_MODE_CSEG, $protcseg**.  
>  Correct execution of above instruction should allow processor to execute in 32 bit mode.  
>  But since ld address is wrong, so processor jumps to wrong address causing seg fault and this
>  Results in restart of bootload process.

> ####  Exercise 6.
>  Reset the machine (exit QEMU/GDB and start them again). Examine the 8 words of memory at  
>  0x00100000 at the point the BIOS enters the boot loader, and then again at the point the boot  
>  loader enters the kernel. Why are they different? What is there at the second breakpoint?  

> #### Solution  
> When BIOS enters (0x7c00) bootloader 0x00100000 does not have any instruction or data and all the memory  
> Region is unset or filled with zeros.  
> When Bootloader enters kernel (0x7d6b) at this point the paging is enabled at virtual to  
> Physical address mappings are created. So when memory region starting from 0x00100000 is   
> Checked it is filled with kernel code 
> **0x100000: 0x1badb002 0x00000000 ...**


> #### Exercise 7.  
>  Use QEMU and GDB to trace into the JOS kernel and stop at the movl %eax, %cr0. Examine memory   
>  at 0x00100000 and at 0xf0100000. Now, single step over that instruction using the stepi   
>  GDB command. Again, examine memory at 0x00100000 and at 0xf0100000.   

>  What is the first instruction after the new mapping is established that would fail to work  
>  properly if the mapping weren't in place? Comment out the movl %eax, %cr0 in kern/entry.S,  
>  Trace into it, and see if you were right.

> #### Solution  
> Paging is enabled by setting 31st bit of CR0 register and in  Kern/entry.S this is done  
> By instruction **movl %eax %cr0**  
> Before this instruction paging is not enabled while address 0x00100000 points to  
> Kernel start, 0xf0100000 contains 0x00000000. Afer this execution of above instruction  
> Paging is enabled and 0xf0100000 is mapped to 0x00100000 and this results in 0xf0100000  
> pointing to 0x00100000 and hence pointing to kernel's start.  

> Commenting movl %eax %cr0 doesn't enables paging and so any virtual address access results  
> SEG FAULT. And this is what exactly happens when kernel tries to execute **jmp %eax**  
> In Kern/entry.s, which involves jumping to relocated point.
