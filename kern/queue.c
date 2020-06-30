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
#include <kern/queue.h>

int enqueue(queue* que, int value)
{   
    if((que->qrear + 1)%que->qsize == que->qfront)
    {   
        return -ERR_QUEUE_FULL;
    }
    que->queue[que->qrear] = value;
    que->qrear= (que->qrear + 1) % que->qsize;
    return OP_SUCCESSFUL;
}

int dequeue(queue* que, int * value)
{
    if((que->qrear == que->qfront))
    {
        return -ERR_QUEUE_EMPTY;
    }
    *value = que->queue[que->qfront];
    que->queue[que->qfront] = -1;
    que->qfront = (que->qfront + 1)%que->qsize;
    return OP_SUCCESSFUL;   
} 

void print_queue(queue* que)
{
   uint32_t i = 0;
   for(i = que->qrear; i < que->qfront; i++)
   {
       cprintf("%x ,",que->queue[i]);
   }
   cprintf("\n");
}

void init_queue(queue* que)
{   
    que->qfront = FRONT_DEFAULT;
    que->qrear = REAR_DEFAULT;
    que->qsize = QUEUE_SIZE;
}