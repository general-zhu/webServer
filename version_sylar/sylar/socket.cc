/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-04-04 14:15
 */

#include "sylar/socket.h"
#include "iomanager.h"
#include "fd_manager.h"
#include "log.h"
#include "macro.h"
#include "hook.h"
#include <limits.h>

namespace sylar {

static sylar::Logger::Ptr g_logger = LOG_NAME("system");

Socket::Ptr Socket::CreateTCP(sylar::Address::Ptr address) {
  Socket::Ptr sock(new Socket(address->GetFamily(), TCP,0));
  return sock;
}

Socket::Ptr Socket::CreateUDP(sylar::Address::Ptr address) {
  Socket::Ptr sock(new Socket(address->GetFamily(), UDP,0));
  sock->NewSock();
  sock->is_connected_ = true;
  return sock;
}

Socket::Ptr Socket::CreateTCPSocket() {
  Socket::Ptr sock(new Socket(IPv4, TCP, 0));
  return sock;
}

Socket::Ptr Socket::CreateUDPSocket() {
  Socket::Ptr sock(new Socket(IPv4, UDP, 0));
  sock->NewSock();
  sock->is_connected_ = true;
  return sock;
}

Socket::Ptr Socket::CreateTCPSocket6() {
  Socket::Ptr sock(new Socket(IPv6, TCP, 0));
  return sock;
}

Socket::Ptr Socket::CreateUDPSocket6() {
  Socket::Ptr sock(new Socket(IPv6, UDP, 0));
  sock->NewSock();
  sock->is_connected_ = true;
  return sock;
}

Socket::Ptr Socket::CreateUnixTCPSocet() {
  Socket::Ptr sock(new Socket(UNIX, TCP, 0));
  return sock;
}

Socket::Ptr Socket::CreateUnixUDPSocket() {
   Socket::Ptr sock(new Socket(UNIX, UDP, 0));
  return sock;
}

Socket::Socket(int family, int type, int protocol) :
    sock_(-1),
    family_(family),
    type_(type),
    protocol_(protocol),
    is_connected_(false) {
}

Socket::~Socket() {
  Close();
}

int64_t Socket::GetSendTimeout() {
  FdCtx::Ptr ctx = FdMgr::GetInstance()->Get(sock_);
  if (!ctx) {
    return ctx->GetTimeout(SO_SNDTIMEO);
  }
  return -1;
}

void Socket::SetSendTimeout(int64_t v) {
  struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
  SetOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

int64_t Socket::GetRecvTimeout() {
  FdCtx::Ptr ctx = FdMgr::GetInstance()->Get(sock_);
  if (ctx) {
    return ctx->GetTimeout(SO_RCVTIMEO);
  }
  return -1;
}
void Socket::SetRecvTimeout(int64_t v) {
  struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
  SetOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}

bool Socket::GetOption(int level, int option, void *result, socklen_t *len) {
  int rt = getsockopt(sock_, level, option, result, (socklen_t*)len);
  if (rt) {
    LOG_DEBUG(g_logger) << "GetOption sock=" << sock_ << " level=" << level << " option="
        << option << " errno=" << errno << " errstr=" << strerror(errno);
    return false;
  }
  return true;
}

bool Socket::SetOption(int level, int option, const void* result, socklen_t len) {
  if (setsockopt(sock_, level, option, result, (socklen_t)len)) {
    LOG_DEBUG(g_logger) << "SetOption sock=" << sock_ << " level=" << level << " option="
        << option << " errno=" << errno << " errstr=" << strerror(errno);
    return false;
  }
  return true;
}

Socket::Ptr Socket::Accept() {
  Socket::Ptr sock(new Socket(family_, type_, protocol_));
  int newsock = ::accept(sock_, nullptr, nullptr);
  if (newsock == -1) {
    LOG_ERROR(g_logger) << "accept(" << sock_ << ") errno="
        << errno << " errstr=" << strerror(errno);
    return nullptr;
  }
  if (sock->Init(newsock)) {
    return sock;
  }
  return nullptr;
}

bool Socket::Bind(const Address::Ptr addr) {
  if(!IsValid()) {
    NewSock();
    if (UNLIKELY(!IsValid())) {
      return false;
    }
  }
  if (UNLIKELY(addr->GetFamily() != family_)) {
    LOG_ERROR(g_logger) << "bind sock.family(" << family_ << ") addr.family("
        << addr->GetFamily() << ") not equal, addr=" << addr->ToString();
    return false;
  }
  UnixAddress::Ptr uaddr = std::dynamic_pointer_cast<UnixAddress>(addr);
  if (uaddr) {
    Socket::Ptr sock = Socket::CreateUnixTCPSocet();
    if (sock->Connect(uaddr)) {
      return false;
    } else {
      // syalr::FSUtil::Unlink(uaddr->GetPath(), true);
    }
  }
  if (::bind(sock_, addr->GetAddr(), addr->GetAddrLen())) {
    LOG_ERROR(g_logger) << "bind error errno=" << errno << " errstr=" << strerror(errno);
    return false;
  }
  GetLocalAddress();
  return true;
}

bool Socket::Connect(const Address::Ptr addr, uint64_t timeout_ms) {
  remote_address_ = addr;
  if (!IsValid()) {
    NewSock();
    if (UNLIKELY(!IsValid())) {
      return false;
    }
  }
  if (UNLIKELY(addr->GetFamily() != family_)) {
    LOG_ERROR(g_logger) << "connect sock.family(" << family_ << ") addr.family("
        << addr->GetFamily() << ") not equal, addr=" << addr->ToString();
    return false;
  }
  if (timeout_ms == (uint64_t)-1) {
    if (::connect(sock_, addr->GetAddr(), addr->GetAddrLen())) {
      LOG_ERROR(g_logger) << "sock=" << sock_ << " connect(" << addr->ToString()
          << ") error errno=" << errno << " errstr=" << strerror(errno);
      Close();
      return false;
    }
  }
  is_connected_ = true;
  GetRemoteAddress();
  GetLocalAddress();
  return true;
}

bool Socket::Reconnect(uint64_t timeout_ms) {
  if (!remote_address_) {
    LOG_ERROR(g_logger) << "reconnect remote_address is null";
    return false;
  }
  local_address_.reset();
  return Connect(remote_address_, timeout_ms);
}

bool Socket::Listen(int backlog) {
  if (!IsValid()) {
    LOG_ERROR(g_logger) << "listen error sock=-1";
    return false;
  }
  if (::listen(sock_, backlog)) {
    LOG_ERROR(g_logger) << "listen error errno=" << errno << " errstr=" << strerror(errno);
    return false;
  }
  return true;
}

bool Socket::Close() {
  if (!is_connected_ && sock_ == -1) {
    return true;
  }
  is_connected_ = false;
  if (sock_ != -1) {
    ::close(sock_);
    sock_ = -1;
    return true;
  }
  return false;
}

int Socket::Send(const void* buffer, size_t length, int flags) {
  if (IsConnected()) {
    return ::send(sock_, buffer, length, flags);
  }
  return -1;
}

int Socket::Send(const iovec* buffers, size_t length, int flags) {
  if (IsConnected()) {
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    return ::sendmsg(sock_, &msg, flags);
  }
  return -1;
}

int Socket::SendTo(const void* buffer, size_t length, const Address::Ptr to, int flags) {
  if (IsConnected()) {
    return ::sendto(sock_, buffer, length, flags, to->GetAddr(), to->GetAddrLen());
  }
  return -1;
}

int Socket::SendTo(const iovec* buffers, size_t length, const Address::Ptr to, int flags) {
  if (IsConnected()) {
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    msg.msg_name = to->GetAddr();
    msg.msg_namelen = to->GetAddrLen();
    return ::sendmsg(sock_, &msg, flags);
  }
  return -1;
}

int Socket::Recv(void* buffer, size_t length, int flags) {
  if (IsConnected()) {
    return ::recv(sock_, buffer, length, flags);
  }
  return -1;
}

int Socket::Recv(iovec* buffers, size_t length, int flags) {
  if (IsConnected()) {
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    return ::recvmsg(sock_, &msg, flags);
  }
  return -1;
}

int Socket::RecvFrom(void* buffer, size_t length, Address::Ptr from, int flags) {
  if (IsConnected()) {
    socklen_t len = from->GetAddrLen();
    return ::recvfrom(sock_, buffer, length, flags, from->GetAddr(), &len);
  }
  return -1;
}

int Socket::RecvFrom(iovec* buffers, size_t length, Address::Ptr from, int flags) {
  if (IsConnected()) {
    msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = (iovec*)buffers;
    msg.msg_iovlen = length;
    msg.msg_name = from->GetAddr();
    msg.msg_namelen = from->GetAddrLen();
    return ::recvmsg(sock_, &msg, flags);
  }
  return -1;
}

Address::Ptr Socket::GetRemoteAddress() {
  if (remote_address_) {
    return remote_address_;
  }
  Address::Ptr result;
  switch(family_) {
    case AF_INET:
      result.reset(new IPv4Address());
      break;
    case AF_INET6:
      result.reset(new IPv6Address());
      break;
    case AF_UNIX:
      result.reset(new UnixAddress());
      break;
    default:
      result.reset(new UnknownAddress(family_));
      break;
  }
  socklen_t addrlen = result->GetAddrLen();
  if (getpeername(sock_, result->GetAddr(), &addrlen)) {
    return Address::Ptr(new UnknownAddress(family_));
  }
  if (family_ == AF_UNIX) {
    UnixAddress::Ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
    addr->SetAddrLen(addrlen);
  }
  remote_address_ = result;
  return remote_address_;
}

Address::Ptr Socket::GetLocalAddress() {
  if (local_address_) {
    return local_address_;
  }
  Address::Ptr result;
  switch (family_) {
    case AF_INET:
      result.reset(new IPv4Address());
      break;
    case AF_INET6:
      result.reset(new IPv6Address());
      break;
    case AF_UNIX:
      result.reset(new UnknownAddress(family_));
      break;
  }
  socklen_t addrlen = result->GetAddrLen();
  if (getsockname(sock_, result->GetAddr(), &addrlen)) {
    LOG_ERROR(g_logger) << " getsockname error sock=" << sock_ << " errno=" << errno
        << " errstr=" << strerror(errno);
    return Address::Ptr(new UnknownAddress(family_));
  }
  if (family_ == AF_UNIX) {
    UnixAddress::Ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
    addr->SetAddrLen(addrlen);
  }
  local_address_ = result;
  return local_address_;
}

bool Socket::IsValid() const {
  return sock_ != -1;
}

int Socket::GetError() {
  int error = 0;
  socklen_t len = sizeof(error);
  if (!GetOption(SOL_SOCKET, SO_ERROR, &errno, &len)) {
    error = errno;
  }
  return error;
}

std::ostream& Socket::Dump(std::ostream& os) const {
  os << "[Socket sock=" << sock_ << " is_connected=" << is_connected_ << " family=" << family_
      << " type=" << type_ << " protocol=" << protocol_;
  if (local_address_) {
    os << " local_address=" << local_address_->ToString();
  }
  if (remote_address_) {
    os << " remote_address=" << remote_address_->ToString();
  }
  os << "]";
  return os;
}

std::string Socket::ToString() const {
  std::stringstream ss;
  Dump(ss);
  return ss.str();
}

bool Socket::CancelRead() {
  return IOManager::GetThis()->CancelEvent(sock_, sylar::IOManager::READ);
}

bool Socket::CancelWrite() {
  return IOManager::GetThis()->CancelEvent(sock_, sylar::IOManager::WRITE);
}

bool Socket::CancelAccept() {
  return IOManager::GetThis()->CancelEvent(sock_, sylar::IOManager::READ);
}

bool Socket::CancelAll() {
  return IOManager::GetThis()->CancelAll(sock_);
}

void Socket::InitSock() {
  int val = 1;
  SetOption(SOL_SOCKET, SO_REUSEADDR, val);
  if (type_ == SOCK_STREAM) {
    SetOption(IPPROTO_TCP, TCP_NODELAY, val);
  }
}

void Socket::NewSock() {
  sock_ = socket(family_, type_, protocol_);
  if (LIKELY(sock_ != -1)) {
    InitSock();
  } else {
    LOG_ERROR(g_logger) << "socket(" << family_ << ", " << type_ << ", " << protocol_
        << ") errno=" << errno << " errstr=" << strerror(errno);
  }
}

bool Socket::Init(int sock) {
  FdCtx::Ptr ctx = FdMgr::GetInstance()->Get(sock);
  if (ctx && ctx->IsSocket() && !ctx->IsClose()) {
    sock_ = sock;
    is_connected_ = true;
    InitSock();
    GetLocalAddress();
    GetRemoteAddress();
    return true;
  }
  return false;
}

}  // namespace syalr
