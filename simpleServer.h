#ifndef _SIMPLE_SERVER_H
#define _SIMPLE_SERVER_H
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define BUF_LEN 512

int open_listenfd(int);

//Worker Queue
void insert(int socket);
int rmv(void);
int isEmpty(void);
int isFull(void);
long int getLength(void);

//Logger Queue
void insert_log(char* socket);
char* rmv_log(void);
int isEmpty_log(void);
int isFull_log(void);
long int getLength_log(void);

extern int fifo_queue[64];
#endif
