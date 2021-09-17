#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<dlfcn.h>

int getFiles(){
    char data[10];
    FILE* pf = popen("ls /proc/self/fd/ | wc -l","r");  // Proc file for checking no of files open
    fgets(data, 10 , pf);
    fclose(pf);
    return atoi(data);
}

// Testing no of files fetched
int test_file()
{
    printf("Testing Files\n");
    FILE *a,*b,*c;
    a = fopen("Readme.txt","r");
    b = fopen("server.c","r");
    c = fopen("client.c","r");

    int files = getFiles();
    if(files==7)
        printf("[+] Testset 1 Passed\n");
    else
        printf("[-] Testset 1 Failed\n");
    
    fclose(a);
    fclose(b);
    fclose(c);
}


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

// Testing DLL function execution
int test_dll_function( char *f_name, char *dll_name, double arg)
{
    printf("Testing DLL\n");
    char *err;
    double (*dll_func)(double);
    void *handle = dlopen (dll_name, RTLD_LAZY); // Open the DLL

    if (!handle)
    {
        printf("[-] Testset 3 failed\n");
        return -1;
    }

    dll_func = dlsym(handle,f_name); // Get the desired function from the DLL
    
    if ((err = dlerror()) != NULL) // If some error occurs, send error message to client and exit the thread  
    {
        fputs(err, stderr);
        printf("[-] Testset 3 failed\n");
        return -1;
    }

    double answer = (*dll_func)(arg);
    return answer;
}

// Testing memory fetched from Proc
int test_memory(){
    printf("Testing Memory\n");
    long long int mem1 = getMemory();
    char *string;
  
    string = malloc(sizeof(char) * 5);
    long long int mem2 = getMemory();
    if(mem2-mem1 == 16)
        printf("[+] Testset 2 Passed\n");
    else
        printf("[-] Testset 2 Failed\n");
}

int main()
{
    test_file();
    
    test_memory();
    
    if(test_dll_function("cos","/lib/x86_64-linux-gnu/libm.so.6",0)==1)
        printf("[+] Testset 3 Passed -- cos function on Math DLL\n");
    else
        printf("[+] Testset 3 Failed\n");

    if(test_dll_function("sin","/lib/x86_64-linux-gnu/libm.so.6",0)==0)
        printf("[+] Testset 4 Passed -- sin function on Math DLL\n");
    else
        printf("[-] Testset 4 Failed\n");

    if(test_dll_function("exp","/lib/x86_64-linux-gnu/libm.so.6",0)==1)
        printf("[+] Testset 5 Passed -- exp function on Math DLL\n");
    else
        printf("[-] Testset 5 Failed\n");
}