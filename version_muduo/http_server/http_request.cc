/**
 * Copyright(c) All rights reserved
 * Author: 244553899@qq.com
 * Time:   2022-06-20 17:41
 */

#include "http_request.h"
#include <assert.h>

namespace http {

bool HttpRequest::SetMethod(const char* start, const char* end) {
  assert(method_ == kInvalid);
  std::string method(start, end);
  if (method == "GET") {
    method_ = kGet;
  } else if (method == "POST") {
    method_ = kPost;
  } else if (method == "HEAD") {
    method_ = kHead;
  } else if (method == "PUT") {
    method_ = kPut;
  } else if (method == "DELETE") {
    method_ = kDelete;
  } else {
    method_ = kInvalid;
  }
  return method_ != kInvalid;
}

const char* HttpRequest::MethodString() const {
  const char* result = "UNKNOWN";
  switch (method_) {
    case kGet:
      result = "GET";
      break;
    case kPost:
      result = "POST";
      break;
    case kHead:
      result = "HEAD";
      break;
    case kPut:
      result = "PUT";
      break;
    case kDelete:
      result = "DELETE";
      break;
    default:
      break;
  }
  return result;
}

void HttpRequest::AddHeader(const char* start, const char* colon, const char* end) {
  std::string field(start, colon);
  ++colon;
  while (colon < end && isspace(*colon)) {
    ++colon;
  }
  std::string value(colon, end);
  while (!value.empty() && isspace(value.size() - 1)) {
    value.resize(value.size() - 1);
  }
  headers_[field] = value;
}

std::string HttpRequest::GetHeader(const std::string& field) const {
  std::string result;
  auto it = headers_.find(field);
  if (it != headers_.end()) {
    result = it->second;
  }
  return result;
}

void HttpRequest::swap(HttpRequest& that) {
  std::swap(method_, that.method_);
  std::swap(version_, that.version_);
  path_.swap(that.path_);
  receive_time_.swap(that.receive_time_);
  headers_.swap(that.headers_);
}

}  // namespace http

