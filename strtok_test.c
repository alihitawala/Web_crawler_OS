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
            printf("word : %s\n", token);
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

int main(int a, char** v) {
    char s[] = "word : Welcome!\n"
            "\n"
            "link:pageb\n"
            "link:pagec";
    int count, j;
    char** result = get_links(s, &count);
    for(j=0;j<count;j++) {
        printf("Extracted word :: %s \n", result[j]);
    }
    return 0;
}