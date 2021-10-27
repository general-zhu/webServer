#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include "threadpool.h"

const int DEFAULT_TIME = 10;//管理线程每10秒查询一次
const int MIN_WAIT_TASK_NUM = 10;//最多等待任务个数
const int DEFAULT_THREAD_VARY = 10;//每次创建与销毁的个数

ThreadPool* threadPoolCreate(int minThreadNum, int maxThreadNum, int queueMaxSize) {
	int i;
	ThreadPool* pool = NULL;
	do {
		if ((pool = (ThreadPool*)malloc(sizeof(ThreadPool))) == NULL) {
			printf("new threadPool fail");
			break;//失败退出
		}

		pool->minThreadNum = minThreadNum;
		pool->maxThreadNum = maxThreadNum;
		pool->liveThreadNum = minThreadNum;//活着的线程数初始值为最小线程数
		pool->busyThreadNum = 0;//还线程去处理没有任务
		pool->waitExitThreadNum = 0;
		pool->queueMaxSize = queueMaxSize;
		pool->queueSize = 0;//还没有任务
		pool->queueFront = 0;
		pool->queueRear = 0;
		pool->shutdown = 0;//不关闭线程池

		//根据最大线程个数，给工作线程数组开辟空间，并清零
		pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * maxThreadNum);
		if (pool->threads == NULL) {
			printf("new threads fail");
			break;
		}
		memset(pool->threads, 0, sizeof(pthread_t) * maxThreadNum);

		//任务队列开辟空间
		pool->taskQueue = (ThreadPoolTask*)malloc(sizeof(ThreadPoolTask) * queueMaxSize);
		if (pool->taskQueue == NULL) {
			printf("new taskQueu fail");
			break;
		}

		//初始化互斥锁和条件变量
		if (pthread_mutex_init(&(pool->lock), NULL) != 0
			|| pthread_mutex_init(&(pool->threadCounter), NULL) != 0
			|| pthread_cond_init(&(pool->queueNotEmpty), NULL) != 0
			|| pthread_cond_init(&(pool->queueNotFull), NULL) != 0) {
			printf("init the lock or cond fail\n");
			break;
		}

		//创建工作线程
		//设置线程分离属性即线程可以自己释放自己
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		for (i = 0; i < minThreadNum; ++i) {
			//threadPoolThread,为线程处理函数即创建线程后线程跳转的地方
			pthread_create(&(pool->threads[i]), &attr, threadPoolThread, (void*)pool);
			printf("start thread 0x%x...\n", (unsigned int)pool->threads[i]);
			//printf("1");
		}
		//printf("1");
		//pthread_create(&(pool->adjustTid), &attr, adjustThread, (void*)pool);
		printf("adjust thread and three thread init sucess\n");
		return pool;
	} while (0);

	//前面代码调用失败，释放poll存储空间
	threadPoolFree(pool);
	return NULL;
}

