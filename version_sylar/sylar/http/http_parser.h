#ifndef SYLAR_HTTP_HTTP_PARSER_H_
#define SYLAR_HTTP_HTTP_PARSER_H_
#pragma once

#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"

namespace sylar {
namespace http {

class HttpRequestParser {
 public:
  typedef std::shared_ptr<HttpRequestParser> Ptr;

  HttpRequestParser();
  size_t Execute(char* data, size_t len);
  int IsFinished();
  int HasError();
  HttpRequest::Ptr GetData() const { return data_; }
  void SetError(int v) { error_ = v; }
  uint64_t GetContentLength();
  const http_parser& GetParser() const { return parser_; }

 public:
  static uint64_t GetHttpRequestBufferSize();
  static uint64_t GetHttpRequestMaxBodySize();

 private:
  http_parser parser_;
  HttpRequest::Ptr data_;
  // 错误码
  // 1000: invalid method
  // 1001: invalid version
  // 1002: invalid field
  int error_;
};

class HttpResponseParser {
 public:
  typedef std::shared_ptr<HttpResponseParser> Ptr;

  HttpResponseParser();
  // chunck 是否在解析chunck
  size_t Execute(char* data, size_t len, bool chunck);
  int IsFinished();
  int HasError();
  HttpResponse::Ptr GetData() const { return data_; }
  void SetError(int v) { error_ = v; }
  uint64_t GetContentLength();
  const httpclient_parser& GetParser() const { return parser_; }

 public:
  static uint64_t GetHttpResponseBufferSize();
  static uint64_t GetHttpResponseMaxBodySize();

 private:
  httpclient_parser parser_;
  HttpResponse::Ptr data_;
  // 错误码
  // 1001: invalid version
  // 1002:invalid field
  int error_;
};
}  // namespace http
}  // namespace sylar
#endif  // SYLAR_HTTP_HTTP_PARSER_H_
