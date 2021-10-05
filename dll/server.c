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
#include <signal.h> // For pthread_kill()
#include <dlfcn.h> // For DLL


struct thread_arguments{
    json_object* jsonData;
    int socketfd;
    int retval;
    int* status;
};

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void *func(void *arg)
{
    struct thread_arguments args = *(struct thread_arguments*)arg;
    json_object *parsed_json = args.jsonData;
        /*Creating a json string*/
    json_object *jfunction;
    json_object *jvalue;
    json_object *jdll;
    int *threadstatus = args.status;
    
    json_object_object_get_ex(parsed_json,"FunctionName", &jfunction);
    json_object_object_get_ex(parsed_json,"FunctionInput", &jvalue);
    json_object_object_get_ex(parsed_json,"DLLName", &jdll);

    char message[100] = "The answer is ";
    const char *functionname = json_object_get_string(jfunction);
    const char *value = json_object_get_string(jvalue);
    const char *dll = json_object_get_string(jdll);
    double val = atof(value);
    val = 1.0;
    void *handle;
    const char *math_dll = dll;
    double (*dll_func)(double);
    char *err;

    if(strcmp(dll,"math") == 0)
    {
        handle = dlopen ("/lib/x86_64-linux-gnu/libm.so.6", RTLD_LAZY);
        if (!handle)
        {
            perror("Could not open the DLL math");
        }
        dll_func = dlsym(handle,functionname);
        if ((err = dlerror()) != NULL)  {
            fputs(err, stderr);
            strcpy(message,err);
            write(args.socketfd,message,strlen(message));
            *args.status = 1;
             pthread_exit(NULL);
        }

        double answer = (*dll_func)(val); // TODO fix seg fault here

        char answermessage[100];
        sprintf(answermessage,"%f", answer); // converts answer to string and saves in answermessage;

        strcat(message,answermessage);

        dlclose(handle);
    }
    else
    {
        strcpy(message,"DLL was not found / is not supported. Only 'math' is supported");
    }

    printf("Function executed in thread %ld for socket %d\n",pthread_self(),args.socketfd);
    int n = write(args.socketfd,message,strlen(message));

    if(n<0)
        error("ERROR could not execute write() in the socket"); 
    sleep(1);

    *args.status = 1;
    pthread_exit(NULL);
}



int main(int argc,char *argv[])
{
    int sockfd;//socket file descriptor (for socket())
    int newsockfd; // socket file descriptor(for accept())

    int portno; // port no on which we will accept connection
    int n; // no of characters
    int clilen; // length of address of client

    int maxthreads = 5; // Max no of threads allowed to execute concurrently

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
    

    pthread_t threadpool[maxthreads]; // Pool of threads
    int threadstatus[maxthreads]; // Maintains status of each thread => 1 when thread is free, -1 when thread is executing, 0 when no thread has been initialized yet(threadpool[i] is null)

    for(int i=0;i<maxthreads;i++)
    {
        threadstatus[i] = 0;
        threadpool[i] = '\0';
    }

    while(1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr*) &cli_addr,&clilen); //Accepts new connection and returns new socket file descriptor thorugh which the communication will take place
        if(newsockfd<0)
            error("ERROR executing accept()");
        printf("Accepted a Connection\n");

        if (read(newsockfd, buffer, 1000) == -1)
            error("ERROR executing read() in server");
        // printf("Data Received: %s\n", buffer);

        json_object *parsed_json;

        parsed_json = json_tokener_parse(buffer);
        
        struct thread_arguments args;

        args.jsonData = parsed_json;
        args.socketfd = newsockfd;
  
        for(int i=0;i<maxthreads;i++)
        {
            // printf("i=%d ",i);
            if(threadstatus[i] >= 0) // Thread is dead or no thread was created earlier
            {
                // printf("Creating thread at i=%d with threadstatus=%d\n",i,threadstatus[i]);
                
                threadstatus[i] = -1;
                args.status = &threadstatus[i];
                
                pthread_create(&threadpool[i],NULL,func,&args);
                // printf("Created thread\n");
                break;
            }
            else if(i==maxthreads - 1)
                i=-1;
        }
        // printf("Out of the loop\n");
    }
 
    close(sockfd);

    return 0;
}


