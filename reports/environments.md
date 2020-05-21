#### Environment 
<pre>
Processes are referred as environments in JOS

All the environments are maintain in envs Array. Which is initialized using boot_alloc
and memset method in pmap.c and correspoding read only mapping is created via boot_map_region  
Which is pointed by virtual address UENVS (0xEEC00000)

After initialization of memory management unit, environment related data-structures are   Intialized. The first step in this process is to call env_init function from init.c.
This function creates a linked-list corresponding to environments array.
Also it initialized the enivonment structure while adding them to list.
The order of environment structure is same as that in environent array.
This is done because after loading environment binary, kernel run environment
At index 0 of environment array. So correct order is necesary to place first environment on
Correct index. 

After environment data-structures are initialized, an environment is created for the 
First "hello-world" binary. This process involves making a call to env_create,  
Which allocates requires pages and creates mappings required for the environment through
"env_alloc" function. Also env_create function loads the binary at environment's address space.
This is done  by load_icode method

Function "env_alloc" first gets a free environment structure for from env_free list.
If it is not able to do so, then it throws an error. Once environment structure is obtained.
Corresponding page directory, page tables and mapping is done in "env_setup_vm" function.
And other values of structure like trap-frame data, segement registers etc are set up.

env_setup_vm allocates a single page for environment's page directory.
And creates identical mapping as that for kernel page directory, as each environment has
its own mappings for kernel, which is identical to that of kernel's page directory.
Page directory entry from environment's page directory table is done in itself.

load_icode function loads the binary of environent to its corresponding ELF structure
And read the program header and creates region for codesection and stack from ELF, in
Environment's address space through region_alloc code. After allocation of regions,
load_icode, parse the loadable segments information of program header, and loads
The code at virtual address provided considering size on image and size on address space.
This process is done iteratively for all loadable type progeam headers.

region_alloc, allocates page required for the storing environment's binary code and stack  
Sections. And it also insets those pages in environment's page directoy.

env_run check current environment and switch to new environment  (currenv is not NULL)  
Sets up some structure values, loads the environment's page directory to in CR3 register.
And loads the trap frame via env_pop_tf


</pre>
