#include<stdio.h>
#include<pthread.h>


void *fun(void *a){
    
    printf("%d",*(int*)a);
    pthread_exit(NULL);
}

int main()
{

    pthread_t t[10];
    

    for(int i=0;i<4;i++)
    {
        pthread_create(&t[i],NULL,fun,&i); //thread,attr,start_routine,arguments to fun
    }
    
    pthread_exit(NULL);

}