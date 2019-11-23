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

> ## Exercise 8
> 1. Explain the interface between printf.c and console.c.  
>    Specifically, what function does console.c export? How is this function used by printf.c?
> ##### Solution:  
>     Character passed for the purpose of printing in cprintf function are passed to console.c
>     "cputchar" function to carryout I/O tasks required to print a character on screen

> 2. Explain the following from console.c 
> <code>
>         if (crt_pos >= CRT_SIZE) {
>            int i;
>            memmove(crt_buf, crt_buf + CRT_COLS, (CRT_SIZE - CRT_COLS) * sizeof(uint16_t));
>            for (i = CRT_SIZE - CRT_COLS; i < CRT_SIZE; i++)
>                crt_buf[i] = 0x0700 | ' ';
>            crt_pos -= CRT_COLS;
>         }
>  </code>

> ##### Solution: 
> This creates a new line in the end of the code and scrolling the output down.
> Console output requires tp shpw the most recent n bytes which could be stored in display buffer  
> When buffer size is greater than screen size of the display, the display buffer is updated  
> The display buffer can contain 25x80 = 2000 characters. When update is required, the code  
> Replaced first 80 characters with next 80 characters and then last 80 characters are filled  
> With space or blank, producing a downwards scroll effect.

> 3. Trace the execution of the following code step-by-step:  
>  <code>
>    int x = 1, y = 3, z = 4;
>    cprintf("x %d, y %x, z %d\n", x, y, z);
>  </code> 
> In the call to cprintf(), to what does fmt point? To what does ap point?  
> List (in order of execution) each call to cons_putc, va_arg, and vcprintf.  
> For cons_putc, list its argument as well. For va_arg, list what ap points  
> to before and after the call. For vcprintf list the values of its two arguments. 

> ##### Solution:
>  Code will be executed by running command "test83". Detailed logic can be found in monitor.c.
> "fmt" points to format string or character array <"x %d, y %x, z %d">
> "ap" points to variable argument list's pointer or starting address and it is incremented
   Each time with increment size depending on format specifier, after call to va_arg.

> ###### Execution order
> <code>
>   vprintf(fmt = 0xf0101d80 "x %d, y %x, z %d" , ap = 0xf010ff44 "\001")  
>   cons_putc(c = 120)  
>   cons_putc(c = 32)    
>   ap = 0xf010ff44  
>   va_arg()  
>   ap = 0xf010ff48  
>   cons_putc(c = 49)  
>   cons_putc(c = 44)  
>   cons_putc(c = 32)  
>   cons_putc(c = 121)  
>   cons_putc(c = 32)  
>   ap = 0xf010ff48  
>   va_arg()  
>   ap = 0xf010ff4c  
>   cons_putc(c = 51)  
>   cons_putc(c = 44)  
>   cons_putc(c = 32)  
>   cons_putc(c = 122)  
>   cons_putc(c = 32)    
>   ap = 0xf010ff4c  
>   va_arg()  
>   ap = 0xf010ff50  
>   cons_putc(c = 52)  
>   cons_putc(c = 10)  
> </code>    
 
> 4. Run the following code.  
> <code>  
>    unsigned int i = 0x00646c72;
>    cprintf("H%x Wo%s", 57616, &i);
> </code>    
>  What is the output? Explain how this output is arrived at in the step-by-step manner of the  
>  Previous exercise.  
>  The output depends on that fact that the x86 is little-endian.  
>  If the x86 were instead big-endian what would you set i to in order to yield the same output?  
>  Would you need to change 57616 to a different value?

> ##### Solution:
> Code will be executed by running command "test84". Detailed logic can be found in monitor.c.
> Output:  He110 World
> Order of Execution:
>   cons_putc(c = 72)
>   vprintfmt(putch = 0xf0100a7b,
              putdat 0xf010fef2
              fmt = 0xf0101d92 "H%x Wo%s\n"
              ap = 0xf010ff34)
>   cons_putc(c = 101)
>   cons_putc(c = 49)
>   cons_putc(c = 49)
>   cons_putc(c = 48)
>   cons_putc(c = 32)
>   cons_putc(c = 87)
>   cons_putc(c = 111)
>   ap = 0xf010ff38
>   va_arg()
>   ap = 0xf010ff38
>   cons_putc(c = 114)
>   cons_putc(c = 108)
>   cons_putc(c = 100)
>   cons_putc(c = 10)

