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
}ThreadPool;

// �����߳���ں���
void* workerFunc(void* arg)
{
    /******
     * ��Ϊ�����߳���ں�����Ӧ��ʵ�ִ�������л�ȡ����Ȼ����ִ�����񣨼�����Task�ṹ���funcָ��ָ��ĺ�����
     * �˹�����Ҫע���߳�ͬ������
     */
    ThreadPool* pool = (ThreadPool*)arg;

    // ���������ȡ������
    pthread_mutex_lock(&pool->threadPoolMutex);
    Task task =  pool->taskQ[pool->tHead];
    pool->tHead++;
    pool->tTail = (pool->tTail + 1)%pool->tCapacity;
    pool->tNum--; // ��������1
    pthread_mutex_unlock(&pool->threadPoolMutex);

    // ִ������
    task.func(task.arg);
}

// �����߳���ں���
void* managerFunc(void* arg)
{
    /********
     * �����߳���Ҫ�������У������̳߳��е��������������߳���������߳�����̬���ٻ������߳�
     * �������߳�Ӧ���ǲ�ͣ��ѭ�����
     */
    ThreadPool* pool = (ThreadPool*)arg;

    while (1)
    {
        // ����߳�
        if (pool->tNum > pool->tLive && pool->tLive < pool->tMax) // �����������ڴ���߳������Ҵ���߳���С�ڿɽ�������߳���
        {
           pthread_t pid;
           pthread_create(&pid, NULL, workerFunc, pool);
        }

        // �����߳�
    }
    
}

ThreadPool* initThreadPool(int tMin, int tMax, int tCapacity)
{
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));

    // ���������س�ʼ��
    pool->taskQ = (Task*)malloc(sizeof(Task)*tCapacity);
    pool->tCapacity = tCapacity;
    pool->tHead = pool->tTail = pool->tNum = 0;
    for (int i = 0; i < tMin; ++i)
    {
        pthread_create(&pool->taskQ[i], NULL, workerFunc, pool);
    }

    // �����߳���س�ʼ��
    pool->workers = (pthread_t*)malloc(sizeof(pthread_t)*tMin);
    pool->tMin = tMin;
    pool->tMax = tMax;
    pool->tBusy = 0;
    pool->tLive = tMin; // ....

    // �����߳���س�ʼ��
    pool->manager = (pthread_t*)malloc(sizeof(pthread_t));
    pthread_create(pool->manager, NULL, managerFunc, pool);

    return NULL;
}