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
#include <ctype.h>

// #define NTHREADS 2				// Max number of threads
#define MAX_LEN_MSG 1024		// Max message length -> should not be longer than this
#define TIMEOUT 60				// Time after which player times out
#define GRID_SIZE 3

/*
 * Message macros
 */
#define FYI 0x01
#define MYM 0x02
#define END 0x03
#define TXT 0x04
#define MOV 0x05
#define CON 0x06

// pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// pthread_t tid[NTHREADS]; 		// List of threads
// int thread_id;					// Current thread id


/*
 * Stores the socket information of the two clients
 * For each client:
 * 	- struct sockaddr
 * 	- socklen_t
 */

struct sockaddr_in server_addr;
socklen_t server_addr_len;

struct sockaddr_in client_addr[2];
socklen_t client_addr_len[2];

// struct clientinfos {
// 	struct sockaddr_in *clientsock1;
// 	socklen_t addrlen1;
// 	struct sockaddr_in *clientsock2;
// 	socklen_t addrlen2; 
// };

// Game state
// Cells in grid contain a 0 if empty, 1 if occupied by player 1 and a 2 otherwise

int grid[GRID_SIZE][GRID_SIZE] = {};							// Hold the board state
// memset(grid, 0, sizeof(grid[0][0]) * GRID_SIZE * GRID_SIZE);
int turn = 0;							// Initially player 1s turn
int move_count = 0;						// Count the number of moves made to determine when the game is over

// int connected = 0;						// Number of connected clients
// struct clientinfos client_addresses;	// Holds client information
int sockfd;								// Socket file descriptor
int port;

/* Function descriptors */
int play_game(void);
int connect_players(void);
int check_valid(int move[2]);
void update_game(int move[2]);
int construct_FYI(char *msg);
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

	// if (!isnumber(argv[1])) {
	// 	fprintf(stderr, "Port should be a number.\n");
	// }

	port = atoi(argv[1]);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
		return 1;
	}

	puts("Socket created");

	// memset(&sockaddr, 0, sizeof sockaddr);
	// sockaddr.sin_family = AF_INET;
	// sockaddr.sin_port = htons(port);
	// inet_pton(AF_INET, "127.0.0.1", &(sockaddr.sin_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr));

	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof server_addr)) {
		fprintf(stderr, "Error binding to socket: %s\n", strerror(errno));
		return 1;
	}

	puts("Binded to socket");

	// Wait for two clients to connect
	
	int connection = connect_players();
	if (connection) {
		return 1;
	}

	printf("Both clients connected.\n");

	// After this point both clients are connected and ready to play the game

	play_game();
}

/*
 * Connects to two players. Returns 0
 * upon success and 1 upon failure.
 */
int connect_players(void) {

	int connected = 0;

	while (connected < 2) {

		struct sockaddr_in sockaddr_client;
		socklen_t addrlen_client = sizeof sockaddr_client;

		char *msg = (char *)malloc(MAX_LEN_MSG * sizeof(char));
		int bytes_received = recvfrom(sockfd, (char *)msg, MAX_LEN_MSG, 0, (struct sockaddr *)&sockaddr_client, &addrlen_client);

		if (bytes_received == -1) {
			fprintf(stderr, "Error while receiving message: %s\n", strerror(errno));
			return 1;
		}

		if (msg[0] == CON) {
			free(msg);
			printf("Client %d connected\n", connected + 1);
		}
		else {
			free(msg);
			break;
		}
		
		if (connected == 0) {
			client_addr[0] = sockaddr_client;
			client_addr_len[0] = sizeof client_addr[0];
		}
		else if (connected == 1) {
			client_addr[1] = sockaddr_client;
			client_addr_len[1]= sizeof client_addr[1];
		}

		char *connection_msg = malloc(sizeof(char));
		memset(connection_msg, CON, sizeof(char));

		// Send connection_msg to client
		int bytes_sent = sendto(sockfd, (char *)connection_msg, sizeof(char), 0, (struct sockaddr *)&sockaddr_client, addrlen_client);
		if (bytes_sent == -1) {
			fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
			return 1;
		}

		connected++;
		free(connection_msg);

	}

	return 0;

}

