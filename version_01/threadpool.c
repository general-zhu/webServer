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

const int DEFAULT_TIME = 10;//�����߳�ÿ10���ѯһ��
const int MIN_WAIT_TASK_NUM = 10;//���ȴ��������
const int DEFAULT_THREAD_VARY = 10;//ÿ�δ��������ٵĸ���

ThreadPool* threadPoolCreate(int minThreadNum, int maxThreadNum, int queueMaxSize) {
	int i;
	ThreadPool* pool = NULL;
	do {
		if ((pool = (ThreadPool*)malloc(sizeof(ThreadPool))) == NULL) {
			printf("new threadPool fail");
			break;//ʧ���˳�
		}

		pool->minThreadNum = minThreadNum;
		pool->maxThreadNum = maxThreadNum;
		pool->liveThreadNum = minThreadNum;//���ŵ��߳�����ʼֵΪ��С�߳���
		pool->busyThreadNum = 0;//���߳�ȥ����û������
		pool->waitExitThreadNum = 0;
		pool->queueMaxSize = queueMaxSize;
		pool->queueSize = 0;//��û������
		pool->queueFront = 0;
		pool->queueRear = 0;
		pool->shutdown = 0;//���ر��̳߳�

		//��������̸߳������������߳����鿪�ٿռ䣬������
		pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * maxThreadNum);
		if (pool->threads == NULL) {
			printf("new threads fail");
			break;
		}
		memset(pool->threads, 0, sizeof(pthread_t) * maxThreadNum);

		//������п��ٿռ�
		pool->taskQueue = (ThreadPoolTask*)malloc(sizeof(ThreadPoolTask) * queueMaxSize);
		if (pool->taskQueue == NULL) {
			printf("new taskQueu fail");
			break;
		}

		//��ʼ������������������
		if (pthread_mutex_init(&(pool->lock), NULL) != 0
			|| pthread_mutex_init(&(pool->threadCounter), NULL) != 0
			|| pthread_cond_init(&(pool->queueNotEmpty), NULL) != 0
			|| pthread_cond_init(&(pool->queueNotFull), NULL) != 0) {
			printf("init the lock or cond fail\n");
			break;
		}

		//���������߳�
		//�����̷߳������Լ��߳̿����Լ��ͷ��Լ�
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		for (i = 0; i < minThreadNum; ++i) {
			//threadPoolThread,Ϊ�̴߳������������̺߳��߳���ת�ĵط�
			pthread_create(&(pool->threads[i]), &attr, threadPoolThread, (void*)pool);
			printf("start thread 0x%x...\n", (unsigned int)pool->threads[i]);
			//printf("1");
		}
		//printf("1");
		//pthread_create(&(pool->adjustTid), &attr, adjustThread, (void*)pool);
		printf("adjust thread and three thread init sucess\n");
		return pool;
	} while (0);

	//ǰ��������ʧ�ܣ��ͷ�poll�洢�ռ�
	threadPoolFree(pool);
	return NULL;
}

