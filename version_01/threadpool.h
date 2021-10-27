#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <pthread.h>

//extern struct MyEvents;//������, ���Ǳ�warning
struct MyEvents;
//�ص�������������hett prequest
typedef struct ThreadPoolTask {
	int(*function)(void* arg, struct MyEvents* ev);//����ָ�룬�ص�����
	void* arg;//���溯���Ĳ���
	struct MyEvents* ev;
}ThreadPoolTask;
//�̳߳ص������Ϣ
typedef struct ThreadPool {
	pthread_mutex_t lock;//������ס���ṹ��
	pthread_mutex_t threadCounter;//������ס��¼æ״̬�̸߳�������������סbusyThreadNum����

	pthread_cond_t queueNotFull; //�����������ʱ�����������߳�Ҫ�������ȴ�����������
	pthread_cond_t queueNotEmpty;//��������в�Ϊ��ʱ������֪ͨ�ȴ�������߳�

	pthread_t* threads;//���飬����̳߳���ÿ���̵߳�tid
	pthread_t adjustTid;//������̵߳�tid
	ThreadPoolTask* taskQueue;//�������

	int minThreadNum;//�̳߳�����С�߳�����
	int maxThreadNum;//�̳߳�������߳�����
	int liveThreadNum;//�̳߳��д����߳�����������û��delete���߳�
	int busyThreadNum;//æ״̬�̸߳�����������������߳���
	int waitExitThreadNum;//Ҫ���ٵ��̸߳���

	int queueFront;//taskQueue��ͷ�±�
	int queueRear;//taskQueue��β�±�
	int queueSize;//taskQueue������ʵ��������
	int queueMaxSize;//taskQueue���п����ɵ�����������

	int shutdown;//��־λ���̳߳�ʹ��״̬,true����false
}ThreadPool;


ThreadPool* threadPoolCreate(int minThreadNum, int maxThreadNum, int queueMaxSize);
int isThreadAlive(pthread_t tid);
void* threadPoolThread(void* threadpool);
void* adjustThread(void* threadpool);
int threadPoolDestroy(ThreadPool* pool);
int threadPoolFree(ThreadPool* pool);
int threadpoolAdd(ThreadPool* pool, int (*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd);

#endif 
