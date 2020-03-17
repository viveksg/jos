#### Environment 
<pre>
Processes are referred as environments in JOS

All the environments are maintain in envs Array. Which is initialized using boot_alloc
and memset method in pmap.c and correspoding readonly mapping is created via boot_map_region  
Which is pointed by virtual address UENVS (0xEEC00000)
</pre>