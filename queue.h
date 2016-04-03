//
// Created by alihitawala on 4/2/16.
//

#ifndef P4A_QUEUE_H
#define P4A_QUEUE_H
typedef struct __node_t {
    char* parent;
    char* value;
    struct __node_t *next;
} node_t;

typedef struct __queue_t {
    int temp;
    int max_length;
    int current_length;
    node_t *head;
    node_t *tail;
    pthread_mutex_t headLock;
    pthread_mutex_t tailLock;
} queue_t;

void Queue_Init(queue_t *q, int length);
void Queue_Enqueue(queue_t *q, char* value, char* parent);
int Queue_Dequeue(queue_t *q, char** value, char** parent);
int Queue_IsEmpty(queue_t *q);
int Queue_IsFull(queue_t *q);

#endif //P4A_QUEUE_H
