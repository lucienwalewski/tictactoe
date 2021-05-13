/* 
 * Tictactoe server.c
 * 
 * Lucien Walewski and Alban Puech
 */


// structure to store the arguments to give to the function while using pthread_create 
// https://stackoverflow.com/questions/16230542/passing-multiple-arguments-to-threaded-function-from-pthread-create
struct thread_arg {                                                   
  char buffer[MAX_LEN_MSG]; 
  struct sockaddr from;                                                             
}; 

struct clientinfos {
    char client1[8];
    char client2[8];
    
};


memset(&(clientinfos.client1),0,sizeof(clientinfos.client1));
memset(&(clientinfos.client2),0,sizeof(clientinfos.client2));




int grid[9];
memset(&grid,0,sizeof(grid));

int mysocket;
pthread_mutex_t lock;
struct clientinfos clientinfo;



