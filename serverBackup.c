#include "simpleServer.h"

/*
char* logQ[64];
int count, empty_log, next_log;

void insert_log(char* socket) {
        printf("inserting\n");
        logQ[empty_log] = socket;
        count++;
        empty_log = (empty_log + 1) % 64;

        int i;
        for(i = 0; i < 64; i++){
//                printf("location %d logQ %s", i, logQ[i]);
        }
}

char* rmv_log(void) {
        char *temp;
        temp = logQ[next_log];
        count--;
        next_log = (next_log + 1) % 64;
        printf("removing\n");
        return temp;
}
*/
//An extremely simple server that connects to a given port.
//Once the server is connected to the port, it will listen on that port for a user connection.
//A user will connect through telnet, the user also needs to know the port number.
//If the user connects successfully, a socket descriptor will be created.
//The socket descriptor works exactly like a file descriptor, allowing us to read/write on it.
//The server will then use the socket descriptor to communicate with the user, sending and receiving messages.

pthread_mutex_t logger_lock;
pthread_mutex_t worker_lock;
pthread_cond_t logger_sleepwake;
pthread_cond_t worker_sleepwake;
pthread_cond_t temp;

void worker_add(int socket);
int worker_remove(void);
void logger_add(char* socket);
char* logger_remove(void);
void *logger_func(void *args);
void *worker_func(void *args);
char* spellCheck(char* client_word, int bytesReturned);
char** load_dict(char* path);

#define MAX_WORKERS 4

char** dictionary;
size_t dict_length = 0;

int main(int argc, char** argv)
{
/*
	count = 0;
	empty_log = 0;
	next_log = 0;
*/
	//Erases existing output.txt file data, if any.
	FILE *fp = fopen("output.txt", "w");
	fclose(fp);

        //Initialize the locks.
        pthread_mutex_init(&logger_lock, NULL);
        pthread_mutex_init(&worker_lock, NULL);

        //Initialize the condition variables.
        pthread_cond_init(&logger_sleepwake, NULL);
        pthread_cond_init(&worker_sleepwake, NULL);
//	pthread_cond_init(&temp, NULL);
//        printf("Intializations finished\n");


	//ASSIGN LOGGER/WORKER THREADS.
        pthread_t workers[MAX_WORKERS];

        if(pthread_create(&workers[0], NULL, logger_func, NULL) != 0){
                perror("Error :(");
		exit(1);
        }

        int i;
        for(i = 1; i < MAX_WORKERS; i++){
                if(pthread_create(&workers[i], NULL, worker_func, NULL) != 0){
                        perror("Error :(");
                        exit(1);
                }
        }

	//LOAD IN THE DICTIONARY!
	dictionary = load_dict("words.txt");
//	for(i = 0; i < 5; i++){
//		printf("%s", dictionary[i]);
//	}
//	printf("Dictionary loaded.\n");

	//SETTING UP THE NETWORK CONNECTION.
/*
	if(argc == 1){
		printf("No port number entered.\n");
		return -1;
	}
*/
	//sockaddr_in holds information about the user connection.
	//We don't need it, but it needs to be passed into accept().
	struct sockaddr_in client;
	int clientLen = sizeof(client);
//	int connectionPort = atoi(argv[1]);
	int connectionSocket, clientSocket, bytesReturned;

	int connectionPort = 30421;

	//We can't use ports below 1024 and ports above 65535 don't exist.
	if(connectionPort < 1024 || connectionPort > 65535){
		printf("Port number is either too low(below 1024), or too high(above 65535).\n");
		return -1;
	}

	//Does all the hard work for us.
	connectionSocket = open_listenfd(connectionPort);
	if(connectionSocket == -1){
		printf("Could not connect to %s, maybe try another port number?\n", argv[1]);
		return -1;
	}

	//accept() waits until a user connects to the server, writing information about that server
	//into the sockaddr_in client.
	//If the connection is successful, we obtain A SECOND socket descriptor.
	//There are two socket descriptors being used now:
	//One by the server to listen for incoming connections.
	//The second that was just created that will be used to communicate with the connected user.
   	printf("Enter ctrl + c to end the program\n");

while(1){

	if((clientSocket = accept(connectionSocket, (struct sockaddr*)&client, &clientLen)) == -1){
		printf("Error connecting to client.\n");
		return -1;
	}
	worker_add(clientSocket);

}

	//SET UP A QUEUE THAT INCOMING CONNECTIONS WILL BE PLACED INTO FOR THE WORKER THREADS..
	//Note: queue.c is worker queue, queue_log.c is logger queue. They are already defined in these files.
/*
	//ASSIGN LOGGER/WORKER THREADS.
	pthread_t workers[MAX_WORKERS];

	if(pthread_create(&workers[0], NULL, logger_func, NULL) != 0){
		perror("Error :(");
		exit(1);
	}

	int i;
	for(i = 1; i < MAX_WORKERS; i++){
		if(pthread_create(&workers[i], NULL, worker_func, NULL) != 0){
			perror("Error :(");
			exit(1);
		}
	}
*/
	return 0;
}

