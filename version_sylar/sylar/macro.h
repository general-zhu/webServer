#ifndef SYLAR_MACRO_H_
#define SYLAR_MACRO_H_
#pragma once

#include <string>
#include <assert.h>
#include "util.h"

#define ASSERT(x) \
  if (!(x)) { \
    LOG_ERROR(LOG_ROOT()) << "ASSERTION: " #x \
        << "\nbacktrace:\n" \
        << sylar::BacktraceToString(100, 2, "    "); \
      assert(x); \
  }

#define ASSERT2(x, w) \
  if (!(x)) { \
    LOG_ERROR(LOG_ROOT()) << "ASSERTION: " #x \
        << "\n" << w \
        << "\nbacktrace:\n" \
        << sylar::BacktraceToString(100, 2, "    "); \
      assert(x); \
  }

#endif  // SYLAR_MACRO_H_
