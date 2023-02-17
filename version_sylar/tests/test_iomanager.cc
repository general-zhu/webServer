#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include "sylar/sylar.h"
#include "sylar/iomanager.h"

sylar::Logger::Ptr g_logger = LOG_ROOT();

int sock = 0;

void FiberTest() {
  LOG_INFO(g_logger) << "test fiber sock=" << sock;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);
  inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);

  if (!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
  } else if (errno == EINPROGRESS) {
    LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
    sylar::IOManager::GetThis()->AddEvent(sock, sylar::IOManager::READ, [](){
      LOG_INFO(g_logger) << "read callback";
    });
    sylar::IOManager::GetThis()->AddEvent(sock, sylar::IOManager::WRITE, [](){
      LOG_INFO(g_logger) << "write callback";
      // close(sock); // 不支持主动关闭
      sylar::IOManager::GetThis()->CancelEvent(sock, sylar::IOManager::READ);
      close(sock);
    });
  } else {
    LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
  }
}

void Test1() {
  std::cout << "EPOLLIN=" << EPOLLIN << " EPOLLOUT=" << EPOLLOUT << std::endl;
  sylar::IOManager iom;
  iom.Schedule(&FiberTest);
}

void TimerTest() {
  sylar::IOManager iom(2);
  sylar::Timer::Ptr timer = iom.AddTimer(500, [&timer](){
    LOG_INFO(g_logger) << "hello world";
    static int i = 0;
    if (++i == 5) {
      timer->Cancel();
    }
  }, true);
}

int main(int argc, char **argv) {
  // Test1();
  TimerTest();
  return 0;
}
