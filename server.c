/*
 * Tictactoe server.c
 * 
 * Lucien Walewski and Alban Puech
 */
#include <server.h>
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
#include <pthread.h>
#define NTHREADS 2
#define MAX_LEN_MSG 2000



void *recsend(void *argu){
  // function to receive and send back the messag
 struct thread_arg *args = argu;
 
 // the client is not part of our 2 players :
 if (args->from!=clientinfo.client1 && args->from!=clientinfo.client2) {
 char[] msg = "END 255";
 sendto(mysocket, msg, sizeof(msg), 0,&(args->from), sizeof(args->from))==-1);
 return NULL;
 }
 
 // it is part of our 2 players but not his turn :
 if (   (args->from==info.client1 && !info.turn)   ||  (args->from==info.client2 && info.turn)  )   {
  char[] msg = "pas ton tour";
  sendto(mysocket, msg, sizeof(msg), 0,&(args->from), sizeof(args->from))==-1);
  return NULL;
 }
  
 //it is his turn :
  
 
 char MSG[3];
 strncpy(MSG,args->buffer,3); 
 if (strcmp(MSG, "LFT") == 0) 
{
  // do something
} 
else if (strcmp(MSG, "MOV") == 0)
{
  // do something else
}
/* more else if clauses */
else /* default: */
{
}
 
  
  
 pthread_mutex_lock(&lock);      // START critical region
 if( sendto(mysocket, args->buffer, sizeof(args->buffer), 0,&(args->from), sizeof(args->from))==-1){
     fprintf(stderr,"error while sending\n");
 exit(EXIT_FAILURE);   
 }
 pthread_mutex_unlock(&lock);     // END critical region
 return NULL;

}




int main (int argc, char *argv []) {
  
struct sockaddr_in addr;
int port;
mysocket = socket(AF_INET, SOCK_DGRAM, 0);

 if (argc<2){
   fprintf(stderr,"missing arguments\n");
   return 1 ;
 }

 if (argc>2){
   fprintf(stderr,"too many arguments\n");
   return 1 ;
 }



port = atol(argv[1]);
 if (port==0){
 fprintf(stderr,"incorrect port\n");
 return 1; 
 }

if (mysocket==-1){
  fprintf(stderr,"Could not create socket\n");
  return 1; 
}

memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
inet_pton(AF_INET,"127.0.0.1",&(addr.sin_addr));
addr.sin_port = htons(port);

 if (bind(mysocket, (const struct sockaddr*) &addr, sizeof(addr))==-1){
   fprintf(stderr,"impossible to bind\n");
   return 1;
 }

// initialize the mutex
 if (pthread_mutex_init(&lock, NULL) != 0){
   printf("\n mutex init failed\n");
   return 1;
 }

 // store the threads 
 pthread_t thread[NTHREADS];
 // counter for the number of threads  
 int count = 0;
 

 while(1){
   
   // allocate space for struct of arguments to give to the function 
   struct thread_arg *args = malloc(sizeof(struct thread_arg));
   socklen_t  fromlen = (socklen_t)  sizeof(struct sockaddr);

   // receive the message and store the address of the sender
   int bytes_received =  recvfrom(mysocket, args->buffer, MAX_LEN_MSG, 0,&(args->from), &fromlen);
   

   
 
   if  (bytes_received == -1){
     printf("error while receiving the message from the client\n");
     return 1;}

   // if less than N threads, create a new thread to handle the communication with the client 
     if (count < NTHREADS){
     if(pthread_create(&thread[count-1], NULL, recsend, args)) {
       fprintf(stderr, "Error creating thread\n");
       return 1;
     }}

     // otherwise, wait for all the threads to terminate and create a new thread after that 
     else{
       int j;
       for (j=1; j<NTHREADS; j++)
	 {
	   if(pthread_join(thread[j-1],NULL )) {
	     fprintf(stderr, "Error waiting for thread\n");
	     return 1;
	   }
	 }     
      
       count = 0;
       if(pthread_create(&thread[count-1], NULL, recsend, args)) {
	 fprintf(stderr, "Error creating thread\n");
	 return 1;
       }
     }
     count+=1; // increment the counter of threads
   }
 
 return 0;
}



