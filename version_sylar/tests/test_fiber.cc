#include "sylar/sylar.h"

sylar::Logger::Ptr g_logger = LOG_ROOT();

void run_in_fiber() {
  LOG_INFO(g_logger) << "run_in_fiber begin";
  sylar::Fiber::YieldToHold();
  LOG_INFO(g_logger) << "run_in_fiber end";
  sylar::Fiber::YieldToHold();
}

void FiberTest() {
  {
    sylar::Fiber::GetThis();
    LOG_INFO(g_logger) << "main begin";
    sylar::Fiber::Ptr fiber(new sylar::Fiber(run_in_fiber));
    fiber->SwapIn();
    LOG_INFO(g_logger) << "main after SwapIn";
    fiber->SwapIn();
    LOG_INFO(g_logger) << "main after end";
    fiber->SwapIn();
  }
  LOG_INFO(g_logger) << "main after end2";
}

void SchedulerFiberTest() {
  sylar::Thread::SetName("main");

  std::vector<sylar::Thread::Ptr> thrs;
  for (int i = 0; i < 3; ++i) {
    thrs.push_back(sylar::Thread::Ptr(
        new sylar::Thread(&FiberTest, "name_" + std::to_string(i))));
  }
  for (auto i : thrs) {
    i->Join();
  }
}

void RunInFiber() {
  LOG_INFO(g_logger) << "run_in_fiber\n";
  // sylar::Fiber::YieldToHold();
  // sylar::Fiber::YieldToHold();
}

void TestFiber() {
  {
    sylar::Fiber::GetThis();
    LOG_INFO(g_logger) << "main begin";
    sylar::Fiber::Ptr fiber(new sylar::Fiber(RunInFiber, 0, true));
    fiber->Call();
  }
  LOG_INFO(g_logger) << "main after end2";
}

void UseFiberTest() {
  sylar::Thread::Ptr th(new sylar::Thread(&TestFiber, "main"));
  th->Join();
}
int main(int argc, char **argv) {
  // SchedulerFiberTest();
  //
  UseFiberTest();
  return 0;
}
