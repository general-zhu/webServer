#include <iostream>
#include <thread>
#include "../sylar/log.h"
#include "../sylar/util.h"


int main() {
  sylar::Logger::Ptr logger(new sylar::Logger);
  logger->AddAppender(sylar::LogAppender::Ptr(new sylar::StdoutLogAppender));

  sylar::FileLogAppender::Ptr file_appender(new sylar::FileLogAppender("./log.txt"));
  sylar::LogFormatter::Ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
  file_appender->SetFormatter(fmt);
  file_appender->SetLevel(sylar::LogLevel::ERROR);
  logger->AddAppender(file_appender);

  // sylar::LogEvent::Ptr event(new sylar::LogEvent(logger, sylar::LogLevel::DEBUG,
  //    __FILE__, __LINE__, sylar::GetFiberId(), (int32_t)sylar::GetThreadId(), 2, time(0)));
  // event->GetSS() << "hello world";
  // logger->Log(sylar::LogLevel::DEBUG, event);
  std::cout << "hello\n";
  LOG_INFO(logger) << "test macro";
  LOG_ERROR(logger) << "test macro error";
  LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

  auto l = sylar::LoggerMgr::GetInstance()->GetLogger("xx");
  LOG_INFO(l) << "xxx";
  return 0;
}
