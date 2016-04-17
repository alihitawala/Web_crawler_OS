//
// Created by alihitawala on 4/17/16.
//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include "queue.h"
#include "hashtable.h"
typedef struct __functions {
    char * (*_fetch_fn)(char *url);
    void (*_edge_fn)(char *from, char *to);
}functions_t;

char** get_links(char* page, int* count);
queue_t* link_q;
queue_t* page_q;
hashtable_t *hashtable;
pthread_mutex_t mutex, dec_lock, dec2_lock;
pthread_cond_t link_q_fill;
pthread_cond_t link_q_empty;
pthread_cond_t page_q_fill;
pthread_cond_t work_finish;
int downloader_still_working = 0;
int parser_still_working = 0;

void *download_worker(void* func) {
    functions_t* functions = (functions_t*) func;
    int i=0;
    while(1){
        i++;
        pthread_mutex_lock(&mutex);
        while (Queue_IsEmpty(link_q)) //todo FATAL premeption after this line
        {
            printf("DOWNLOADER:: LINK Q Empty \n");
            pthread_cond_signal(&work_finish);
            pthread_cond_wait(&link_q_fill, &mutex);
        }
        char* link, *parent;
        printf("DOWNLOADER:: About to dequeue\n");
        Queue_Dequeue(link_q, &link, &parent);
        printf("DOWNLOADER:: Link received :: %s, parent :: %s\n", link, parent);
        pthread_cond_signal(&link_q_empty);
        printf("DOWNLOADER:: Signalled link queue empty \n");
        downloader_still_working++;
        pthread_mutex_unlock(&mutex);
        char *page = functions->_fetch_fn(link);
        printf("DOWNLOADER:: Signalled link queue empty II \n");
        if (page != NULL) {
            printf("Page :: %s \n %s", link, page);
            Queue_Enqueue(page_q, page, link);
            pthread_mutex_lock(&mutex);
            pthread_cond_signal(&page_q_fill);
            pthread_mutex_unlock(&mutex);
        }
        else
            printf("DOWNLOADER:: Page NULL \n");
        pthread_mutex_lock(&dec_lock);
        downloader_still_working--;
        pthread_mutex_unlock(&dec_lock);
    }
//    printf("DOWNLOADER:: EXIT \n");
    pthread_exit(NULL);
}


void *parser_worker(void* func) {
    functions_t* functions = (functions_t*) func;
    int i=0, j=0;
    int len = 0;
    while(1) {
        i++;
        pthread_mutex_lock(&mutex);
        while (Queue_IsEmpty(page_q)) {
            printf("PARSER:: PAGE Q EMPTY! \n");
            pthread_cond_signal(&work_finish);
            pthread_cond_wait(&page_q_fill, &mutex);
        }
        char* page, *parent;
        Queue_Dequeue(page_q, &page, &parent);
        printf("PARSER:: Page received from parent :: %s\n", parent);
        //get links from page and enq
        char** links = get_links(page, &len);
        printf("PARSER:: length of links %d \n", len);
        for(j=0;j<len;j++) {
            char* link = links[j];
            if (ht_get(hashtable, link) != NULL)
                continue;
            else
                ht_set(hashtable, link, link);
            while (Queue_IsFull(link_q)) {
                printf("PARSER:: Link queue full \n");
                pthread_cond_wait(&link_q_empty, &mutex);
            }
            printf("PARSER:: Link queue not full \n");
            Queue_Enqueue(link_q, link, parent);
            pthread_cond_signal(&link_q_fill);
            printf("PARSER:: Enqueueing Link %s, parent %s\n", link, parent);
        }
        free(page);
        parser_still_working++;
        pthread_mutex_unlock(&mutex);
        for(j=0;j<len;j++) {
            char *link = links[j];
            if (parent != NULL)
                functions->_edge_fn(parent, link);
        }
        pthread_mutex_lock(&dec2_lock);
        parser_still_working--;
        pthread_mutex_unlock(&dec2_lock);
    }
    //printf("PARSER:: EXIT \n");
    pthread_exit(NULL);
}

char** get_links(char* page, int* count) {
    int i=0,j,k=0,length, count_word=0;
    char *token, *space = " ", *link = "link:", *new_line="\n", *line_token, *sp_1, *sp_2;
    int link_length = strlen(link);
    char* page_copy = strdup(page);
    line_token = strtok_r(page_copy, new_line, &sp_1);
    while(line_token != NULL) {
        token = strtok_r(line_token, space, &sp_2);
        while (token != NULL) {
            count_word++;
            token = strtok_r(NULL, space, &sp_2);
        }
        line_token = strtok_r(NULL, new_line, &sp_1);
    }
    char **list = (char **)malloc(sizeof(char*)*count_word);
    char **result = (char **)malloc(sizeof(char*)*count_word);
    page_copy = strdup(page);
    line_token = strtok_r(page_copy, new_line, &sp_1);
    while(line_token != NULL) {
        token = strtok_r(line_token, space, &sp_2);
        while (token != NULL) {
            list[i++] = strdup(token);
            token = strtok_r(NULL, space, &sp_2);
        }
        line_token = strtok_r(NULL, new_line, &sp_1);
    }
    for(j=0;j<i;j++) {
        if (strncmp(link, list[j], link_length) == 0) {
            length = strlen(list[j]) - link_length;
            if (length == 0)
                continue;
            result[k] = (char*)malloc(sizeof(char*) * length);
            strncpy(result[k], list[j] + link_length, length);
            k++;
        }
    }
    *count = k;
    return result;
}

int crawl(char *start_url,
          int download_workers,
          int parse_workers,
          int queue_size,
          char * (*_fetch_fn)(char *url),
          void (*_edge_fn)(char *from, char *to)) {
    functions_t* funct = (functions_t*)malloc(sizeof(functions_t));
    funct->_fetch_fn = _fetch_fn;
    funct->_edge_fn = _edge_fn;
    //queues
    link_q = (queue_t *) malloc(sizeof(queue_t));
    page_q = (queue_t *) malloc(sizeof(queue_t));
    hashtable = ht_create(65536);
    ht_set(hashtable, start_url, start_url);
    Queue_Init(link_q, queue_size);
    Queue_Init(page_q, -1);
    Queue_Enqueue(link_q, start_url, NULL);
    //threads
    pthread_t download_threads[download_workers];
    pthread_t parser_threads[parse_workers];
    int i;
    for (i = 0; i < download_workers; i++)
        pthread_create(&download_threads[i], NULL, download_worker, (void *) funct);
    for (i = 0; i < parse_workers; i++)
        pthread_create(&parser_threads[i], NULL, parser_worker, (void *) funct);
//    pthread_exit(NULL);
//    sleep(15);
    pthread_mutex_lock(&mutex);
    while ((!Queue_IsEmpty(link_q) || !Queue_IsEmpty(page_q))
           || parser_still_working>0 || downloader_still_working>0) {
        pthread_cond_wait(&work_finish, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

