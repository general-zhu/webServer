#include "sylar/tcp_server.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"
#include "sylar/address.h"
#include "sylar/bytearray.h"

static sylar::Logger::Ptr g_logger = LOG_ROOT();

class EchoServer : public sylar::TcpServer {
 public:
  EchoServer(int type);
  void HandleClient(sylar::Socket::Ptr client);

 private:
  int type_ = 0;
};

EchoServer::EchoServer(int type) : type_(type) {
}

/*
void EchoServer::HandleClient(sylar::Socket::Ptr client) {
  LOG_INFO(g_logger) << "HandleClient" << *client;
  sylar::ByteArray::Ptr ba(new sylar::ByteArray);
  while (true) {
    ba->Clear();
    std::vector<iovec> iovs;
    ba->GetWriteBuffers(iovs, 1024);
    int rt = client->Recv(&iovs[0], iovs.size());
    if (rt == 0) {
      LOG_INFO(g_logger) << "client close: " << *client;
      break;
    } else if (rt < 0) {
      LOG_INFO(g_logger) << "client error rt=" << rt
          << " errno=" << errno << " errstr=" << strerror(errno);
      break;
    }
    ba->SetPosition(ba->GetPosition() + rt);
    ba->SetPosition(0);
    if (type_ == 1) {
      // text
      std::cout << ba->ToString();
    } else {
      std::cout << ba->ToHexString();
    }
    std::cout.flush();
  }
}
*/

void EchoServer::HandleClient(sylar::Socket::Ptr client) {
  LOG_INFO(g_logger) << "HandleClient" << *client;
  std::string buf;
  buf.resize(1024);
  char* data = &buf[0];
  while (true) {
    int rt = client->Recv(data, 1024);
    if (rt == 0) {
      LOG_INFO(g_logger) << "client close: " << *client;
      break;
    } else if (rt < 0) {
      LOG_INFO(g_logger) << "client error rt=" << rt
          << " errno=" << errno << " errstr=" << strerror(errno);
      break;
    }
    std::cout << buf;
  }
}

int type = 1;

void run() {
  LOG_INFO(g_logger) << "server type=" << type;
  EchoServer::Ptr es(new EchoServer(type));
  auto addr = sylar::Address::LookupAny("0.0.0.0:8020");
  while (!es->Bind(addr)) {
    sleep(2);
  }
  es->Start();
}

int main(int argc, char** argv) {
  if (argc < 2) {
    LOG_INFO(g_logger) << "used as[" << argv[0] << " -t] or [" << argv[0] << " -b]";
    return 0;
  }
  if (!strcmp(argv[1], "-b")) {
    type = 2;
  }
  sylar::IOManager iom(2);
  iom.Schedule(run);
  return 0;
}
