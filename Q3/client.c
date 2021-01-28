/*
        TCP_Client. This Program will implement the Client Side for TCP_Socket Programming.
        It will get some data from user and will send to the server and as a reply from the
        server, it will get its data back.
*/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr

int main(void)
{
        int socket_desc;
        struct sockaddr_in server_addr;
        char server_message[2000], client_message[2000];
        
        //Cleaning the Buffers
        
        memset(server_message,'\0',sizeof(server_message));//setting character array to null character
        memset(client_message,'\0',sizeof(client_message));//setting character array to null character
        
        //Creating Socket
        
        socket_desc = socket(AF_INET, SOCK_STREAM, 0); //creating a socket for communication between server and client
        
        if(socket_desc < 0) //check if the socket is created or not.
        {
                printf("Could Not Create Socket. Error!!!!!\n");
                return -1;
        }
        
        printf("Socket Created\n");
        
        //Specifying the IP and Port of the server to connect
        
        server_addr.sin_family = AF_INET; /* Address family = Internet */
        server_addr.sin_port = htons(2058); // Set port number, using htons function to use proper byte order */
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Set IP address to localhost */
        
        //Now connecting to the server accept() using connect() from client side
        
        if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)//connecting to the socket of the server using 
        //builtin funtion availabe in sys/socket.h
        {
                printf("Connection Failed. Error!!!!!\n");
                return -1;
        }
        int loop = 1;
        printf("Connected\n");
        char * sign_out_msg = "DISCONNECT";
        char * msg_full = "Server Full"; 
        //Get Input from the User
        while(loop == 1){
        printf("Enter Message: ");
        gets(client_message);          //to get input from user of the client , One is that gets() will only get character string data. 
                                                     //		will get only one variable at a time.
	 if(strcmp(client_message,sign_out_msg) == 0){
         	loop = 0;
       }													//  reads characters from stdin and loads them into str
        //Send the message to Server
        
        if(send(socket_desc, client_message, strlen(client_message),0) < 0)//sending message to the client, send is a builtin function 
        //sys/socket.h
        {
                printf("Send Failed. Error!!!!\n");
                return -1;
        }
        
        //Receive the message back from the server
        
        if(recv(socket_desc, server_message, sizeof(server_message),0) < 0)//receving the message from the client using the builtin 
        //function available in sys/socket.h
        {
                printf("Receive Failed. Error!!!!!\n");
                return -1;
        }
        if(strcmp(server_message,msg_full) == 0){ //checking the server sent a server full message, if yes then end the loop
         	loop = 0;
       }
        printf("Server Message: %s\n",server_message);
        
        memset(server_message,'\0',sizeof(server_message)); //setting character array to null character
        memset(client_message,'\0',sizeof(client_message)); //setting character array to null character
 }       
        //Closing the Socket
        
        close(socket_desc);
        
        return 0;
}
