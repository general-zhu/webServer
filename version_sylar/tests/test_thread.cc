#include "sylar/sylar.h"
#include <unistd.h>
#include <vector>

sylar::Logger::Ptr g_logger = LOG_ROOT();

int count = 0;
sylar::RWMutex rw_mutex;
sylar::Mutex mutex;

void func1() {
  LOG_INFO(g_logger) << "name: " << sylar::Thread::GetThreadName()
      << " this.name: " << sylar::Thread::GetThis()->GetName()
      << " id: " << sylar::GetThreadId()
      << " this.id_: " << sylar::Thread::GetThis()->GetId();
  for (int i = 0; i < 100000; ++i) {
    // sylar::RWMutex::WriteLock lock(rwmutex);
    sylar::Mutex::MutexLock loc(mutex);
    ++count;
  }
}

void func2() {
  int i = 100;
  while (i--) {
    LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  }
}

void func3() {
  int i = 100;
  while (i--) {
    LOG_INFO(g_logger) << "==========================================";
  }
}

int main(int argc, char** argv) {
  LOG_INFO(g_logger) << "thread test begin";
  YAML::Node root = YAML::LoadFile("/home/zhuchun/workspace/webServer"
      "/version_sylar/bin/conf/log2.yml");
  sylar::Config::LoadFromYaml(root);
  std::vector<sylar::Thread::Ptr> thrs;
  for (int i = 0; i < 10; ++i) {
    sylar::Thread::Ptr thr(new sylar::Thread(&func2, "name_" + std::to_string(i * 2)));
    sylar::Thread::Ptr thr2(new sylar::Thread(&func3, "name_" + std::to_string(i * 2 + 1)));
    thrs.push_back(thr);
    thrs.push_back(thr2);
  }
  for (size_t i = 0; i < thrs.size(); ++i) {
    thrs[i]->Join();
  }
  LOG_INFO(g_logger) << "thread test end";
  LOG_INFO(g_logger) << "count=" << count;

  return 0;
}