//���̳߳��� ���һ������Ӧ�þ���http request
int threadpoolAdd(ThreadPool* pool, int (*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd)
{
	pthread_mutex_lock(&(pool->lock));
	printf("thread pool add task\n");
	//==Ϊ�棬 �����Ѿ�������wait����
	while ((pool->queueSize == pool->queueMaxSize) && (!pool->shutdown)) {
		pthread_cond_wait(&(pool->queueNotFull), &(pool->lock));
	}

	if (pool->shutdown) {
		pthread_cond_broadcast(&(pool->queueNotEmpty));
		pthread_mutex_unlock(&(pool->lock));
		return 0;
	}

	//��� �����߳� ���õĻص������Ĳ���arg 
	if (pool->taskQueue[pool->queueRear].arg != NULL) {
		pool->taskQueue[pool->queueRear].arg = NULL;
	}

	//����������������
	pool->taskQueue[pool->queueRear].function = function;
	pool->taskQueue[pool->queueRear].arg = arg;
	pool->taskQueue[pool->queueRear].ev = ev;
	pool->queueRear = (pool->queueRear + 1) % pool->queueMaxSize; //�ƶ���βָ�룬ģ�⻷�ζ���
	//�����Ǽ������һ����
	pool->queueSize++;
	//����������в�Ϊ��, �����̳߳��� �ȴ�����������߳�
	pthread_cond_signal(&(pool->queueNotEmpty));
	pthread_mutex_unlock(&(pool->lock));
	printf("thread pool add task is over\n");
	return 0;
}
//�̳߳��У������̵߳Ĺ�������
void* threadPoolThread(void* threadpool) {
	ThreadPool* pool = (ThreadPool*)threadpool;
	ThreadPoolTask task;
	while (1) {
		//Lock must be taken to wait on conditional variable
		//�ոմ����߳�ʱ���ȴ���������������񣬷����������ȴ������������������ڻ��ѣ�������������ȥ��������
		pthread_mutex_lock(&(pool->lock));

		//pool->queueSize == 0 ˵��û�����񣬹����������������ϣ���Ҫ�˳�������Ҳ������������������
		printf("pool->queueSize==[%d]\n", pool->queueSize);
		while ((pool->queueSize == 0) && (!pool->shutdown)) {
			printf("thread 0x%x is waiting\n", (unsigned int)pthread_self());
			pthread_cond_wait(&(pool->queueNotEmpty), &(pool->lock));

			//���ָ����Ŀ�Ŀ����̣߳����Ҫ�������̸߳�������0�������߳�
			if (pool->waitExitThreadNum > 0) {
				pool->waitExitThreadNum--;

				//����̳߳����̸߳���������Сֵʱ���Խ�����ǰ����
				if (pool->liveThreadNum > pool->minThreadNum) {
					printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
					pool->liveThreadNum--;
					pthread_mutex_unlock(&(pool->lock));
					pthread_exit(NULL);
				}
			}
		}
		//printf("you ren wu le\n");
		//��Ϊ�������ʱ�������������
		//�������˳������߳�
		if (pool->shutdown) {
			pthread_mutex_unlock(&(pool->lock));
			printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
			pthread_exit(NULL);
		}

		//�����������ȡ����, �����Ӳ���
		task.function = pool->taskQueue[pool->queueFront].function;
		task.arg = pool->taskQueue[pool->queueFront].arg;
		task.ev = pool->taskQueue[pool->queueFront].ev;

		pool->queueFront = (pool->queueFront + 1) % pool->queueMaxSize;//ģ�⻷�Ͷ���
		pool->queueSize--;

		//֪ͨ����ؿ��Է����������
		pthread_cond_broadcast(&(pool->queueNotFull));

		//����ȡ�������̽��̳߳������
		pthread_mutex_unlock(&(pool->lock));

		//ִ������
		printf("thread 0x%x start working\n", (unsigned int)pthread_self());
		pthread_mutex_lock(&(pool->threadCounter));//æ״̬�߳���������
		pool->busyThreadNum++;
		pthread_mutex_unlock(&(pool->threadCounter));

		(*(task.function))(task.arg, task.ev);

		//�������
		printf("thread 0x%x end working\n", (unsigned int)pthread_self());
		pthread_mutex_lock(&(pool->threadCounter));
		pool->busyThreadNum--;
		pthread_mutex_unlock(&(pool->threadCounter));
	}

	pthread_exit(NULL);
}

//�����߳�
void* adjustThread(void* threadpool) {
	ThreadPool* pool = (ThreadPool*)threadpool;
	int i;

	while (!pool->shutdown) {
		sleep(DEFAULT_TIME);//��ʱ �����̳߳�

		pthread_mutex_lock(&(pool->lock));
		int queueSize = pool->queueSize;
		int liveThreadNum = pool->liveThreadNum;

		pthread_mutex_lock(&(pool->threadCounter));
		int busyThreadNum = pool->busyThreadNum;
		pthread_mutex_unlock(&(pool->threadCounter));

		//�����̷߳������Լ��߳̿����Լ��ͷ��Լ�
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		//�����̵߳��㷨���������������õ��̸߳������Ҵ����߳�����������̸߳���
		if (queueSize >= MIN_WAIT_TASK_NUM && liveThreadNum < pool->maxThreadNum) {
			pthread_mutex_lock(&(pool->lock));
			int add = 0;

			//һ���Լ� DEFAULT_THREAD_VARY ���߳�
			for (i = 0; i < pool->maxThreadNum && add < DEFAULT_THREAD_VARY && pool->liveThreadNum < pool->maxThreadNum; ++i) {
				//���ҿ���λ�ò���
				if (pool->threads[i] == 0 || !isThreadAlive(pool->threads[i])) {
					//������������ⴴ�������߳�û�з���
					//pthread_create(&(pool->threads[i]), NULL, threadPoolThread, (void*)pool);
					pthread_create(&(pool->threads[i]), &attr, threadPoolThread, (void*)pool);
					add++;
					pool->liveThreadNum++;
				}
			}
			pthread_mutex_unlock(&(pool->lock));
		}

		//���ٶ�����߳�
		//�㷨��æ�߳� X2 С�� �����߳��Ҵ����̴߳�����С�߳���
		if (busyThreadNum * 2 < liveThreadNum && liveThreadNum < pool->minThreadNum) {
			//һ�������� DEFAULT_THREAD_VARY ���߳�
			pthread_mutex_lock(&(pool->lock));
			pool->waitExitThreadNum = DEFAULT_THREAD_VARY;
			pthread_mutex_unlock(&pool->lock);
			for (int i = 0; i < DEFAULT_THREAD_VARY; i++) {
				//֪ͨ����״̬�Ľ��̣��������Զ���ֹ
				pthread_cond_signal(&(pool->queueNotEmpty));
			}
		}
	}
	return NULL;
}

int isThreadAlive(pthread_t tid) {
	int kill_rc = pthread_kill(tid, 0);//���� 0 �źţ������߳��Ƿ���
	if (kill_rc == ESRCH) {
		return 0;//����
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
		//֪ͨ���п����߳�
		//�о������⣬���߳����ڹ����أ�
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