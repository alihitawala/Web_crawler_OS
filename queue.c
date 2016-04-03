//
// Created by alihitawala on 4/2/16.
//
// Implementation taken from: http://pages.cs.wisc.edu/~remzi/Classes/537/Spring2016/Book/threads-locks-usage.pdf
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <queue.h>


void Queue_Init(queue_t *q, int length) {
    node_t *tmp = malloc(sizeof(node_t));
    tmp->next = NULL;
    q->head = q->tail = tmp;
    q->max_length = length;
    q->current_length = 0;
    pthread_mutex_init(&q->headLock, NULL);
    pthread_mutex_init(&q->tailLock, NULL);
}

void Queue_Enqueue(queue_t *q, char* value) {
    node_t *tmp = malloc(sizeof(node_t));
    assert(tmp != NULL);
    tmp->value = value;
    tmp->next = NULL;

    pthread_mutex_lock(&q->tailLock);
    q->tail->next = tmp;
    q->tail = tmp;
    q->current_length = q->current_length + 1;
    pthread_mutex_unlock(&q->tailLock);
}

int Queue_Dequeue(queue_t *q, char* value) {
    pthread_mutex_lock(&q->headLock);
    node_t *tmp = q->head;
    node_t *newHead = tmp->next;
    if (newHead == NULL) {
        pthread_mutex_unlock(&q->headLock);
        return -1; // queue was empty
    }
    value = newHead->value;
    q->head = newHead;
    q->current_length = q->current_length - 1;
    pthread_mutex_unlock(&q->headLock);
    free(tmp);
    return 0;
}

int Queue_IsEmpty(queue_t *q) {
    pthread_mutex_lock(&q->headLock);
    node_t *tmp = q->head;
    node_t *newHead = tmp->next;
    if (newHead == NULL) {
        pthread_mutex_unlock(&q->headLock);
        return 0; // queue was empty
    }
    pthread_mutex_unlock(&q->headLock);
    return 1;
}

int Queue_IsFull(queue_t *q) {
    pthread_mutex_lock(&q->headLock);
    pthread_mutex_lock(&q->tailLock);
    if (q->max_length == q->current_length) {
        pthread_mutex_unlock(&q->tailLock);
        pthread_mutex_unlock(&q->headLock);
        return 1;
    }
    pthread_mutex_unlock(&q->tailLock);
    pthread_mutex_unlock(&q->headLock);
    return 0;
}