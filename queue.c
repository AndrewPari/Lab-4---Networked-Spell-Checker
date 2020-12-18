#include "simpleServer.h"

int workQ[64];
int *front = workQ, *rear = workQ;
int *back = workQ + sizeof(workQ)/sizeof(int) - 1;

//inserts an object into the queue
//if queue is full, prints error and returns
void insert(int socket) {

    if(isEmpty()) {
        *rear = socket;
        return;
    }

    if(rear == back) {
        rear = workQ;
        *rear = socket;
        return;
    }
    rear++;
    *rear = socket;
}

//removes an object from the queue
int rmv(void) {
    int temp;

    if(front == back){
        temp = *front;
        *front = 0;
        front = workQ;
        return temp;
    }

    if(front == rear) {
        temp = *front;
        *front = 0;
        return temp;
    }

    temp = *front;
    *front = 0;
    front++;
    return(temp);

}

//checks if the queue is empty
int isEmpty(void) {
    if(rear == front && *rear == 0)
        return 1;
    else
        return 0;

}

//checks to see if queue is full
int isFull(void) {
    if(rear == front - 1 || (rear == back && front == workQ))
	return 1;
    else return 0;
}

//returns the length of the queue.
long int getLength(void) {
    long int length;
    if(isEmpty())
        return 0;

    long int range = labs((long int)front - (long int)rear);
    //printf("\n%d\n", range);

    if(range == 0)
        length = 1;

    length = range/sizeof(int);

    return length;
}


