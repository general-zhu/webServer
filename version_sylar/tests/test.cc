#include <iostream>
#include "../sylar/log.h"


int main() {
  sylar::Logger::Ptr logger(new sylar::Logger);
  logger->AddAppender(sylar::LogAppender::Ptr(new sylar::StdoutLogAppender));
  sylar::LogEvent::Ptr event(new sylar::LogEvent(logger, sylar::LogLevel::DEBUG,
      __FILE__, __LINE__, 0, 1, 2, time(0)));
  event->GetSS() << "hello world";
  logger->Log(sylar::LogLevel::DEBUG, event);
  std::cout << "hello\n";
  return 0;
}
