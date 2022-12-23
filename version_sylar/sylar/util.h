#ifndef SYLAR_UTIL_H_
#define SYLAR_UTIL_H_
#pragma once

#include <stdint.h>
#include <unistd.h>

namespace sylar {

pid_t GetThreadId();
uint32_t GetFiberId();

}

#endif  // SYLAR_UTIL_H_
