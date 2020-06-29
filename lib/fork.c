// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *)utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).
	uint32_t perms = (PTE_P | PTE_U | PTE_W);
	pte_t pte = uvpt[PGNUM(addr)];
	envid_t envid = sys_getenvid();
	void *target_addr = (void *)(ROUNDDOWN(addr, PGSIZE));
	if ((err & FEC_WR) != FEC_WR || (pte & (PTE_COW)) != PTE_COW)
	{   
		panic("pgfault: access not write or page not COW");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	if ((r = sys_page_alloc(envid, (void *)PFTEMP, perms)))
	{
		panic("pgfault handler: %e", r);
	}

	memcpy((void *)PFTEMP, target_addr, PGSIZE);
	if ((r = sys_page_map(envid, (void *)PFTEMP, envid, target_addr, perms)))
	{
		panic("pgfault handler: %e", r);
	}
	if ((r = sys_page_unmap(envid, (void *)PFTEMP)))
	{
		panic("pgfault handler: %e", r);
	}
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	uint32_t perms = (PTE_P | PTE_U | PTE_COW);
	void *addr = (void *)(pn * PGSIZE);
	pte_t pte = uvpt[pn];
	envid_t parent = sys_getenvid();
	if ((pte & PTE_W) == PTE_W || (pte & PTE_COW) == PTE_COW)
	{
		r = sys_page_map(parent, addr, envid, addr, perms) |
			sys_page_map(parent, addr, parent, addr, perms);
		if (r)
		{
			panic("duppage: %e", r);
		}
	}
	else
	{
		if ((r = sys_page_map(parent, addr, envid, addr, PTE_P | PTE_U)))
		{
			panic("duppage (readonly): %e", r);
		}
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{

	set_pgfault_handler(pgfault);
	envid_t envid = sys_exofork();
	uint32_t va = 0, page = 0;
	pde_t pgdir_entry = 0;
	pte_t pt_entry = 0;
	uint32_t r;
	if (envid < 0)
	{
		panic("fork %e", envid);
	}
	else if (envid == 0)
	{
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	for (va = 0; va < USTACKTOP; va += PGSIZE)
	{
		if (((uvpd[PDX(va)] & PTE_P) == PTE_P) && ((uvpt[PGNUM(va)] & PTE_P) == PTE_P))
		{
			duppage(envid, PGNUM(va));
		}
	}
	extern void _pgfault_upcall();
	uint32_t uxperms = (PTE_P | PTE_U | PTE_W);
	r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), uxperms) |
		sys_env_set_pgfault_upcall(envid, _pgfault_upcall) |
		sys_env_set_status(envid, ENV_RUNNABLE) |
		sys_init_queue(envid);

	if (r)
	{
		panic("fork %e", r);
	}
	return envid;
}

// Challenge!
int sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
