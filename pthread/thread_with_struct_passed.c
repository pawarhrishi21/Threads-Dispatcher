#include<stdio.h>
#include<pthread.h>

struct fun_args
{
    int a;
    char b;
    char c[10];
};


void *fun(void *arguments){
    struct fun_args arg = *(struct fun_args*)arguments;
    printf("%d %c %s\n",arg.a,arg.b,arg.c);
        
    pthread_exit(NULL);
}

int main()
{

    pthread_t t[10];
    
    void *arguments_to_fun;

    struct fun_args arguments = {1,'A',"Hello"};



    pthread_create(t,NULL,fun,&arguments); //thread,attr,
    
    pthread_exit(NULL);

}