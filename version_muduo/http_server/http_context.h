/**
 * Copyright(c) All rights reserved
 * Author: 244553899@qq.com
 * Time:   2022-06-20 15:25
 */

#ifndef HTTP_CONTEXT_H_
#define HTTP_CONTEXT_H_
#pragma once

#include "http_request.h"

namespace muduo {
namespace net {
class Buffer;

}  // namespace net
}  // namespace mduo

namespace http {
class HttpContext {
 public:
  enum HttpRequestParseState {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
  };

 HttpContext() : state_(kExpectRequestLine) {}

 bool ParseRequest(muduo::net::Buffer* buf, muduo::Timestamp receive_time);
 bool GotAll() const { return state_ == kGotAll; }
 void Reset() {
  state_ = kExpectRequestLine;
  HttpRequest dummy;
  request_.swap(dummy);
 }
 const HttpRequest& GetRequest() const { return request_; }
 HttpRequest& GetRequest() { return request_; }

 private:
  bool ProcessRequestLine(const char* begin, const char* end);
  HttpRequestParseState state_;
  HttpRequest request_;
};

}  // namespace http

#endif  // HTTP_CONTEXT_H_
