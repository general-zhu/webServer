#include <sys/epoll.h>
#include <fcntl.h>

#include "threadpool.h"
#include "epoll.h"
#include "wrap.h"
#include "pub.h"

//从epoll监听的红黑树中删除一个文件描述符
void eventDel(int epfd, struct MyEvents* ev) {
	struct epoll_event epv = { 0, {0} };

	if (ev->status != 1) {//不在红黑树上
		return;
	}

	epv.data.ptr = ev;
	ev->status = 0;//修改状态
	epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, &epv);//从红黑树efd上将ev->fd 摘除
	return;
}

//向epoll监听的红黑树 添加一个文件描述符
void eventAdd(int epfd, int events, struct MyEvents* ev) {
	struct epoll_event epv = { 0, {0} };
	int op;
	epv.data.ptr = ev;
	epv.events = ev->events = events; //EPOLLIN 或者 EPOLLOUT

	if (ev->status == 1) {
		op = EPOLL_CTL_MOD; //已经在红黑 epfd里，修改其属性
	}
	else {
		op = EPOLL_CTL_ADD;//不在红黑树里，将其加入到红黑树 epfd，并将status置1
		ev->status = 1;
	}
	if (epoll_ctl(epfd, op, ev->fd, &epv) < 0) {	//实际添加/修改
		printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
	}
	else {
		printf("event add OK [fd=%d], op=%d, events[%0X]\n", ev->fd, op, events);
	}
	return;
}

//将结构体 MyEvents 成员变量 初始化
//(struct ThreadPool* pool, void*(*function)(void * arg), void* arg, int fd)
void eventSet(struct MyEvents* ev, int fd, int(*call_back)(struct ThreadPool* pool, int(*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd), void* arg) {
	ev->fd = fd;
	ev->call_back = call_back;
	ev->events = 0;
	ev->arg = arg;
	ev->status = 0;//应该为0，0是EPOLL_CTL_ADD
	//ev->status = 1;
	ev->last_active = time(NULL);//调用eventSet函数的时间 unix时间戳

	return;
}

//当文件描述符（监听）就绪, epoll返回，调用该函数 与客户端连接
//回调函数（原来是监听的文件描述符发送读事件时被调用），我自己感觉要放线程池添加任务的函数
//int (*call_back)(ThreadPool* pool, void*(*function)(void * arg), void* arg, int fd)
int acceptConn(struct ThreadPool* pool, int(*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd) {
	struct sockaddr_in cin;
	int lfd = fd;
	socklen_t len = sizeof(cin);
	int cfd, i;
	cfd = Accept(lfd, (struct sockaddr*)&cin, &len);
	printf("cfd==[%d], lfd==[%d]\n", cfd, lfd);
	//使用do while(0)的目的
	do {
		for (i = 0; i < MAXEVENTS; i++) {	//从全局myEvents中找到一个空闲的元素
			if (myEvents[i].status == 0) {	//类似与select中找值为-1的元素
				break;//找到第一个能用的 跳出for
			}
		}
		if (i == MAXEVENTS) {
			printf("%s: max connect limit[%d]\n", __func__, MAXEVENTS);
			break;//避免goto， 跳出do while(0) 不执行后续的代码
		}


		//将cfd设置为非阻塞为了httprequest的read函数，如果阻塞的话他会一直停在哪里（其实有代码，下一个优化版本的时候可以加上）
		int flags = 0;
		flags = fcntl(cfd, F_GETFL, 0);
		flags |= O_NONBLOCK;
		if ((flags = fcntl(cfd, F_SETFL, flags)) < 0) {
			printf("%s: fcntl nonblock failed, %s\n", __func__, strerror(errno));
			break;//避免goto
		}


		//给cfd设置一个MyEvents结构体，回调函数设置为线程添加（先用recvdata占位）
		//eventSet(&myEvents[i], cfd, recvdata, &myEvents[i]);
		eventSet(&myEvents[i], cfd, threadpoolAdd, &myEvents[i]);
		//将cfd添加到红黑树epfd中，监听读事件
		//eventAdd(epfd, EPOLLIN|EPOLLOUT| EPOLLET, &myEvents[i]);
		eventAdd(epfd, EPOLLIN, &myEvents[i]);
	} while (0);

	printf("new connect [%s:%d] [time:%ld], pos[%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), myEvents[i].last_active, i);
	return 0;
}