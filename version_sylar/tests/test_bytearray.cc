#include "sylar/bytearray.h"
#include "sylar/sylar.h"
#include "sylar/log.h"

static sylar::Logger::Ptr g_logger = LOG_ROOT();

void test() {
#define XX(type, len, write_fun, read_fun, base_size) { \
  std::vector<type> vec; \
  for (int i = 0; i < len; ++i) { \
    vec.push_back(rand()); \
  } \
  sylar::ByteArray::Ptr ba(new sylar::ByteArray(base_size)); \
  for (auto& i : vec) { \
    ba->write_fun(i); \
  } \
  ba->SetPosition(0); \
  for (size_t i = 0; i < vec.size(); ++i) { \
    type v = ba->read_fun(); \
    ASSERT(v == vec[i]); \
  } \
  ASSERT(ba->GetReadSize() == 0); \
  LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type ") len=" << len \
    << " base_size=" << base_size << " size=" << ba->GetSize(); \
}

  XX(int8_t, 100, WriteFint8, ReadFint8, 1);
  XX(uint8_t, 100, WriteFuint8, ReadFuint8, 1);
  XX(int16_t, 100, WriteFint16, ReadFint16, 1);
  XX(uint16_t, 100, WriteFuint16, ReadFuint16, 1);
  XX(int32_t, 100, WriteFint32, ReadFint32, 1);
  XX(uint32_t, 100, WriteFuint32, ReadFuint32, 1);
  XX(int64_t, 100, WriteFint64, ReadFint64, 1);
  XX(uint64_t, 100, WriteFuint64, ReadFuint64, 1);

  XX(int32_t, 100, WriteInt32, ReadInt32, 1);
  XX(uint32_t, 100, WriteUint32, ReadUint32, 1);
  XX(int64_t, 100, WriteInt64, ReadInt64, 1);
  XX(uint64_t, 100, WriteUint64, ReadUint64, 1);
#undef XX

#define XX(type, len, write_fun, read_fun, base_size) { \
  std::vector<type> vec; \
  for (int i = 0; i < len; ++i) { \
    vec.push_back(rand()); \
  } \
  sylar::ByteArray::Ptr ba(new sylar::ByteArray(base_size)); \
  for (auto& i : vec) { \
    ba->write_fun(i); \
  } \
  ba->SetPosition(0); \
  for (size_t i = 0; i < vec.size(); ++i) { \
    type v = ba->read_fun(); \
    ASSERT(v == vec[i]); \
  } \
  ASSERT(ba->GetReadSize() == 0); \
  LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type ") len=" << len \
    << " base_size=" << base_size << " size=" << ba->GetSize(); \
  ba->SetPosition(0); \
  ASSERT(ba->WriteToFile("./tmp/" #type "_" #len "_" #read_fun ".dat")); \
  sylar::ByteArray::Ptr ba2(new sylar::ByteArray(base_size * 2)); \
  ASSERT(ba2->ReadFromFile("./tmp/" #type "_" #len "_" #read_fun ".dat")); \
  ba2->SetPosition(0); \
  ASSERT(ba->ToString() == ba2->ToString()); \
  ASSERT(ba->GetPosition() == 0); \
  ASSERT(ba2->GetPosition() == 0); \
}

  XX(int8_t, 100, WriteFint8, ReadFint8, 1);
  XX(uint8_t, 100, WriteFuint8, ReadFuint8, 1);
  XX(int16_t, 100, WriteFint16, ReadFint16, 1);
  XX(uint16_t, 100, WriteFuint16, ReadFuint16, 1);
  XX(int32_t, 100, WriteFint32, ReadFint32, 1);
  XX(uint32_t, 100, WriteFuint32, ReadFuint32, 1);
  XX(int64_t, 100, WriteFint64, ReadFint64, 1);
  XX(uint64_t, 100, WriteFuint64, ReadFuint64, 1);

  XX(int32_t, 100, WriteInt32, ReadInt32, 1);
  XX(uint32_t, 100, WriteUint32, ReadUint32, 1);
  XX(int64_t, 100, WriteInt64, ReadInt64, 1);
  XX(uint64_t, 100, WriteUint64, ReadUint64, 1);
#undef XX
}

int main() {
  test();
  return 0;
}
