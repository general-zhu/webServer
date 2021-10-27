#ifndef __EPOLL_H
#define __EPOLL_H

#include "threadpool.h"
#define MAXEVENTS 5000
//extern struct ThreadPool;//������, ���Ǳ�warning
struct ThreadPool;

int initialization(int port, int listenq);//��ʼ��
void handleSigpipe();//����sigpipe�ź�
int epollInit(int listenq);//��epoll��
int setSocketNonBlocking(int fd);//�ļ�����������Ϊ������

//const int BUFLEN = 4096;
#define BUFLEN 4096

//events[i].data.ptrָ�������
//���������ļ������������Ϣ,�ص����������̳߳����������
struct MyEvents {
	int fd;//Ҫ�������ļ�������
	int events;//��Ӧ�ļ����¼�
	void* arg; //���Ͳ���
	int (*call_back)(struct ThreadPool* pool, int(*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd);//�ص������������������̳߳��������
	int status;//�Ƿ��ڼ�����1->�ں�����ϣ���������0->���ڣ���������
	char buf[BUFLEN];
	int len;
	long last_active;//��¼ÿ�μ���������ʱ��ֵ
};
struct MyEvents myEvents[MAXEVENTS + 1];;
int epfd;//ȫ�ֱ���������epoll_create���ص��ļ���������������

void eventSet(struct MyEvents* ev, int fd, int(*call_back)(struct ThreadPool* pool, int (*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd), void* arg);
int acceptConn(struct ThreadPool* pool, int(*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int lfd);
void eventAdd(int efd, int events, struct MyEvents* ev);
void eventDel(int epfd, struct MyEvents* ev);
#endif 
