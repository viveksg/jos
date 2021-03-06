#include <kern/semaphore.h>
#include <kern/queue.h>
#include <inc/x86.h>

//todo: make processs sleep based implementation later

int wait(semaphore * sema, envid_t env)
{    
     while(sema->value == 0); // change this in process sleep later
     uint32_t value = sema->value -1;
     xchg(&(sema->value), value);
     return OP_SUCCESSFUL;
}

int signal(semaphore * sema, envid_t env)
{
    uint32_t value = sema->value + 1;
    xchg(&(sema->value), value);
    return OP_SUCCESSFUL;
}

void init_semaphore(semaphore *sema, uint32_t value)
{
    sema->value = value;
    init_queue(&(sema->blocked_process));
}