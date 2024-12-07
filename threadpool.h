#ifndef THREADPOOL_H
#define THREADPOOL_H

typedef struct ThreadPool ThreadPool;

// ��ʼ���̳߳�
ThreadPool* initThreadPool(int tMin, int tMax, int tCapacity);

// �����߳���ں���
void* workerFunc(void* arg); 

// �����߳���ں���
void* managerFunc(void* arg);

// �������
int addTask(ThreadPool* pool, void (*func)(void*), void* arg);

#endif