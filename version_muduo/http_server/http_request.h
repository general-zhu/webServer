/**
 * Copyright(c) All rights reserved
 * Author: 244553899@qq.com
 * Time:   2022-06-20 16:43
 */

#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_
#pragma once

#include <iostream>
#include <map>
#include <string>
#include "thirdparty/muduo/base/Timestamp.h"
// #include "thirdparty/muduo/base/Types.h"

namespace http {
class HttpRequest {
 public:
  enum Method {
    kInvalid,
    kGet,
    kPost,
    kHead,
    kPut,
    kDelete,
  };
  enum Version {
    kUnknown,
    kHttp10,
    kHttp11,
  };
  HttpRequest() : method_(kInvalid), version_(kUnknown) { path_.clear(); }
  void SetVersion(Version v) { version_ = v; }
  Version GetVersion() const { return version_; }
  bool SetMethod(const char* start, const char* end);
  Method GetMethod() const { return method_; }
  const char* MethodString() const;
  void SetPath(const char* start, const char* end) {
    std::cerr << "path_=" << std::string(start, end) << "\n";
    // path_ = std::string(start, end);
    // path_.assign(start, end);
    path_ = "/1";
  }
  const std::string& GetPath() const { return path_; }
  void SetQuery(const char* start, const char* end) { query_.assign(start, end); }
  const std::string& GetQuery() const { return query_; }
  void SetReceiveTime(muduo::Timestamp t) { receive_time_ = t; }
  muduo::Timestamp GetReceiveTime() const { return receive_time_; }
  void AddHeader(const char* start, const char* colon, const char* end);
  std::string GetHeader(const std::string& filed) const;
  const std::map<std::string, std::string>& GetHeaders() const { return headers_; }
  void swap(HttpRequest& that);


 private:
  Method method_;
  Version version_;
  std::string path_;
  std::string query_;
  muduo::Timestamp receive_time_;
  std::map<std::string, std::string> headers_;
};
}  // namespace http

#endif  // HTTP_REQUEST_H_
