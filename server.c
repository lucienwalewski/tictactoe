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
#define GRID_SIZE 3
#define MOVE_COUNT 0			// Count the number of moves made to determine when the game is over

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
// Cells in grid contain a 0 if empty, 1 if occupied by player 1 and a 2 otherwise
int grid[3][3];							// Hold the board state
int turn = 0;							// Initially player 1s turn

int connected = 0;						// Number of connected clients
struct clientinfos *client_addresses;	// Holds client information
int sockfd;								// Socket file descriptor

/* Function descriptors */
void *play_game(void);
void *connect_players(void);
int check_valid(int move[2]);
void *update_game(int move[2]);
void *construct_FYI(char *msg);
int check_status(void);

int main(int argc, char *argv[]) {

	if (argc < 2) {
		fprintf(stderr, "Missing argument. Please enter PORT number.\n");
		return 1;
	}

	if (argc > 2) {
		fprintf(stderr, "Too many arguments given.\n");
		return 1;
	}

	long port = atol(argv[1]);
	struct sockaddr_in sockaddr;
	// socklen_t addrlen;

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
		struct sockaddr_in sockaddr_client;
		socklen_t addrlen_client = sizeof sockaddr_client;

		char msg[MAX_LEN_MSG];
		int bytes_received = recvfrom(sockfd, (char *)msg, MAX_LEN_MSG, 0, (struct sockaddr *)&sockaddr_client, &addrlen_client);

		if (bytes_received == -1) {
			fprintf(stderr, "Error while receiving message: %s\n", strerror(errno));
			return 1;
		}
		if (strncpy(msg, "CON", 3 * sizeof(char)) != 0) {
			break;
		}

		printf("Client %d connected\n", connected + 1);

		if (connected == 0) {
			client_addresses->clientsock1 = (struct sockaddr *)&sockaddr_client;
			client_addresses->arrdlen1 = sizeof addrlen_client;
		}
		else if (connected == 1) {
			client_addresses->clientsock2 = (struct sockaddr *)&sockaddr_client;
			client_addresses->addrlen2 = sizeof addrlen_client;
		}

		char *connection_msg = (connected == 0) ? "CON. Client 1 connected" : "CON. Client 2 connected";

		// send connection_msg to client
		int bytes_sent = sendto(sockfd, (char *)connection_msg, MAX_LEN_MSG, 0, (struct sockaddr *)&sockaddr_client, addrlen_client);
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

		// Get current player's information
		curr_play_sockaddr = (!turn) ? (struct sockaddr *)client_addresses->clientsock1 : (struct sockaddr *)client_addresses->clientsock2;
		curr_play_addrlen = (!turn) ? client_addresses->arrdlen1 : client_addresses->addrlen2;

		// Prepare FYI message
		char* msg = NULL;
		construct_FYI(msg);

		// Send FYI message
		int bytes_sent = sendto(sockfd, &msg, 3, 0, curr_play_sockaddr, curr_play_addrlen);
		if (bytes_sent == -1) {
			fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
		}

		// Prepare MYM message
		memset(msg, 0, sizeof msg);
		strncpy(msg, "MYM", 3);

		// Send MYM message
		bytes_sent = sendto(sockfd, &msg, 3, 0, curr_play_sockaddr, curr_play_addrlen);
		if (bytes_sent == -1) {
			fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
		}

		// Get response
		char *response = malloc(MAX_LEN_MSG * sizeof(char));
		int received = 1; // While waiting for reception

		while (received) {
			int bytes_received = recvfrom(sockfd, &response, MAX_LEN_MSG, 0, curr_play_sockaddr, &curr_play_addrlen);
			if (bytes_received == -1) {
				fprintf(stderr, "Error while receiving message: %s\n", strerror(errno));
			}
			
			if (strncmp(response, "MOV", 3) != 0) {
				// Send another request
			}
		}
		

		// Parse response etc.
		if (strncmp(response, "MOV", 3) != 0) {

		}
		turn = !turn;

	}
}

/* Returns 1 if the move is valid otherwise 0 */
int check_valid(int move[2]) {
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

/* 
 * Construct FYI message to send
 */
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
	memset(msg + 3, n, 1);

	int k = 1;
	char *temp = msg + 4;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (grid[i][j] != 0) {
				char positions[3];
				itoa(grid[i][j], positions[0], 10);
				itoa(i, positions[1], 10);
				itoa(j, positions[2], 10);
				strncpy(msg + (3 * k), positions, 3);	
				k++;
			}
		}
	}
	
	return NULL;
}


/*
 * Checks the status of the game. Return 0 if the game is unfinished, 
 * 1 if the first player won, 2 if the second player won and 3 if the game
 * ended in a draw. 
 */
int check_status(void) {
	int i, j;
	// Check rows and columns
	for (i = 0; i < GRID_SIZE; i++) {
		for (j = 1; j < GRID_SIZE; j++) { // Check if all the elements in the row belong to this player
			if (grid[i][j] != grid[i][0]) {
				break;
			}
		}
		if (j == GRID_SIZE) {
			return grid[i][0];
		}

		for (j = 1; j < GRID_SIZE; j++) { // Check if all the elements in the column belong to this player
			if (grid[j][i] != grid[0][i]) {
				break;
			}
		}
		if (j == GRID_SIZE) {
			return grid[0][i];
		}
	}

	// Check first diagonal
	for (i = 0; i < GRID_SIZE; i++) {
		if (grid[i][i] != grid[0][0]) {
			break;
		}
	}
	if (i == GRID_SIZE) {
		return grid[0][0];
	}

	// Check second diagonal
	for (i = 0; i < GRID_SIZE; i++) {
		if (grid[i][GRID_SIZE - i - 1] != grid[0][GRID_SIZE - 1]) {
			break;
		}
	}
	if (i == GRID_SIZE) {
		return grid[0][GRID_SIZE - 1];
	}

	// Draw 
	return 3;
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
 
  
  
