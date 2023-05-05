#ifndef SYLAR_HTTP_HTTP_CONNECTION_H_
#define SYLAR_HTTP_HTTP_CONNECTION_H_
#pragma once

#include <list>
#include "sylar/streams/socket_stream.h"
#include "http.h"
#include "sylar/uri.h"
#include "sylar/thread.h"

namespace sylar {
namespace http {

struct HttpResult {
 typedef std::shared_ptr<HttpResult> Ptr;

 enum class Error {
  OK = 0,
  INVALID_URL = 1,
  INVALID_HOST = 2,
  INVALID_FAIL = 3,
  SEND_CLOSE_BY_PEER = 4,
  SEND_SOCKET_ERROR = 5,
  TIMEOUT = 6,
  CREATE_SOCKET_ERROR = 7,
  POOL_GET_CONNECTION = 8,
  POOL_INVALID_CONNECTION = 9,
 };

 HttpResult(int _result, HttpResponse::Ptr _response, const std::string& _error)
    :result(_result),
     response(_response),
     error(_error) {}

  int result;
  HttpResponse::Ptr response;
  std::string error;
  std::string ToString() const;
};

class HttpConnectionPool;

class HttpConnection : public SocketStream {
 friend class HttpConnectionPool;
 public:
  typedef std::shared_ptr<HttpConnection> Ptr;

  static HttpResult::Ptr DoGet(const std::string& url,
                               uint64_t timeout_ms,
                               const std::map<std::string, std::string>& headers = {},
                               const std::string& body = "");
  static HttpResult::Ptr DoGet(Uri::Ptr uri,
                               uint64_t timeout_ms,
                               const std::map<std::string, std::string>& headers = {},
                               std::string& body = "");
  static HttpResult::Ptr DoPost(const std::string& url,
                                uint64_t timeout_ms,
                                const std::map<std::string, std::string>& headers = {},
                                const std::string& body = "");
  static HttpResult::Ptr DoPost(Uri::Ptr uri,
                                uint64_t timeout_ms,
                                const std::map<std::string, std::string>& headers = {},
                                const std::string& body = "");
  static HttpResult::Ptr DoRequest(HttpMethod method,
                                   Uri::Ptr uri,
                                   uint64_t timeout_ms,
                                   const std::map<std::string, std::string>& headers = {},
                                   const std::string& body = "");
  static HttpResult::Ptr DoRequest(HttpRequest::Ptr req,
                                    Uri::Ptr uri,
                                    uint64_t timeout_ms);
  HttpConnection(Socket::Ptr sock, bool owner = true);
  ~HttpConnection();
  HttpResponse::Ptr RecvResponse();
  int SendRequst(HttpRequest::Ptr req);
 private:
  uint64_t create_time_ = 0;
  uint64_t request_ = 0;
};

class HttpConnectionPool {
 public:
  typedef std::shared_ptr
};
}  // namespace http

}  // namespace sylar

#endif  // SYLAR_HTTP_HTTP_CONNECTION_H_
