#ifndef SYLAR_STREAMS_SOCKET_STREAM_H_
#define SYLAR_STREAMS_SOCKET_STREAM_H_
#pragma once

#include "sylar/stream.h"
#include "sylar/socket.h"
#include "sylar/mutex.h"
#include "sylar/iomanager.h"

namespace sylar {

class SocketStream : public Stream {
 public:
  typedef std::shared_ptr<SocketStream> Ptr;

  SocketStream(Socket::Ptr sock, bool owner = true);
  ~SocketStream();
  virtual int Read(void* buffer, size_t length) override;
  virtual int Read(ByteArray::Ptr ba, size_t length) override;
  virtual int Write(const void* buffer, size_t length) override;
  virtual int Write(ByteArray::Ptr ba, size_t length) override;
  virtual void Close() override;
  Socket::Ptr GetSocket() const { return socket_; }
  bool IsConnected() const;
  Address::Ptr GetRemoteAddress();
  Address::Ptr GetLocalAddress();
  std::string GetRemoteAddressString();
  std::string GetLocalAddressString();

 protected:
  Socket::Ptr socket_;
  bool owner_;
};

}  // namespace sylar

#endif  // SYLAR_STREAMS_SOCKET_STREAM_H_
