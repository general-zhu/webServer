#ifndef SYLAR_URI_H_
#define SYLAR_URI_H_
#pragma once

#include <memory>
#include <string>
#include <stdint.h>
#include "address.h"

namespace sylar {

/*
      foo://user@sylar.com:8042/over/there?name=ferret#nose
        \_/   \______________/\_________/ \_________/ \__/
         |           |            |            |        |
      scheme     authority       path        query   fragment
*/

class Uri {
 public:
  typedef std::shared_ptr<Uri> Ptr;

  static Uri::Ptr Create(const std::string& uri);

  Uri();
  const std::string& GetScheme() const { return scheme_; }
  const std::string& GetUserinfo() const { return userinfo_;}
  const std::string& GetHost() const { return host_; }
  const std::string& GetPath() const;
  const std::string& GetQuery() const { return query_; }
  const std::string& GetFragment() const { return fragment_; }
  int32_t GetPort() const;
  void SetScheme(const std::string& v) { scheme_ = v; }
  void SetUserinfo(const std::string& v) { userinfo_ = v; }
  void SetHost(const std::string& v) { host_ = v; }
  void SetPath(const std::string& v) { path_ = v; }
  void SetQuery(const std::string& v) { query_ = v; }
  void SetFragment(const std::string& v) { fragment_ = v; }
  void SetPort(int32_t v) { port_ = v; }

  std::ostream& Dump(std::ostream& os) const;
  std::string ToString() const;
  Address::Ptr CreateAddress() const;

 private:
  bool IsDefaultPort() const;

 private:
  std::string scheme_;
  std::string userinfo_;
  std::string host_;
  std::string path_;
  std::string query_;
  std::string fragment_;
  int32_t port_;
};

}  // namespace sylar

#endif  // SYLAR_URI_H_