/* 
 * Plays the game until it terminates. 
 * Returns 0 upon success and 1 upon failure.
 */
int play_game(void) {

	int not_terminated = 1;

	while (not_terminated) {

		struct sockaddr_in curr_play_sockaddr;
		socklen_t curr_play_addrlen;

		// Get current player's information
		curr_play_sockaddr = (!turn) ? client_addr[0] : client_addr[1];
		curr_play_addrlen = (!turn) ? client_addr_len[0] : client_addr_len[1];

		// Prepare FYI message
		char* fyi_msg = NULL;
		int fyi_msg_len = construct_FYI(fyi_msg);


		// Send FYI message
		int bytes_sent = sendto(sockfd, fyi_msg, fyi_msg_len, 0, (struct sockaddr *)&curr_play_sockaddr, curr_play_addrlen);
		if (bytes_sent == -1) {
			fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
			return 1;
		}

		printf("Sent FYI message.\n");
		free(fyi_msg);

		// Prepare MYM message
		char *mym_msg = malloc(sizeof(char));
		memset(mym_msg, MYM, sizeof(char));

		// Send MYM message
		bytes_sent = sendto(sockfd, &mym_msg, 1, 0, (struct sockaddr *)&curr_play_sockaddr, curr_play_addrlen);
		if (bytes_sent == -1) {
			fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
			return 1;
		}

		// Get response
		char *response = malloc(MAX_LEN_MSG * sizeof(char));
		int received = 1; // While waiting for reception
		int valid = 1; // While waiting for a valid move
		int move[2];

		while (valid) {

			received = 1;

			while (received) {

				int bytes_received = recvfrom(sockfd, &response, MAX_LEN_MSG, 0, (struct sockaddr *)&curr_play_sockaddr, &curr_play_addrlen);
				if (bytes_received == -1) {
					fprintf(stderr, "Error while receiving message: %s\n", strerror(errno));
					return 1;
				}

				if (response[0] != MOV) { // Send another request
					bytes_sent = sendto(sockfd, &mym_msg, 1, 0, (struct sockaddr *)&curr_play_sockaddr, curr_play_addrlen);
					if (bytes_sent == -1) {
						fprintf(stderr, "Error while sending message: %s\n", strerror(errno));
						return 1;
					}
				}
				else {
					received = 0;
				}
			}

			move[0] = mym_msg[1];
			move[1] = mym_msg[2];
			valid = !check_valid(move); // If the move is valid, !check_valid returns 0 and we exit the loop

		}
		
		free(mym_msg);

		// Parse response etc.

		update_game(move);
		
		int status = check_status();

		if (status) {
			if (status == 1) {
				// Player 1 wins
			}
			else if (status == 2) {
				// Player 2 wins
			}
			else if (status == 3) {
				// Draw
			}
		}

		turn = !turn;

		move_count += 1;
		if (move_count == GRID_SIZE * GRID_SIZE) {
			not_terminated = 0;
		}
	}
	return 0;
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
void update_game(int move[2]) {
	int x, y;
	x = move[0];
	y = move[1];
	grid[x][y] = (!turn) ? 1 : 2;
}

/* 
 * Construct FYI message to send
 */
int construct_FYI(char *msg) {
	int n = 0; // Number of filled positions
	int i, j;
	for (i = 0; i < 3; i++) { // Compute n
		for (j = 0; j < 3; j++) {
			if (grid[i][j] != 0) {
				n++;
			}
		}
	}

	msg = (char*) malloc((2 + (3 * n)) * sizeof(char)); // Allocate memory for message
	memset(msg, 0, sizeof(char) * (2 + (3 * n)));
	memset(msg, FYI, 1); // Set FYI char
	memset(msg + 1, (char) n, 1); // Set number of blocks filled

	// printf("%s\n", msg);

	int k = 1;
	char *temp = msg + 2;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (grid[i][j] != 0) {
				memset(temp + (3 * k), grid[i][j], 1);
				memset(temp + (3 * k) + 1, i, 1);
				memset(temp + (3 * k) + 2, j, 1);

				k++;
			}
		}
	}
	
	return (2 + (3 * n));
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
	return 0;
}
