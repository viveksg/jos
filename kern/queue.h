#ifndef JOS_KERN_QUEUE_H
#define JOS_KERN_QUEUE_H

#include <inc/env.h>
int enqueue(queue* , ipc_msg );
int dequeue(queue* , ipc_msg *);
void init_queue(queue *);
#endif