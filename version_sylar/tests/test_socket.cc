#include <unistd.h>
#include "sylar/sylar.h"
#include "sylar/socket.h"
#include "sylar/iomanager.h"

static sylar::Logger::Ptr g_logger = LOG_ROOT();

void SocketTest() {
  sylar::IPAddress::Ptr addr = sylar::Address::LookupAnyIPAddress("www.baidu.com");
  if (addr) {
    LOG_INFO(g_logger) << "get address: " << addr->ToString();
  } else {
    LOG_INFO(g_logger) << "get address fail";
    return;
  }
  sylar::Socket::Ptr sock = sylar::Socket::CreateTCP(addr);
  addr->SetPort(80);
  if (!sock->Connect(addr)) {
    LOG_ERROR(g_logger) << "connect " << addr->ToString() << " fail";
    return;
  } else {
    LOG_INFO(g_logger) << "connect " << addr->ToString() << " connected";
  }
  // const char buff[] = "GET / HTTP/1.0\r\n\r\n";
  const char buff[] = "GET / HTTP/1.0\r\n\r\n";
  int rt = sock->Send(buff, sizeof(buff));
  if (rt <= 0) {
    LOG_INFO(g_logger) << "send fail rt=" << rt;
    return;
  }
  std::string buffs;
  buffs.resize(4096);
  rt = sock->Recv(&buffs[0], buffs.size());
  if (rt <= 0) {
    LOG_INFO(g_logger) << "recv fail rt=" << rt;
    return;
  }
  LOG_INFO(g_logger) << buffs;
}

void UnixSocketTest() {
  // auto addr2 = sylar::UnixAddress::Ptr(new sylar::UnixAddress("./tmp/unix_addr"));
  std::string path = "/home/zhuchun/workspace/webServer/version_sylar/tmp/unix_addr";
  unlink(path.c_str());
  auto addr2 = sylar::UnixAddress::Ptr(new sylar::UnixAddress(path));
  if (addr2) {
    LOG_INFO(g_logger) << "get address: " << addr2->ToString();
  } else {
    LOG_INFO(g_logger) << "get address fail";
    return;
  }
  sylar::Socket::Ptr sock = sylar::Socket::CreateTCP(addr2);
  if (!sock) {
    // LOG_ERROR(g_logger) << perror("create listen_fd error");
    perror("create listen_fd error");
  }
  if (!sock->Bind(addr2)) {
    LOG_ERROR(g_logger) << "bind fail errno="
        << errno << " errstr=" << strerror(errno)
        << " addr=[" << addr2->ToString() << "]";
    return;
  }
  if (!sock->Listen()) {
    LOG_ERROR(g_logger) << "listen fail errno="
        << errno << " errstr=" << strerror(errno)
        << " addr=[" << addr2->ToString() << "]";
  }
}

int main() {
  sylar::IOManager iom;
  iom.Schedule(SocketTest);
  // iom.Schedule(UnixSocketTest);
  // UnixSocketTest();
  return 0;
}
