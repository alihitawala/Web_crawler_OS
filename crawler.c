#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include "queue.h"

queue_t* link_q;
queue_t* page_q;
pthread_mutex_t download_mutex;
pthread_mutex_t parser_mutex;
pthread_cond_t link_q_fill;
pthread_cond_t link_q_empty;
pthread_cond_t page_q_fill;

void *download_worker(void* fetch_func, void* _edge_fn) {
    char* (*fetch)(char* );
    fetch = (char* (*) (char*)) fetch_func;
    char* (*edge)(char* );
    edge = (char* (*) (char*)) _edge_fn;
    int i=0;
    while(1){
        i++;
        pthread_mutex_lock(&download_mutex);
        while (Queue_IsEmpty(link_q))
            pthread_cond_wait(&link_q_fill, &download_mutex);
        char* link, *parent;
        Queue_Dequeue(link_q, &link, &parent);
        pthread_cond_signal(&link_q_empty);
        char *page = fetch(link);
        if (page != NULL) {
            Queue_Enqueue(page_q, page, link);
            pthread_cond_signal(&page_q_fill);
        }
        pthread_mutex_unlock(&download_mutex);
        printf("%s\n", link);
        printf("PAGE DUMP:: %s\n", page);
        if (i == 10)
            break;
    }
    pthread_exit(NULL);
}


void *parser_worker() {
    int i=0;
    while(1){
        i++;
        pthread_mutex_lock(&parser_mutex);
        while (Queue_IsEmpty(page_q))
            pthread_cond_wait(&page_q_fill, &parser_mutex);
        char* page, *parent;
        Queue_Dequeue(page_q, &page, &parent);
        //get links from page and enq
        char** links = get_links(page);
        pthread_cond_signal(&link_q_fill);
        free(page);
        pthread_mutex_unlock(&parser_mutex);
        if (i == 10)
            break;
    }
    pthread_exit(NULL);
}

char** get_links(char* page) {
    char *token;
    char *link;
    token = strtok_r(page, "link:");
    link = strtok_r(NULL, " ");
    while(token != NULL) {
        printf("link : %s", link);
        token = strtok_r(NULL, "link:");
        link = strtok_r(NULL, " ");
    }
}

int crawl(char *start_url,
	  int download_workers,
	  int parse_workers,
	  int queue_size,
	  char * (*_fetch_fn)(char *url),
	  void (*_edge_fn)(char *from, char *to)) {
    //queues
    link_q = (queue_t*) malloc(sizeof(queue_t));
    page_q = (queue_t*) malloc(sizeof(queue_t));
    Queue_Init(link_q, queue_size);
    Queue_Init(page_q, -1);
    Queue_Enqueue(link_q, start_url, NULL);
    //threads
    pthread_t download_threads[download_workers];
    pthread_t parser_threads[parse_workers];
    int i;
    for (i = 0; i < download_workers; i++)
        pthread_create(&download_threads[i], NULL, download_worker, (void *) _fetch_fn, (void *) _edge_fn);
    for (i = 0; i < parse_workers; i++)
        pthread_create(&parser_threads[i], NULL, parser_worker, NULL);
    pthread_exit(NULL);
    return -1;
}

