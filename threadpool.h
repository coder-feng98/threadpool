#ifndef THREADPOOL_H
#define THREADPOOL_H

typedef struct ThreadPool ThreadPool;

// 初始化线程池
ThreadPool* initThreadPool(int tMin, int tMax, int tCapacity);

// 工作线程入口函数
void* workerFunc(void* arg); 

// 管理线程入口函数
void* managerFunc(void* arg);

// 添加任务
int addTask(ThreadPool* pool, void (*func)(void*), void* arg);

#endif