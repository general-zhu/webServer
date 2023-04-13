#ifndef SYLAR_MACRO_H_
#define SYLAR_MACRO_H_
#pragma once

#include <string>
#include <assert.h>
#include "util.h"

#if defined __GNUC__ || defined __llvm__
/// LICKLY 宏的封装，告诉编译器优化，条件大概率成立
#define LIKELY(x)   __builtin_expect(!!(x), 1)
/// LICKLY 宏的封装，告诉编译器优化，条件大概率成立
#define UNLIKELY(x)   __builtin_expect(!!(x), 0)
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x)   (x)
#endif



#define ASSERT(x) \
  if (UNLIKELY(!(x))) { \
    LOG_ERROR(LOG_ROOT()) << "ASSERTION: " #x \
        << "\nbacktrace:\n" \
        << sylar::BacktraceToString(100, 2, "    "); \
      assert(x); \
  }

#define ASSERT2(x, w) \
  if (UNLIKELY(!(x))) { \
    LOG_ERROR(LOG_ROOT()) << "ASSERTION: " #x \
        << "\n" << w \
        << "\nbacktrace:\n" \
        << sylar::BacktraceToString(100, 2, "    "); \
      assert(x); \
  }

#endif  // SYLAR_MACRO_H_
