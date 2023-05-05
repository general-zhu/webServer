#include "sylar/http/http.h"
#include "sylar/sylar.h"
#include "sylar/log.h"

void RequestTest() {
  sylar::http::HttpRequest::Ptr req(new sylar::http::HttpRequest);
  req->SetHeader("host", "www.baidu.com");
  req->SetBody("hello sylar");
  req->Dump(std::cout) << std::endl;
}

void ResponseTest() {
  sylar::http::HttpResponse::Ptr rsp(new sylar::http::HttpResponse);
  rsp->SetHeader("X-X", "sylar");
  rsp->SetBody("hello sylar");
  rsp->SetStatus((sylar::http::HttpStatus)400);
  rsp->SetClose(false);
  rsp->Dump(std::cout) << std::endl;
}
int main() {
  RequestTest();
  ResponseTest();
  return 0;
}
