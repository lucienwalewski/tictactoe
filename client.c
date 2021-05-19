
/* 
 * Tictactoe client.h
 * 
 * Lucien Walewski and Alban Puech
 */

/* J'ai fait expres de faire ca car tu importais plusiers
 * header files en double
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

#define MAX_LEN_MSG 2048

/*
 * Message macros
 */
#define FYI 0x01
#define MYM 0x02
#define END 0x03
#define TXT 0x04
#define MOV 0x05
#define LFT 0x06

int allow_write = 0;

void *decrypt_fyi(void * msg);

int main(int argc, char *argv[]) {

	if (argc < 3) {
		fprintf(stderr, "Missing arguments.\n");
		return 1;
	}

	if (argc > 3) {
		fprintf(stderr, "Too many arguments given.\n");
		return 1;	
	}

	int sockfd;
	struct sockaddr_in sockaddr;
	long port;

	char *address = argv[1]; // Get address
	port = atol(argv[2]); // Get port number

	if (port == 0) {
		fprintf(stderr, "Incorrect port: %s\n", strerror(errno));
		return 1;
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Create socket
	if (sockfd == -1) {
		fprintf(stderr, "Could not create socket: %s\n", strerror(errno));
		return 1;
	}

	memset(&sockaddr, 0, sizeof sockaddr);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	if (inet_pton(AF_INET, address, &(sockaddr.sin_addr)) == 0) {
		fprintf(stderr, "Invalid address: %s\n", strerror(errno));
		return 1;
	}

	// Connect to server

	char msg_co[] = "CON";

	int bytes_sent = sendto(sockfd, &msg_co, 3, 0, &sockaddr, sizeof(sockaddr));
	if (bytes_sent == -1) {
		fprintf(stderr, "Error while connecting (sending first message): %s\n", strerror(errno));
	}

	char msg_back[MAX_LEN_MSG];
	int bytes_received = recvfrom(sockfd, (char *)msg_back, MAX_LEN_MSG, 0, &sockaddr, sizeof sockaddr);
	if (bytes_received == -1) {
		fprintf(stderr, "Error while connecting (receiving message): %s\n", strerror(errno));
	}

	if (strncmp(msg_back, "CON", 3 * sizeof(char)) != 0) {
		printf("Connection failed.\n");
		return 1;
	}

	printf("%s\n", *msg_back + 3);

	// We are connected

	while (1) {

		char *line = NULL;
		size_t len = 0;

		if (!allow_write) {
			// if (getline(&line, &len, stdin)>0){printf("not your turn !/n");}

			char msg_rec[MAX_LEN_MSG];
			int bytes_received = 0;

			bytes_received = recvfrom(sockfd, (char *)msg_rec, MAX_LEN_MSG, 0, &sockaddr, sizeof(sockaddr));
			if (bytes_received == -1) {
				fprintf(stderr, "Error while receiving message: %s\n", strerror(errno));
			}
			else if (strncmp(msg_rec, "FYI", 3)) { 
				printf(decrypt_fyi(msg_rec));
			}
			else if (strncmp(msg_rec, "TXT", 3)) {
				printf("%s\n", msg_rec + 4);
			}
			else if (strncmp(msg_rec, "MYM", 3)) {
				printf("it is your turn ! \n");
				allow_write = 1;
			}
			else if (strncmp(msg_rec, "END", 3)) {
				int winner = atol(msg_rec + 3);
				if (!winner) {
					printf("it is a draw ! \n", winner);
				}
				else {
					printf("player %d won ! \n", winner);
				}
				return 0;
			}
		}

		if (getline(&line, &len, stdin) == -1) {
			fprintf(stderr, "error reading the command /n");
		}

		if ((strlen(line) < 7) || (strncmp(line, "MOV", 3) != 0) || (strncmp(line + 3, " ", 1) != 0) || (strncmp(line + 5, ",", 1) != 0) || (strncmp(line + 7, "\0", 1) != 0)) {
			fprintf(stderr, "incorrect command /n");
		}

		char raw = line[4];
		char col = line[6];

		if ((atol(&raw) < 1) || (atol(&col) < 1) || (atol(&raw) > 3) || (atol(&col) > 3)) { 
			fprintf(stderr, "incorrect row or column /n");
		}

		sendto(sockfd, line, len, 0, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
	}

	return 0;
}

// #define MAX_LEN_MSG 2048

// pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// pthread_t thread_received;
// int thread_activated = 1;

// int main(int argc, char *argv[]) {

// 	if (argc < 3) {
// 		fprintf(stderr, "Missing arguments.\n");
// 		return 1;
//     }
// 	if (argc > 3) {
// 		fprintf(stderr, "Too many arguments give.\n");
// 		return 1;
// 	}

// 	char *address = argv[1]; // Get address
// 	long port = atol(argv[2]); // Get port number
// 	struct sockaddr_in sockaddr;

// 	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
// 	if (sockfd == -1) {
// 		fprintf(stderr, "Error while creating socket: %s\n", strerror(errno));
// 		return 1;
// 	}

// 	puts("Socket created\n");

// 	memset(&sockaddr, 0, sizeof sockaddr);
// 	sockaddr.sin_family = AF_INET;
// 	sockaddr.sin_port = htons(port);
// 	inet_pton(AF_INET, address, &(sockaddr.sin_addr));

// 	// Connect to server

// 	char msg[] = "CON";

// 	int bytes_sent = sendto(sockfd, &msg, 3, 0, &sockaddr, sizeof sockaddr);
// 	if (bytes_sent == -1) {
// 		fprintf(stderr, "Error while connecting (sending first message): %s\n", strerror(errno));
// 	}

// 	char msg[MAX_LEN_MSG];
// 	int bytes_received = recvfrom(sockfd, (char *)msg, MAX_LEN_MSG, 0, &sockaddr, sizeof sockaddr);
// 	if (bytes_received == -1) {
// 		fprintf(stderr, "Error while connecting (receiving message): %s\n", strerror(errno));
// 	}

// 	if (strncmp(msg, "CON", 3 * sizeof(char)) != 0) {
// 		printf("Connection failed.\n");
// 		return 1;
// 	}

// 	printf("%s\n", *msg + 3);

// 	// We are connected

// }