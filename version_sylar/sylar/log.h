#ifndef SYLAR_LOG_H_
#define SYLAR_LOG_H_
#pragma once

#include <string>
#include <stdin.h>
#include <memory>
#include <list>
#include <sstrream>
#include <fstream>

namespace sylar {

// 日志事件
class LogEvent {
 public:
  typedef std::shared_ptr<LogEvent> Ptr;
  LogEvent();
  const char* GetFile() const { return file_; }
  int32_t GetLine() const { return line_; }
  uint32_t GetElapse() const { return elapse_; }
  uint32_t GetThreadId() const { return thread_id_; }
  uint32_t GetFiberId() const { return fiber_id_; }
  uint64_t GetTime() const { return time_; }
  const std::string& GetContent() const { return content_; }

 private:
  const char* file_ = nullptr;   // 文件名
  int32_t line_ = 0;             // 行号
  uint32_t elapse_ = 0;          // 程序启动开始到现在的毫秒数
  uint32_t thread_id_ = 0;       // 线程id
  uint32_t fiber_id_ = 0;        // 协程id
  uint64_t time_ = 0;            // 时间戳
  std::string content_;
};

// 日志级别
class LogLevel {
 public:
  enum Level {
    UNKNOW = 0;
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
  };
  static const char* ToString(LogLevel::Level level);
};

// 日志格式
class LogFormatter {
 public:
  typedef std::shared_ptr<LogFormatter> Ptr;

  LogFormatter(std::string& pattern);
  // %t %thread_id %m%n
  std::string Format(LogEvent::Ptr event);

 public:
  class FormatItem {
   public:
     typedef std::shared_ptr<FormatItem> Ptr;
     virtual ~FormatItem() {}
     virtual std::string Format(std::ostream os, LogEvent::Ptr event) = 0;
  };

  void Init();
 private:
  std::string pattern_;
  std::vector<FormatItem::Ptr> items_;
};

// 日志输出地
class LogAppender {
 public:
  typedef std::shared_ptr<LogAppender> Ptr;

  virtual ~LogAppender() {}
  void Log(LogLevel::Level level, LogEvent::Ptr event) = 0;
  void SetFormatter(LogFormatter::Ptr val) { formatter_ = val; }
  LogFormatter::Ptr GetFormatter() const { return formatter_; }

 protected:
  LogLevel::Level level_;
  LogFormatter::Ptr formatter_;
};

// 日志器
class Logger {
 public:
  typedef std::shared_ptr<Logger> Ptr;

  Logger(std::string name = "root");
  void Log(LogLevel::Level level, LogEvent::Ptr event);
  void Debug(LogEvent::Ptr event);
  void Info(LogEvent::Ptr event);
  void Warn(LogEvent::Ptr event);
  void Error(LogEvent::Ptr event);
  void Fatal(LogEvent::Ptr event);
  void AddAppender(LogAppender::Ptr appender);
  void DelAppender(LogAppender::Ptr appender);
  LogLevel::Level GetLevel() const { return level_; }
  void SetLevel(LogLevel::Level level) { level_ = level; }

 private:
  std::string name_;                       // 日志名称
  LogLevel::Level level_;                  // 日志级别
  std::List<LogAppender::Ptr> appenders_;  // Appender集合
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<StdoutLogAppender> Ptr

  void Log(LogLevel::Level level, LogEvent::Ptr event) override;
};

// 定义输出到文件的Appender
class FileLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<FileLogAppender> Ptr;

  FileLogAppender(const std::string& filename);
  void Log(LogLevel::Level level, LogEvent::Ptr event) override;
  // 重新打开文件，文件打开成功返回true
  bool Reopen();

 private:
  std::string filename_;
  std::ofstream filestream_;
};

}  // namespace sylar
#endif  // SYLAR_LOG_H_