void worker_add(int socket){
	//"Get" a lock for the worker queue.
        pthread_mutex_lock(&worker_lock);

	//This method is a "producer", creates/inserts.
	//Note: if the queue is full, a "consumer" must take an "object" off the queue.
        while(isFull() == 1){
                pthread_cond_wait(&worker_sleepwake, &worker_lock);
        }

        //Add the socket to the queue, use the written insert function.
	insert(socket);

//	socket = rmv();
//	printf("appended to queue: %d\n", socket);
//	printf("length of queue after insert: %ld\n", getLength());
//	socket = rmv();
//	printf("length of queue after remove: %d\n", socket);

	//Producer has completed its part and inserted a socket, thus...
	//Wakes up all worker threads (64)
	pthread_cond_signal(&worker_sleepwake);

        //"Unlock"
        pthread_mutex_unlock(&worker_lock);
}

int worker_remove(){
	int socket;
//	printf("inside worker_remove\n");

	//Must lock before accessing the worker queue, in order to avoid data from being prematurely read.
	//**Don't want any worker threads to have old data.
	pthread_mutex_lock(&worker_lock);

	while(isEmpty() == 1){
		pthread_cond_wait(&worker_sleepwake, &worker_lock);
	}

	socket = rmv();
//	printf("socket in worker_remove is %d\n", socket);
	pthread_cond_signal(&worker_sleepwake);
	pthread_mutex_unlock(&worker_lock);
	return socket;
}

void logger_add(char* socket){
	pthread_mutex_lock(&logger_lock);
//	while(count == 64){
	while(isFull_log() == 1){
		pthread_cond_wait(&logger_sleepwake, &logger_lock);
        }
	insert_log(socket);
//	printf("added socket:%s", socket);
	pthread_cond_signal(&logger_sleepwake);
	pthread_mutex_unlock(&logger_lock);
}

char* logger_remove(){
	char* socket;
	pthread_mutex_lock(&logger_lock);
//	while(count == 0){
	while(isEmpty_log() == 1){
		pthread_cond_wait(&logger_sleepwake, &logger_lock);
	}
	socket = rmv_log();
//	printf("Socket removed: %s", socket);
	pthread_cond_signal(&logger_sleepwake);
	pthread_mutex_unlock(&logger_lock);
	return socket;
}

