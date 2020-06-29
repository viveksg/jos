#ifndef JOS_KERN_IPC_HELPER_H
#include <kern/env.h>
#define JOS_KERN_IPC_HELPER_H

int enqueue_process(envid_t, envid_t );
int dequeue_process(envid_t* , envid_t );
void print_queue(struct Env* );
#endif