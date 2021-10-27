#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include "epoll.h"
#include "threadpool.h"
#include "pub.h"
#include "wrap.h"

//超参数
const int PORT = 9999;//端口
const int LISTENQ = 1024;//建树时和listen时
struct epoll_event* events;
//const int MAXEVENTS = 5000;//event堆的大小

struct MyEvents;

int httpRequest(void* arg, struct MyEvents* ev);
//http发送头
int sendHeader(int cfd, char* code, char* msg, char* fileType, int len) {
	char buf[1024] = { 0 };
	sprintf(buf, "HTTP/1.1 %s %s\r\n", code, msg);
	sprintf(buf + strlen(buf), "Content-Type:%s\r\n", fileType);
	if (len > 0) {
		sprintf(buf + strlen(buf), "Content-Length:%d\r\n", len);
	}
	strcat(buf, "\r\n");
	Write(cfd, buf, strlen(buf));
	return 0;
}

//http发送文件
int sendFile(int cfd, char* fileName) {
	//打开文件
	printf("fileName==[%s]\n", fileName);
	int fd = open(fileName, O_RDONLY);
	if (fd < 0) {
		perror("open error");
		return -1;
	}

	//循环读文件，然后发送
	int n;
	char buf[1024];
	while (1) {
		memset(buf, 0x00, sizeof(buf));
		n = read(fd, buf, sizeof(buf));
		if (n <= 0) {
			break;
		}
		else {
			Write(cfd, buf, n);
		}
	}
	return 0;
}

//数据请求
//int httpRequest(int cfd, int epfd, struct MyEvents* ev)
int httpRequest(void* arg, struct MyEvents* ev) {
	int* p = (int*)arg;
	int cfd = p[0];

	int epfd = p[1];
	int n;
	char buf[1024];
	//读取请求行，分析出要请求的资源文件名
	memset(buf, 0x00, sizeof(buf));
	n = Readline(cfd, buf, sizeof(buf));
	printf("http request[cfd == %d]\n", cfd);
	if (n <= 0) {
		//关闭连接
		close(cfd);

		//将文件描述符从epoll树上删除
		eventDel(cfd, ev);
		return -1;
	}
	printf("buf==[%s]\n", buf);
	//GET /hanzi.c HTTP/1.1
	char reqType[16] = { 0 };
	char fileName[255] = { 0 };
	char protocal[16] = { 0 };
	sscanf(buf, "%[^ ] %[^ ] %[^ \r\n]", reqType, fileName, protocal);
	printf("--[%s]--\n", fileName);

	char* pFile = fileName;
	if (strlen(fileName) <= 1) {
		strcpy(pFile, "./");
	}
	else {
		pFile = fileName + 1;
	}

	//转换汉字编码
	strdecode(pFile, pFile);
	printf("[%s]\n", pFile);

	//循环读取完剩余的数据，避免产生粘包
	while ((n = Readline(cfd, buf, sizeof(buf))) > 0);

	//判断文件是否存在
	struct stat st;
	if (stat(pFile, &st) < 0) {
		printf("file not exist\n");

		//发送头部信息
		sendHeader(cfd, "404", "NOT FOUND", get_mime_type(".html"), 0);

		//发送文件内容
		sendFile(cfd, "error.html");
	}
	else {//文件存在
		//判断文件类型
		if (S_ISREG(st.st_mode)) {
			printf("file exist\n");
			//发送头部信息
			sendHeader(cfd, "200", "OK", get_mime_type(pFile), st.st_size);

			//发送文件内容
			sendFile(cfd, pFile);
		}
		else if (S_ISDIR(st.st_mode)) {//目录文件
			printf("目录文件\n");

			char buffer[1024];
			//发送头部信息
			sendHeader(cfd, "200", "OK", get_mime_type(".html"), 0);

			//发送html文件头部
			sendFile(cfd, "html/dir_header.html");

			//发送列表信息
			struct dirent** namelist;
			int num;

			num = scandir(pFile, &namelist, NULL, alphasort);
			if (num < 0) {
				perror("scandir");
				close(cfd);
				//将文件描述符从epoll树上删除
				eventDel(cfd, ev);
				return -1;
			}
			else {
				while (num--) {
					printf("%s\n", namelist[num]->d_name);
					memset(buffer, 0x00, sizeof(buffer));
					if (namelist[num]->d_type == DT_DIR) {
						sprintf(buffer, "<li><a href=%s/>%s</a></li>", namelist[num]->d_name, namelist[num]->d_name);
					}
					else {
						sprintf(buffer, "<li><a href=%s>%s</a></li>", namelist[num]->d_name, namelist[num]->d_name);
					}
					free(namelist[num]);
					Write(cfd, buffer, strlen(buffer));
				}
				free(namelist);
			}
			//发送html尾部
			sleep(10);
			sendFile(cfd, "html/dir_tail.html");
		}
	}
	return 0;
}



