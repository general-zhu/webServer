#ifndef SYLAR_UTIL_H_
#define SYLAR_UTIL_H_
#pragma once

#include <cxxabi.h>
#include <stdint.h>
#include <unistd.h>
#include <vector>
#include <string>

namespace sylar {

pid_t GetThreadId();
uint32_t GetFiberId();

void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

// 时间ms
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();

std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");

class StringUtil {
 public:
   static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
   static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

   static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
};

template<class T>
const char* TypeToName() {
  static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
  return s_name;
}
}

#endif  // SYLAR_UTIL_H_
