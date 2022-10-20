/**
 * Copyright(c) All rights reserved
 * Author: 244553899@qq.com
 * Time:   2022-06-20 10:47
 */

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_
#pragma once

#include "thirdparty/muduo/net/TcpServer.h"

namespace http {
class HttpRequest;
class HttpResponse;

class HttpServer {
 public:
  typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpCallback;

 public:
  HttpServer(muduo::net::EventLoop* loop, const muduo::net::InetAddress& listen_addr,
      const std::string& name, muduo::net::TcpServer::Option option =
      muduo::net::TcpServer::kNoReusePort);
  void SetHttpCallback(const HttpCallback& cb) { http_callback_ = cb; }
  void Start();
  void SetServerThreads(int server_threads) { server_.setThreadNum(server_threads); }

 private:
  void OnConnection(const muduo::net::TcpConnectionPtr& conn);
  void OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf,
      muduo::Timestamp reveive_time);
  void OnRequest(const muduo::net::TcpConnectionPtr& conn, const HttpRequest&);

private:
  muduo::net::TcpServer server_;
  HttpCallback http_callback_;
};

}  // namespace http

#endif  // HTTP_SERVER_H_
