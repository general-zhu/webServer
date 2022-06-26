/**
 * Copyright(c) All rights reserved
 * Author: 244553899@qq.com
 * Time:   2022-06-20 15:56
 */

#include "http_context.h"
#include <algorithm>
#include <string>
#include "thirdparty/muduo/base/Timestamp.h"
#include "thirdparty/muduo/net/Buffer.h"

namespace http {
bool HttpContext::ProcessRequestLine(const char* begin, const char* end) {
  bool success = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  if (space != end && request_.SetMethod(start, space)) {
    start = space + 1;
    space = std::find (start, end, ' ');
    if (space != end) {
      const char* question = std::find(start, space, '?');
      if (question != space) {
        request_.SetPath(start, question);
        request_.SetQuery(question, space);
      } else {
        request_.SetPath(start, space);
      }
      start = space + 1;
      success = (end - start == 8) && (std::equal(start, end - 1, "HTTP/1."));
      if (success) {
        if (*(end - 1) == '1') {
          request_.SetVersion(HttpRequest::kHttp11);
        } else if (*(end - 1) == '0') {
          request_.SetVersion(HttpRequest::kHttp10);
        } else {
          success = false;
        }
      }
    }
  }
  return success;
}

bool HttpContext::ParseRequest(muduo::net::Buffer* buf, muduo::Timestamp receive_time) {
  bool ok = true;
  bool has_more = true;
  while (has_more) {
    if (state_ == kExpectRequestLine) {
      const char* crlf = buf->findCRLF();
      if (crlf) {
        ok = ProcessRequestLine(buf->peek(), crlf);
        if (ok) {
          request_.SetReceiveTime(receive_time);
          buf->retrieveUntil(crlf + 2);
          state_ = kExpectHeaders;
        } else {
          has_more = false;
        }
      } else {
        has_more = false;
      }
    } else if (state_ == kExpectHeaders) {
      const char* crlf = buf->findCRLF();
      if (crlf) {
        const char* colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf) {
          request_.AddHeader(buf->peek(), colon, crlf);
        } else {
          // empty line, end of head
          // FIXME:
          state_ = kGotAll;
          has_more = false;
        }
        buf->retrieveUntil(crlf + 2);
      } else {
        has_more = false;
      }
    } else if (state_ == kExpectBody) {
      // FIXME;
    }
  }
  return ok;
}

}  // namespace http
