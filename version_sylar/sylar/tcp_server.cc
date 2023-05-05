/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-04-18 19:54
 */

#include "tcp_server.h"
#include <sstream>
#include "config.h"
#include "log.h"

namespace sylar {

static sylar::ConfigVar<uint64_t>::Ptr g_tcp_server_read_timeout = sylar::Config::Lookup(
    "tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2), "tcp server read timeout");

static sylar::Logger::Ptr g_logger = LOG_NAME("system");


TcpServer::TcpServer(sylar::IOManager* worker, sylar::IOManager* io_worker,
    sylar::IOManager* accept_worker) :
    worker_(worker),
    io_worker_(io_worker),
    accept_worker_(accept_worker),
    recv_timeout_(g_tcp_server_read_timeout->GetValue()),
    name_("sylar/1.0.0"),
    is_stop_(true) {
}

TcpServer:: ~TcpServer() {
  for (auto& i : socks_) {
    i->Close();
  }
  socks_.clear();
}

bool TcpServer::Bind(sylar::Address::Ptr addr, bool ssl) {
  std::vector<Address::Ptr> addrs;
  std::vector<Address::Ptr> fails;
  addrs.push_back(addr);
  return Bind(addrs, fails, ssl);
}

bool TcpServer::Bind(const std::vector<Address::Ptr>& addrs, std::vector<Address::Ptr>& fails,
    bool ssl) {
  ssl_ = ssl;
  for (auto& addr : addrs) {
    Socket::Ptr sock = ssl ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
    if (!sock->Bind(addr)) {
      LOG_ERROR(g_logger) << "bind fail errno=" << errno << " errstr=" << strerror(errno)
          << " addr=[" << addr->ToString() << "]";
      fails.push_back(addr);
      continue;
    }
    if (!sock->Listen()) {
      LOG_ERROR(g_logger) << "listen fail errno=" << errno << " errstr=" << strerror(errno)
          << " addr=[" << addr->ToString() << "]";
      fails.push_back(addr);
      continue;
    }
    socks_.push_back(sock);
  }
  if (!fails.empty()) {
    socks_.clear();
    return false;
  }
  for (auto& i : socks_) {
    LOG_INFO(g_logger) << "type=" << type_ << " name=" << name_ << " ssl=" << ssl_
        << " server bind success: " << *i;
  }
  return true;
}

bool TcpServer::LoadCertificates(const std::string& cert_file, const std::string& key_file) {
  for (auto& i : socks_) {
    auto ssl_socket = std::dynamic_pointer_cast<SSLSocket>(i);
    if (ssl_socket) {
      if (!ssl_socket->LoadCertificates(cert_file, key_file)) {
        return false;
      }
    }
  }
  return true;
}

bool TcpServer::Start() {
  if (!is_stop_) {
    return true;
  }
  is_stop_ = false;
  for (auto& sock : socks_) {
    accept_worker_->Schedule(std::bind(&TcpServer::StartAccept, shared_from_this(), sock));
  }
  return true;
}

void TcpServer::Stop() {
  is_stop_ = true;
  auto self = shared_from_this();
  accept_worker_->Schedule([this, self]() {
      for (auto& sock : socks_) {
        sock->CancelAll();
        sock->Close();
      }
  });
}

void TcpServer::SetConf(const TcpServerConf& v) {
  conf_.reset(new TcpServerConf(v));
}

std::string TcpServer::ToString(const std::string& prefix) {
  std::stringstream ss;
  ss << prefix << "[type=" << type_ << " name=" << name_ << " ssl=" << ssl_
      << " worker=" << (worker_ ? worker_->GetName() : "")
      << " accept=" << (accept_worker_ ? accept_worker_->GetName() : "")
      << " recv_timeout=" << recv_timeout_ << "]" << std::endl;
  std::string pfx = prefix.empty() ? "    " : prefix;
  for (auto& i : socks_) {
    ss << pfx << pfx << *i << std::endl;
  }
  return ss.str();
}

void TcpServer::HandleClient(Socket::Ptr client) {
  LOG_INFO(g_logger) << "HandleClient: " << *client;
}

void TcpServer::StartAccept(Socket::Ptr sock) {
  while (!is_stop_) {
    Socket::Ptr client = sock->Accept();
    if (client) {
      client->SetRecvTimeout(recv_timeout_);
      io_worker_->Schedule(std::bind(&TcpServer::HandleClient, shared_from_this(), client));
    } else {
      LOG_ERROR(g_logger) << "accept errno=" << errno << " errstr=" << strerror(errno);
    }
  }
}

}  // namespace sylar
