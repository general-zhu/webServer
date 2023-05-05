#ifndef SYLAR_TCP_SERVER_H_
#define SYLAR_TCP_SERVER_H_
#pragma once

#include <memory>
#include <functional>
#include "iomanager.h"
#include "socket.h"

namespace sylar {

struct TcpServerConf {
  typedef std::shared_ptr<TcpServerConf> Ptr;

  std::vector<std::string> address;
  int keepalive = 0;
  int timeout = 1000 * 2 * 60;
  int ssl = 0;
  std::string id;
  std::string type = "http";
  std::string name;
  std::string cert_file;
  std::string key_file;
  std::string accept_worker;
  std::string io_worker;
  std::string process_worker;
  std::map<std::string, std::string> args;

  bool IsValid() const {
    return !address.empty();
  }

  bool operator==(const TcpServerConf& oth) const {
    return address == oth.address && keepalive == oth.keepalive && timeout == oth.timeout
        && name == oth.name && ssl == oth.ssl && cert_file == oth.cert_file
        && key_file == oth.key_file && accept_worker == oth.accept_worker
        && io_worker == oth.io_worker && process_worker == oth.process_worker
        && args == oth.args && id == oth.id && type == oth.type;
  }
};

class TcpServer : public std::enable_shared_from_this<TcpServer>, Noncopyable {
 public:
  typedef std::shared_ptr<TcpServer> Ptr;

  TcpServer(sylar::IOManager* worker = sylar::IOManager::GetThis(), sylar::IOManager* io_worker =
      sylar::IOManager::GetThis(), sylar::IOManager* accept_worker = sylar::IOManager::GetThis());
  virtual ~TcpServer();
  virtual bool Bind(sylar::Address::Ptr addr, bool ssl = false);
  virtual bool Bind(const std::vector<Address::Ptr>& addrs, std::vector<Address::Ptr>& fails,
      bool ssl = false);
  bool LoadCertificates(const std::string& cert_file, const std::string& key_file);
  virtual bool Start();
  virtual void Stop();
  uint64_t GetRecvTimeout() const { return recv_timeout_; }
  std::string GetName() const { return name_; }
  void SetRecvTimeout(uint64_t v) { recv_timeout_ = v; }
  virtual void SetName(const std::string& v) { name_ = v; }
  bool IsStop() const { return is_stop_; }
  TcpServerConf::Ptr GetConf() const { return conf_; }
  void SetConf(const TcpServerConf& v);
  virtual std::string ToString(const std::string& prefix = "");
  std::vector<Socket::Ptr> GetSocks() const { return socks_; }

 protected:
  virtual void HandleClient(Socket::Ptr client);
  virtual void StartAccept(Socket::Ptr sock);

 protected:
  std::vector<Socket::Ptr> socks_;
  IOManager* worker_;
  IOManager* io_worker_;
  IOManager* accept_worker_;
  uint64_t recv_timeout_;
  std::string name_;
  std::string type_ = "tcp";
  bool is_stop_;
  bool ssl_ = false;
  TcpServerConf::Ptr conf_;
};

}  // namespace sylar
#endif  // SYLAR_TCP_SERVER_H_
