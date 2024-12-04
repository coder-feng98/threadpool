#include "threadpool.h"
#include <pthread.h>
#include <stdlib.h>

typedef struct
{
    /* data */
    void* (*func)(void*);
    void* arg;
}Task;

typedef struct
{
    // I. 任务队列
    Task* taskQ; // 任务队列 - 数组 - 循环队列
    int tCapacity; // 任务队列容量
    int tHead;
    int tTail;
    int tNum; 

    // II. 工作线程
    pthread_t* workers; // 工作线程队列 - 存储线程id
    int tMin; // 队列长度最小值 - 用来初始化工作队列的线程数量
    int tMax; // 队列长度最大值
    int tBusy; // 忙碌（正在工作）的线程数
    int tLive; // 存活的线程数

    // III. 管理线程
    pthread_t* manager; // 管理线程id
    pthread_mutex_t threadPoolMutex; // 整个线程池的互斥锁
    pthread_mutex_t tBusyMutex; // tBusy成员的互斥锁
    pthread_cond_t notFullCond; // 阻塞任务队列的条件变量 - 工作线程消费任务后解除阻塞
    pthread_cond_t notEmptyCond; // 阻塞工作线程的条件变量 - 外部向任务队列添加任务后解除阻塞
}ThreadPool;

// 工作线程入口函数
void* workerFunc(void* arg)
{
    /******
     * 作为工作线程入口函数，应当实现从任务队列获取任务、然后再执行任务（即调用Task结构体的func指针指向的函数）
     * 此过程需要注意线程同步问题
     */
    ThreadPool* pool = (ThreadPool*)arg;

    // 从任务队列取出任务
    pthread_mutex_lock(&pool->threadPoolMutex);
    Task task =  pool->taskQ[pool->tHead];
    pool->tHead++;
    pool->tTail = (pool->tTail + 1)%pool->tCapacity;
    pool->tNum--; // 任务数减1
    pthread_mutex_unlock(&pool->threadPoolMutex);

    // 执行任务
    task.func(task.arg);
}

// 管理线程入口函数
void* managerFunc(void* arg)
{
    /********
     * 管理线程需要做的事有：根据线程池中的任务数、工作线程数、存活线程数动态销毁或生成线程
     * 管理者线程应该是不停地循环检测
     */
    ThreadPool* pool = (ThreadPool*)arg;

    while (1)
    {
        // 添加线程
        if (pool->tNum > pool->tLive && pool->tLive < pool->tMax) // 当任务数大于存活线程数，且存活线程数小于可接受最大线程数
        {
           pthread_t pid;
           pthread_create(&pid, NULL, workerFunc, pool);
        }

        // 销毁线程
    }
    
}

ThreadPool* initThreadPool(int tMin, int tMax, int tCapacity)
{
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));

    // 任务队列相关初始化
    pool->taskQ = (Task*)malloc(sizeof(Task)*tCapacity);
    pool->tCapacity = tCapacity;
    pool->tHead = pool->tTail = pool->tNum = 0;
    for (int i = 0; i < tMin; ++i)
    {
        pthread_create(&pool->taskQ[i], NULL, workerFunc, pool);
    }

    // 工作线程相关初始化
    pool->workers = (pthread_t*)malloc(sizeof(pthread_t)*tMin);
    pool->tMin = tMin;
    pool->tMax = tMax;
    pool->tBusy = 0;
    pool->tLive = tMin; // ....

    // 管理线程相关初始化
    pool->manager = (pthread_t*)malloc(sizeof(pthread_t));
    pthread_create(pool->manager, NULL, managerFunc, pool);

    return NULL;
}