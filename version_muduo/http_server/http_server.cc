/**
 * Copyright(c) All rights reserved
 * Author: 244553899@qq.com
 * Time:   2022-06-20 10:51
 */

#include "http_server.h"
#include <functional>
#include "http_context.h"
#include "http_response.h"
#include "thirdparty/muduo/base/Logging.h"

namespace http {
namespace detail {
void DefaultHttpCallback(const http::HttpRequest&, http::HttpResponse* resp) {
  resp->SetStatusCode(http::HttpResponse::k404NotFound);
  resp->SetStatusMessage("Not Found");
  resp->SetCloseConnection(true);
}
}  // namespace detail

HttpServer::HttpServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listen_addr,
    const std::string& name, muduo::net::TcpServer::Option option):
    server_(loop, listen_addr, name, option),
    http_callback_(detail::DefaultHttpCallback) {
  server_.setConnectionCallback(std::bind(&HttpServer::OnConnection, this, std::placeholders::_1));
  server_.setMessageCallback(std::bind(&HttpServer::OnMessage, this, std::placeholders::_1,
  std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::start() {
  LOG_WARN << "httpServer:" << server_.name() << " starts listening on " << server_.ipPort();
  server_.start();
}

void HttpServer::OnConnection (const muduo::net::TcpConnectionPtr& conn) {
  if (conn->connected()) {
    conn->setContext(HttpContext());
  }
}

void HttpServer::OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf,
    muduo::Timestamp receive_time) {
  // http::HttpContext* context = boost::any_cast<http::HttpContext>(conn->getMutableContext());
  http::HttpContext* context =
  reinterpret_cast<http::HttpContext*>(conn->getMutableContext());
  if (!context->ParseRequest(buf, receive_time)) {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }
  if (context->GotAll()) {
    OnRequest(conn, context->GetRequest());
    context->Reset();
  }
}

void HttpServer::OnRequest(const muduo::net::TcpConnectionPtr& conn, const http::HttpRequest& req) {
  const std::string& connection = req.GetHeader("Connection");
  bool close = connection == "close" || (req.GetVersion() == HttpRequest::kHttp10 &&
      connection != "keep-Alive");
  HttpResponse response(close);
  http_callback_(req, &response);
  muduo::net::Buffer buf;
  response.AppendToBuffer(&buf);
  conn->send(&buf);
  if (response.IsCloseConnection()) {
    conn->shutdown();
  }
}
}  // namespace http

