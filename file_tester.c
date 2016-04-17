#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "crawler.h"
#include <pthread.h>
#include <semaphore.h>

/*void *Malloc(size_t size) {
  void *r = malloc(size);
  assert(r);
  return r;
}

char *Strdup(const char *s) {
  void *r = strdup(s);
  assert(r);
  return r;
}*/
#define MAX_THREADS 20
#define check(exp, msg) if(exp) {} else {\
   printf("%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
   exit(1);}

void *Malloc(size_t size) {
  void *r = malloc(size);
  assert(r);
  return r;
}

char *Strdup(const char *s) {
  void *r = strdup(s);
  assert(r);
  return r;
}

char *parseURL(const char *link)
{
  char *url = strdup(link);
  char *saveptr;
  char *token = strtok_r(url, "/", &saveptr);
  char *word = token;
  while(token != NULL)
  {
    word = token;
    token = strtok_r(NULL, "/", &saveptr);
  }
  return word;
}

int compare(const void* line1, const void* line2)
{
  char *str1 = (char *) line1;
  char *str2 = (char *) line2;
  //printf("%s %s %d\n", str1, str2, strcmp(str1, str2));
  return strcmp(str1, str2);
}

int isDifferent(char actual[100][50], char expected[100][50], int size)
{
  int i = 0;
  for(i = 0; i < size; ++i)
  {
    if(strcmp(actual[i], expected[i]) != 0)
      return 1;
  }
  return 0;
}

void print(char buffer[100][50], int size)
{
  int i = 0;
  for(i = 0; i < size; ++i)
  {
    printf("%s", buffer[i]);
  }
}

void printData(char actual[100][50], int size1, char expected[100][50], int size2)
{
  printf("Actual Output (sorted):\n\n");
  print(actual, size1);
  printf("\nExpected Output (sorted): \n\n");
  print(expected, size2);
}

int compareOutput(char actual[100][50], int size1, char *file)
{
  char expected[100][50];
  FILE *fp = fopen(file, "r");
  if(fp == NULL)
    fprintf(stderr, "Unable to open the file %s", file);
  int size2 = 0;
  while(fgets(expected[size2], 50, fp) != NULL)
  {
    size2++;
  }
  qsort(actual, size1, 50, compare);
  qsort(expected, size2, 50, compare);
  fclose(fp);
  if(size1 != size2)
  {
    printf("wrong size\n");
    printData(actual, size1, expected, size2);
    return 1;
  }
  else if(isDifferent(actual, expected, size1))
  {
    printf("mismatch\n");
    printData(actual, size1, expected, size2);
    return 1;
  }
  return 0;
}
/*char *fetch(char *link) {
  int fd = open(link, O_RDONLY);
  if (fd < 0) {
    perror("failed to open file");
    return NULL;
  }
  int size = lseek(fd, 0, SEEK_END);
  assert(size >= 0);
  char *buf = Malloc(size+1);
  buf[size] = '\0';
  assert(buf);
  lseek(fd, 0, SEEK_SET);
  char *pos = buf;
  while(pos < buf+size) {
    int rv = read(fd, pos, buf+size-pos);
    assert(rv > 0);
    pos += rv;
  }
  close(fd);
  return buf;
}*/

/*void edge(char *from, char *to) {
  printf("%s -> %s\n", from, to);
}*/


pthread_mutex_t buffer_mutex, bounded_queue_mutex;
int buffer_consumed = 0;
int fill = 0, pages_downloaded = 0;
char buffer[100][50];

char *fetch(char *link) {
  pthread_mutex_lock(&bounded_queue_mutex);
  unsigned long current_id = (unsigned long) pthread_self();
  printf("THREAD ID :: %ld\n", current_id);
  if(pages_downloaded == 1)
  {
    pthread_mutex_unlock(&bounded_queue_mutex);
    sleep(5);
  }
  else
    pthread_mutex_unlock(&bounded_queue_mutex);
  int fd = open(link, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "failed to open file: %s", link);
    return NULL;
  }
  int size = lseek(fd, 0, SEEK_END);
  assert(size >= 0);
  char *buf = Malloc(size+1);
  buf[size] = '\0';
  assert(buf);
  lseek(fd, 0, SEEK_SET);
  char *pos = buf;
  while(pos < buf+size) {
    int rv = read(fd, pos, buf+size-pos);
    assert(rv > 0);
    pos += rv;
  }
  pthread_mutex_lock(&bounded_queue_mutex);
  pages_downloaded++;
  pthread_mutex_unlock(&bounded_queue_mutex);
  close(fd);
  return buf;
}

//140269343610624

void edge(char *from, char *to) {
  if(!from || !to)
    return;
  char temp[50];
  temp[0] = '\0';
  char *fromPage = parseURL(from);
  char *toPage = parseURL(to);
  strcpy(temp, fromPage);
  strcat(temp, "->");
  strcat(temp, toPage);
  strcat(temp, "\n");
  pthread_mutex_lock(&buffer_mutex);
  strcpy(buffer[fill++], temp);
  pthread_mutex_unlock(&buffer_mutex);
  pthread_mutex_lock(&bounded_queue_mutex);
  if(pages_downloaded == 1 && fill > 4)
    buffer_consumed = 1;
  pthread_mutex_unlock(&bounded_queue_mutex);

}

int main(int argc, char *argv[]) {
  pthread_mutex_init(&buffer_mutex, NULL);
  pthread_mutex_init(&bounded_queue_mutex, NULL);
  int rc = crawl("/u/c/s/cs537-1/ta/tests/4a/tests/files/small_buffer/pagea", 5, 1, 1, fetch, edge);
  assert(rc == 0);
  check(buffer_consumed == 1, "All the links should have been parsed as multiple downloader threads should have consumed the buffer\n");
  return compareOutput(buffer, fill, "/u/c/s/cs537-1/ta/tests/4a/tests/files/output/small_buffer.out");
}
