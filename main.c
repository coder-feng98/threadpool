#include "threadpool.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* func()
{
    for (int i = 0; i < 50; i++)
    {
        printf("%ld-thread --- %d", pthread_self(), i);
        usleep(50);
    }
}

int main(int argc, char* argv[])
{
    printf("---------------- in main --------------\n");
    printf("main thread id : %ld\n", pthread_self());
    ThreadPool* pool = initThreadPool(4, 8, 20);
    for (int i = 0; i < 30; i++)
    {
        addTask(pool, func, NULL);
    }

    return 1;
}