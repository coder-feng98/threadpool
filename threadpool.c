#include "threadpool.h"
#include <pthread.h>
#include <stdlib.h>
// #include <unistd.h>

#define ACTIVATE_THREAD_NUM 2

typedef struct
{
    void* (*func)(void*);
    void* arg;
}Task;

struct ThreadPool 
{
    // I. �������
    Task* taskQ; // ������� - ���� - ѭ������
    int tCapacity; // �����������
    int tHead;
    int tTail;
    int tNum; 

    // II. �����߳�
    pthread_t* workers; // �����̶߳��� - �洢�߳�id
    int tMin; // ���г�����Сֵ - ������ʼ���������е��߳�����
    int tMax; // ���г������ֵ
    int tBusy; // æµ�����ڹ��������߳���
    int tLive; // �����߳���

    // III. �����߳�
    pthread_t* manager; // �����߳�id
    pthread_mutex_t threadPoolMutex; // �����̳߳صĻ�����
    pthread_mutex_t tBusyMutex; // tBusy��Ա�Ļ�����
    pthread_cond_t notFullCond; // ����������е��������� - �����߳����������������
    pthread_cond_t notEmptyCond; // ���������̵߳��������� - �ⲿ�����������������������
};

// �����߳���ں���
void* workerFunc(void* arg)
{
    /******
     * ��Ϊ�����߳���ں�����Ӧ��ʵ�ִ�������л�ȡ����Ȼ����ִ�����񣨼�����Task�ṹ���funcָ��ָ��ĺ�����
     * �˹�����Ҫע���߳�ͬ������
     */
    printf("in %ld thread\n", pthread_self());
    ThreadPool* pool = (ThreadPool*)arg;

    // ���������ȡ������
    pthread_mutex_lock(&pool->threadPoolMutex);
    while(pool->tNum == 0)
    {
        pthread_cond_wait(&pool->notEmptyCond, &pool->threadPoolMutex);
        if (pool->tNum == 0)
        {
            pthread_mutex_unlock(&pool->threadPoolMutex);
            pthread_exit(NULL);
            return NULL;
        }
    }
    Task task =  pool->taskQ[pool->tHead];
    pool->tHead = (pool->tHead + 1)%pool->tCapacity;
    pool->tNum--; // ��������1
    // pthread_cond_broadcast(&pool->notFullCond);
    pthread_mutex_unlock(&pool->threadPoolMutex);

    // ִ������
    task.func(task.arg);
    return NULL;
}

// �����߳���ں���
void* managerFunc(void* arg)
{
    /********
     * �����߳���Ҫ�������У������̳߳��е��������������߳���������߳�����̬���ٻ������߳�
     * �������߳�Ӧ���ǲ�ͣ��ѭ�����
     */
    printf("in manager %ld thread\n", pthread_self());
    ThreadPool* pool = (ThreadPool*)arg;

    while (1)
    {
        // ����߳�
        pthread_mutex_lock(&pool->threadPoolMutex);
        if (pool->tNum > pool->tLive && pool->tLive < pool->tMax) // �����������ڴ���߳������Ҵ���߳���С�ڿɽ�������߳���
        {
            int cnt = 0;
            for(int i = 0; i < pool->tMax && cnt < ACTIVATE_THREAD_NUM; ++i)
            {
                if (pool->workers[i] != 0)
                {
                    pthread_create(&pool->workers[i], NULL, workerFunc, pool);
                    cnt++;
                    // continue;
                }
            }
        }
        pthread_mutex_unlock(&pool->threadPoolMutex);

        // �����߳�
        if (pool->tLive > 2*pool->tNum && pool->tLive > pool->tMin)
        {
            for(int i = 0; i < ACTIVATE_THREAD_NUM; ++i)
            {
                pthread_cond_signal(&pool->notEmptyCond);
            }
        }

        sleep(2);
    }
    return NULL;
}

ThreadPool* initThreadPool(int tMin, int tMax, int tCapacity)
{
    printf("---------------- in initThreadPool --------------\n");
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));

    // ���������س�ʼ��
    pool->taskQ = (Task*)malloc(sizeof(Task)*tCapacity);
    pool->tCapacity = tCapacity;
    pool->tHead = pool->tTail = pool->tNum = 0;
    

    // �����߳���س�ʼ��
    pool->workers = (pthread_t*)malloc(sizeof(pthread_t)*tMax);
    pool->tMin = tMin;
    pool->tMax = tMax;
    pool->tBusy = 0;
    pool->tLive = tMin; // ....
    for (int i = 0; i < tMin; ++i)
    {
        pthread_create(&pool->workers[i], NULL, workerFunc, pool);
    }

    // �����߳���س�ʼ��
    pool->manager = (pthread_t*)malloc(sizeof(pthread_t));
    pthread_create(pool->manager, NULL, managerFunc, pool);
    pthread_mutex_init(&pool->tBusyMutex, NULL);
    pthread_cond_init(&pool->notEmptyCond, NULL);
    pthread_cond_init(&pool->notFullCond, NULL);

    return NULL;
}

int addTask(ThreadPool* pool, void (*func)(void*), void* arg)
{
    pthread_mutex_lock(&pool->threadPoolMutex);
    if (pool->tNum >= pool->tCapacity)
    {
        pthread_mutex_unlock(&pool->threadPoolMutex);
        return -1;
    }
    
    pool->taskQ[pool->tTail+1].func = func;
    pool->taskQ[pool->tTail+1].arg = arg;
    pool->tTail = (pool->tTail + 1)%pool->tCapacity;
    pool->tNum++;
    pthread_cond_broadcast(&pool->notEmptyCond);
    pthread_mutex_unlock(&pool->threadPoolMutex);

    return 1;
}


