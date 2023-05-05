/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-04-22 09:55
 */

#include "stream.h"

namespace sylar {

int Stream::ReadFixSize(void* buffer, size_t length) {
  size_t offset = 0;
  int64_t left = length;
  while (left > 0) {
    int64_t len = Read((char*)buffer + offset, left);
    if (len <= 0) {
      return len;
    }
    offset += len;
    left -= len;
  }
  return length;
}

int Stream::ReadFixSize(ByteArray::Ptr ba, size_t length) {
  int64_t left = length;
  while (left > 0) {
    int64_t len = Read(ba, left);
    if (len <= 0) {
      return len;
    }
    left -= len;
  }
  return length;
}

int Stream::WriteFixSize(const void* buffer, size_t length) {
  size_t offset = 0;
  int64_t left = length;
  while (left > 0) {
    int64_t len = Write((char*)buffer + offset, left);
    if (len <= 0) {
      return len;
    }
    offset += len;
    left -= len;
  }
  return length;
}

int Stream::WriteFixSize(ByteArray::Ptr ba, size_t length) {
  int64_t left = length;
  while (left > 0) {
    int64_t len = Write(ba, left);
    if (len <= 0) {
      return len;
    }
    left -= len;
  }
  return length;
}

}  // namespace sylar
