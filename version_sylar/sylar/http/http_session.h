#ifndef SYLAR_HTTP_HTTP_SESSION_H_
#define SYLAR_HTTP_HTTP_SESSION_H_
#pragma once

#include "sylar/streams/socket_stream.h"
#include "http.h"

namespace sylar {
namespace http {

class HttpSession : public SocketStream {
 public:
  typedef std::shared_ptr<HttpSession> Ptr;

  HttpSession(Socket::Ptr sock, bool owner = true);
  HttpRequest::Ptr RecvRequest();
  int SendResponse(HttpResponse::Ptr rsp);
};

}  // namespace http

}  // namespace sylar
#endif  // SYLAR_HTTP_HTTP_SESSION_H_