//向线程池中 添加一个任务，应该就是http request
int threadpoolAdd(ThreadPool* pool, int (*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd)
{
	pthread_mutex_lock(&(pool->lock));
	printf("thread pool add task\n");
	//==为真， 队列已经满，调wait阻塞
	while ((pool->queueSize == pool->queueMaxSize) && (!pool->shutdown)) {
		pthread_cond_wait(&(pool->queueNotFull), &(pool->lock));
	}

	if (pool->shutdown) {
		pthread_cond_broadcast(&(pool->queueNotEmpty));
		pthread_mutex_unlock(&(pool->lock));
		return 0;
	}

	//清空 工作线程 调用的回调函数的参数arg 
	if (pool->taskQueue[pool->queueRear].arg != NULL) {
		pool->taskQueue[pool->queueRear].arg = NULL;
	}

	//添加任务到任务队列中
	pool->taskQueue[pool->queueRear].function = function;
	pool->taskQueue[pool->queueRear].arg = arg;
	pool->taskQueue[pool->queueRear].ev = ev;
	pool->queueRear = (pool->queueRear + 1) % pool->queueMaxSize; //移动队尾指针，模拟环形队列
	//你忘记加下面的一句了
	pool->queueSize++;
	//添加任务后队列不为空, 唤醒线程池中 等待处理任务的线程
	pthread_cond_signal(&(pool->queueNotEmpty));
	pthread_mutex_unlock(&(pool->lock));
	printf("thread pool add task is over\n");
	return 0;
}
//线程池中，各个线程的工作内容
void* threadPoolThread(void* threadpool) {
	ThreadPool* pool = (ThreadPool*)threadpool;
	ThreadPoolTask task;
	while (1) {
		//Lock must be taken to wait on conditional variable
		//刚刚创建线程时，等待任务队列里有任务，否则阻塞，等待任务队列里有任务后在唤醒（条件变量），去处理任务
		pthread_mutex_lock(&(pool->lock));

		//pool->queueSize == 0 说明没有任务，故阻塞在条件变量上，若要退出进程则也会阻塞在条件变量上
		printf("pool->queueSize==[%d]\n", pool->queueSize);
		while ((pool->queueSize == 0) && (!pool->shutdown)) {
			printf("thread 0x%x is waiting\n", (unsigned int)pthread_self());
			pthread_cond_wait(&(pool->queueNotEmpty), &(pool->lock));

			//清除指定数目的空闲线程，如果要结束的线程个数大于0，结束线程
			if (pool->waitExitThreadNum > 0) {
				pool->waitExitThreadNum--;

				//如果线程池里线程个数大于最小值时可以结束当前进程
				if (pool->liveThreadNum > pool->minThreadNum) {
					printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
					pool->liveThreadNum--;
					pthread_mutex_unlock(&(pool->lock));
					pthread_exit(NULL);
				}
			}
		}
		//printf("you ren wu le\n");
		//因为有任务的时候就跳到这里了
		//这里是退出所有线程
		if (pool->shutdown) {
			pthread_mutex_unlock(&(pool->lock));
			printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
			pthread_exit(NULL);
		}

		//从任务队列里取任务, 即出队操作
		task.function = pool->taskQueue[pool->queueFront].function;
		task.arg = pool->taskQueue[pool->queueFront].arg;
		task.ev = pool->taskQueue[pool->queueFront].ev;

		pool->queueFront = (pool->queueFront + 1) % pool->queueMaxSize;//模拟环型队列
		pool->queueSize--;

		//通知任务池可以放任务进来了
		pthread_cond_broadcast(&(pool->queueNotFull));

		//任务取出后立刻将线程池锁解除
		pthread_mutex_unlock(&(pool->lock));

		//执行任务
		printf("thread 0x%x start working\n", (unsigned int)pthread_self());
		pthread_mutex_lock(&(pool->threadCounter));//忙状态线程数变量锁
		pool->busyThreadNum++;
		pthread_mutex_unlock(&(pool->threadCounter));

		(*(task.function))(task.arg, task.ev);

		//任务结束
		printf("thread 0x%x end working\n", (unsigned int)pthread_self());
		pthread_mutex_lock(&(pool->threadCounter));
		pool->busyThreadNum--;
		pthread_mutex_unlock(&(pool->threadCounter));
	}

	pthread_exit(NULL);
}

//管理线程
void* adjustThread(void* threadpool) {
	ThreadPool* pool = (ThreadPool*)threadpool;
	int i;

	while (!pool->shutdown) {
		sleep(DEFAULT_TIME);//定时 管理线程池

		pthread_mutex_lock(&(pool->lock));
		int queueSize = pool->queueSize;
		int liveThreadNum = pool->liveThreadNum;

		pthread_mutex_lock(&(pool->threadCounter));
		int busyThreadNum = pool->busyThreadNum;
		pthread_mutex_unlock(&(pool->threadCounter));

		//设置线程分离属性即线程可以自己释放自己
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		//创建线程的算法：任务数大于设置的线程个数，且存活的线程数少于最大线程个数
		if (queueSize >= MIN_WAIT_TASK_NUM && liveThreadNum < pool->maxThreadNum) {
			pthread_mutex_lock(&(pool->lock));
			int add = 0;

			//一次性加 DEFAULT_THREAD_VARY 个线程
			for (i = 0; i < pool->maxThreadNum && add < DEFAULT_THREAD_VARY && pool->liveThreadNum < pool->maxThreadNum; ++i) {
				//查找空余位置插入
				if (pool->threads[i] == 0 || !isThreadAlive(pool->threads[i])) {
					//这里好像有问题创建的新线程没有分离
					//pthread_create(&(pool->threads[i]), NULL, threadPoolThread, (void*)pool);
					pthread_create(&(pool->threads[i]), &attr, threadPoolThread, (void*)pool);
					add++;
					pool->liveThreadNum++;
				}
			}
			pthread_mutex_unlock(&(pool->lock));
		}

		//销毁多余的线程
		//算法：忙线程 X2 小于 存活的线程且存活的线程大于最小线程数
		if (busyThreadNum * 2 < liveThreadNum && liveThreadNum < pool->minThreadNum) {
			//一次性销毁 DEFAULT_THREAD_VARY 个线程
			pthread_mutex_lock(&(pool->lock));
			pool->waitExitThreadNum = DEFAULT_THREAD_VARY;
			pthread_mutex_unlock(&pool->lock);
			for (int i = 0; i < DEFAULT_THREAD_VARY; i++) {
				//通知空闲状态的进程，让他们自动终止
				pthread_cond_signal(&(pool->queueNotEmpty));
			}
		}
	}
	return NULL;
}

int isThreadAlive(pthread_t tid) {
	int kill_rc = pthread_kill(tid, 0);//发送 0 信号，测试线程是否存活
	if (kill_rc == ESRCH) {
		return 0;//死了
	}
	return 1;
}

int threadPoolDestroy(ThreadPool* pool) {
	int i;
	if (pool == NULL) {
		return -1;
	}
	pool->shutdown = 1;


	for (i = 0; i < pool->liveThreadNum; ++i) {
		//通知所有空闲线程
		//感觉有问题，当线程正在工作呢？
		pthread_cond_broadcast(&(pool->queueNotEmpty));
	}
	threadPoolFree(pool);
	return 0;
}

int threadPoolFree(ThreadPool* pool) {
	if (pool == NULL) {
		return -1;
	}

	if (pool->taskQueue) {
		free(pool->taskQueue);
	}
	if (pool->threads) {
		free(pool->threads);
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		pthread_mutex_lock(&(pool->threadCounter));
		pthread_mutex_destroy(&(pool->threadCounter));
		pthread_cond_destroy(&(pool->queueNotEmpty));
		pthread_cond_destroy(&(pool->queueNotFull));
	}
	free(pool);
	pool = NULL;

	return 0;
}