void *worker_func(void *args){
//	printf("inside worker_func\n");
	int clientSocket, bytesReturned, i;
	char recvBuffer[BUF_LEN];
        recvBuffer[0] = '\0';
	char *spellCheckResp;

	char* clientMessage = "Hello! I hope you can see this.\n";
	char* msgRequest = "Send some text.\n";
	char* msgResponse = "Response\n";
	char* msgPrompt = ">>>";
        char* msgError = "I didn't get your message. ):\n";
        char* msgClose = "Goodbye!\n";

	//Begin sending and receiving messages.
	while(1){
	clientSocket = worker_remove();
//	printf("clientSocket is %d\n", clientSocket);
	while(1){
		send(clientSocket, msgPrompt, strlen(msgPrompt), 0);

		//recv() will store the message from the user in the buffer, returning how many bytes we received.
		bytesReturned = recv(clientSocket, recvBuffer, BUF_LEN, 0);

//		printf("received %s", recvBuffer);

		//Check if we got a message, send a message back or quit if the user specified it.
		if(bytesReturned == -1){
			send(clientSocket, msgError, strlen(msgError), 0);
		}

		//'27' is the escape key.
                else if(recvBuffer[0] == 27){
                        send(clientSocket, msgClose, strlen(msgClose), 0);
			close(clientSocket);
			break;
		} else {
//			printf("before spellcheck call\n");
			spellCheckResp = spellCheck(recvBuffer, bytesReturned);
			send(clientSocket, spellCheckResp, strlen(spellCheckResp), 0);
			logger_add(spellCheckResp);
//                        send(clientSocket, msgResponse, strlen(msgResponse), 0);
//			send(clientSocket, recvBuffer, bytesReturned, 0);

			//This line will send it back to the server, it also clears the old buffer
//			fflush(recvBuffer);

			for(i = 0; i < BUF_LEN; i++){
				recvBuffer[i] = '\0';
			}
//			write(1, recvBuffer, bytesReturned);
		}
        }
	close(clientSocket);
	}

}

void *logger_func(void *args){
	char* spellCheckResp;
	while(1){
		spellCheckResp = logger_remove();
//		printf("%s", spellCheckResp);
		FILE *output = fopen("output.txt","a");
		fprintf(output, "%s", spellCheckResp);
		fclose(output);
	}

}

char* spellCheck(char* client_word, int bytesReturned){
//	printf("In spellcheck\n");
	int flag = 0, i;
	char* dict_word = malloc(bytesReturned + 11);
//	printf("length of dict: %ld\n", dict_length);

//	printf("length of client_word %d\n", strlen(client_word));


	for(i = 0; i < dict_length; i++){
//		printf("length of dictionary[%d] %d\n", i, strlen(dictionary[i]));
		if (strncmp(dictionary[i], client_word, bytesReturned - 2) == 0){
			flag = 1;
//			strcpy(client_word, client_word-1)
//			for(i = 0; i < strlen(client_word); i++){
///				if(client_word[i] == '\n'){
//					client_word[i] = '\0';
//				}
//			}

			client_word[bytesReturned - 2] = '\0';
			strcat(dict_word, client_word);
			strcat(dict_word, "OK\n");
			break;
		}
//		dict_word;
	}

	if(flag == 0){
		client_word[bytesReturned - 2] = '\0';
		strcat(dict_word, client_word);
		strcat(dict_word, "MISSPELLED\n");
	}
//	printf("Client word @ end: %s\n", client_word);

	return dict_word;
}

//LOAD THE DICTIONARY FILE INTO MEMORY.
char** load_dict(char* path){
	size_t i, buffer = 0;
	char *temp;
	int num = 0;

	//Counts the lines in the dictionary file. Note that there is only one word per line.
	FILE *dict = fopen(path, "r");
	while((num = fgetc(dict)) != EOF){
		if(num == '\n'){
			dict_length++;
		}
	}

	//Sets the file pointer to the beginning of the file.
	fseek(dict, 0, SEEK_SET);

	//Collect the words from the dictionary into the array.
	char** word_array = (char**)malloc(dict_length * sizeof(char*));
//        FILE *dict = fopen("words.txt", "r");
	for(i = 0; i < dict_length; i++){
		getline(&word_array[i], &buffer, dict);
		buffer++;
	}

	fclose(dict);
	return word_array;
}
