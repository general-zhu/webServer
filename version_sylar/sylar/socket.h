#ifndef SYLAR_SOCKET_H_
#define SYLAR_SOCKET_H_
#pragma once

#include <memory>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "address.h"
#include "noncopyable.h"

namespace sylar {

class Socket : public std::enable_shared_from_this<Socket>, Noncopyable {
 public:
  typedef std::shared_ptr<Socket> Ptr;
  typedef std::weak_ptr<Socket> WeakPtr;

  enum Type {
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM
  };

  enum Family {
    IPv4 = AF_INET,
    IPv6 = AF_INET6,
    UNIX = AF_UNIX,
  };

  static Socket::Ptr CreateTCP(sylar::Address::Ptr address);
  static Socket::Ptr CreateUDP(sylar::Address::Ptr address);
  static Socket::Ptr CreateTCPSocket();
  static Socket::Ptr CreateUDPSocket();
  static Socket::Ptr CreateTCPSocket6();
  static Socket::Ptr CreateUDPSocket6();
  static Socket::Ptr CreateUnixTCPSocet();
  static Socket::Ptr CreateUnixUDPSocket();

  Socket(int family, int type, int protocol = 0);
  virtual ~Socket();

  int64_t GetSendTimeout();
  void SetSendTimeout(int64_t v);
  int64_t GetRecvTimeout();
  void SetRecvTimeout(int64_t v);

  bool GetOption(int level, int option, void *result, socklen_t *len);
  template<class T>
  bool GetOption(int level, int option, T& result) {
    socklen_t length = sizeof(T);
    return GetOption(level, option, &result, &length);

  }

  bool SetOption(int level, int option, const void* result, socklen_t len);
  template<class T>
  bool SetOption(int level, int option, const T& value) {
    return SetOption(level, option, &value, sizeof(T));
  }

  virtual Socket::Ptr Accept();
  virtual bool Bind(const Address::Ptr addr);
  virtual bool Connect(const Address::Ptr addr, uint64_t timeout_ms = -1);
  virtual bool Reconnect(uint64_t timeout_ms = -1);
  virtual bool Listen(int backlog = SOMAXCONN);
  virtual bool Close();

  virtual int Send(const void* buffer, size_t length, int flags = 0);
  virtual int Send(const iovec* buffers, size_t length, int flags = 0);
  virtual int SendTo(const void* buffer, size_t length, const Address::Ptr to, int flags = 0);
  virtual int SendTo(const iovec* buffers, size_t length, const Address::Ptr to, int flags = 0);
  virtual int Recv(void* buffer, size_t length, int flags = 0);
  virtual int Recv(iovec* buffers, size_t length, int flags = 0);
  virtual int RecvFrom(void* buffer, size_t length, Address::Ptr from, int flags = 0);
  virtual int RecvFrom(iovec* buffers, size_t length, Address::Ptr from, int flags = 0);

  Address::Ptr GetRemoteAddress();
  Address::Ptr GetLocalAddress();
  int GetFamily() const { return family_; }
  int GetType() const { return type_; }
  int GetProtocol() const { return protocol_; }
  bool IsConnected() const { return is_connected_; }
  bool IsValid() const;
  int GetError();
  // 输出信息到流中
  virtual std::ostream& Dump(std::ostream& os) const;
  virtual std::string ToString() const;
  int GetSocket() const { return sock_; }
  bool CancelRead();
  bool CancelWrite();
  bool CancelAccept();
  bool CancelAll();

 protected:
  void InitSock();
  void NewSock();
  virtual bool Init(int sock);

 protected:
  int sock_;
  int family_;
  int type_;
  int protocol_;
  bool is_connected_;
  Address::Ptr local_address_;
  Address::Ptr remote_address_;
};
}  // namespace sylar
#endif  // SYLAR_SOCKET_H_
