#ifndef __EPOLL_H
#define __EPOLL_H

#include "threadpool.h"
#define MAXEVENTS 5000
//extern struct ThreadPool;//不可以, 这是报warning
struct ThreadPool;

int initialization(int port, int listenq);//初始化
void handleSigpipe();//忽略sigpipe信号
int epollInit(int listenq);//建epoll树
int setSocketNonBlocking(int fd);//文件描述符设置为非阻塞

//const int BUFLEN = 4096;
#define BUFLEN 4096

//events[i].data.ptr指向的内容
//描述就绪文件描述符相关信息,回调函数是往线程池里添加任务
struct MyEvents {
	int fd;//要监听的文件描述符
	int events;//对应的监听事件
	void* arg; //泛型参数
	int (*call_back)(struct ThreadPool* pool, int(*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd);//回调函数，这里用于往线程池里加任务
	int status;//是否在监听：1->在红黑树上（监听），0->不在（不监听）
	char buf[BUFLEN];
	int len;
	long last_active;//记录每次加入红黑树的时间值
};
struct MyEvents myEvents[MAXEVENTS + 1];;
int epfd;//全局变量，保存epoll_create返回的文件描述符，即树根

void eventSet(struct MyEvents* ev, int fd, int(*call_back)(struct ThreadPool* pool, int (*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd), void* arg);
int acceptConn(struct ThreadPool* pool, int(*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int lfd);
void eventAdd(int efd, int events, struct MyEvents* ev);
void eventDel(int epfd, struct MyEvents* ev);
#endif 
