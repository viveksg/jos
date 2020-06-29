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
#include <kern/ipc_helper.h>
#include <kern/spinlock.h>
int enqueue_process(envid_t envid, envid_t receiver)
{   
    struct Env* recv = NULL;
    int status = envid2env(receiver, &recv, 0);
    spin_lock(&recv->qlock);
    if(recv->qfront == recv->qrear)
    {   
        spin_unlock(&recv->qlock);
        return -ERR_QUEUE_FULL;
    }
    cprintf("\n-----------------------------------------------------------------\n");
    cprintf("queue addr = %x\n",&recv->queue);
    cprintf("enqueue start:( %x - %x ) %x\n",recv->qfront, recv->qrear, (recv->qfront - recv->qrear));
    recv->queue[recv->qfront] = envid;
    recv->qfront = (recv->qfront + 1) % IPC_QUEUE_SIZE;
    cprintf("enqueue end:( %x - %x ) %x\n",recv->qfront, recv->qrear, (recv->qfront - recv->qrear));
    spin_unlock(&recv->qlock);
    return INSERT_SUCCESSFUL;
}

int dequeue_process(envid_t * envid , envid_t receiver)
{    
    struct Env* recv = NULL;
    int status = envid2env(receiver, &recv, 0);
    spin_lock(&recv->qlock);
    if(recv->qrear > -1 && recv->qrear == recv->qfront)
    {
        return -ERR_QUEUE_EMPTY;
        spin_unlock(&recv->qlock);
    }
    cprintf("\n-----------------------------------------------------------------\n");
    cprintf("queue addr = %x\n",&recv->queue);
    cprintf("dequeue start:( %x - %x ) %x\n",recv->qfront, recv->qrear, (recv->qfront - recv->qrear));
    recv->qrear = (recv->qrear + 1) % IPC_QUEUE_SIZE;
    *envid = recv->queue[recv->qrear];
    recv->queue[recv->qrear] = -1;
    cprintf("dequeue end:( %x - %x ) %x\n",recv->qfront, recv->qrear, (recv->qfront - recv->qrear));
    spin_unlock(&recv->qlock);
    return 0;
}

void print_queue(struct Env* recv)
{
   uint32_t i = 0;
   for(i = recv->qrear; i < recv->qfront; i++)
   {
       cprintf("%x ,",recv->queue[i]);
   }
   cprintf("\n");
}