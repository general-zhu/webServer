#include "sylar/hook.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"

sylar::Logger::Ptr g_logger = LOG_ROOT();

void SleepTest() {
  sylar::IOManager iom;
  iom.Schedule([](){
    sleep(2);
    LOG_INFO(g_logger) << "sleep 2";
  });
  iom.Schedule([](){
    sleep(3);
    LOG_INFO(g_logger) << "sleep 3";
  });
  LOG_INFO(g_logger) << "test sleep";
}

int main() {
  SleepTest();
  return 0;
}
