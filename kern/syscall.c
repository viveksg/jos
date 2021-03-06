/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>
#include <kern/semaphore.h>
#include <kern/queue.h>
// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
static void
sys_cputs(const char *s, size_t len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.

	user_mem_assert(curenv, s, len, PTE_U);

	// Print the string supplied by the user.
	cprintf("%.*s", len, s);
}

// Read a character from the system console without blocking.
// Returns the character, or 0 if there is no input waiting.
static int
sys_cgetc(void)
{
	return cons_getc();
}

// Returns the current environment's envid.
static envid_t
sys_getenvid(void)
{
	return curenv->env_id;
}

// Destroy a given environment (possibly the currently running environment).
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;
	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;
	env_destroy(e);
	return 0;
}

// Deschedule current environment and pick a different one to run.
static void
sys_yield(void)
{
	sched_yield();
}

// Allocate a new environment.
// Returns envid of new environment, or < 0 on error.  Errors are:
//	-E_NO_FREE_ENV if no free environment is available.
//	-E_NO_MEM on memory exhaustion.
static envid_t
sys_exofork(void)
{
	// Create the new environment with env_alloc(), from kern/env.c.
	// It should be left as env_alloc created it, except that
	// status is set to ENV_NOT_RUNNABLE, and the register set is copied
	// from the current environment -- but tweaked so sys_exofork
	// will appear to return 0.

	struct Env *new_env = NULL;
	envid_t envid = curenv->env_id;
	uint32_t env_alloc_status = env_alloc(&new_env, envid);
	if (env_alloc_status == 0)
	{
		new_env->env_status = ENV_NOT_RUNNABLE;
		new_env->env_tf = curenv->env_tf;
		new_env->env_tf.tf_regs.reg_eax = 0;
		return new_env->env_id;
	}

	return env_alloc_status;
}

// Set envid's env_status to status, which must be ENV_RUNNABLE
// or ENV_NOT_RUNNABLE.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if status is not a valid status for an environment.
static int
sys_env_set_status(envid_t envid, int status)
{
	if (status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE)
	{
		return -E_INVAL;
	}
	struct Env *candidate_env = NULL;
	int op_status = envid2env(envid, &candidate_env, 0x1);
	if (op_status)
	{
		return op_status;
	}
	candidate_env->env_status = status;
	return 0;
}

// Set envid's trap frame to 'tf'.
// tf is modified to make sure that user environments always run at code
// protection level 3 (CPL 3), interrupts enabled, and IOPL of 0.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
	// LAB 5: Your code here.
	// Remember to check whether the user has supplied us with a good
	// address!
	panic("sys_env_set_trapframe not implemented");
}

// Set the page fault upcall for 'envid' by modifying the corresponding struct
// Env's 'env_pgfault_upcall' field.  When 'envid' causes a page fault, the
// kernel will push a fault record onto the exception stack, then branch to
// 'func'.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_pgfault_upcall(envid_t envid, void *func)
{
	struct Env *env = NULL;
	int env_status = envid2env(envid, &env, 0x1);
	if (env_status)
	{
		return env_status;
	}
	env->env_pgfault_upcall = func;
	return 0;
}

// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
//
// perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
//         but no other bits may be set.  See PTE_SYSCALL in inc/mmu.h.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
//	-E_INVAL if perm is inappropriate (see above).
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	struct Env *candidate_env = NULL;
	int env_status = envid2env(envid, &candidate_env, 0x1);
	uint32_t inverted_psyscall = ~PTE_SYSCALL;
	if (env_status)
	{
		return env_status;
	}
	if ((!utop_validate(va)) || (inverted_psyscall & perm))
	{
		return -E_INVAL;
	}

	struct PageInfo *pinfo = page_alloc(ALLOC_ZERO);
	if (pinfo != NULL)
	{
		uint32_t insert_status = page_insert(candidate_env->env_pgdir, pinfo, va, perm);
		if (insert_status)
		{
			page_free(pinfo);
			return insert_status;
		}
		return 0;
	}
	page_free(pinfo);
	return -E_NO_MEM;
}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate any necessary page tables.
static int
sys_page_map(envid_t srcenvid, void *srcva,
			 envid_t dstenvid, void *dstva, int perm)
{
	struct Env *src_env = NULL;
	struct Env *dst_env = NULL;
	uint32_t src_va_int = (uint32_t)srcva;
	uint32_t dst_va_int = (uint32_t)dstva;
	int src_status = envid2env(srcenvid, &src_env, 0x1);
	int dst_status = envid2env(dstenvid, &dst_env, 0x1);
	if (src_status || dst_status)
	{   
		return src_status | dst_status;
	}
	if ((!utop_validate(srcva)) || (!utop_validate(dstva)))
	{   
		return -E_INVAL;
	}
	pte_t *src_pte = NULL;
	pte_t *dst_pte = NULL;
	struct PageInfo *src_pinfo = page_lookup(src_env->env_pgdir, srcva, &src_pte);
	uint32_t src_perms = (*src_pte & 0xFFF);
	uint32_t enval = (src_pte == NULL) ;
	enval |= (perm & (~PTE_SYSCALL));
	enval |= ((perm & PTE_W) && (!(src_perms & PTE_W)));
	if (enval)
	{
		return -E_INVAL;
	}
	if (page_insert(dst_env->env_pgdir, src_pinfo, dstva, perm))
	{
		return -E_NO_MEM;
	}
	return 0;
}

