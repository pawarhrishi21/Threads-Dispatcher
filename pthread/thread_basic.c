#include<stdio.h>
#include<pthread.h>

int ret  = 5;
void *fun(){
    for(int i=0;i<9;i++)
        printf("Ha");
    ret = 5;
    printf("\nfun thread id: %ld\n",pthread_self());
    pthread_exit(&ret);
}

int main()
{

    pthread_t t1,t2;
    
    void *arguments_to_fun;
    void *ret_from_thread;

    pthread_create(&t1,NULL,fun,NULL); //thread,attr,start_routine,arguments to fun
    printf("\nmain thread id: %ld\n",pthread_self());

    pthread_join(t1,&ret_from_thread);
    printf("Returned from thread t1 into main : %d\n", *(int*)(ret_from_thread));



}