#include<stdio.h>
#include<pthread.h>


void *fun(void *a){
    printf("%d",*(int*)a);
    char x = 65 + *(int *)a;
    for(int i=0;i<9;i++)
    {
        printf("%c",x);
        fflush(stdin);
    }
        
    pthread_exit(NULL);
}

int main()
{

    pthread_t t[10];
    
    // void *arguments_to_fun;
    // void *ret_from_thread;
    // int a = 3;

    for(int i=0;i<4;i++)
    {
  //      printf("\nCall %d %d\n", i,*(int*)&i);
        pthread_create(t+i,NULL,fun,&i); //thread,attr,start_routine,arguments to fun
    }
    
    pthread_exit(NULL);

}