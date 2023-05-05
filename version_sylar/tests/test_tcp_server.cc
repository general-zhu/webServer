#include "sylar/tcp_server.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"

static sylar::Logger::Ptr g_logger = LOG_ROOT();

void run() {
  auto addr = sylar::Address::LookupAny("0.0.0.0:8020");
  //auto addr2 = sylar::UnixAddress::Ptr(new sylar::UnixAddress("./tmp/unix_addr"));
  // LOG_INFO(g_logger) << *addr << "--" << *addr2;
  std::vector<sylar::Address::Ptr> addrs;
  addrs.push_back(addr);
  //addrs.push_back(addr2);
  sylar::TcpServer::Ptr tcp_server(new sylar::TcpServer);
  std::vector<sylar::Address::Ptr> fails;
  while (!tcp_server->Bind(addrs, fails)) {
    sleep(2);
  }
  tcp_server->Start();
}

int main() {
  sylar::IOManager iom(2);
  iom.Schedule(run);
  return 0;
}
