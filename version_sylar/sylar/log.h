#ifndef SYLAR_LOG_H_
#define SYLAR_LOG_H_
#pragma once

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include "util.h"
#include "singleton.h"

#define SYLAR_LOG_LEVEL(logger, level) \
  if (logger->GetLevel() <= level) \
    sylar::LogEventWrap(sylar::LogEvent::Ptr(new sylar::LogEvent(logger, level, \
        __FILE__, __LINE__, 0, sylar::GetThreadId(), \
        sylar::GetFiberId(), time(0)))).GetSS()

#define LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...) \
  if (logger->GetLevel() <= level) \
    sylar::LogEventWrap(sylar::LogEvent::Ptr(new sylar::LogEvent(logger, level, \
        __FILE__, __LINE__, 0, sylar::GetThreadId(), sylar::GetFiberId(), \
        time(0)))).GetEvent()->Format(fmt, __VA_ARGS__)

#define LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define LOG_FMT_INFO(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define LOG_FMT_WARN(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

#define LOG_ROOT() sylar::LoggerMgr::GetInstance()->GetRoot()
#define LOG_NAME(name) sylar::LoggerMgr::GetInstance()->GetLogger(name)

namespace sylar {

class Logger;

// 日志级别
class LogLevel {
 public:
  enum Level {
    UNKNOW = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
  };
  static LogLevel::Level FromString(const std::string& str);
  static const char* ToString(LogLevel::Level level);
};

// 日志事件
class LogEvent {
 public:
  typedef std::shared_ptr<LogEvent> Ptr;

  LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line,
      uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time);
  const char* GetFile() const { return file_; }
  int32_t GetLine() const { return line_; }
  uint32_t GetElapse() const { return elapse_; }
  uint32_t GetThreadId() const { return thread_id_; }
  uint32_t GetFiberId() const { return fiber_id_; }
  uint64_t GetTime() const { return time_; }
  std::string GetContent() const { return ss_.str(); }
  std::stringstream& GetSS() { return ss_; }
  std::shared_ptr<Logger> GetLogger() const { return logger_; }  // 智能指针不能引用传递
  LogLevel::Level GetLevel() const { return level_; }
  void Format(const char *fmt, ...);  // 格式化写入日志内容
  void Format(const char *fmt, va_list al);

 private:
  const char* file_ = nullptr;      // 文件名
  int32_t line_ = 0;                // 行号
  uint32_t elapse_ = 0;             // 程序启动开始到现在的毫秒数
  uint32_t thread_id_ = 0;          // 线程id
  uint32_t fiber_id_ = 0;           // 协程id
  uint64_t time_ = 0;               // 时间戳
  std::stringstream ss_;            // 日志内容流
  std::shared_ptr<Logger> logger_;  // 为了封装LogEventWrap
  LogLevel::Level level_;           // 级别
};

class LogEventWrap {
 public:
  LogEventWrap(LogEvent::Ptr e);
  ~LogEventWrap();
  LogEvent::Ptr GetEvent() const { return event_; }
  std::stringstream& GetSS();

 private:
  LogEvent::Ptr event_;
};


// 日志格式
class LogFormatter {
 public:
  typedef std::shared_ptr<LogFormatter> Ptr;

  LogFormatter(const std::string& pattern);
  // %t %thread_id %m%n
  std::string Format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::Ptr event);

 public:
  class FormatItem {
   public:
    typedef std::shared_ptr<FormatItem> Ptr;
    virtual ~FormatItem() {}
    virtual void Format(std::ostream& os, std::shared_ptr<Logger> logger,
        LogLevel::Level level, LogEvent::Ptr event) = 0;
  };

  void Init();
  bool IsError() const { return error_; }
  const std::string GetPattern() const { return pattern_; }
 private:
  std::string pattern_;
  std::vector<FormatItem::Ptr> items_;
  bool error_ = false;
};

// 日志输出地
class LogAppender {
  friend class Logger;
 public:
  typedef std::shared_ptr<LogAppender> Ptr;

  virtual ~LogAppender() {}
  virtual void Log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::Ptr event) = 0;
  virtual std::string ToYamlString() = 0;
  void SetFormatter(LogFormatter::Ptr val);
  LogFormatter::Ptr GetFormatter() const { return formatter_; }
  LogLevel::Level GetLevel() const { return level_; }
  void SetLevel(LogLevel::Level val) { level_ = val; }

 protected:
  LogLevel::Level level_ = LogLevel::Level::DEBUG;
  LogFormatter::Ptr formatter_;
  bool has_formatter_ = false;
};

// 日志器
class Logger : public std::enable_shared_from_this<Logger> {
  friend class LoggerManager;
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
  void ClearAppenders();
  LogLevel::Level GetLevel() const { return level_; }
  void SetLevel(LogLevel::Level level) { level_ = level; }
  const std::string& GetName() const { return name_; }
  void SetFormatter(LogFormatter::Ptr val);
  void SetFormatter(const std::string& val);
  LogFormatter::Ptr GetFormatter();

  std::string ToYamlString();
 private:
  std::string name_;                       // 日志名称
  LogLevel::Level level_;                  // 日志级别
  std::list<LogAppender::Ptr> appenders_;  // Appender集合
  LogFormatter::Ptr formatter_;            // 日志格式
  Logger::Ptr root_;                       // 主日志器
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<StdoutLogAppender> Ptr;

  void Log(Logger::Ptr logger, LogLevel::Level level, LogEvent::Ptr event) override;
  virtual std::string ToYamlString() override;
};

// 定义输出到文件的Appender
class FileLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<FileLogAppender> Ptr;

  FileLogAppender(const std::string& filename);
  void Log(Logger::Ptr logger, LogLevel::Level level, LogEvent::Ptr event) override;
  // 重新打开文件，文件打开成功返回true
  bool Reopen();
  virtual std::string ToYamlString() override;

 private:
  std::string filename_;
  std::ofstream filestream_;
};

class LoggerManager {
 public:
  LoggerManager();
  Logger::Ptr GetLogger(const std::string& name);
  void Init();
  Logger::Ptr GetRoot() const { return root_; }

  std::string ToYamlString();
 private:
  std::map<std::string, Logger::Ptr> loggers_;
  Logger::Ptr root_;
};

typedef sylar::Singleton<LoggerManager> LoggerMgr;

}  // namespace sylar
#endif  // SYLAR_LOG_H_

