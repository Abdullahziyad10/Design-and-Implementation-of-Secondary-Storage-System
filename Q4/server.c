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
#include <math.h>

#define BLOCK_SIZE 128
#define MAX_FILE_NAME 16
#define filemaxblocks (BLOCK_SIZE * 8 /2)
#define numberofblocks BLOCK_SIZE * 8

//struct to hold data member needed to keep track of the disk, similar to class but class not available in C so we use struct
typedef struct disk_details disk_details; 
struct disk_details{
//all are data members of the struct
   int numberofcylinders;
   int numberofsectors;
   char * disk;
   int fd;
   size_t disksize; 
};
disk_details * diskdet;  //struct pointer which will be used in the program to handle disk
//struct to hold info of about every file stored in the file system
typedef struct fileinfo fileinfo; 
struct fileinfo{
//all are data members of the struct
    char filename[MAX_FILE_NAME];
    int fileblocks[filemaxblocks];
    size_t filesize;
};

//struct to hold info about the filesystem
typedef struct fatsystem fatsystem; 
struct fatsystem{
//all are data members of the struct
    struct fileinfo files[filemaxblocks];
    unsigned char bitmap[numberofblocks];
};

fatsystem * fatsys;  //struct pointer which will be used in the program to handle filesystem
int numberoffilesinfat = 0;

//function which will be called when a file is need to be created
int createfile(char * filename){
	int flag = 1;
        for(int i = 0; i< numberoffilesinfat; i++){
	   if(strcmp(fatsys->files[i].filename, filename) == 0){ //comparing the filename to see if it already exists
                flag = 0;
                return 1;
           }
	}
        if(flag == 1){
        //since we have struct fileinfo array in the file system struct so to access the members of fileinfo of each file we use the
        //format fatsys->files[numberoffilesinfat].filesize, similarly each member of file info can be access which is located in the
        //file system struct  
 
                strcpy(fatsys->files[numberoffilesinfat].filename, filename); //copying the filename into fileinfo
                fatsys->files[numberoffilesinfat].filesize = 0; 
                fatsys->files[numberoffilesinfat].fileblocks[0] = -1;
                numberoffilesinfat += 1;
                return 0;
        }
        return 2;
}
//function to mark the block free in the bitmap of blocks
void makeblockfree(int blocknum){
        int byte = blocknum /8; //finding the byte
        int bit = blocknum%8; //finding the bit in that byte
        unsigned char m = ~(1 << bit); // assign 0 to value m
        fatsys->bitmap[byte] &= m; //taking AND of m with previous bit to mark it free
}

//function when a file need to be deleted from the file system
int deletefile(char * filename){
	int flag = 1;
	for(int i = 0; i< numberoffilesinfat; i++){ //iterating over all the files in the file system
	   if(strcmp(fatsys->files[i].filename, filename) == 0){ //checking if the file exists in the system
                fatsys->files[i].filename[0] = '\0'; //assigning null to the name of the file
                size_t size = fatsys->files[i].filesize; //assiging size with size of the file
                for(int j = 0; j < floor(size/BLOCK_SIZE); j++){ //finding all te data blocks where the file was stored
                   int blocknum = fatsys->files->fileblocks[j]; 
                   makeblockfree(blocknum); //freeing all the blocks occupied by the deleted file
                }
                fatsys->files[i].filesize = 0; //set file size to 0
                flag = 0; //file is deleted mark, no need to iterate more
                numberoffilesinfat -= 1; //decrease the number of files in filesystem
                return 0;
           }
	}
        if(flag == 1){
                return 1;
        }
        return 2;
}

void directorylisting(int booldir, char * dirdata){
        for(int i = 0; i< numberoffilesinfat; i++){ //iterating over all the files in the file system
           if(booldir == 0){ //if the user parameter is 0 then
           int len = strlen(dirdata); //calculating the length of the data to be sent;
           strcpy(dirdata + len, fatsys->files[i].filename); //copying the data of each file in the dirdata char array using strcpy which
           //is builtin
	 }//similar here
         else if(booldir == 1){
           int len = strlen(dirdata);
           strcpy(dirdata + len, fatsys->files[i].filename);
           len = strlen(dirdata);
           strcpy(dirdata + len, (char *)fatsys->files->filesize);
         }
        }
}

//function to read a file in the file system
int readfile(char * filename, char * readdata){
        int blocktoread = 1;
        for(int i = 0; i< numberoffilesinfat; i++){ //iterating through all the files in the file system
          if(strcmp(fatsys->files[i].filename, filename) == 0){ //if file exists in the file system
           for(int j = 0; j < blocktoread; j++){ //finding all the blocks occupied by the file
                int blocknum = fatsys->files[i].fileblocks[j]; 
                size_t offset = (blocknum * BLOCK_SIZE) - BLOCK_SIZE; //calculating the offset to read from file depending on the blocknum
                int len = strlen(readdata); // calculating the length of the data already read
                memcpy(readdata + len, diskdet->disk + offset, BLOCK_SIZE); //copying data from each data block occupied by the file
           }
           return 0;
         }
        }
        return 1;
}

