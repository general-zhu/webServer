#ifndef SYLAR_ENDIAN_H_
#define SYLAR_ENDIAN_H_
#pragma once

#define SYLAR_LITTLE_ENDIAN 1
#define SYLAR_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

namespace sylar {

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
Byteswap(T value) {
  return (T)bswap_64((uint64_t)value);
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
Byteswap(T value) {
  return (T)bswap_32((uint32_t)value);
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
Byteswap(T value) {
  return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define SYLAR_BYTE_ORDER SYLAR_BIG_ENDIAN
#else
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN
#endif

#if SYLAR_BYTE_ORDER == SYLAR_BIG_ENDIAN

// 外面以为是小端要转成大端，而原本就是大端所以不变
template<class T>
T ByteswapOnLittleEndian(T t) {
  return t;
}

template<class T>
T ByteswapOnBigEndian(T t) {
  return Byteswap(t);
}

#else

template<class T>
T ByteswapOnLittleEndian(T t) {
  return Byteswap(t);
}

template<class T>
T ByteswapOnBigEndian(T t) {
  return t;
}
#endif

}  // namespace sylar
#endif  // SYLAR_ENDIAN_H_
