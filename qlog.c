#include "simpleServer.h"

char* logQ[64];
int count=0, empty_log=0, next_log=0;
char **frontP = logQ, **rearP = logQ;
char **end = logQ + sizeof(logQ)/sizeof(int) - 1;

//Note: why are there two stars above, for rearP for example, but not in insert?
// * refers to a string. ** refers to the characters of the string, no asterisks refers to the pointer to the array.

//inserts an object into the queue
//if queue is full, prints error and returns
void insert_log(char* socket) {
	logQ[empty_log] = socket;
	count++;
	empty_log = (empty_log + 1) % 64;

	int i;
	for(i = 0; i < 64; i++){
//		printf("location %d logQ %s", i, logQ[i]);
	}


/*	printf("printing socket in insert_log %s", socket);

    if(isEmpty()) {
	logQ[empty_log] = socket;
	count++;
	empty_log++;
        return;
    }

    else if(rearP == end) {
        rearP = logQ;
        *rearP = socket;
        return;
    }
    rearP++;
    *rearP = socket;
*/}

//removes an object from the queue
char* rmv_log(void) {
	char *temp;
	temp = logQ[next_log];
	count--;
	next_log = (next_log + 1) % 64;
//	printf("removing\n");
	return temp;
/*
    char* temp;

    if(frontP == end){
        temp = *frontP;
        *frontP = 0;
        frontP = logQ;
        return temp;
    }

    if(frontP == rearP) {
        temp = *frontP;
        *frontP = 0;
        return temp;
    }

    temp = *frontP;
	printf("In queue_log printing temp %s", temp);
    *frontP = 0;
    frontP++;
    return(temp);
*/
}

//checks if the queue is empty
int isEmpty_log(void) {
    if(count == 0)
        return 1;
    else
        return 0;

}

//checks to see if queue is full
int isFull_log(void) {
    if(count == 64)
	return 1;
    else return 0;
}
/*
//returns the length of the queue.
long int getLength_log(void) {
    long int length;
    if(isEmpty())
        return 0;

    long int range = labs((long int)frontP - (long int)rearP);
    //printf("\n%d\n", range);

    if(range == 0)
        length = 1;

    length = range/sizeof(int);

    return length;
}

*/
