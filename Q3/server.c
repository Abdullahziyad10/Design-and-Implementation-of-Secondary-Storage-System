#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>


#define BLOCK_SIZE 128
//struct to hold information about the disk
typedef struct disk_details disk_details; 
struct disk_details{
   int numberofcylinders;
   int numberofsectors;
   char * disk;
   int fd;
   size_t disksize; 
};

int count = 0;
disk_details * diskdet;
void* receive_Client(void * cli_sock){

 int client_sock = *(int *)cli_sock; 
 char * sign_out_msg = "DISCONNECT";
 //Receive the message from the client
 int loop = 1;
  while(loop == 1){
        char server_message[2000], client_message[2000];
        memset(server_message,'\0',sizeof(server_message));
        memset(client_message,'\0',sizeof(client_message));     
        if (recv(client_sock, client_message, sizeof(client_message),0) < 0)
        {
                printf("Receive Failed. Error!!!!!\n");
                return -1;
        }
        
        printf("Client Message: %s\n",client_message);
        if(strcmp(client_message,sign_out_msg) == 0){  //checking if the client requested to disconnect from the server
        	count -= 1;
         	loop = 0;
       }
       //checking if the user requested info of disk
       if(client_message[0] == 'I'){
       	char sizes[2];
       	sizes[0] = diskdet->numberofcylinders + '0';
       	sizes[1] = diskdet->numberofsectors + '0';
       	strcpy(server_message, sizes);
        }
		//checking if the client requested to read from disk
        if(client_message[0] == 'R'){
        
       	int readcylinder = client_message[2] - '0';
       	int readsector  = client_message[4] - '0';
       	if(readcylinder <= diskdet->numberofcylinders && readsector <= diskdet->numberofsectors){
       		off_t readoffset = (readcylinder * readsector * BLOCK_SIZE) - BLOCK_SIZE;
       		server_message[0] = '1';
       		char readdata[BLOCK_SIZE];
       		memcpy(server_message, diskdet->disk + readoffset, BLOCK_SIZE);
       	}
       	else{
       		char * cylsecerror = "0 - Cylinder or Sector Number Ivalid";
       		strcpy(server_message, cylsecerror);
       	}
        }
		//checking if the client requested to write on disk
        if(client_message[0] == 'W'){
       	int writecylinder = client_message[2] - '0';
       	int writesector  = client_message[4] - '0';
       	ssize_t writelen = client_message[6] - '0';
       	char data[strlen(client_message + 9) + 1];
       	strcpy(data, client_message + 9);
       	size_t len = strlen(data) + 1;
       	data[len] = '\0';
       	if(writecylinder <= diskdet->numberofcylinders && writesector <= diskdet->numberofsectors){
       		off_t writeoffset = (writecylinder * writesector * BLOCK_SIZE) - BLOCK_SIZE;
       		server_message[0] = '1';
       		memcpy(diskdet->disk + writeoffset, data, len);
       	}
       	else{
       		char * cylsecerror = "0 - Cylinder or Sector Number Ivalid";
       		strcpy(server_message, cylsecerror);
       	}
        }
        //Send the message back to client
        
        if (send(client_sock, server_message, strlen(server_message),0)<0)
       {
                printf("Send Failed. Error!!!!!\n");
                return -1;
       }
        	 
        memset(server_message,'\0',sizeof(server_message));
        memset(client_message,'\0',sizeof(client_message));
 }
 close(client_sock);
}

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

//function to inialize the strcut with the information passed as commadline parameters and mapping the file using mmap
disk_details * initdiskstorage(char * filename, int numberofcylinders, int numberofsectors){
	
	disk_details * dd = malloc(sizeof(disk_details));
	dd->disksize = numberofcylinders * numberofsectors * BLOCK_SIZE;
	dd->fd = open(filename, O_CREAT | O_RDWR, 0666);
	dd->numberofcylinders = numberofcylinders;
	dd->numberofsectors = numberofsectors;
	if(dd->fd < 0){
		printf("File Not Open\n");
	}
	ftruncate(dd->fd, dd->disksize);
	dd->disk = mmap(NULL, dd->disksize, PROT_READ | PROT_WRITE, MAP_SHARED, dd->fd, 0);

	if(dd->disk == MAP_FAILED){  //to handle any error given by mmap
  		handle_error("mmap");
        	return NULL;
  	}
  	memset(dd->disk, '\0', dd->disksize);
  	return dd;

}

int main(int argc, char *argv[])
{
	int numberofcylinders = atoi(argv[1]);
	int numberofsectors = atoi(argv[2]);
	char filename[sizeof(argv[3]) + 1];
	strcpy(filename, argv[3]);
	diskdet = initdiskstorage(filename, numberofcylinders, numberofsectors);
        int socket_desc, client_sock, client_size; 
        struct sockaddr_in server_addr, client_addr;         //SERVER ADDR will have all the server address
	pthread_t threads[3];
        //Cleaning the Buffers
        
        // Set all bits of the padding field//
        
        //Creating Socket
        
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);
        
        if(socket_desc < 0)
        {
                printf("Could Not Create Socket. Error!!!!!\n");
                return -1;
        }
        
        printf("Socket Created\n");
        
        //Binding IP and Port to socket
        
        server_addr.sin_family = AF_INET;               /* Address family = Internet */
        server_addr.sin_port = htons(2059);               // Set port number, using htons function to use proper byte order */
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");    /* Set IP address to localhost */
		
		
		
		// BINDING FUNCTION
        
        if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0)    // Bind the address struct to the socket.  /
	                            	//bind() passes file descriptor, the address structure,and the length of the address structure
        {
                printf("Bind Failed. Error!!!!!\n");
                return -1;
        }        
        
        printf("Bind Done\n");
        int i = 0;
        //Put the socket into Listening State
       while(1){
        if(listen(socket_desc, 1) < 0)                               //This listen() call tells the socket to listen to the incoming connections.
     // The listen() function places all incoming connection into a "backlog queue" until accept() call accepts the connection.
        {
                printf("Listening Failed. Error!!!!!\n");
                return -1;
        }
        
        printf("Listening for Incoming Connections.....\n");
        
        //Accept the incoming Connections
        
        client_size = sizeof(client_addr);
		
		
		
        client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);          // heree particular client k liye new socket create kr rhaa ha
        
        if (client_sock < 0)
        {
                printf("Accept Failed. Error!!!!!!\n");
                return -1;
        }
        
   if(count == 3){
        char * msg = "Server Full";       
     if (send(client_sock, msg, strlen(msg),0)<0)
        {
                printf("Send Failed. Error!!!!!\n");
                return -1;
        }
           
        }
     else
        {
        count++;
        printf("Client Connected with IP: %s and Port No: %i\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		       //inet_ntoa() function converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
	
	
	void * cli_sock = &client_sock;
        
        int ret1 = pthread_create(&threads[i],NULL,receive_Client,(void *)cli_sock);
        if(ret1!=0)
        {
                printf("Thread 1 Creation Failed\n");
        }
        i++;       
 }           
     }
        //Closing the Socket
        close(socket_desc);
        int n = 0;
        for (n = 0; n<3;n++){
        pthread_join(threads[n],NULL);
        }
        
        return 0;       
}
