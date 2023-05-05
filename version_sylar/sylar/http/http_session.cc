/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-04-22 11:05
 */

#include "http_session.h"
#include "http_parser.h"

namespace sylar {
namespace http {

HttpSession::HttpSession(Socket::Ptr sock, bool owner) : SocketStream(sock, owner) {
}

HttpRequest::Ptr HttpSession::RecvRequest() {
  HttpRequestParser::Ptr parser(new HttpRequestParser);
  uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
  std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr){
      delete[] ptr;
  });
  char* data = buffer.get();
  int offset = 0;
  do {
    int len = Read(data + offset, buff_size - offset);
    if (len <= 0) {
      Close();
      return nullptr;
    }
    len += offset;
    size_t nparse = parser->Execute(data, len);
    if (parser->HasError()) {
      Close();
      return nullptr;
    }
    offset = len - nparse;
    if (offset == (int)buff_size) {
      Close();
      return nullptr;
    }
    if (parser->IsFinished()) {
      break;
    }
  } while (true);
  int64_t length = parser->GetContentLength();
  if (length > 0) {
    std::string body;
    body.resize(length);
    int len = 0;
    if (length >= offset) {
      memcpy(&body[0], data, offset);
      len = offset;
    } else {
      memcpy(&body[0], data, length);
      len = length;
    }
    length -= offset;
    if (length > 0) {
      if (ReadFixSize(&body[len], length) <= 0) {
        Close();
        return nullptr;
      }
    }
    parser->GetData()->SetBody(body);
  }
  parser->GetData()->Init();
  return parser->GetData();
}

int HttpSession::SendResponse(HttpResponse::Ptr rsp) {
  std::stringstream ss;
  ss << *rsp;
  std::string data = ss.str();
  return WriteFixSize(data.c_str(), data.size());
}
}  // namespace http

}  // namespace sylar
