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
#define CON 0x06

struct sockaddr_in server_addr;
socklen_t server_addr_len;
int port;
int allow_write = 0;

void *decrypt_fyi(void *msg);
int connect_server(int sockfd, struct sockaddr_in server_addr);
int main(int argc, char *argv[])
{

	if (argc < 3)
	{
		fprintf(stderr, "Missing arguments.\n");
		return 1;
	}

	if (argc > 3)
	{
		fprintf(stderr, "Too many arguments given.\n");
		return 1;
	}



	char *address = argv[1]; // Get address
	port = atoi(argv[2]);	 // Get port number

	if (port == 0)
	{
		fprintf(stderr, "Incorrect port: %s\n", strerror(errno));
		return 1;
	}

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Create socket
	if (sockfd == -1)
	{
		fprintf(stderr, "Could not create socket: %s\n", strerror(errno));
		return 1;
	}

	memset(&server_addr, 0, sizeof server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr_len = sizeof(server_addr);
	if (inet_pton(AF_INET, address, &(server_addr.sin_addr)) == 0)
	{
		fprintf(stderr, "Invalid address: %s\n", strerror(errno));
		return 1;
	}

	int connection = connect_server(sockfd, server_addr);

	if (!connection)
	{
		printf("Error connecting.\n");
		return 1;
	}

	// We are connected

	while (1)
	{

		char *line = NULL;
		size_t len = 0;

		if (!allow_write)
		{
			// if (getline(&line, &len, stdin)>0){printf("not your turn !/n");}

			char msg_rec[MAX_LEN_MSG];
			int bytes_received = 0;

			bytes_received = recvfrom(sockfd, msg_rec, MAX_LEN_MSG, 0, (struct sockaddr *)&server_addr, &server_addr_len);
			printf("Type: %d\n", msg_rec[0]);
			if (bytes_received == -1)
			{
				fprintf(stderr, "Error while receiving message: %s\n", strerror(errno));
			}
			else if (msg_rec[0] == FYI)
			{
				// printf(decrypt_fyi(msg_rec));
				// printf()
				continue;
			}
			else if (msg_rec[0] == TXT)
			{
				printf("%s\n", msg_rec + 1);
			}
			else if (msg_rec[0] == MYM)
			{
				printf("it is your turn ! \n");
				allow_write = 1;
			}
			else if (msg_rec[0] == END)
			{
				unsigned int winner = (unsigned int)msg_rec[1];
				if (!winner)
				{
					printf("it is a draw ! \n");
				}
				else
				{
					printf("player %d won ! \n", winner);
				}
				return 0;
			}
		}

		if (getline(&line, &len, stdin) == -1)
		{
			fprintf(stderr, "error reading the command /n");
		}

		if ((strlen(line) < 7) || (strncmp(line, "MOV", 3) != 0) || (strncmp(line + 3, " ", 1) != 0) || (strncmp(line + 5, ",", 1) != 0) || (strncmp(line + 7, "\0", 1) != 0))
		{
			fprintf(stderr, "incorrect command /n");
		}

		char raw = line[4];
		char col = line[6];

		if ((atol(&raw) < 1) || (atol(&col) < 1) || (atol(&raw) > 3) || (atol(&col) > 3))
		{
			fprintf(stderr, "incorrect row or column /n");
		}
		printf(line);

		//sendto(sockfd, line, len, 0, (struct server_addr *)&server_addr, sizeof(server_addr));
	}

	return 0;
}

/*
 * Connect to server given initiated sockfd and server_addr.
 * Return 1 on success otherwise 0
 */
int connect_server(int sockfd, struct sockaddr_in server_addr)
{

	char *connection_msg = malloc(sizeof(char));
	memset(connection_msg, CON, sizeof(char));
	socklen_t addrlen = sizeof server_addr;

	int bytes_sent = sendto(sockfd, (char *)connection_msg, sizeof(char), 0, (struct server_addr *)&server_addr, addrlen);
	if (bytes_sent == -1)
	{
		fprintf(stderr, "Error while sending connection message: %s\n", strerror(errno));
		return 0;
	}

	free(connection_msg);

	char *response = malloc(MAX_LEN_MSG * sizeof(char));
	memset(response, 0, MAX_LEN_MSG * sizeof(char));
	int bytes_received = recvfrom(sockfd, response, MAX_LEN_MSG, 0, (struct server_addr *)&server_addr, &addrlen);
	if (bytes_received == -1)
	{
		fprintf(stderr, "Error while receiving connection message: %s\n", strerror(errno));
		return 0;
	}

	if (response[0] == CON)
	{
		free(response);
		printf("Connected to server.\n");
	}
	else
	{
		printf("Error connecting: Wrong messaged received.\n");
		return 0;
	}

	return 1;
}
