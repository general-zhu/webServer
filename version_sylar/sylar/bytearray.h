#ifndef SYLAR_BYTEARRAY_H_
#define SYLAR_BYTEARRAY_H_
#pragma once

#include <memory>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

namespace sylar {

class ByteArray {
 public:
  typedef std::shared_ptr<ByteArray> Ptr;

  struct Node {
    Node(size_t s);
    Node();
    ~Node();

    char* ptr;
    Node* next;
    size_t size;
  };

  ByteArray(size_t base_size = 4096);
  ~ByteArray();

  void WriteFint8(int8_t value);
  void WriteFuint8(uint8_t value);
  void WriteFint16(int16_t value);
  void WriteFuint16(uint16_t value);
  void WriteFint32(int32_t value);
  void WriteFuint32(uint32_t value);
  void WriteFint64(int64_t value);
  void WriteFuint64(uint64_t value);

  void WriteInt32(int32_t value);
  void WriteUint32(uint32_t value);
  void WriteInt64(int64_t value);
  void WriteUint64(uint64_t value);

  void WriteFloat(float value);
  void WriteDouble(double value);

  // length: int16, data
  void WriteStringF16(const std::string& value);
  void WriteStringF32(const std::string& value);
  void WriteStringF64(const std::string& value);
  // length: varint, data
  void WriteStringVint(const std::string& value);
  void WriteStringWithoutLength(const std::string& value);

  int8_t ReadFint8();
  uint8_t ReadFuint8();
  int16_t ReadFint16();
  uint16_t ReadFuint16();
  int32_t ReadFint32();
  uint32_t ReadFuint32();
  int64_t ReadFint64();
  uint64_t ReadFuint64();

  int32_t ReadInt32();
  uint32_t ReadUint32();
  int64_t ReadInt64();
  uint64_t ReadUint64();
  float ReadFloat();
  double ReadDouble();
  std::string ReadStringF16();
  std::string ReadStringF32();
  std::string ReadStringF64();
  std::string ReadStringVint();

  // 内部操作
  void Clear();
  void Write(const void* buf, size_t size);
  void Read(void* buf, size_t size);
  void Read(void* buf, size_t size, size_t position) const;
  size_t GetPosition() const { return position_; }
  void SetPosition(size_t v);
  bool WriteToFile(const std::string& name) const;
  bool ReadFromFile(const std::string& name);
  size_t GetBaseSize() const { return base_size_; }
  size_t GetReadSize() const { return size_ - position_; }
  bool IsLittleEndian() const;
  void SetIsLittleEndian(bool val);
  std::string ToString() const;
  std::string ToHexString() const;
  uint64_t GetReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;
  uint64_t GetReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;
  uint64_t GetWriteBuffers(std::vector<iovec>& buffers, uint64_t len);
  size_t GetSize() const { return size_; }

 private:
  void AddCapacity(size_t size);
  size_t GetCapacity() const { return capacity_ - position_; }

 private:
  size_t base_size_;
  size_t position_;
  size_t capacity_;
  size_t size_;
  int8_t endian_;
  Node* root_;
  Node* cur_;
};

}  // namespace sylar
#endif  // SYLAR_BYTEARRAY_H_
