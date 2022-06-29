#ifndef HTTP_SERVER_INSPECTOR_H_
#define HTTP_SERVER_INSPECTOR_H_
#pragma once

#include <string>
#include <map>
#include <vector>
// #include "http_server/http_response.h"
#include "http_server/http_server.h"
#include "http_server/http_request.h"
#include "thirdparty/muduo/base/Mutex.h"

namespace muduo {
class MutexLock;

namespace net {
class EventLoop;
class InetAddress;

}  // namespace net
}  // namespace muduo

namespace http {
class ProcessInspector;
class HttpResponse;

class Inspector {
 public:
  typedef std::vector<std::string> ArgList;
  typedef std::function<std::string (HttpRequest::Method, const ArgList& args)> Callback;
  Inspector (muduo::net::EventLoop* loop, const muduo::net::InetAddress& http_addr,
      const std::string& name);
  ~Inspector();
  // mudule:/proc command:pid cb:
  void Add(const std::string& module, const std::string& command,
      const Callback& cb, const std::string& help);
  void Remove(const std::string& mudule, const std::string& command);

 private:
  typedef std::map<std::string, Callback> CommandList;
  typedef std::map<std::string, std::string> HelpList;

  void Start();
  void OnRequest(const HttpRequest& req, HttpResponse* resp);

  http::HttpServer server_;
  std::unique_ptr<ProcessInspector> process_inspector_;
  // ProcessInspector* process_inspector_;
  muduo::MutexLock mutex_;
  std::map<std::string, CommandList> modules_;
  std::map<std::string, HelpList> helps_;
};
}  // namespace http
#endif  // HTTP_SERVER_INSPECTOR_H_