//function to get free block number using bitmap
int getfreeblocknum(){
        for(int i = 0; i < BLOCK_SIZE; i++){
           if(fatsys->bitmap[i] != 0xFF){ //checking if the current byte or index of char array is full or not
             for(int j = 0; j < 8; j++){ //if not full then iterate over that byte 
                int m = 1 << j; //assiging 1 to m on the bit number depending on value of j
                if(j & ~fatsys->bitmap[i]){ //if block / bit is free
                    fatsys->bitmap[i] |= m; //mark that block as occupied by doing OR operation
                    return (i * 8) + j;    //return the blockmnumber which is found
                }
             }
           }
        }
}

//function to write on a file in the file system
int writefile(char * filename, char * writedata, size_t datasize){
        int blocksreq = 1;
        int flag = 1;
        size_t lenwritten = 0;
        for(int i = 0; i< numberoffilesinfat; i++){ //iterating over all the files in the filesystem
          if(strcmp(fatsys->files[i].filename, filename) == 0){ //if the file exists
                for(int k = 0; k < blocksreq; k++){ //iterate times the blocks required by the data
                   if(flag == 1){ //if writing on the first block
                    int blocknum = getfreeblocknum(); //get the free block number 
                    fatsys->files[i].fileblocks[k] = blocknum; //include the block number in the block occupied by the file 
                    fatsys->files[i].fileblocks[k + 1] = -1; //ending the block number array with -1
                    fatsys->files[i].filesize = datasize; //assigning file size to the new data size
                    lenwritten = BLOCK_SIZE;
                    size_t offset = (blocknum * BLOCK_SIZE) - BLOCK_SIZE; //calculating offset to write on a disk
                    memcpy(diskdet->disk + offset, writedata, BLOCK_SIZE); //copying data on the disk using mmap
                   }
                   if(flag == 0){
                    int blocknum = getfreeblocknum(); //get the free block number 
                    fatsys->files[i].fileblocks[k] = blocknum; //include the block number in the block occupied by the file
                    fatsys->files[i].fileblocks[k + 1] = -1; //ending the block number array with -1
                    size_t offset = (blocknum * BLOCK_SIZE) - BLOCK_SIZE; //calculating offset to write on a disk
                    memcpy(diskdet->disk + offset, writedata + lenwritten, BLOCK_SIZE);//copying data on the disk using mmap
                    lenwritten += BLOCK_SIZE;
                   }
                   flag = 0; // marking that the first block has been wrote
                }
            return 0;
           }
         
        }
        return 1;
}


