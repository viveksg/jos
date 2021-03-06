#include <kern/queue.h>


int enqueue(queue* queue , ipc_msg value)
{
    if(queue->rear - queue->front == QUEUE_SIZE)
    {
        return -ERROR_QUEUE_FULL;
    }
    queue->msgs[queue->rear] = value;
    queue->rear = (queue->rear + 1) % QUEUE_SIZE;
    queue->qsize++;
    return OP_SUCCESSFUL;
}

int dequeue(queue* queue, ipc_msg* value)
{
   if(queue->front  == queue->rear)
   {
       return -ERROR_QUEUE_EMPTY;
   }
   *value = queue->msgs[queue->front];
   queue->front  = (queue->front + 1) % QUEUE_SIZE;
   queue->qsize--;
   return OP_SUCCESSFUL;
}

void init_queue(queue *queue)
{
    queue->front = FRONT_DEFAULT;
    queue->rear = REAR_DEFAULT;
    queue->qsize = 0;
}