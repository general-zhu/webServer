#ifndef HTTP_SERVER_HTTP_RESPONSE_H_
#define HTTP_SERVER_HTTP_RESPONSE_H_
#pragma once

#include <map>
#include <string>
// #include "thirdparty/base/Types.h"

namespace muduo {
namespace net {
class Buffer;

}  // namespace net
}  // namespace muduo

namespace http {
class HttpResponse {
 public:
  enum HttpStatusCode {
    kUnknown,
    k200OK = 200,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404,
  };
  explicit HttpResponse(bool close) : status_code_(kUnknown), close_connection_(close) {}
  void SetStatusCode(HttpStatusCode code) { status_code_ = code; }
  void SetStatusMessage(const std::string& message) { status_message_ = message; }
  void SetCloseConnection(bool on) { close_connection_ = on; }
  bool IsCloseConnection() const { return close_connection_; }
  void SetContentType(const std::string& content_type) { AddHeader("Content-Type", content_type); }
  void AddHeader(const std::string& key, const std::string& value) { headers_[key] = value; }
  void SetBody(const std::string& body) { body_ = body; }
  void AppendToBuffer(muduo::net::Buffer* output) const;

 private:
  std::map<std::string, std::string> headers_;
  HttpStatusCode status_code_;
  // add http version
  std::string status_message_;
  bool close_connection_;
  std::string body_;
};

}  // namespace http

#endif  // HTTP_SERVER_HTTP_RESPONSE_H_
