/*
 * Tictactoe server.c
 * 
 * Lucien Walewski and Alban Puech
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>

#define NTHREADS 2				// Max number of threads
#define MAX_LEN_MSG 2048		// Max message length -> should not be longer than this
#define TIMEOUT 60				// Time after which player times out

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid[NTHREADS]; 		// List of threads
int thread_id;					// Current thread id

struct thread_arg {
	char buffer[MAX_LEN_MSG];
	struct sockaddr from;
};

/*
 * Stores the socket information of the two clients
 * For each client:
 * 	- struct sockaddr
 * 	- socklen_t
 */
struct clientinfos {
	struct sockaddr *clientsock1;
	socklen_t arrdlen1;
	struct sockaddr *clientsock2;
	socklen_t addrlen2; 
};

// Game state
int grid[3][3];							// Hold the board state
int turn = 0;							// Initially player 1s turn

int connected = 0;						// Number of connected clients
struct clientinfos *client_addresses;	// Holds client information
int sockfd;								// Socket file descriptor

/* Function descriptors */
void *play_game(void);
void *connect_players(void);
int *check_valid(int move[2]);
void *update_game(int move[2]);
void *construct_FYI(char *msg);

// void *recsend(void *argu) {
// 	struct thread_arg *args = (struct thread_arg *)argu;

// 	// Third client attempts to connect
// 	if (args->from != clientinfo.client1 && args->from != clientinfo.client2) {
// 		char msg[] = "END 255";
// 		int bytes_sent = sendto(sockfd, msg, sizeof msg, 0, &(args->from), sizeof args->from);
// 		if (bytes_sent == -1) {
// 			fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
// 			return NULL;
// 		}
// 	}

// 	// Not client's turn
// 	if ((args->from == info.client1 && !info.turn) || (args->from == info.client2 && info.turn)) {
// 		char msg[] = "Not your turn";
// 		int bytes_sent = sendto(sockfd, msg, sizeof msg, 0, &(args->from), sizeof args->from);
// 		if (bytes_sent == -1) {
// 			fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
// 			return NULL;
// 		}
// 	}
// 	char msg[3];
// 	strncpy(msg, args->buffer, 3);
// }

int main(int argc, char *argv[]) {

	if (argc < 2) {
		fprintf(stderr, "Missing argument. Please enter PORT number.\n");
		return 1;
	}

	if (argc > 2) {
		fprintf(stderr, "Too many arguments given.\n");
		return 1;
	}

	long port = argv[1];
	struct sockaddr_in sockaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
		return 1;
	}

	puts("Socket created\n");

	memset(&sockaddr, 0, sizeof sockaddr);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &(sockaddr.sin_addr));

	if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof sockaddr)) {
		fprintf(stderr, "Error binding to socket: %s\n", strerror(errno));
		return 1;
	}

	puts("Binded to socket\n");

	// Wait for two clients to connect
	

	while (connected < 2) {
		char msg[MAX_LEN_MSG];
		int bytes_received = recvfrom(sockfd, (char *)msg, MAX_LEN_MSG, 0, (struct  sockaddr *)&sockaddr, sizeof sockaddr);

		if (bytes_received == -1) {
			fprintf(stderr, "Error while receiving message: %s\n", strerror(errno));
			return 1;
		}
		if (strncpy(msg, "CON", 3 * sizeof(char)) != 0) {
			break;
		}

		printf("Client %d connected\n", connected + 1);

		if (connected == 0) {
			client_addresses->clientsock1 = (struct sockaddr *)&sockaddr;
			client_addresses->arrdlen1 = sizeof sockaddr;
		}
		else if (connected == 1) {
			client_addresses->clientsock2 = (struct sockaddr *)&sockaddr;
			client_addresses->addrlen2 = sizeof sockaddr;
		}

		char connection_msg[] = (connected == 0) ? "CON. Client 1 connected" : "CON. Client 2 connected";

		// send connection_msg to client
		int bytes_sent = sendto(sockfd, (char *)connection_msg, MAX_LEN_MSG, 0, (struct sockaddr *)&sockaddr, sizeof sockaddr);
		if (bytes_sent == -1) {
			fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
		}

		connected++;

	}

	// After this point both clients are connected and ready to play the game

	play_game();



}

/* Plays the game until it terminates */
void *play_game(void) {

	int not_terminated = 1;

	while (not_terminated) {

		struct sockaddr *curr_play_sockaddr;
		socklen_t curr_play_addrlen;

		curr_play_sockaddr = (!turn) ? (struct sockaddr *)client_addresses->clientsock1 : (struct sockaddr *)client_addresses->clientsock2;
		curr_play_addrlen = (!turn) ? client_addresses->arrdlen1 : client_addresses->addrlen2;

		char* msg = NULL;
		construct_FYI(msg);

		int bytes_sent = sendto(sockfd, &msg, 3, 0, curr_play_sockaddr, curr_play_addrlen);
		if (bytes_sent == -1) {
			fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
		}

		memset(msg, 0, sizeof msg);
		strncpy(msg, "MYM", 3);

		int bytes_sent = sendto(sockfd, &msg, 3, 0, curr_play_sockaddr, curr_play_addrlen);
		if (bytes_sent == -1) {
			fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
		}

		char response[MAX_LEN_MSG];
		int bytes_received = recvfrom(sockfd, &response, MAX_LEN_MSG, 0, curr_play_sockaddr, curr_play_addrlen);
		if (bytes_received == -1) {
			fprintf(stderr, "Error while receiving message: %s\n", strerror(errno));
		}

		// Parse response etc.


	}
}

