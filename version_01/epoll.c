#include <sys/epoll.h>
#include <fcntl.h>

#include "threadpool.h"
#include "epoll.h"
#include "wrap.h"
#include "pub.h"

//��epoll�����ĺ������ɾ��һ���ļ�������
void eventDel(int epfd, struct MyEvents* ev) {
	struct epoll_event epv = { 0, {0} };

	if (ev->status != 1) {//���ں������
		return;
	}

	epv.data.ptr = ev;
	ev->status = 0;//�޸�״̬
	epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, &epv);//�Ӻ����efd�Ͻ�ev->fd ժ��
	return;
}

//��epoll�����ĺ���� ���һ���ļ�������
void eventAdd(int epfd, int events, struct MyEvents* ev) {
	struct epoll_event epv = { 0, {0} };
	int op;
	epv.data.ptr = ev;
	epv.events = ev->events = events; //EPOLLIN ���� EPOLLOUT

	if (ev->status == 1) {
		op = EPOLL_CTL_MOD; //�Ѿ��ں�� epfd��޸�������
	}
	else {
		op = EPOLL_CTL_ADD;//���ں�����������뵽����� epfd������status��1
		ev->status = 1;
	}
	if (epoll_ctl(epfd, op, ev->fd, &epv) < 0) {	//ʵ�����/�޸�
		printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
	}
	else {
		printf("event add OK [fd=%d], op=%d, events[%0X]\n", ev->fd, op, events);
	}
	return;
}

//���ṹ�� MyEvents ��Ա���� ��ʼ��
//(struct ThreadPool* pool, void*(*function)(void * arg), void* arg, int fd)
void eventSet(struct MyEvents* ev, int fd, int(*call_back)(struct ThreadPool* pool, int(*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd), void* arg) {
	ev->fd = fd;
	ev->call_back = call_back;
	ev->events = 0;
	ev->arg = arg;
	ev->status = 0;//Ӧ��Ϊ0��0��EPOLL_CTL_ADD
	//ev->status = 1;
	ev->last_active = time(NULL);//����eventSet������ʱ�� unixʱ���

	return;
}

//���ļ�������������������, epoll���أ����øú��� ��ͻ�������
//�ص�������ԭ���Ǽ������ļ����������Ͷ��¼�ʱ�����ã������Լ��о�Ҫ���̳߳��������ĺ���
//int (*call_back)(ThreadPool* pool, void*(*function)(void * arg), void* arg, int fd)
int acceptConn(struct ThreadPool* pool, int(*function)(void* arg, struct MyEvents* ev), void* arg, struct MyEvents* ev, int fd) {
	struct sockaddr_in cin;
	int lfd = fd;
	socklen_t len = sizeof(cin);
	int cfd, i;
	cfd = Accept(lfd, (struct sockaddr*)&cin, &len);
	printf("cfd==[%d], lfd==[%d]\n", cfd, lfd);
	//ʹ��do while(0)��Ŀ��
	do {
		for (i = 0; i < MAXEVENTS; i++) {	//��ȫ��myEvents���ҵ�һ�����е�Ԫ��
			if (myEvents[i].status == 0) {	//������select����ֵΪ-1��Ԫ��
				break;//�ҵ���һ�����õ� ����for
			}
		}
		if (i == MAXEVENTS) {
			printf("%s: max connect limit[%d]\n", __func__, MAXEVENTS);
			break;//����goto�� ����do while(0) ��ִ�к����Ĵ���
		}


		//��cfd����Ϊ������Ϊ��httprequest��read��������������Ļ�����һֱͣ�������ʵ�д��룬��һ���Ż��汾��ʱ����Լ��ϣ�
		int flags = 0;
		flags = fcntl(cfd, F_GETFL, 0);
		flags |= O_NONBLOCK;
		if ((flags = fcntl(cfd, F_SETFL, flags)) < 0) {
			printf("%s: fcntl nonblock failed, %s\n", __func__, strerror(errno));
			break;//����goto
		}


		//��cfd����һ��MyEvents�ṹ�壬�ص���������Ϊ�߳���ӣ�����recvdataռλ��
		//eventSet(&myEvents[i], cfd, recvdata, &myEvents[i]);
		eventSet(&myEvents[i], cfd, threadpoolAdd, &myEvents[i]);
		//��cfd��ӵ������epfd�У��������¼�
		//eventAdd(epfd, EPOLLIN|EPOLLOUT| EPOLLET, &myEvents[i]);
		eventAdd(epfd, EPOLLIN, &myEvents[i]);
	} while (0);

	printf("new connect [%s:%d] [time:%ld], pos[%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), myEvents[i].last_active, i);
	return 0;
}