#ifndef SYLAR_HTTP_HTTP_SERVER_H_
#define SYLAR_HTTP_HTTP_SERVER_H_
#pragma once

#include "sylar/tcp_server.h"
#include "http_session.h"
#include "servlet.h"

namespace sylar {
namespace http {

class HttpServer : public TcpServer {
 public:
  typedef std::shared_ptr<HttpServer> Ptr;

  HttpServer(bool keepalive = false, sylar::IOManager* worker = sylar::IOManager::GetThis(),
      sylar::IOManager* io_worker = sylar::IOManager::GetThis(),
      sylar::IOManager* accept_worker = sylar::IOManager::GetThis());
  ServletDispatch::Ptr GetServletDispatch() const { return dispatch_; }
  void SetServletDispatch(ServletDispatch::Ptr v) { dispatch_ = v; }
  virtual void SetName(const std::string& v) override;

 protected:
  virtual void HandleClient(Socket::Ptr client) override;
 private:
  bool is_keepalive_;
  ServletDispatch::Ptr dispatch_;
};

}  // namespace http

}  // namespace sylar
#endif  // SYLAR_HTTP_HTTP_SERVER_H_
