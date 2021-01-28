/*
        TCP_Server. This Program will will create the Server side for TCP_Socket Programming.
        It will receive the data from the client and then send the same data back to client.
*/

#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

int count = 0;

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
        int temp = 1;
        if(strcmp(client_message,sign_out_msg) == 0){ //checking if the client has asked to disconnect from the server
        	count -= 1;
         	loop = 0;
         	temp = 0;
       }
        //Send the message back to client
        
        int pip[2];  //using pipes to get output of the ls command after output redirections
        pid_t pid;
        if(pipe(pip) == -1){
        	printf("Pipe Not Created!!\n");
        }
        pid = fork();
        if (pid == -1){
        	printf("Not Forked!!\n");
        }
        if(pid == 0 && temp == 1){
        	dup2(pip[1], STDOUT_FILENO); //redirecting output of the ls to the pipe write end
        	execl("/bin/ls", "ls", client_message, NULL);
        }
        else if(pid > 0 && temp == 1){
        
        	wait(NULL);
        	close(pip[1]);
        	read(pip[0], server_message, sizeof(server_message));  //reading output of the ls from pipe read end.
        	close(pip[0]);
        
        	if (send(client_sock, server_message, strlen(server_message),0)<0)
       	 {
                	printf("Send Failed. Error!!!!!\n");
               	 return -1;
        	}
        	
        }
        
        memset(server_message,'\0',sizeof(server_message));
        memset(client_message,'\0',sizeof(client_message));
 }
 close(client_sock);
}

int main(void)
{
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
        server_addr.sin_port = htons(2011);               // Set port number, using htons function to use proper byte order */
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
