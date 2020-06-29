#ifndef JOS_KERN_SEMA_H
#define JOS_KERN_SEMA_H
#include <inc/types.h>
#define DEFAULT_SEMQ_LENGTH 0xa;

typedef struct{
    int count;
    envid_t queue[DEFAULT_SEMQ_LENGTH];
    int sqrear;
    int sqfront
} semaphore;

void init_semaphore(semaphore * ,int );
void sema_wait(semaphore*, struct Env*);
void sema_signal(semaphore*, struct Env*);
#endif
