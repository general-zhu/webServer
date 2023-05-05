#ifndef SYLAR_STREAM_H_
#define SYLAR_STREAM_H_
#pragma once

#include <memory>
#include "bytearray.h"

namespace sylar {

class Stream {
 public:
  typedef std::shared_ptr<Stream> Ptr;
  virtual ~Stream() {}
  virtual int Read(void* buffer, size_t length) = 0;
  virtual int Read(ByteArray::Ptr ba, size_t length) = 0;
  virtual int ReadFixSize(void* buffer, size_t length);
  virtual int ReadFixSize(ByteArray::Ptr ba, size_t length);
  virtual int Write(const void* buffer, size_t length) = 0;
  virtual int Write(ByteArray::Ptr ba, size_t length) = 0;
  virtual int WriteFixSize(const void* buffer, size_t length);
  virtual int WriteFixSize(ByteArray::Ptr ba, size_t length);
  virtual void Close() = 0;
};

}  // namespace sylar
#endif  // SYLAR_STREAM_H_
