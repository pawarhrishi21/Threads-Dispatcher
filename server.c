#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

void error(char *msg)
{
    printf("ERROR: %s\n",msg);
    perror(msg);
    exit(1);
}

int main(int argc,char *argv[])
{
    int sockfd;//socket file descriptor (for socket())
    int newsockfd; // socket file descriptor(for accept())

    int portno; // port no on which we will accept connection
    int n; // no of characters
    int clilen; // length of address of client

    char buffer[256]; // Buffer to store char read from connection

    struct sockaddr_in serv_addr; // for Server internet address (defined in netinet/in.h library){sin_family,sin_port,(struct)sin_addr->s_addr,sin_zero}
    struct sockaddr_in cli_addr; // for Client internet address


    if(argc<2)
    {
        fprintf(stderr,"ERROR: No port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // socket(Address domain{AF_INET for Internet},Type of Socket{SOCK_STREAM or SOCK_DGRAM},Protocol(0 chooses appropriate protocol{TCP or UDP})

    if(sockfd < 0) // -1 if error occurs in above function
    {
        error("ERROR executing socket()");
    }

    bzero((char*)&serv_addr,sizeof(serv_addr)); // sets buffer to zero (pointer to the buffer, size of the buffer)

    portno = atoi(argv[1]); // string to int

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno); // host byte order to network byte order (converts little-endian to big-endian)
    serv_addr.sin_addr.s_addr = INADDR_ANY; // IP Adress of the host
    // serv_addr.zero is already set to zero

    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) // binding the socket to the server address 
        error("ERROR executing bind()");

    listen(sockfd,5); // Listen for connections (Second argument denotes maximum no of allowed waiting connections in queue for the socket)

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,(struct sockaddr*) &cli_addr,&clilen); //Accepts new connection and returns new socket file descriptor thorugh which the communication will take place

    if(newsockfd<0)
        error("ERROR executing accept()");

    bzero(buffer,256);

    n = read(newsockfd,buffer,255); //Reads from client write() into buffer, returns no of characters read
    if(n<0)
        error("ERROR could not read() in the socket");
    printf("Message Recieved: %s\n", buffer);

    n = write(newsockfd,"Recieved the message. Thanks!",29); // Writing to client
     if(n<0)
        error("ERROR could not write() from the socket");   


    return 0;

}