/* Returns 1 if the move is valid otherwise 0 */
int *check_valid(int move[2]) {
	int x, y;
	x = move[0];
	y = move[1];
	if (grid[x][y] != 0) {
		return 0;
	}
	return 1;

}

/* Updates the game board assuming the move was valid */
void *update_game(int move[2]) {
	int x, y;
	x = move[0];
	y = move[1];
	grid[x][y] = (!turn) ? 1 : 2;
}

void *construct_FYI(char *msg) {
	int n; // Number of filled positions
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (grid[i][j] != 0) {
				n++;
			}
		}
	}
	msg = (char*) malloc((4 + 3 * n) * sizeof(char));
	memset(msg, 0, sizeof msg);
	strncpy(msg, "FYI", 3);
	strncpy(msg + 3, n, 1);

	int k;
	char *temp = msg + 4;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (grid[i][j] != 0) {
				char positions[3];
				itoa(grid[i][j], positions[0], 10);
				itoa(i, positions[1], 10);
				itoa(j, positions[2], 10);
				strncp(msg + (3 * k), positions, 3);	
			}
		}
	}
	
	return NULL;
}



// void *recsend(void *argu){
//   // function to receive and send back the messag
//  struct thread_arg *args = argu;
 
//  // the client is not part of our 2 players :
//  if (args->from!=clientinfo.client1 && args->from!=clientinfo.client2) {
//  char[] msg = "END 255";
//  sendto(mysocket, msg, sizeof(msg), 0,&(args->from), sizeof(args->from))==-1);
//  return NULL;
//  }
 
//  // it is part of our 2 players but not his turn :
//  if (   (args->from==info.client1 && !info.turn)   ||  (args->from==info.client2 && info.turn)  )   {
//   char[] msg = "pas ton tour";
//   sendto(mysocket, msg, sizeof(msg), 0,&(args->from), sizeof(args->from))==-1);
//   return NULL;
//  }
  
//  //it is his turn :
  
 
//  char MSG[3];
//  strncpy(MSG,args->buffer,3); 
//  if (strcmp(MSG, "LFT") == 0) 
// {
//   // do something
// } 
// else if (strcmp(MSG, "MOV") == 0)
// {
//   // do something else
// }
// /* more else if clauses */
// else /* default: */
// {
// }
 
  
  
//  pthread_mutex_lock(&lock);      // START critical region
//  if( sendto(mysocket, args->buffer, sizeof(args->buffer), 0,&(args->from), sizeof(args->from))==-1){
//      fprintf(stderr,"error while sending\n");
//  exit(EXIT_FAILURE);   
//  }
//  pthread_mutex_unlock(&lock);     // END critical region
//  return NULL;

// }




// int main (int argc, char *argv []) {
  
// struct sockaddr_in addr;
// int port;
// mysocket = socket(AF_INET, SOCK_DGRAM, 0);

//  if (argc<2){
//    fprintf(stderr,"missing arguments\n");
//    return 1 ;
//  }

//  if (argc>2){
//    fprintf(stderr,"too many arguments\n");
//    return 1 ;
//  }



// port = atol(argv[1]);
//  if (port==0){
//  fprintf(stderr,"incorrect port\n");
//  return 1; 
//  }

// if (mysocket==-1){
//   fprintf(stderr,"Could not create socket\n");
//   return 1; 
// }

// memset(&addr, 0, sizeof(addr));
// addr.sin_family = AF_INET;
// inet_pton(AF_INET,"127.0.0.1",&(addr.sin_addr));
// addr.sin_port = htons(port);

//  if (bind(mysocket, (const struct sockaddr*) &addr, sizeof(addr))==-1){
//    fprintf(stderr,"impossible to bind\n");
//    return 1;
//  }

// // initialize the mutex
//  if (pthread_mutex_init(&lock, NULL) != 0){
//    printf("\n mutex init failed\n");
//    return 1;
//  }

//  // store the threads 
//  pthread_t thread[NTHREADS];
//  // counter for the number of threads  
//  int count = 0;
 

//  while(1){
   
//    // allocate space for struct of arguments to give to the function 
//    struct thread_arg *args = malloc(sizeof(struct thread_arg));
//    socklen_t  fromlen = (socklen_t)  sizeof(struct sockaddr);

//    // receive the message and store the address of the sender
//    int bytes_received =  recvfrom(mysocket, args->buffer, MAX_LEN_MSG, 0,&(args->from), &fromlen);
   

   
 
//    if  (bytes_received == -1){
//      printf("error while receiving the message from the client\n");
//      return 1;}

//    // if less than N threads, create a new thread to handle the communication with the client 
//      if (count < NTHREADS){
//      if(pthread_create(&thread[count-1], NULL, recsend, args)) {
//        fprintf(stderr, "Error creating thread\n");
//        return 1;
//      }}

//      // otherwise, wait for all the threads to terminate and create a new thread after that 
//      else{
//        int j;
//        for (j=1; j<NTHREADS; j++)
// 	 {
// 	   if(pthread_join(thread[j-1],NULL )) {
// 	     fprintf(stderr, "Error waiting for thread\n");
// 	     return 1;
// 	   }
// 	 }     
      
//        count = 0;
//        if(pthread_create(&thread[count-1], NULL, recsend, args)) {
// 	 fprintf(stderr, "Error creating thread\n");
// 	 return 1;
//        }
//      }
//      count+=1; // increment the counter of threads
//    }
 
//  return 0;
// }