int count = 0;
//thread function which is called whenever a client gets connected to the server and most of the message communication is done in it
void* receive_Client(void * cli_sock){
 int client_sock = *(int *)cli_sock; 
 char * sign_out_msg = "DISCONNECT";
 //Receive the message from the client
 int loop = 1;
  while(loop == 1){
        char server_message[2000], client_message[2000];
        memset(server_message,'\0',sizeof(server_message)); //setting character array to null character
        memset(client_message,'\0',sizeof(client_message)); //setting character array to null character     
        if (recv(client_sock, client_message, sizeof(client_message),0) < 0)//receving the message from the client using the builtin 
        //function available in sys/socket.h
        {
                printf("Receive Failed. Error!!!!!\n");
                return -1;
        }
        
        printf("Client Message: %s\n",client_message);
        if(strcmp(client_message,sign_out_msg) == 0){ //checking if client has asked to disconnect from the server
        	count -= 1; //decreasing the number of client connected to server so that a space for new one can be created
         	loop = 0; //setting loop to 0 so that while breaks
       }
       //when client user wants to create a file on the disk
       if(client_message[0] == 'C'){
         char filename[MAX_FILE_NAME];
         strcpy(filename, client_message + 2);
       	 int ret = createfile(filename);//calling the function
         //assiging the successful or unsuccessful code to the message to be sent to the client
         if(ret == 0){
           server_message[0] = '0';
         }
         else if(ret == 1){
          server_message[0] = '1';
         }
        }
	//when client user wants to delete a file on the disk
        if(client_message[0] == 'D'){
         char filename[MAX_FILE_NAME];
         strcpy(filename, client_message + 2);
       	 int ret = deletefile(filename); //calling the function
        //assiging the successful or unsuccessful code to the message to be sent to the client
         if(ret == 0){
           server_message[0] = '0';
         }
         else if(ret == 1){
          server_message[0] = '1';
         }
        }
	//when client user wants to List the directories on the disk
        if(client_message[0] == 'L'){
         int booldir = client_message[2] - '0'; //converting char to int by doing -'0'
         char * dirdata = (char *)malloc(numberoffilesinfat * 32); //dynamically allocating the memory of char * dirdata
         directorylisting(booldir, dirdata); //calling the function created above
         strcpy(server_message, dirdata); //copying the data to message to be sent to to client
        }
	//when client user wants to Read from a file on the disk
        if(client_message[0] == 'R'){
         char filename[MAX_FILE_NAME];
         strcpy(filename, client_message + 2); //copying file name from the client message
         char * data = (char *)malloc(numberoffilesinfat * 2024); //dynamically allocating the memory of char * data
         readfile(filename, data); //calling the function
         strcpy(server_message, data); //copying the data to the message to be sent to client
        }
	//when client user wants to Write on a file on the disk
        if(client_message[0] == 'W'){
         char filename[MAX_FILE_NAME];
         int j = 2, k = 0;
         //iterating over client_message until encounter a space, that will be the filename
         while(client_message[j] != ' '){
           filename[k] = client_message[j];
           filename[k + 1] = '\0';
           k += 1;
           j += 1;
         }
         j+=1;
         //iterating to the start of the data
         while(client_message[j] != ' '){
           k += 1;
           j += 1;
         }
         j+=1;
         k = 0;
         char data[strlen(client_message + j) + 1];
         //iterating over the client_message to get all the data needed to be written on the file
         while(client_message[j] != '\0'){
           data[k] = client_message[j];
           data[k + 1] = '\0';
           k += 1;
           j += 1;
         }
         size_t size = strlen(data);
         int ret = writefile(filename, data, size); //calling the function
         //returning the success/unsuccess codes
         if(ret == 0){
            server_message[0] = '0';
         }
         else if(ret == 1){
           server_message[0] = '1';
         }
        }
	//when we wanto to format the file system
        if(client_message[0] == 'F'){
         char * msg = "System Formated!";
         strcpy(server_message, msg);
        }

        //Send the message back to client
        
        if (send(client_sock, server_message, strlen(server_message),0)<0)//sending message to the client, 
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
//to handle the errors if they occur
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

//function to initialize the disk by the paramters provided on commandline by the user
disk_details * initdiskstorage(char * filename, int numberofcylinders, int numberofsectors){
	
	disk_details * dd = malloc(sizeof(disk_details)); //initializing a struct of disk which will store info of disk
	dd->disksize = numberofcylinders * numberofsectors * BLOCK_SIZE; //calculating the disk size
	dd->fd = open(filename, O_CREAT | O_RDWR, 0666);//opening the file with some flags and permission to read and written using 0666
	dd->numberofcylinders = numberofcylinders;//putting number of cylinder in the disk struct
	dd->numberofsectors = numberofsectors;
	if(dd->fd < 0){//if file not open
		printf("File Not Open\n");
	}
	ftruncate(dd->fd, dd->disksize);//increasing the size of the file according to the size of the disk
	dd->disk = mmap(NULL, dd->disksize, PROT_READ | PROT_WRITE, MAP_SHARED, dd->fd, 0);//mapping the disk on the file 
	//which return pointer to that mapped memory

	if(dd->disk == MAP_FAILED){  //to handle any error given by mmap
  		handle_error("mmap");
        	return NULL;
  	}
  	memset(dd->disk, '\0', dd->disksize); //setting character array to null character
  	return dd;  //return the struct of the disk

}

int main(int argc, char *argv[])
{
	int numberofcylinders = atoi(argv[1]); //converting the first commandline argument to integer using atoi builtin function
	int numberofsectors = atoi(argv[2]); //converting the second commandline argument to integer using atoi builtin function
	char filename[sizeof(argv[3]) + 1];//declaring an array to store file name, size will be size of commandline 3rd argument + 1
	strcpy(filename, argv[3]); //copying the filename from third commandline argument to filename variable
	diskdet = initdiskstorage(filename, numberofcylinders, numberofsectors); //creating a diskstorage using the function
        fatsys = malloc(sizeof(fatsystem)); //initializing the file system
        int socket_desc, client_sock, client_size; 
        struct sockaddr_in server_addr, client_addr;         //SERVER ADDR will have all the server address
	pthread_t threads[3]; //accomodating 3 clients at a time so initializing thread for each
        
        //Creating Socket
        socket_desc = socket(AF_INET, SOCK_STREAM, 0); //creating a socket for communication between server and client
        
        if(socket_desc < 0) //if socket not created
        {
                printf("Could Not Create Socket. Error!!!!!\n");
                return -1;
        }
        
        printf("Socket Created\n");
        
        //Binding IP and Port to socket
        
        server_addr.sin_family = AF_INET;               /* Address family = Internet */
        server_addr.sin_port = htons(2015);               // Set port number, using htons function to use proper byte order */
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
		
		
		
        client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);   // heree creating socket for particular client
        
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
        { //if less than 3 clients are connected to the server
        count++; //increasing the clients connected count
        printf("Client Connected with IP: %s and Port No: %i\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		       //inet_ntoa() function converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation
	
	
	void * cli_sock = &client_sock; //casting the int value of client socket to the void * type to send it to the thread function
        
        int ret1 = pthread_create(&threads[i],NULL,receive_Client,(void *)cli_sock); //creating a thread for each client connected
        if(ret1!=0) //checking if the thread is created or not
        {
                printf("Thread 1 Creation Failed\n");
        }
        i++;   //increasing the thread count of threads created   
 }           
     }
        //Closing the Socket
        close(socket_desc);
        int n = 0;
        for (n = 0; n<3;n++){
        pthread_join(threads[n],NULL); //joining all the created threads
        }
        
        return 0;       
}
