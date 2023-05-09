#include <iostream>
#include <thread>
#include "../sylar/log.h"
#include "../sylar/util.h"


void Log() {
  sylar::Logger::Ptr logger(new sylar::Logger);
  //logger->AddAppender(sylar::LogAppender::Ptr(new sylar::StdoutLogAppender));

  sylar::FileLogAppender::Ptr file_appender(new sylar::FileLogAppender("./log.txt"));
  sylar::LogFormatter::Ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
  file_appender->SetFormatter(fmt);
  //file_appender->SetLevel(sylar::LogLevel::ERROR);
  file_appender->SetLevel(sylar::LogLevel::DEBUG);
  logger->AddAppender(file_appender);

  sylar::LogEvent::Ptr event(new sylar::LogEvent(logger, sylar::LogLevel::DEBUG,
     __FILE__, __LINE__, 0,  sylar::GetThreadId(), sylar::GetFiberId(), time(0), sylar::Thread::GetThreadName()));
  event->GetSS() << "hello world";
  logger->Log(sylar::LogLevel::DEBUG, event);
  std::cout << "hello\n";
  //LOG_INFO(logger) << "test macro";
  LOG_ERROR(logger) << "test macro error";
  //LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

  //  auto l = sylar::LoggerMgr::GetInstance()->GetLogger("xx");
  //  LOG_INFO(l) << "xxx";
}

void FormatTest() {
  std::string str_format = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";
  sylar::LogFormatter::Ptr fmt(new sylar::LogFormatter(str_format));
}

int main() {
  // FormatTest();
  Log();
  return 0;
}
