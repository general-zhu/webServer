#include "sylar/sylar.h"

static sylar::Logger::Ptr g_logger = LOG_ROOT();

void FiberTest() {
  static int s_count = 5;
  LOG_INFO(g_logger) << " test in fiber s_count=" << s_count;
  sleep(1);
  if (--s_count) {
    sylar::Scheduler::GetThis()->Schedule(&FiberTest);
  }
}

void SchedulerTest() {
  LOG_INFO(g_logger) << "main";
  sylar::Scheduler sc(3, true, "test");
  sc.Start();
  // sleep(2);
  LOG_INFO(g_logger) << "scheduler";
  sc.Schedule(&FiberTest);
  sc.Stop();
  LOG_INFO(g_logger) << "over";
}

int main(int argc, char **argv) {
  sylar::Scheduler sc(3, true, "test");
 // SchedulerTest();
  return 0;
}
