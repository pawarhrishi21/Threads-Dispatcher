|| Thread scheduler/ Dispatcher Simulation with File and Memory Limits ||

The program mimics how process threads are managed by the dispatcher in an Operating System. Given the maximum limits on numbers of files open and amount of memory getting used, the server schedules requests from multiple clients for a DLL function invocation using limited no of threads.
______________________________________________________________________________________________________________________________________________________________________

Logic, Description and the Design Specifications

There are two main parts to the program - Server and Client.

The client takes input of the DLL name and a function name inside the DLL with its parameters. It request the server over a socket connection to execute the user entered function with its parameters from the user entered DLL. This request's data is serialized into JSON format and sent over the socket connection. After completion of the request, the client recieves the function result from the server, prints it to the user's terminal and closes itself.

The server is a socket which listens for client connections and schedules its request on the pool of threads it has and using the limits specified by the user on no. of threads, memory amount and no. of files. It takes the data sent by the client in the form of DLL_Name, Function and its Parameters. It schedules this DLL function execution on the threads, if available, in the thread pool and waits for a thread to be free if no thread is available. Similarly, before executing a function on an available thread, the server checks for current usage of files and memory. If any of the file or memory limit is getting exceeded, then the program waits until the usage of files and memory reduces below the maximum limits and then executes the function on the available thread.

Inside the server:
    Thread Pool - Thread pool is represented by an array of thread identifiers. The size of the array is equal to the maximum allowed threads (thread limit). Another array (of integers) of the same size stores the statuses of these threads of the thread pool. 
        Status 0 - No thread has been ever created at that index in the thread pool (Hence, can be used). [NOT YET USED]
        Status -1 - the thread at that index is getting currently executed (Hence, busy and cannot be used at that point of time). [RUNNING]
        Status 1 - that the thread at that index has completed executing its previous assigned function (Hence, can be used). [COMPLETED]
    Using these statuses, the dispatcher allots requests on these threads. The dispatcher keeps checking continuosly for thread pool indices with Status = 0 or 1 and then assigns the request to the thread adn changes its status to -1 (RUNNING). It then accepts another request from the client and again schedules it on the thread pool using the same logic. If all thread statuses are -1 (RUNNING), then the dispatcher keeps checking until some threads finishes (changes its status to 1 (COMPLETED)). 

    Memory and File Limits - After the dispatcher finds a thread pool index with status 0 or 1 (can be used to execute the request), it checks the no of files and amount of memory in use in the process (this is done using the proc file for the corresponding process in linux). If any of these values exceed the maximum limit then the thread allotment is halted until the the files or memory are in the required limits. After the limits are ensured, only then the thread is alloted the request and it executes. (NOTE THAT the standard files - Standard input file, standard output file, and standard error file, and the server socket file descriptor, files opened before thread scheduling are removed from the files count obtained from the proc file to ensure that only the files opened by the threads are getting considered to check the limits)

    Thread Running (executing the request) - The thread function takes in parameters as socketFD (to send message to client), the JSON data recieved by server and deserializes it into string and int datatypes to obtain the DLL_Name, dll function name and its parameters. The thread then opens the requested DLL, gets the requested function and then executes it using the parameters. Based on the success or failure of these DLL operations, the thread sends the output or error message to the client. Finally, it updates the status of its thread to 'completed' in the thread pool status array and closes the client-server connection socket as well as the thread.  

______________________________________________________________________________________________________________________________________________________________________

Instructions to compile and run this program.

Prerequisite libraries - libjson-c-dev
Ensure that the above libraries are installed (Use $ sudo apt install libjson-c-dev) 

The general format of execution is: (MEMORY_LIMIT is in KB)
For server:
    $ gcc -o server server.c -lpthread -ljson-c -ldl
    $ ./server PORTNO THREAD_LIMIT MEMORY_LIMIT FILES_LIMIT

For Client:
    $ gcc -o client client.c -lpthread -ljson-c -ldl
    $ ./client PORTNO FUNCTION_NAME FUNCTION_PARAMETER DLL_NAME

For running tests:
    $ gcc -o tests tests.c -ldl
    $ ./tests

Example:
    Open two seperate linux terminals, say Terminal1 and Terminal2.

    In Terminal1, execute:
    $ gcc -o server server.c -lpthread -ljson-c -ldl
    $ ./server 11115 100 3000 12

    In Terminal2, execute:
    $ gcc -o client client.c -lpthread -ljson-c -ldl
    $ ./client 11115 cos 1 math

NOTE - Execute server commands before the client commands. Only the below mentioned functions are supported.

Supported DLL - Only 'math' DLL is supported.
Supported functions for math DLL - (In below list, x is the parameter)
    cos x
    sin x
    tan x
    sinh x
    cosh x
    tanh x
    exp x
    atan x
    asin x
    acos x
    sqrt x
    ceil x
    floor x

______________________________________________________________________________________________________________________________________________________________________


Please check sample run snapshots are present in the samples directory.

