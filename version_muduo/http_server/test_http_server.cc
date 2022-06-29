/**
 * Copyright(c) All rights reserved
 * Author: 244553899@qq.com
 * Time:   2022-06-28 16:00
 */

#include <iostream>
#include <string>
#include "http_server/http_server.h"
#include "http_server/http_request.h"
#include "http_server/http_response.h"
#include "thirdparty/muduo/net/EventLoop.h"

void OnRequest(const http::HttpRequest& req, http::HttpResponse* resp) {
  std::cout << "Headers " << req.MethodString() << " " << req.GetPath() << std::endl;
  const std::map<std::string, std::string>& headers = req.GetHeaders();
  for (const auto& header : headers) {
    std::cout << header.first << ": " << header.second << std::endl;
  }
  if (req.GetPath() == "/") {
    resp->SetStatusCode(http::HttpResponse::HttpStatusCode::k200OK);
    resp->SetStatusMessage("OK");
    resp->SetContentType("text/plain");
    resp->AddHeader("Server", "Muduo");
    resp->SetBody("hello, world!\n");
  } else {
    resp->SetStatusCode(http::HttpResponse::HttpStatusCode::k404NotFound);
    resp->SetStatusMessage("Not Found");
    resp->SetCloseConnection(true);
  }
}
int main() {
  muduo::net::EventLoop loop;
  http::HttpServer http_server(&loop, muduo::net::InetAddress(8000), "test_http_server");
  http_server.SetHttpCallback(OnRequest);
  http_server.Start();
  loop.loop();

}
