#ifndef JOS_KERN_SEMA_H
#define JOS_KERN_SEMA_H
#include <inc/env.h>

int wait(semaphore * , envid_t);
int signal(semaphore * , envid_t);
void init_semaphore(semaphore *, uint32_t value);
#endif