// Semaphore implementation of 
#include <inc/types.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <inc/memlayout.h>
#include <inc/string.h>
#include <kern/cpu.h>
#include <kern/spinlock.h>
#include <kern/kdebug.h>
#include <kern/semaphore.h>

void init_semaphore(semaphore * sema, int value)
{
      sema->count = value;
      sema->sqfront =    
}

void sema_wait(semaphore* sema, struct Env* env)
{

}

void sema_signal(semaphore* sema, struct Env* env)
{

}