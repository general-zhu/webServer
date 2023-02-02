#include "sylar/sylar.h"
#include <assert.h>

sylar::Logger::Ptr g_logger = LOG_ROOT();

void test_assert() {
  LOG_INFO(g_logger) << sylar::BacktraceToString(10, 2, "    ");
  ASSERT(false);
}

int main(int argc, char **argv) {
  test_assert();
  return 0;
}
