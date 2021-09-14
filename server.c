#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // for read(), write()
#include <string.h> // for bzero()
#include <pthread.h>


struct message{
    int a;
    char b;
    char *c;
};

void error(char *msg)
{
    printf("ERROR: %s\n",msg);
    perror(msg);
    exit(1);
}
void *func(void *tid)
{
    printf("Function executed in thread %ld\n", *(pthread_t*)tid);
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
    printf("Listening for Connections\n");
    // while(1)
    // {
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,(struct sockaddr*) &cli_addr,&clilen); //Accepts new connection and returns new socket file descriptor thorugh which the communication will take place
    printf("Accepted a Connection\n");

    if(newsockfd<0)
        error("ERROR executing accept()");

    bzero(buffer,256);
    struct message read_message;

    n = read(newsockfd,&read_message,255); //Reads from client write() into buffer, returns no of characters read
    if(n<0)
        error("ERROR could not read() in the socket");
    printf("Message Recieved: %d %c %d\n", read_message.a, read_message.b, read_message.c);
    unsigned long long loc = (unsigned long long)read_message.c;
    
    printf("%lld\n",loc);
    printf("%c\n",*(char*)loc);

    n = write(newsockfd,"Executed the func. Thanks!",26);
    if(n<0)
        error("ERROR could not write() from the socket"); 
 
//    }


    return 0;
}


