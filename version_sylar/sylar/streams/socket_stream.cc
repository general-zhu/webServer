#include "socket_stream.h"
#include "sylar/util.h"

namespace sylar {

SocketStream::SocketStream(Socket::Ptr sock, bool owner) : socket_(sock), owner_(owner) {
}

SocketStream::~SocketStream() {
  if (owner_ && socket_) {
    socket_->Close();
  }
}

int SocketStream::Read(void* buffer, size_t length) {
  if (!IsConnected()) {
    return -1;
  }
  return socket_->Recv(buffer, length);
}

int SocketStream::Read(ByteArray::Ptr ba, size_t length) {
  if (!IsConnected()) {
    return -1;
  }
  std::vector<iovec> iovs;
  ba->GetWriteBuffers(iovs, length);
  int rt = socket_->Recv(&iovs[0], iovs.size());
  if (rt > 0) {
    ba->SetPosition(ba->GetPosition() + rt);
  }
  return rt;
}

int SocketStream::Write(const void* buffer, size_t length) {
  if (!IsConnected()) {
    return -1;
  }
  return socket_->Send(buffer, length);
}

int SocketStream::Write(ByteArray::Ptr ba, size_t length) {
  if (!IsConnected()) {
    return -1;
  }
  std::vector<iovec> iovs;
  ba->GetReadBuffers(iovs, length);
  int rt = socket_->Send(&iovs[0], iovs.size());
  if (rt > 0) {
    ba->SetPosition(ba->GetPosition() + rt);
  }
  return rt;
}

void SocketStream::Close() {
  if (socket_) {
    socket_->Close();
  }
}

bool SocketStream::IsConnected() const {
  return socket_ && socket_->IsConnected();
}

Address::Ptr SocketStream::GetRemoteAddress() {
  if (socket_) {
    return socket_->GetRemoteAddress();
  }
  return nullptr;
}

Address::Ptr SocketStream::GetLocalAddress() {
  if (socket_) {
    return socket_->GetLocalAddress();
  }
  return nullptr;
}

std::string SocketStream::GetRemoteAddressString() {
  auto addr = GetRemoteAddress();
  if (addr) {
    return addr->ToString();
  }
  return "";
}

std::string SocketStream::GetLocalAddressString() {
  auto addr = GetLocalAddress();
  if (addr) {
    return addr->ToString();
  }
  return "";
}

}  // namespace sylar
