/**
 * Copyright(c) All rights reserved
 * Author: 244553899@qq.com
 * Time:   2022-06-26 22:21
 */

#include "http_server/inspector.h"
#include <vector>
#include <string>
#include <map>
#include "http_server/http_response.h"
#include "http_server/process_inspector.h"
#include "thirdparty/muduo/base/Logging.h"
#include "thirdparty/muduo/net/EventLoop.h"
#include "thirdparty/muduo/net/InetAddress.h"

namespace http {
std::vector<std::string> split(const std::string& str) {
  std::vector<std::string> result;
  size_t start = 0;
  size_t pos = str.find('/');
  while (pos != std::string::npos) {
    if (pos > start) {
      result.push_back(str.substr(start, pos - start));
    }
    start = pos + 1;
    pos = str.find('/', start);
  }
  if (start < str.length()) {
    result.push_back(str.substr(start));
  }
  return result;
}

Inspector::Inspector(muduo::net::EventLoop* loop, const muduo::net::InetAddress& http_addr,
    const std::string& name) :
    server_(loop, http_addr, "Inspector:" + name),
    process_inspector_(new ProcessInspector) {
  server_.SetHttpCallback(std::bind(&Inspector::OnRequest, this, std::placeholders::_1,
      std::placeholders::_2));
  server_.SetServerThreads(3);
  process_inspector_->RegisterCommands(this);
  loop->runAfter(0, std::bind(&Inspector::Start, this));
}

Inspector::~Inspector() = default;

void Inspector::Add(const std::string& module, const std::string& command,
    const Callback& cb, const std::string& help) {
  muduo::MutexLockGuard lock(mutex_);
  modules_[module][command] = cb;
  helps_[module][command] = help;
}

void Inspector::Remove(const std::string& module, const std::string& command) {
  muduo::MutexLockGuard lock(mutex_);
  std::map<std::string, CommandList>::iterator it = modules_.find(module);
  if (it != modules_.end()) {
    it->second.erase(command);
    helps_[module].erase(command);
  }
}

void Inspector::Start() {
  server_.Start();
}

void Inspector::OnRequest(const HttpRequest& req, http::HttpResponse* resp) {
  if (req.GetPath() == "/") {
    std::string result;
    muduo::MutexLockGuard lock(mutex_);
    for (std::map<std::string, HelpList>::const_iterator it = helps_.begin();
        it != helps_.end(); ++it) {
      const HelpList& list = it->second;
      for (const auto& obj : list) {
        result += "/";
        result += it->first;  // module:/proc
        result += "/";
        result += obj.first;  // command:/
        size_t len = it->first.size() + obj.first.size();
        result += std::string(len >= 25 ? 1 : 25 - len, ' ');
        result += obj.second;
        result += "\n";
      }
    }
    resp->SetStatusCode(HttpResponse::HttpStatusCode::k200OK);
    resp->SetStatusMessage("OK");
    resp->SetContentType("text/plain");
    resp->SetBody(result);
  } else {
    std::vector<std::string> result = split(req.GetPath());
    bool ok = false;
    if (result.size() == 0) {
      LOG_DEBUG << req.GetPath();
    } else {
      std::string module = result[0];
      muduo::MutexLockGuard lock(mutex_);
      std::map<std::string, CommandList>::const_iterator comm_map_it = modules_.find(module);
      if (comm_map_it != modules_.end()) {
        std::string command = result[1];
        const CommandList& comm_list = comm_map_it->second;
        CommandList::const_iterator it = comm_list.find(command);
        if (it != comm_list.end()) {
          ArgList argv(result.begin() + 2, result.end());
          if (it->second) {
            resp->SetStatusCode(HttpResponse::HttpStatusCode::k200OK);
            resp->SetStatusMessage("OK");
            resp->SetContentType("text/plain");
            const Callback& cb = it->second;
            resp->SetBody(cb(req.GetMethod(), argv));
            ok = true;
          }
        }
      }
    }
    if (!ok) {
      resp->SetStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
      resp->SetStatusMessage("Not Found");
    }
  }
}
}  // namespace http
