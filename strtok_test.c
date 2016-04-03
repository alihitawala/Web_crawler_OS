//
// Created by alihitawala on 4/2/16.
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>

char** get_links(char* page, int* count) {
    int i=0,j,k=0,length, count_word=0;
    char *token, *space = " ", *link = "link:";
    int link_length = strlen(link);
    char* page_copy = strdup(page);
    token = strtok(page_copy, space);
    while(token != NULL) {
        count_word++;
        token = strtok(NULL, space);
    }
    char **list = (char **)malloc(sizeof(char*)*count_word);
    char **result = (char **)malloc(sizeof(char*)*count_word);
    page_copy = strdup(page);
    token = strtok(page_copy, space);
    while(token != NULL) {
        printf("word : %s\n", token);
        list[i++] = strdup(token);
        token = strtok(NULL, space);
    }
    for(j=0;j<i;j++) {
        if (strncmp(link, list[j], link_length) == 0) {
            length = strlen(list[j]) - link_length;
            result[k] = (char*)malloc(sizeof(char*) * length);
            strncpy(result[k], list[j] + link_length, length);
            k++;
        }
    }
    *count = k;
    return result;
}

int main(int a, char** v) {
    char s[] = "fdsgfgfdsgfsdg link:pagea fjdjasfkl link:dj asfkjdkasf link:pagec dkasfj link: dasfdasfdasf link:pagee dfasdas,mnfdsahfkjdhasfs li";
    int count, j;
    char** result = get_links(s, &count);
    for(j=0;j<count;j++) {
        printf("Extracted word :: %s \n", result[j]);
    }
    return 0;
}