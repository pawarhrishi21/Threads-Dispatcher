// #define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // for read(), write()
#include <string.h> // for bzero()
#include <pthread.h>
#include <json-c/json.h>
#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
struct thread_arguments{
    json_object* jsonData;
    int socketfd;
};

void error(char *msg)
{
    perror(msg);
    exit(1);
}

// Temporary function for testing
void *func(void *arg)
{

    struct thread_arguments args = *(struct thread_arguments*)arg;
    json_object *parsed_json = args.jsonData;
        /*Creating a json string*/
    json_object *jfunction;
    json_object *jvalue;
    json_object *jdll;

    json_object_object_get_ex(parsed_json,"FunctionName", &jfunction);
    json_object_object_get_ex(parsed_json,"FunctionInput", &jvalue);
    json_object_object_get_ex(parsed_json,"DLLName", &jdll);

    const char *functionname = json_object_get_string(jfunction);
    const char *value = json_object_get_string(jvalue);
    const char *dll = json_object_get_string(jdll);

    // printf("AAA %s %s %s", functionname,value,dll);
    // printf("Function executed in thread %ld for socket %d\n",pthread_self(),args.socketfd);
    int n = write(args.socketfd,"Executed the func. Thanks!",26);

    if(n<0)
        error("ERROR could not execute write() in the socket"); 
    sleep(5000);

}



int main(int argc,char *argv[])
{
    int sockfd;//socket file descriptor (for socket())
    int newsockfd; // socket file descriptor(for accept())

    int portno; // port no on which we will accept connection
    int n; // no of characters
    int clilen; // length of address of client

    int maxthreads = 5;

    char buffer[1000]; // Buffer to store char read from connection

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
    
    pthread_t threadpool[maxhthreads];
    memset(threadpool, '\0', sizeof(threadpool));
    while(1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr*) &cli_addr,&clilen); //Accepts new connection and returns new socket file descriptor thorugh which the communication will take place
        if(newsockfd<0)
            error("ERROR executing accept()");
        printf("Accepted a Connection\n");

        if (read(newsockfd, buffer, 1000) == -1)
            error("ERROR executing read() in server");
        printf("Data Received: %s\n", buffer);

        json_object *parsed_json;

        parsed_json = json_tokener_parse(buffer);
        
        struct thread_arguments args;
        
        args.jsonData = parsed_json;
        args.socketfd = newsockfd;
    
        int threadstatus = -1;
        for(int i=0;i<maxthreads;i++)
        {
            printf("i=%d ",i);
            if(!threadpool[i]) //ESRCH
            {
                printf("NO thread present\n");
                // Thread is dead or no thread was created earlier
                pthread_create(&threadpool[i],NULL,func,&args);
                
                break;
            }
            else //if(pthread_kill(threadpool[i],0) == 3)
            {
                int status = pthread_kill(threadpool[i],0);
                    printf("thread is %d",status);
             //   pthread_create(&threadpool[i],NULL,func,&args);
                
                // break;        
            }
            // else 
            // {
            //     if(i==maxthreads-1)
            //         i=0;
            // }
        }
        printf("Out of the loop\n");
        // n = write(newsockfd,"Executed the func. Thanks!",26);

        // if(n<0)
        //     error("ERROR could not execute write() in the socket"); 
    }
 
    close(sockfd);

    return 0;
}