int setSocketNonBlocking(int fd) {
	int flag = fcntl(fd, F_GETFL, 0);
	if (flag == -1) {
		return -1;
	}
	flag |= O_NONBLOCK;
	if (fcntl(fd, F_SETFD, flag) == -1) {
		return -1;
	}
	return 0;
}
int epollInit(int listenq) {
	//建树
	int epfd = epoll_create(listenq + 1);
	if (epfd == -1) {
		return -1;
	}
	return epfd;
}
/*
创建socket--
设置端口复用---
bind
*/
int initialization(int port, int listenq) {
	int lfd = tcp4bind(port, NULL);
	Listen(lfd, listenq);
	return lfd;
}

void handleSigpipe() {
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigaction(SIGPIPE, &sa, NULL);
}
int main() {
	handleSigpipe();

	//改变当前进程的工作目录
	char path[255] = { 0 };
	sprintf(path, "%s/%s", getenv("HOME"), "workspace/webPracise");
	chdir(path);

	int lfd = initialization(PORT, LISTENQ);
	/*
	if (setSocketNonBlocking(lfd) < 0) {
		perror("set socket non block failed");
		return -1;
	}
*/
	epfd = epollInit(LISTENQ);
	if (epfd < 0) {
		close(lfd);
		perror("epoll init failed");
		return -1;
	}

	struct epoll_event events[MAXEVENTS + 1];
	//struct MyEvents* myEvents = new struct MyEvents[MAXEVENTS + 1];改成全局变量了
	// 对事件结构体进行赋值
	eventSet(&myEvents[MAXEVENTS], lfd, acceptConn, &events[MAXEVENTS]);//对myEvents进行设置，回调函数acceptConn（里面在调用添加任务）

	//将监听文件描述符上树
	eventAdd(epfd, EPOLLIN, &myEvents[MAXEVENTS]);//第一次默认从0开始，而0就是lfd所以不用传lfd
	//eventAdd(epfd, EPOLLIN | EPOLLET, &myEvents[MAXEVENTS]);//第一次默认从0开始，而0就是lfd所以不用传lfd
	__uint32_t event = EPOLLIN | EPOLLET;//设置为ET模式

	ThreadPool* thp = threadPoolCreate(5, 100, 100);//创建线程池，池里最小3个线程最大100个，任务队列最大100
	int args[2];//httpRequest参数

	int checkpos = 0, i;
	//开始while(1)循环
	while (1) {

		/*
		//超时验证，每次测试100个连接 不测试listenfd 当客户端60秒内没有和服务器通信，则关闭客户端连接
		long now = time(NULL);
		//一次检测100个，使用checkpos控制检测对象
		for (i = 0; i < 100; i++, checkpos++) {
			if (checkpos == MAXEVENTS) {
				checkpos = 0;
			}

			if (myEvents[checkpos].status != 1) {//不在红黑树上
				continue;
			}

			long duration = now - myEvents[checkpos].last_active;//客户不活跃时间
			if (duration >= 60) {
				Close(myEvents[checkpos].fd);
				printf("[fd=%d] timeout\n", myEvents[checkpos].fd);
				eventDel(epfd, &myEvents[checkpos]);//将该客户端从红黑树 epfd移除
			}
		}
		*/

		//监听红黑树epfd，将满足的事件的文件描述符加到events数组中，1秒没有事件满足，返回0
		//int nfd = epoll_wait(epfd, events, MAXEVENTS + 1, 1000);//因为非阻塞所以一直往下走
		int nfd = epoll_wait(epfd, events, MAXEVENTS + 1, -1);
		printf("[nfd=[%d]]\n", nfd);
		if (nfd < 0) {
			if (errno == EINTR) {
				continue;
			}
			printf("epoll_wait error, exit\n");
			break;
		}

		for (i = 0; i < nfd; i++) {
			//使用自定义结构体myEvents类型指针，接收联合体data的void *ptr成员
			struct MyEvents* ev = (struct MyEvents*)events[i].data.ptr;

			////读事件就绪
			//if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) {
			//	ev->call_back(ev->fd, events[i].events, ev);
			//}

			int socket = ev->fd;
			//有客户连接到来
			if (socket == lfd) {
				printf("client arrive\n");
				ev->call_back(NULL, NULL, NULL, NULL, ev->fd);//调用acceptConn，上树
			}
			else {
				//int cfd = p[0];//int epfd = p[1];
				printf("data arrive\n");
				args[0] = socket;
				printf("1socket == %d\n", socket);
				args[1] = epfd;
				ev->call_back(thp, httpRequest, &args, ev, 0);//调用线程添加任务函数,这里填入httprequest函数
				printf("socket == %d\n", socket);
			}

		}
	}

	close(epfd);
	close(lfd);
}
