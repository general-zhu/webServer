#ifndef SYLAR_UTIL_H_
#define SYLAR_UTIL_H_
#pragma once

#include <stdint.h>
#include <unistd.h>
#include <vector>
#include <string>

namespace sylar {

pid_t GetThreadId();
uint32_t GetFiberId();

void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

}

#endif  // SYLAR_UTIL_H_
