#include "util.h"
#include <sys/syscall.h>
#include <unistd.h>

namespace sylar {
pid_t GetThreadId() {
  return syscall(SYS_gettid);
}

uint32_t GetFiberId() {
  // TODO(zhuchun) 目前还没有实现协程
  return 0;
}
}  // sylar
