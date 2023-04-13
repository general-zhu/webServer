#include "sylar/sylar.h"
#include "sylar/socket.h"
#include "sylar/iomanager.h"

static sylar::Logger::Ptr g_loger = LOG_ROOT();

void SocketTest() {
  sylar::IPAddress::Ptr addr = sylar::Address::LookupAnyIPAddress("www.baidu.com");
  if (addr) {
    LOG_INFO(g_loger) << "get address: " << addr->ToString();
  } else {
    LOG_INFO(g_loger) << "get address fail";
    return;
  }
  sylar::Socket::Ptr sock = sylar::Socket::CreateTCP(addr);
  addr->SetPort(80);
  if (!sock->Connect(addr)) {
    LOG_ERROR(g_loger) << "connect " << addr->ToString() << " fail";
    return;
  } else {
    LOG_INFO(g_loger) << "connect " << addr->ToString() << " connected";
  }
  // const char buff[] = "GET / HTTP/1.0\r\n\r\n";
  const char buff[] = "GET / HTTP/1.0\r\n\r\n";
  int rt = sock->Send(buff, sizeof(buff));
  if (rt <= 0) {
    LOG_INFO(g_loger) << "send fail rt=" << rt;
    return;
  }
  std::string buffs;
  buffs.resize(4096);
  rt = sock->Recv(&buffs[0], buffs.size());
  if (rt <= 0) {
    LOG_INFO(g_loger) << "recv fail rt=" << rt;
    return;
  }
  LOG_INFO(g_loger) << buffs;
}

int main() {
  sylar::IOManager iom;
  iom.Schedule(SocketTest);
  return 0;
}
