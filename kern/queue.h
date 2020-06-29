#ifndef JOS_KERN_QUEUE_H
#define JOS_KERN_QUEUE_H

#include <inc/env.h>
#define QUEUE_SIZE 0xa
#define ERR_QUEUE_FULL 0x14
#define ERR_QUEUE_EMPTY 0x15
#define OP_SUCCESSFUL 0x0
#define FRONT_DEFAULT 0x0
#define REAR_DEFAULT 0xffffffff

typedef struct{
    int queue[QUEUE_SIZE];
    int qfront;
    int qrear;
    uint32_t qsize;
} queue;

int enqueue(queue* , int);
int dequeue(queue* , int*);
void print_queue(queue *);
void init_queue(queue *);

#endif