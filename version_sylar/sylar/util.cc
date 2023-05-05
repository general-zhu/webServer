#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <execinfo.h>
#include <vector>
#include "util.h"
#include "sylar/log.h"
#include "sylar/fiber.h"

namespace sylar {

sylar::Logger::Ptr g_logger = LOG_NAME("system");

pid_t GetThreadId() {
  return syscall(SYS_gettid);
}

uint32_t GetFiberId() {
  return sylar::Fiber::GetFiberId();
}

void Backtrace(std::vector<std::string>& bt, int size, int skip) {
  void **array = (void**)malloc(sizeof(void*) * size);
  size_t s = ::backtrace(array, size);
  char **strings = backtrace_symbols(array, s);
  if (strings == NULL) {
    LOG_ERROR(g_logger) << "backtrace_symbols error";
    return;
  }

  for (size_t i = skip; i < s; ++i) {
    // bt.push_back(demangle(strings[i]));
    bt.push_back(strings[i]);
  }
  free(strings);
  free(array);
}

std::string Time2Str(time_t ts, const std::string& format) {
  struct tm tm;
  localtime_r(&ts, &tm);
  char buf[64];
  strftime(buf, sizeof(buf), format.c_str(), &tm);
  return buf;
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
  std::vector<std::string> bt;
  Backtrace(bt, size, skip);
  std::stringstream ss;
  for (size_t i = 0; i < bt.size(); ++i) {
    ss << prefix << bt[i] << std::endl;
  }
  return ss.str();
}

uint64_t GetCurrentMS() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

uint64_t GetCurrentUS() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

static const char uri_chars[256] = {
    /* 0 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 0, 0, 0, 1, 0, 0,
    /* 64 */
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,
    /* 128 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    /* 192 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};

static const char xdigit_chars[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#define CHAR_IS_UNRESERVED(c) (uri_chars[(unsigned char)(c)])

std::string StringUtil::UrlEncode(const std::string& str, bool space_as_plus) {
  static const char *hexdigits = "0123456789ABCDEF";
  std::string* ss = nullptr;
  const char* end = str.c_str() + str.length();
  for (const char* c = str.c_str(); c < end; ++c) {
    if (!CHAR_IS_UNRESERVED(*c)) {
      if (!ss) {
        ss = new std::string;
        ss->reserve(str.size() * 1.2);
        ss->append(str.c_str(), c - str.c_str());
      }
      if (*c == ' ' && space_as_plus) {
        ss->append(1, '+');
      } else {
        ss->append(1, '%');
        ss->append(1, hexdigits[(uint8_t)*c >> 4]);
        ss->append(1, hexdigits[*c & 0xf]);
      }
    } else if (ss) {
      ss->append(1, *c);
    }
  }
  if (!ss) {
    return str;
  } else {
    std::string rt = *ss;
    delete ss;
    return rt;
  }
}

std::string StringUtil::UrlDecode(const std::string& str, bool space_as_plus) {
  std::string* ss = nullptr;
  const char* end = str.c_str() + str.length();
  for (const char*c = str.c_str(); c < end; ++c) {
    if (*c == '+' && space_as_plus) {
      if (!ss) {
        ss = new std::string;
        ss->append(str.c_str(), c - str.c_str());
      }
      ss->append(1, ' ');
    } else if (*c == '%' && (c + 2) < end && isxdigit(*(c + 1)) && isxdigit(*(c + 2))) {
      if (!ss) {
        ss = new std::string;
        ss->append(str.c_str(), c - str.c_str());
      }
      ss->append(1, (char)(xdigit_chars[(int)*(c + 1)] << 4 | xdigit_chars[(int)*(c + 2)]));
      c += 2;
    } else if (ss) {
      ss->append(1, *c);
    }
  }
  if (!ss) {
    return str;
  } else {
    std::string rt = *ss;
    delete ss;
    return rt;
  }
}

std::string StringUtil::Trim(const std::string& str, const std::string& delimit) {
  auto begin = str.find_first_not_of(delimit);
  if (begin == std::string::npos) {
    return "";
  }
  auto end = str.find_last_not_of(delimit);
  return str.substr(begin, end - begin + 1);
}
}  // sylar
