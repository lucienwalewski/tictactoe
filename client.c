/* 
 * Tictactoe client.h
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

#define MAX_LEN_MSG 2048

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_received;
int thread_activated = 1;


int main(int argc, char *argv[]) {

	if (argc < 3) {
		fprintf(stderr, "Missing arguments.\n");
		return 1;
    }
	if (argc > 3) {
		fprintf(stderr, "Too many arguments give.\n");
		return 1;
	}

	char *address = argv[1]; // Get address
	long port = atol(argv[2]); // Get port number
	struct sockaddr_in sockaddr;

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		fprintf(stderr, "Error while creating socket: %s\n", strerror(errno));
		return 1;
	}

	puts("Socket created\n");

	memset(&sockaddr, 0, sizeof sockaddr);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	inet_pton(AF_INET, address, &(sockaddr.sin_addr));

	// Connect to server

	char msg[] = "CON";

	int bytes_sent = sendto(sockfd, &msg, 3, 0, &sockaddr, sizeof sockaddr);
	if (bytes_sent == -1) {
		fprintf(stderr, "Error while connecting (sending first message): %s\n", strerror(errno));
	}

	char msg[MAX_LEN_MSG];
	int bytes_received = recvfrom(sockfd, (char *)msg, MAX_LEN_MSG, 0, &sockaddr, sizeof sockaddr);
	if (bytes_received == -1) {
		fprintf(stderr, "Error while connecting (receiving message): %s\n", strerror(errno));
	}

	if (strncpy(msg, "CON", 3 * sizeof(char)) != 0) {
		printf("Connection failed.\n");
		return 1;
	}

	printf("%s\n", *msg + 3);

	// We are connected










}