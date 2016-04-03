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

int crawl(char *start_url,
	  int download_workers,
	  int parse_workers,
	  int queue_size,
	  char * (*_fetch_fn)(char *url),
	  void (*_edge_fn)(char *from, char *to)) {

    link_q = (queue_t*) malloc(sizeof(queue_t));
    page_q = (queue_t*) malloc(sizeof(queue_t));
    Queue_Init(link_q, queue_size);
    Queue_Init(page_q, -1);


	char* page = _fetch_fn("pagea");
    printf("%s\n", page);
    printf("%d\n", link_q->temp);
    return -1;
}
