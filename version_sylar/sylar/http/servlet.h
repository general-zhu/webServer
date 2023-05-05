/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-04-24 12:07
 */

#ifndef SYLAR_HTTP_SERVLET_H_
#define SYLAR_HTTP_SERVLET_H_
#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "sylar/thread.h"
#include "sylar/util.h"

namespace sylar {
namespace http {

class Servlet {
 public:
  typedef std::shared_ptr<Servlet> Ptr;

  Servlet(const std::string& name) : name_(name) {}
  virtual ~Servlet() {}
  virtual int32_t Handle(sylar::http::HttpRequest::Ptr request,
                         sylar::http::HttpResponse::Ptr response,
                         sylar::http::HttpSession::Ptr session) = 0;
  const std::string& GetName() const { return name_; }

 protected:
  std::string name_;
};

class FunctionServlet : public Servlet {
 public:
  typedef std::shared_ptr<FunctionServlet> ptr;
  typedef std::function<int32_t (sylar::http::HttpRequest::Ptr request,
                                 sylar::http::HttpResponse::Ptr response,
                                 sylar::http::HttpSession::Ptr session)> Callback;

  FunctionServlet(Callback cb);
  virtual int32_t Handle(sylar::http::HttpRequest::Ptr request,
                         sylar::http::HttpResponse::Ptr response,
                         sylar::http::HttpSession::Ptr session);
 private:
  Callback cb_;
};

class IServletCreator {
 public:
  typedef std::shared_ptr<IServletCreator> Ptr;

  virtual ~IServletCreator() {}
  virtual Servlet::Ptr Get() const = 0;
  virtual std::string GetName() const = 0;
};

class HoldServletCreator : public IServletCreator {
 public:
  typedef std::shared_ptr<HoldServletCreator> Ptr;

  HoldServletCreator(Servlet::Ptr slt) : servlet_(slt) {}

  Servlet::Ptr Get() const override {
    return servlet_;
  }

  std::string GetName() const override {
    return servlet_->GetName();
  }

 private:
  Servlet::Ptr servlet_;
};

template<class T>
class ServletCreator : public IServletCreator {
 public:
  typedef std::shared_ptr<ServletCreator> Ptr;

  ServletCreator() {
  }

  Servlet::Ptr Get() const override {
    return Servlet::Ptr(new T);
  }

  std::string GetName() const override {
    return TypeToName<T>();
  }
};

class ServletDispatch : public Servlet {
 public:
  typedef std::shared_ptr<ServletDispatch> Ptr;
  typedef RWMutex RWMutexType;

  ServletDispatch();
  virtual int32_t Handle(sylar::http::HttpRequest::Ptr request,
                         sylar::http::HttpResponse::Ptr response,
                         sylar::http::HttpSession::Ptr session) override;
  void AddServlet(const std::string& uri, Servlet::Ptr slt);
  void AddServlet(const std::string& uri, FunctionServlet::Callback cb);
  void AddGlobServlet(const std::string& uri, Servlet::Ptr slt);
  void AddGlobServlet(const std::string& uri, FunctionServlet::Callback cb);
  void AddServletCreator(const std::string& uri, IServletCreator::Ptr creator);
  void AddGlobServletCreator(const std::string& uri, IServletCreator::Ptr creator);

  template<class T>
  void AddServletCreator(const std::string& uri) {
    AddServletCreator(uri, std::make_shared<ServletCreator<T>>());
  }

  template<class T>
  void AddGlobServletCreator(const std::string& uri) {
    AddGlobServletCreator(uri, std::make_shared<ServletCreator<T>>());
  }

  void DelServlet(const std::string& uri);
  void DelGlobServlet(const std::string& uri);
  Servlet::Ptr GetDefault() const { return default_; }
  void SetDefault(Servlet::Ptr v) { default_ = v; }
  Servlet::Ptr GetServlet(const std::string& uri);
  Servlet::Ptr GetGlobServlet(const std::string& uri);
  Servlet::Ptr GetMatchedServlet(const std::string& uri);

  void ListAllServletCreator(std::map<std::string, IServletCreator::Ptr>& infos);
  void ListAllGlobServletCreator(std::map<std::string, IServletCreator::Ptr>& infos);

 private:
  RWMutexType mutex_;
  std::unordered_map<std::string, IServletCreator::Ptr> datas_;
  std::vector<std::pair<std::string, IServletCreator::Ptr>> globs_;
  Servlet::Ptr default_;
};

class NotFoundServlet : public Servlet {
 public:
  typedef std::shared_ptr<NotFoundServlet> Ptr;

  NotFoundServlet(const std::string& name);
  virtual int32_t Handle(sylar::http::HttpRequest::Ptr request,
                         sylar::http::HttpResponse::Ptr response,
                         sylar::http::HttpSession::Ptr session) override;

 private:
  std::string name_;
  std::string content_;
};
}  // namespace http
}  // namespace sylar

#endif  // SYLAR_HTTP_SERVLET_H_