> If x86 would have been big-endian instead of little endian then value of
> ** i should be 0x726c6400 ** to produce same effect.
> There will no change required for 57616, if x86 is big-endian.
> 57616 will be stored as 0x000010e1, but this value will be interpreted as 57616  
> When it is printed back to console.  


> 5. In the following code, what is going to be printed after 'y='?)? Why does this happen?  
>  <code>
>       cprintf("x=%d y=%d", 3);
>  </code>
> ##### Solution
>  The values to printed against format specifier are stored in a memory location as a    
>  Variable length list, whose pointer is saved in variable "ap". 
>  Every console print requires call to va_arg, which retrives next argument to be printed   
>  And this involves incrementing the pointer to next value  
>  (increment size depends on format specifier or type of value stored).  
>  So in above expression after 3 printed the va_arg will increment to the pointer to   
>  Next location in variable length list, which will contain garbage value,   
>  and that will be printed.

> 6. Let's say that GCC changed its calling convention so that it pushed arguments on the stack  
>    In declaration order, so that the last argument is pushed last. How would you have to   
>    Change cprintf or its interface so that it would still be possible to pass it a  
>    Variable number of arguments?
> ##### Solution  
>  If calling convention is changed for gcc such that last argument is stored last then gcc will
>  Generate address accordingly and internal bultin libraries will be modified accordingly to  
>  Access function arguments and set up access frames. Therefore no changes will be required in  
>  cprintf


> #### Exercise 9. 
> Determine where the kernel initializes its stack, and exactly where in memory 
> Its stack is located. How does the kernel reserve space for its stack?  
> And at which "end" of this reserved area is the stack pointer initialized to point to?

> ##### Solution
> Entry.s creates a initial kernel stack and reset (fill with 0s) 32 KB region above  
> bootstacktop. It can be found in memlayout.h that kstack size = 8x4096 = 32KB  
> <code>
>      value of bootstacktop = 0xf0110000
>      So starting address of stack = 0xf0110000 - 0x00008000 = 0xf0108000
>      So virtual address of kstack is from [0xf0108000, 0xf0110000) or
>      And physical adddress of kstack is [0x00108000, 0x00110000)
> </code>

> #### Exercise 10.
> To become familiar with the C calling conventions on the x86, find the address of the  
> Test_backtrace function in obj/kern/kernel.asm, set a breakpoint there,  
> And examine what happens each time it gets called after the kernel starts.  
> How many 32-bit words does each recursive nesting level of test_backtrace push on the stack,  
> And what are those words?

> ##### Solution
> Each recursive call push following info onto the stack
>  a. Value of test argument
>  b. Return address
> When the Test_backtrace execution is started ** value of previous** ebp is also pushed by  
> Test_backtrace.

> Following is the snapshot after last recursive call (local variables ignored)

>  | ebp register || previous ebp || return address ||  argument  |
>  |              ||  (on stack)  ||    (on stack)  || (on stack) |
>  ----------------------------------------------------------------
>  | 0xf010ff38   ||  0xf010ff58  ||    0xf01000a1  || 0x00000000 |
>  ----------------------------------------------------------------
>  | 0xf010ff58   ||  0xf010ff78  ||    0xf01000a1  || 0x00000001 |
>  ----------------------------------------------------------------
>  | 0xf010ff78   ||  0xf010ff98  ||    0xf01000a1  || 0x00000002 |
>  ----------------------------------------------------------------
>  | 0xf010ff98   ||  0xf010ffb8  ||    0xf01000a1  || 0x00000003 |
>  ----------------------------------------------------------------
>  | 0xf010ffb8   ||  0xf010ffd8  ||    0xf01000a1  || 0x00000004 |
>  ----------------------------------------------------------------
>  | 0xf010ffd8   ||  0xf010fff8  ||    0xf01000a1  || 0x00000005 |
>  ----------------------------------------------------------------


> #### Exercise 11: code implemented
> ##### Comments
>  Based on x86 calling convention, the function arguments are pushed on stack first, with last  
>  Argument being pushed first. Then return address is pushed and function call is made.  
>  In function implementation value of ebp is pushed on stack and then current value of  
>  stack pointer is saved in ebp register. 

>  So at any given instance ebp register holds pointer of previous ebp, subtracting one from ebp  
>  Pointer will provide return address and subtracting 2 provides the agument list.

>  Based on the facts mentioned above a recursive or iterative function could be created to 
   Move from one ebp value to the other and unpacking return address and arguments in process.
   Iterative Implementation can be found "mon_backtrace" function is kern/monitor.c




     


