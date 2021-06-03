#ifndef CLIENTE_QUEUE_H
#define CLIENTE_QUEUE_H

typedef struct Node
{
    void *data;
    struct Node *next;
}node;

typedef struct QueueList
{
    int sizeOfQueue;
    size_t memSize;
    node *head;
    node *tail;
}Queue;

void queueInit(Queue *q, size_t memSize);
int enqueue(Queue *, const void *);
void dequeue(Queue *, void *);
#endif //CLIENTE_QUEUE_H
