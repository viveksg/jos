#ifndef JOS_KERN_QUEUE_H
#define JOS_KERN_QUEUE_H

#include <inc/env.h>
int enqueue(queue* , envid_t );
int dequeue(queue* , envid_t *);
#endif