bool utop_validate(void *va)
{
	uint32_t va_int = (uint32_t)va;
	return (va_int < UTOP && (PGOFF(va) == 0));
}

// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
static int
sys_page_unmap(envid_t envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().
	struct Env *env_store;
	int env_status = envid2env(envid, &env_store, 0x1);
	if (env_status)
	{
		return env_status;
	}
	if (!utop_validate(va))
	{
		return -E_INVAL;
	}
	page_remove(env_store->env_pgdir, va);
	return 0;
}

// Try to send 'value' to the target env 'envid'.
// If srcva < UTOP, then also send page currently mapped at 'srcva',
// so that receiver gets a duplicate mapping of the same page.
//
// The send fails with a return value of -E_IPC_NOT_RECV if the
// target is not blocked, waiting for an IPC.
//
// The send also can fail for the other reasons listed below.
//
// Otherwise, the send succeeds, and the target's ipc fields are
// updated as follows:
//    env_ipc_recving is set to 0 to block future sends;
//    env_ipc_from is set to the sending envid;
//    env_ipc_value is set to the 'value' parameter;
//    env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
// The target environment is marked runnable again, returning 0
// from the paused sys_ipc_recv system call.  (Hint: does the
// sys_ipc_recv function ever actually return?)
//
// If the sender wants to send a page but the receiver isn't asking for one,
// then no page mapping is transferred, but no error occurs.
// The ipc only happens when no errors occur.
//
// Returns 0 on success, < 0 on error.
// Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist.
//		(No need to check permissions.)
//	-E_IPC_NOT_RECV if envid is not currently blocked in sys_ipc_recv,
//		or another environment managed to send first.
//	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
//	-E_INVAL if srcva < UTOP and perm is inappropriate
//		(see sys_page_alloc).
//	-E_INVAL if srcva < UTOP but srcva is not mapped in the caller's
//		address space.
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in the
//		current environment's address space.
//	-E_NO_MEM if there's not enough memory to map srcva in envid's
//		address space.
static int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	struct  Env* env = NULL;
	int enval = 0;
	int status = envid2env(envid, &env, 0);
	uint32_t inverted_psyscall = ~PTE_SYSCALL;
    if(status)
	{
	    return status;
    }
	if(!env->env_ipc_recving)
	{   
		ipc_msg msg = {curenv->env_id, value, srcva, perm};
		status = enqueue(&(env->ipc_queue), msg);
		if(status)
		{
			return status;
		}
		curenv->env_status = ENV_NOT_RUNNABLE;
		sys_yield();
		return 0;
	}	
    status = perform_ipc_msg_transfer(curenv->env_id, envid, value, srcva, perm);
    if(status)
	{
		return status;
	}
	env->env_ipc_recving = 0;
	env->env_status = ENV_RUNNABLE;
	env->env_tf.tf_regs.reg_eax = 0;
    return 0;
}

