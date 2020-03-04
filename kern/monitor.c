// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/console.h>
#include <kern/kdebug.h>
#include <kern/monitor.h>
#include "khelper.h"

#define CMDBUF_SIZE	80	// enough for one VGA text line
#define ARG_LIMIT 5
#define DEBUG_STATUS_FAILED -1
#define NAME_SIZE_MAX = 40;

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "prints stack backtrace", mon_backtrace},
	{ "showmappings", "prints address mappings for give range", showmappings},
	{ "setperms", "sets permissions for give virtual address", setperms},
	{ "dumpdata", "prints data and metadata for given virtual address range", dumpdata},
	{ "dump_pgdir", "prints page directory (till index provided, 1024 MAX value)", dump_pgdir},
	{ "dump_pt", "prints page table for given virtual address (till index provided, 1024 MAX value)", dump_ptable}	
};

/***** Implementations of basic kernel monitor commands *****/

int 
showmappings(int argc, char **argv, struct Trapframe *tf){
   uint32_t start_address = kern_atoi(argv[1]) ;
   uint32_t  end_address =  kern_atoi(argv[2]);
   if(start_address > end_address || start_address < 0 )
       panic("invalid address range\n");
   cprintf("Address Range Mappings%x %x\n", start_address, end_address);
   cprintf("---------------------------------------------------------------------\n");
   print_range_data(start_address, end_address);	   
   return 0;
}

int 
setperms(int argc, char **argv, struct Trapframe *tf){
   uint32_t address = kern_atoi(argv[1]);
   uint32_t mode = kern_atoi(argv[2]);
   uint32_t new_permissions = kern_atoi(argv[3]);
   cprintf("\nxxxxxxxxxxxx Old Metadata xxxxxxxxxxxx\n");
   print_address_metadata((const void *)address);
   update_permissions(address, mode, new_permissions);
   cprintf("\nxxxxxxxxxxxx New Metadata xxxxxxxxxxxx\n");
   print_address_metadata((const void *)address);	
   return 0;
}

int 
dumpdata(int argc, char **argv, struct Trapframe *tf){
   uint32_t start_address = kern_atoi(argv[1]) ;
   uint32_t  end_address =  kern_atoi(argv[2]);
   uint8_t mode = kern_atoi(argv[3]);
   dump_data(start_address, end_address, (bool)mode);
   return 0;	
}

int 
dump_pgdir(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t index_val = kern_atoi(argv[1]);
    dump_page_directory(index_val);
	return 0;
}

int 
dump_ptable(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t virtual_address = kern_atoi(argv[1]);
	uint32_t index_val = kern_atoi(argv[2]);
    dump_page_table(virtual_address, index_val);
	return 0;
}

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t current_ebp, return_address,old_ebp;
	old_ebp = 0x0;
	current_ebp = read_ebp();
	while(current_ebp != old_ebp && current_ebp != 0){
		return_address = *((int*)current_ebp + 1);
		cprintf("ebp %x eip %x args ",current_ebp,return_address);
		print_args(current_ebp, ARG_LIMIT);
		print_line_data(current_ebp);
		old_ebp = current_ebp;
		current_ebp = *((int*)current_ebp);
	}
	return 0;
}

void
print_args(uint32_t addr, int limit){
    int i =0;
	uint32_t* start_addr = (uint32_t*)addr + 2;
	for(i = 0; i < limit; i++){
		cprintf("%08x ", *(start_addr + i));
	}
	cprintf("\n");
}

void
print_line_data(uint32_t addr){
	struct Eipdebuginfo data;
    struct Eipdebuginfo* info = NULL;
	char fname[40];
	memset(fname,'\0',sizeof(fname));
	uint32_t eip = *((uint32_t*)(addr) + 1);
	int debug_status ;
	info = &data;
	debug_status = debuginfo_eip(eip , info);
	if(debug_status != DEBUG_STATUS_FAILED){
		strncpy(fname, info->eip_fn_name, info->eip_fn_namelen);
		cprintf("%s:%d: %s+%d\n",info->eip_file, info->eip_line, fname,(eip - info->eip_fn_addr));
	}
}
/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
