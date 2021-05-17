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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_LEN_MSG 2000


int allow_write = 0;
int main (int argc, char *argv []) {
  

  int mysocket;
  struct sockaddr_in dest;
  int port;


  if (argc<3){
    fprintf(stderr,"missing arguments\n");
    return 1 ;
  }


  port = atol(argv[2]);
  if (port==0)fprintf(stderr,"incorrect port\n"); 


  mysocket = socket(AF_INET, SOCK_DGRAM, 0);
  if (mysocket==-1){
    fprintf(stderr,"Could not create socket\n");
  }

  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  if ( inet_pton(AF_INET,argv[1],&(dest.sin_addr))==0)fprintf(stderr,"Invalid address\n");
  dest.sin_port = htons(port);


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

  if (strncmp(msg, "CON", 3 * sizeof(char)) != 0) {
    printf("Connection failed.\n");
    return 1;
  }

  printf("%s\n", *msg + 3);

  // We are connected


  while(1){

    char *line = NULL;
    size_t len = 0;
    

    if (!allow_write){
      // if (getline(&line, &len, stdin)>0){printf("not your turn !/n");}
      
      char msg[MAX_LEN_MSG];
      int bytes_received = 0;

      bytes_received = recvfrom(sockfd, (char *)msg, MAX_LEN_MSG, 0, &sockaddr, sizeof(sockaddr));
      if (bytes_received == -1) {
	fprintf(stderr, "Error while receiving message: %s\n", strerror(errno));
      }

      if strncmp(msg,"FYI",3){
	  printf(decrypt_fyi(msg));
	}
      if strncmp(msg,"TXT",3){
	  printf(msg+4);
        }
      if strncmp(msg,"MYM",3){
	  printf("it is your turn ! \n");
	  allow_write = 1;
        }
      if strncmp(msg,"END",3){
	  int winner = atol(msg+3);
	  if (!winner){
	    printf("it is a draw ! \n");
	  }
	  else{
	    printf("player %d won ! \n");
	  }
	  return 0;
        }
    }


    if (getline(&line, &len, stdin)==-1){
      fprintf(stderr,"error reading the command /n");
    }

    if (  (strlen(line)<7)       ||    (strcmp(line, "MOV",3)!=0)   ||    (strcmp(line+3, " ",1)!=0)    ||    (strcmp(line+5, ",",1)!=0)    ||    (strcmp(line+7, "\0",1)!=0)   ){
      fprintf(stderr,"incorrect command /n");
    }

    char raw = line[4];
    char col = line[6];
   

    if  (atol(&raw)<1 || atol(&col)<1  || atol(&raw)>3     ||      atol(&col)>3  {
    fprintf(stderr,"incorrect row or column /n");
  }
  

    sendto(mysocket, line, len, 0,(struct sockaddr*) &dest, sizeof(dest)); 

  }

  return 0;

}











// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <netdb.h>
// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <errno.h>
// #include <arpa/inet.h>
// #include <pthread.h>

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