int perform_ipc_msg_transfer(envid_t sender_envid, envid_t envid, uint32_t value, void *srcva, unsigned perm)
{ 
    struct Env* env = NULL;
	struct Env* senderenv = NULL;
	int enval = 0;
	int status = envid2env(envid, &env, 0) |envid2env(sender_envid, &senderenv, 0) ;
	uint32_t inverted_psyscall = ~PTE_SYSCALL;
	if(status)
	{
		return status;
	}
	
	if((uint32_t)srcva < UTOP)
	{   
		pte_t *pte = NULL;
		struct PageInfo* pp = page_lookup(senderenv->env_pgdir, srcva, &pte);
		uint32_t page_perms = (*pte & 0xFFF);
	    enval = PGOFF(srcva);
		enval |= (perm & inverted_psyscall);
	    enval |= (pp == NULL);
		enval |= ((*pte & PTE_W) && (!(perm & PTE_W)));
		if(enval)
		{
			return -E_INVAL;
		}		
        if((status = page_insert(env->env_pgdir, pp, env->env_ipc_dstva, perm)))
		{
            return status;
		}

        env->env_ipc_perm = perm;
	}
	else
	{
		env->env_ipc_perm = 0;
	}
	env->env_ipc_from = senderenv->env_id;
	env->env_ipc_value = value;
	return 0;
}
// Block until a value is ready.  Record that you want to receive
// using the env_ipc_recving and env_ipc_dstva fields of struct Env,
// mark yourself not runnable, and then give up the CPU.
//
// If 'dstva' is < UTOP, then you are willing to receive a page of data.
// 'dstva' is the virtual address at which the sent page should be mapped.
//
// This function only returns on error, but the system call will eventually
// return 0 on success.
// Return < 0 on error.  Errors are:
//	-E_INVAL if dstva < UTOP but dstva is not page-aligned.
static int
sys_ipc_recv(void *dstva)
{
	if((uint32_t)dstva < UTOP && PGOFF(dstva) != 0)
	{
		return -E_INVAL;
	}
    int status = 0;
	
	curenv->env_ipc_dstva = dstva;
	if(curenv->ipc_queue.qsize > 0)
	{  
	   ipc_msg msg = {-1,0,0,0};
	   status =  dequeue(&(curenv->ipc_queue), &msg);
	   status |= (msg.envid == -1);
	   status |= perform_ipc_msg_transfer(msg.envid, curenv->env_id, msg.value, msg.srcva, msg.perm);
       status |= wake_sleep_process(msg.envid);
	   return status;
	}
    curenv->env_ipc_recving = 1;
    curenv->env_status = ENV_NOT_RUNNABLE;
	sys_yield();
	return 0;
}

int wake_sleep_process(envid_t envid)
{
	struct Env* env = NULL;
	int status = 0;
	if((status = envid2env(envid, &env, 0)))
    {
		return status;
	}
	env->env_status = ENV_RUNNABLE;
	env->env_tf.tf_regs.reg_eax = 0;
	return 0;
}

static 
int sys_init_ipc_vals(envid_t envid)
{
	return perform_init(envid);
}

int perform_init(envid_t envid)
{
	struct Env* env = NULL;
	int status = 0;
	if((status = envid2env(envid, &env, 0)))
    {
		return status;
	}

	//init_semaphore(&(env->full),SEMA_FULL);
	//init_semaphore(&(env->empty),SEMA_EMPTY);
    //init_semaphore(&(env->mutex),SEMA_MUTEX);
	init_queue(&(env->ipc_queue));
    return 0;
}
// Dispatches to the correct kernel function, passing the arguments.
int32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	// LAB 3: Your code here.

	switch (syscallno)
	{
	case SYS_cputs:
		sys_cputs((const char *)a1, a2);
		break;
	case SYS_cgetc:
		return sys_cgetc();
	case SYS_getenvid:
		return sys_getenvid();
	case SYS_env_destroy:
		return sys_env_destroy((envid_t)a1);
	case SYS_yield:
		sched_yield();
		break;
	case SYS_exofork:
		return sys_exofork();
	case SYS_env_set_status:
		return sys_env_set_status(a1, a2);
	case SYS_page_alloc:
		return sys_page_alloc(a1, (void *)a2, a3);
	case SYS_page_map:
		return sys_page_map(a1, (void *)a2, a3, (void *)a4, a5);
	case SYS_page_unmap:
		return sys_page_unmap(a1, (void *)a2);
	case SYS_env_set_pgfault_upcall:
		return sys_env_set_pgfault_upcall(a1, (void *)a2);
	case SYS_ipc_recv:
	    return sys_ipc_recv((void *) a1);
	case SYS_ipc_try_send:
	    return sys_ipc_try_send(a1, a2, (void *)a3, a4);	
	case SYS_init_ipc_vals:
	    return sys_init_ipc_vals(a1);					
	default:
		return -E_INVAL;
	}
	return 0;
}
