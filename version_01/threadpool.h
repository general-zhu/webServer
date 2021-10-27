#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <pthread.h>

//extern struct MyEvents;//不可以, 这是报warning
struct MyEvents;
//回调函数这里用于hett prequest
typedef struct ThreadPoolTask {
	int(*function)(void* arg, struct MyEvents* ev);//函数指针，回调函数
	void* arg;//上面函数的参数
	struct MyEvents* ev;
}ThreadPoolTask;
//线程池的相关信息
typedef struct ThreadPool {
	pthread_mutex_t lock;//用于锁住本结构体
	pthread_mutex_t threadCounter;//用于锁住记录忙状态线程个数的锁，即锁住busyThreadNum变量

	pthread_cond_t queueNotFull; //当任务队列满时，添加任务的线程要阻塞，等待此条件变量
	pthread_cond_t queueNotEmpty;//当任务队列不为空时，用于通知等待任务的线程

	pthread_t* threads;//数组，存放线程池中每个线程的tid
	pthread_t adjustTid;//存管理线程的tid
	ThreadPoolTask* taskQueue;//任务队列

	int minThreadNum;//线程池中最小线程数量
	int maxThreadNum;//线程池中最大线程数量
	int liveThreadNum;//线程池中存活的线程数量，即还没有delete的线程
	int busyThreadNum;//忙状态线程个数，即处理任务的线程数
	int waitExitThreadNum;//要销毁的线程个数

	int queueFront;//taskQueue队头下标
	int queueRear;//taskQueue队尾下标
	int queueSize;//taskQueue队列中实际任务数
	int queueMaxSize;//taskQueue队列可容纳的任务数上限

	int shutdown;//标志位，线程池使用状态,true或者false
}ThreadPool;


ThreadPool* threadPoolCreate(int minThreadNum, int maxThreadNum, int queueMaxSize);
int isThreadAlive(pthread_t tid);
void* threadPoolThread(void* threadpool);
void* adjustThread(void* threadpool);
int threadPoolDestroy(ThreadPool* pool);
int threadPoolFree(ThreadPool* pool);
int threadpoolAdd(ThreadPool* pool, int (*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd);

#endif 
