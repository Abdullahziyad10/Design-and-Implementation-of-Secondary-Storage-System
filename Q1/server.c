/*
        TCP_Server. This Program will will create the Server side for TCP_Socket Programming.
        It will receive the data from the client and then send the same data back to client.
*/

#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr
#include <pthread.h>

int count = 0;

//function to reverse the message sent by the client
void reverse(char arr[], int low, int high){
 if(low < high){ 
  int temp = arr[low]; //puting first value in current array to temp
  arr[low] = arr[high];//replacing first value in current array to end value in current array
  arr[high] = temp; //putting the first value in current array in the previos end spot
   reverse(arr, low+1, high-1); //recursive call to the function by shriking array by 1 sopt from left and 1 spot from right 
 }
}

//thread function which is called whenever a client gets connected to the server and most of the message communication is done in it 
void* receive_Client(void * cli_sock){
 int client_sock = *(int *)cli_sock; 
 char * sign_out_msg = "DISCONNECT";
 //Receive the message from the client
 int loop = 1;
  while(loop == 1){ //while will continue to loop until the client sends DISCONNECT message
        char server_message[2000], client_message[2000];
        memset(server_message,'\0',sizeof(server_message)); //setting character array to null character
        memset(client_message,'\0',sizeof(client_message)); //setting character array to null character     
        if (recv(client_sock, client_message, sizeof(client_message),0) < 0) //receving the message from the client using the builtin 
        //function available in sys/socket.h
        {
                printf("Receive Failed. Error!!!!!\n");
                return -1;
        }
        
        printf("Client Message: %s\n",client_message); //printing the message on console
        
        if(strcmp(client_message,sign_out_msg) == 0){ //checking if client has asked to disconnect from the server
        	count -= 1; //decreasing the number of client connected to server so that a space for new one can be created
         	loop = 0; //setting loop to 0 so that while breaks
       }
        //Send the message back to client
        
        strcpy(server_message, client_message); //builtin function to copy characcter array client_message to server_message
        int len = strlen(client_message) - 1; //findig length of char array and subtracting the null from it.
        reverse(server_message, 0, len); //function called to reverse the message and sent it back to the client
        if (send(client_sock, server_message, strlen(client_message),0)<0) //sending message to the client, 
        //send is a builtin function //sys/socket.h
        {
                printf("Send Failed. Error!!!!!\n");
                return -1;
        }
        
        memset(server_message,'\0',sizeof(server_message)); //setting character array to null character
        memset(client_message,'\0',sizeof(client_message)); //setting character array to null character
 }
 close(client_sock);
}

int main(void)
{
        int socket_desc, client_sock, client_size; 
        struct sockaddr_in server_addr, client_addr;         //SERVER ADDR will have all the server address
	pthread_t threads[3]; //accomodating 3 clients at a time so initializing thread for each
        
        //Creating Socket
        
        socket_desc = socket(AF_INET, SOCK_STREAM, 0); //creating a socket for communication between server and client
        
        if(socket_desc < 0) //checking of socket is not initialized
        {
                printf("Could Not Create Socket. Error!!!!!\n");
                return -1;
        }
        
        printf("Socket Created\n");
        
        //Binding IP and Port to socket
        
        server_addr.sin_family = AF_INET;               /* Address family = Internet */
        server_addr.sin_port = htons(2002);               // Set port number, using htons function to use proper byte order */
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
		
		
		
        client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size); // heree creating socket for particular client
        
        if (client_sock < 0) //checking if client socket is created or not
        {
                printf("Accept Failed. Error!!!!!!\n");
                return -1;
        }
        
   if(count == 3){ //if already 3 clients are connected to the server
        char * msg = "Server Full";        
     if (send(client_sock, msg, strlen(msg),0)<0) //send message to the client that server is full
        {
                printf("Send Failed. Error!!!!!\n");
                return -1;
        }
           
        }
     else
        {  //if less than 3 clients are connected to the server
        count++; //increasing the clients connected count
        printf("Client Connected with IP: %s and Port No: %i\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		       //inet_ntoa() function converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
	
	
	void * cli_sock = &client_sock;  //casting the int value of client socket to the void * type to send it to the thread function
        
        int ret1 = pthread_create(&threads[i],NULL,receive_Client,(void *)cli_sock); //creating a thread for each client connected
        if(ret1!=0) //checking if the thread is created or not
        {
                printf("Thread 1 Creation Failed\n");
        }
        i++;      //increasing the thread count of threads created 
 }           
     }
        //Closing the Socket
        close(socket_desc);
        int n = 0;
        for (n = 0; n<3;n++){
        pthread_join(threads[n],NULL);  //joining all the created threads
        }
        
        return 0;       
}
