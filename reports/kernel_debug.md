#### Kernel debugging:
Following are additional kernel debugging functions added to this kernel

1. **showmappings**:
   Usage: showmappings <low_address> <high_address>   
   Description: Prints address metadata like page table base, virtual and physical page address  
                Permission bits, for all the addresses between low and high address.
                
2. **setperms**:
   Usage: setperms <address> <mode> <new_permissions>
   Description: Sets the permission page table entry associated with virtual address provided  
                As first argument. It works in three modes.  
                If mode = 1 permission bits are all unset, for mode = 2 all permisson bits are   Set, for mode = 3 permission bits are set to value provided in "new_permissions".   

3. **dumpdata**:
   Usage: dumpdata <low_address> <high_address> <is_address_range_physical> 
   Description: Prints memory contents for address range between  
                low and high address. Address are considered as virtual addresses  
                If "is_address_range_physical" value is 0,  
                And physical address otherwise.

4. **dump_pgdir**:  
   Usage: dump_pgdir  <number of entries to be printed>  
   Description: Prints contents of page directory from 0th entry, to number provided in    
                command line parameter, max value can be 1024.  

5. **dump_pt**: dump_pt <address>  <number of entries to be printed>  
   Description: Prints the contents of page tables associated with virtual address  
                From 0th entry, to number provided in command line parameter,  
                Max value can be 1024.