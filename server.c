// $ gcc -o server server.c -lpthread -ljson-c -ldl
// $ $ ./server 11115 100 3000 12

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
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>

// Structure used for passing arguments to the func executed by the threads
struct thread_arguments{
    json_object* jsonData;
    int socketfd;
    int retval;
    int* status;
};

// Print the error and exit the program
void error(char *msg)
{
    perror(msg);
    exit(1);
}

// The function executed by threads to invoke DLL and execute a DLL function
void *func(void *arg)
{
    // Deserializing data received in JSON format using json-c library commands
    struct thread_arguments args = *(struct thread_arguments*)arg;
    json_object *parsed_json = args.jsonData;

    json_object *jfunction;
    json_object *jvalue;
    json_object *jdll;
    int *threadstatus = args.status;
    
    json_object_object_get_ex(parsed_json,"FunctionName", &jfunction);
    json_object_object_get_ex(parsed_json,"FunctionInput", &jvalue);
    json_object_object_get_ex(parsed_json,"DLLName", &jdll);

    const char *functionname = json_object_get_string(jfunction);
    const char *value = json_object_get_string(jvalue);
    const char *dll = json_object_get_string(jdll);
    double val = atof(value);
    void *handle;
    const char *math_dll = dll;
    double (*dll_func)(double);
    char *err;
    char message[100] = "Answer: "; //message to send answer to client

    if(strcmp(dll,"math") == 0)
    { 
        handle = dlopen ("/lib/x86_64-linux-gnu/libm.so.6", RTLD_LAZY); // Open the DLL

        if (!handle)
        {
            perror("Could not open the DLL math");
        }

        dll_func = dlsym(handle,functionname); // Get the desired function from the DLL
        
        if ((err = dlerror()) != NULL) // If some error occurs, send error message to client and exit the thread  
        {
            fputs(err, stderr);
            strcpy(message,err);
            write(args.socketfd,message,strlen(message));
            *args.status = 1;
            pthread_exit(NULL);
        }

        double answer = (*dll_func)(val); // Executing the DLL function

        // Constructing the message to send to client
        char answermessage[100];
        bzero(answermessage,100);
        sprintf(answermessage,"%f", answer); // converts answer to string and saves in answermessage;
        strcat(message,answermessage);

        // Close the DLL
        dlclose(handle);
    }
    else
    {
        strcpy(message,"DLL was not found / is not supported. Only 'math' is supported");
    }
    printf("Sending to Client: %s\n",message);
    int n = write(args.socketfd,message,strlen(message)); // Send message to the client
    if(n<0)
        error("ERROR could not execute write() in the socket"); 
   
    sleep(1); // Sleep added to let the threads wait to understand and see the SMP

    *args.status = 1; // Update the thread status as completed
    close(args.socketfd); // Close the client-server connection socket
    pthread_exit(NULL); // Exit the thread
}

// Get total current memory under use by the process
long long int getMemory() {
    // Buffers for storing intermediate string outputs 
    char buffer[1024] = "";
    char memory[20];
    
    // Open the proc file for linux
    FILE* pf = fopen("/proc/self/status", "r");

    // Parse and read the value of Memory in use
    while (fscanf(pf, " %1023s", buffer) == 1) 
    {
        if (strcmp(buffer, "VmRSS:") == 0) 
        {
            fgets(memory,20,pf);
        }
    }

    fclose(pf);
    return atoll(memory);
}

// Get total no of files currently open in the process
int getFiles(){
    char data[10];
    FILE* pf = popen("ls /proc/self/fd/ | wc -l","r");  // Proc file for checking no of files open
    fgets(data, 10 , pf);
    fclose(pf);
    return atoi(data);
}

int main(int argc,char *argv[])
{
    int sockfd;//socket file descriptor (for socket())
    int newsockfd; // socket file descriptor(for accept())

    int portno; // port no on which we will accept connection
    int n; // no of characters
    int clilen; // length of address of client

    int maxthreads = 5; // Max no of threads allowed to execute concurrently
    int maxfiles = 12; // Max no of files allowed to be open at an instance
    long long int maxmemory = 2000; // Max limit on memory in KiloBytes

    char buffer[1000]; // Buffer to store char read from connection

    struct sockaddr_in serv_addr; // for Server internet address (defined in netinet/in.h library){sin_family,sin_port,(struct)sin_addr->s_addr,sin_zero}
    struct sockaddr_in cli_addr; // for Client internet address

    // File pointer for proc file
    FILE* pf;

    int filescount = 0; // Total no of files currently open by the process
    long long int memoryamount = 0; // Total memory in use currently

    if(argc<5) // Verifying user's execution command
    {
        fprintf(stderr,"ERROR: No port provided, Usage : ./server PORTNO THREAD_LIMIT MEMORY_LIMIT FILES_LIMIT\n");
        exit(1);
    }

    // Updating the limits based on user's input
    maxthreads = atoi(argv[2]);
    maxmemory = atoll(argv[3]);
    maxfiles = atoi(argv[4]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // socket(Address domain{AF_INET for Internet},Type of Socket{SOCK_STREAM or SOCK_DGRAM},Protocol(0 chooses appropriate protocol{TCP or UDP})

    if(maxmemory < 3000 || maxfiles < 7 || maxthreads < 1)
    {
        error("ERROR in Input: Provide THREAD_LIMIT > 0 , FILES_LIMIT >= 7 , MEMORY_LIMIT >= 3000\n");
    }

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

    // Initialization of the pool of threads and their statuses
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

        if (read(newsockfd, buffer, 1000) == -1) // Reading data sent from client
            error("ERROR executing read() in server");

        json_object *parsed_json; // json_object from json-c library

        parsed_json = json_tokener_parse(buffer);

        struct thread_arguments args; // Argument to pass to the thread func

        args.jsonData = parsed_json;
        args.socketfd = newsockfd;
  
        for(int i=0;i<maxthreads;i++)
        {
            if(threadstatus[i] >= 0) // Either thread is dead or no thread was created earlier
            {
                // Check for the file limit
                do
                {
                    filescount = getFiles();
                    if(filescount > maxfiles)
                        printf("File limit reached. Waiting for threads to release files. Files open: %d, Max limit: %d\n",filescount - 1,maxfiles);
                    else
                    {
                        printf("Files open: %d\n",filescount - 1);
                    }
                } while (filescount > maxfiles);
                
                // Check for the memory limit
                do{
                    memoryamount = getMemory();
                    if(memoryamount>maxmemory)
                        printf("Memory Limit Exceeded. Waiting for threads to release memory. Memory in use: %lld, Max limit: %lld\n",memoryamount,maxmemory);
                    // else
                    //     printf("Memory in use: %lld\n",memoryamount);
                } while(memoryamount > maxmemory);

                threadstatus[i] = -1; // Set thread Status to Running
                args.status = &threadstatus[i]; // Update the status pointer (which will be used by the thread later)
                
                pthread_create(&threadpool[i],NULL,func,&args); // Create the thread
                break;
            }
            else if(i==maxthreads - 1) // Continue looping from the start if at the last index
                i=-1;
        }
    }
 
    close(sockfd);

    return 0;
}


