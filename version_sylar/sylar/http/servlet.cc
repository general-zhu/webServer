#include "servlet.h"
#include <fnmatch.h>

namespace sylar {
namespace http {

FunctionServlet::FunctionServlet(Callback cb) : Servlet("FunctionServlet"), cb_(cb) {
}

int32_t FunctionServlet::Handle(sylar::http::HttpRequest::Ptr request,
                         sylar::http::HttpResponse::Ptr response,
                         sylar::http::HttpSession::Ptr session) {
  return cb_(request, response, session);
}

ServletDispatch::ServletDispatch() : Servlet("ServletDispatch") {
   default_.reset(new NotFoundServlet("sylar/1.0"));
}

int32_t ServletDispatch::Handle(sylar::http::HttpRequest::Ptr request,
                       sylar::http::HttpResponse::Ptr response,
                       sylar::http::HttpSession::Ptr session) {
  auto slt = GetMatchedServlet(request->GetPath());
  if (slt) {
    slt->Handle(request, response, session);
  }
  return 0;
}

void ServletDispatch::AddServlet(const std::string& uri, Servlet::Ptr slt) {
  RWMutexType::WriteLock lock(mutex_);
  datas_[uri] = std::make_shared<HoldServletCreator>(slt);
}

void ServletDispatch::AddServlet(const std::string& uri, FunctionServlet::Callback cb) {
  RWMutexType::WriteLock lock(mutex_);
  datas_[uri] = std::make_shared<HoldServletCreator>(std::make_shared<FunctionServlet>(cb));
}

void ServletDispatch::AddGlobServlet(const std::string& uri, Servlet::Ptr slt) {
  RWMutexType::WriteLock lock(mutex_);
  for (auto it = globs_.begin(); it != globs_.end(); ++it) {
    if (it->first == uri) {
      globs_.erase(it);
      break;
    }
  }
  globs_.push_back(std::make_pair(uri, std::make_shared<HoldServletCreator>(slt)));
}

void ServletDispatch::AddGlobServlet(const std::string& uri, FunctionServlet::Callback cb) {
  return AddGlobServlet(uri, std::make_shared<FunctionServlet>(cb));
}

void ServletDispatch::AddServletCreator(const std::string& uri, IServletCreator::Ptr creator) {
  RWMutexType::WriteLock lock(mutex_);
  datas_[uri] = creator;
}

void ServletDispatch::AddGlobServletCreator(const std::string& uri, IServletCreator::Ptr creator) {
  RWMutexType::WriteLock lock(mutex_);
  for (auto it = globs_.begin(); it != globs_.end(); ++it) {
    if (it->first == uri) {
      globs_.erase(it);
      break;
    }
  }
  globs_.push_back(std::make_pair(uri, creator));
}

void ServletDispatch::DelServlet(const std::string& uri) {
  RWMutexType::WriteLock lock(mutex_);
  datas_.erase(uri);
}

void ServletDispatch::DelGlobServlet(const std::string& uri) {
  RWMutexType::WriteLock lock(mutex_);
  for (auto it = globs_.begin(); it != globs_.end(); ++it) {
    if (it->first == uri) {
      globs_.erase(it);
      break;
    }
  }
}

Servlet::Ptr ServletDispatch::GetServlet(const std::string& uri) {
  RWMutexType::ReadLock lock(mutex_);
  auto it = datas_.find(uri);
  return it == datas_.end() ? nullptr : it->second->Get();
}

Servlet::Ptr ServletDispatch::GetGlobServlet(const std::string& uri) {
  RWMutexType::ReadLock lock(mutex_);
  for (auto it = globs_.begin(); it != globs_.end(); ++it) {
    if (it->first == uri) {
      return it->second->Get();
    }
  }
  return nullptr;
}

Servlet::Ptr ServletDispatch::GetMatchedServlet(const std::string& uri) {
  RWMutexType::ReadLock lock(mutex_);
  auto mit = datas_.find(uri);
  if (mit != datas_.end()) {
    return mit->second->Get();
  }
  for (auto it = globs_.begin(); it != globs_.end(); ++it) {
    if (!fnmatch(it->first.c_str(), uri.c_str(), 0)) {
      return it->second->Get();
    }
  }
  return default_;
}

void ServletDispatch::ListAllServletCreator(std::map<std::string, IServletCreator::Ptr>& infos) {
  RWMutexType::ReadLock lock(mutex_);
  for (auto& i : datas_) {
    infos[i.first] = i.second;
  }
}

void ServletDispatch::ListAllGlobServletCreator(std::map<std::string, IServletCreator::Ptr>& infos) {
  RWMutexType::ReadLock lock(mutex_);
  for (auto& i : globs_) {
    infos[i.first] = i.second;
  }
}

NotFoundServlet::NotFoundServlet(const std::string& name) : Servlet("NotFoundServlet"), name_(name) {
  content_ = "<html><head><title>404 Not Found </title></head>"
      "<body><center><h1>404 Not Found</h1></center><hr><center>" + name + "</center></body></html>";
}

int32_t NotFoundServlet::Handle(sylar::http::HttpRequest::Ptr request,
                       sylar::http::HttpResponse::Ptr response,
                       sylar::http::HttpSession::Ptr session) {
  response->SetStatus(sylar::http::HttpStatus::NOT_FOUND);
  response->SetHeader("Server", "sylar/1.0.0");
  response->SetHeader("Content-Type", "text/html");
  response->SetBody(content_);
  return 0;
}

}  // namespace http
}  // namespace sylar
