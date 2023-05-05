/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2022-12-13 13:08
 */

#include "log.h"
#include <iostream>
#include <functional>
#include <time.h>
#include <stdarg.h>
#include "config.h"

namespace sylar {

const char* LogLevel::ToString(LogLevel::Level level) {
  switch(level) {
#define XX(name) \
  case LogLevel::name: \
    return #name; \
    break;

    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
  default:
    return "UNKNOW";
  }
  return "UNKNOW";
}

LogLevel::Level LogLevel::FromString(const std::string& str) {
#define XX(level, v) \
  if (str == #v) { \
    return LogLevel::level; \
  }
  XX(DEBUG, debug);
  XX(INFO, info);
  XX(WARN, warn);
  XX(ERROR, error);
  XX(FATAL, fatal);

  XX(DEBUG, DEBUG);
  XX(INFO, INFO);
  XX(WARN, WARN);
  XX(ERROR, ERROR);
  XX(FATAL, FATAL);
  return LogLevel::UNKNOW;
#undef XX
}

LogEventWrap::LogEventWrap(LogEvent::Ptr e) : event_(e) {
}

LogEventWrap::~LogEventWrap() {
  event_->GetLogger()->Log(event_->GetLevel(), event_);
}

std::stringstream& LogEventWrap::GetSS() {
  return event_->GetSS();
}

void LogEvent::Format(const char *fmt, ...) {
  va_list al;
  va_start(al, fmt);
  Format(fmt, al);
  va_end(al);
}

void LogEvent::Format(const char *fmt, va_list al) {
  char *buf = nullptr;
  int len = vasprintf(&buf, fmt, al);
  if (len != -1) {
    ss_ << std::string(buf, len);
    free(buf);
  }
}

class MessageFormatItem : public LogFormatter::FormatItem {
 public:
  MessageFormatItem(const std::string& str = "") {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << event->GetContent();
  }
};

class LevelFormatItem : public LogFormatter::FormatItem {
 public:
  LevelFormatItem(const std::string& str = "") {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << LogLevel::ToString(level);
  }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
 public:
  ElapseFormatItem(const std::string& str = "") {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << event->GetElapse();
  }
};

class NameFormatItem : public LogFormatter::FormatItem {
 public:
  NameFormatItem(const std::string& str = "") {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << event->GetLogger()->GetName();
  }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
 public:
  ThreadIdFormatItem(const std::string& str = "") {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << event->GetThreadId();
  }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
 public:
  FiberIdFormatItem(const std::string& str = "") {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << event->GetFiberId();
  }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
 public:
  ThreadNameFormatItem(const std::string& str = "") {}
  void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << event->GetThreadName();
  }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
 public:
  DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S") : formatter_(format) {
    if (formatter_.empty()) {
      formatter_ = "%Y-%m-%d %H:%M:%S";
    }
  }
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    struct tm tm;
    time_t time = event->GetTime();
    localtime_r(&time, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), formatter_.data(), &tm);
    os << buf;
  }
 private:
  std::string formatter_;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
 public:
  FilenameFormatItem(const std::string& str = "") {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << event->GetFile();
  }
};

class LineFormatItem : public LogFormatter::FormatItem {
 public:
  LineFormatItem(const std::string& str = "") {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << event->GetLine();
  }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
 public:
  NewLineFormatItem(const std::string& str = "") {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << std::endl;
  }
};

class StringFormatItem : public LogFormatter::FormatItem {
 public:
  StringFormatItem(const std::string& str = "") : string_(str) {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << string_;
  }
 private:
  std::string string_;
};

class TabFormatItem : public LogFormatter::FormatItem {
 public:
  TabFormatItem(const std::string& str = "") {}
  virtual void Format(std::ostream& os, Logger::Ptr logger,
      LogLevel::Level level, LogEvent::Ptr event) override {
    os << "\t";
  }
};

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
      const char *file, int32_t line, uint32_t elapse, uint32_t thread_id,
      uint32_t fiber_id, uint64_t time, const std::string& thread_name) :
      file_(file),
      line_(line),
      elapse_(elapse),
      thread_id_(thread_id),
      fiber_id_(fiber_id),
      time_(time),
      thread_name_(thread_name),
      logger_(logger),
      level_(level) {

}

Logger::Logger(std::string name)
    : name_(name),
      level_(LogLevel::Level::DEBUG) {
  formatter_.reset(new LogFormatter(
      "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
  // formatter_.reset(new LogFormatter( "%d [%p] <%f:%l>   %m %n"));
}

void Logger::SetFormatter(LogFormatter::Ptr val) {
  MutexType::MutexLock lock(mutex_);
  formatter_ = val;
  for (auto& i : appenders_) {
    MutexType::MutexLock ll(i->mutex_);
    if (!i->has_formatter_) {
      i->formatter_ = formatter_;
    }
  }
}

void Logger::SetFormatter(const std::string& val) {
  std::cout << "---" << val << std::endl;
  sylar::LogFormatter::Ptr new_val((new sylar::LogFormatter(val)));
  if (new_val->IsError()) {
    std::cout << "Logger SetFormatter name=" << name_
        << " value=" << val << " invalid formatter" << std::endl;
    return;
  }
  SetFormatter(new_val);
}

std::string Logger::ToYamlString() {
  MutexType::MutexLock lock(mutex_);
  YAML::Node node;
  node["name"] = name_;
  if (level_ != LogLevel::UNKNOW) {
    node["level"] = LogLevel::ToString(level_);
  }
  if (formatter_) {
    node["formatter"] = formatter_->GetPattern();
  }
  for (auto& i : appenders_) {
    node["appenders"].push_back(YAML::Load(i->ToYamlString()));
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

LogFormatter::Ptr Logger::GetFormatter() {
  MutexType::MutexLock lock(mutex_);
  return formatter_;
}

void Logger::AddAppender(LogAppender::Ptr appender) {
  MutexType::MutexLock lock(mutex_);
  if (!appender->GetFormatter()) {
    // 如果appender传进来的时候没有设置格式就用logger的格式
    // appender->SetFormatter(formatter_);
    // 用友元类这样就不用修改has_formatter_成员变量了
    MutexType::MutexLock ll(appender->mutex_);
    appender->formatter_ = formatter_;
  }
  appenders_.push_back(appender);
}

void Logger::DelAppender(LogAppender::Ptr appender) {
  MutexType::MutexLock lock(mutex_);
  for (auto it = appenders_.begin(); it != appenders_.end(); ++it) {
    if (*it == appender) {
      appenders_.erase(it);
      break;
    }
  }
}

void Logger::ClearAppenders() {
  MutexType::MutexLock lock(mutex_);
  appenders_.clear();
}

void Logger::Log(LogLevel::Level level, LogEvent::Ptr event) {
  if (level >= level_) {
    auto self = shared_from_this();
    MutexType::MutexLock lock(mutex_);
    if (!appenders_.empty()) {
      for (auto& i : appenders_) {
        i->Log(self, level, event);
      }
    } else if (root_) {
      root_->Log(level, event);
    }
  }
}

void Logger::Debug(LogEvent::Ptr event) {
  Log(LogLevel::Level::DEBUG, event);
}

void Logger::Info(LogEvent::Ptr event) {
  Log(LogLevel::Level::INFO, event);
}

void Logger::Warn(LogEvent::Ptr event) {
  Log(LogLevel::Level::WARN, event);
}

void Logger::Error(LogEvent::Ptr event) {
  Log(LogLevel::Level::ERROR, event);
}

void Logger::Fatal(LogEvent::Ptr event) {
  Log(LogLevel::Level::FATAL, event);
}

void LogAppender::SetFormatter(LogFormatter::Ptr val) {
  MutexType::MutexLock lock(mutex_);
  formatter_ = val;
  if (formatter_) {
    has_formatter_ = true;
  } else {
    has_formatter_ = false;
  }
}

LogFormatter::Ptr LogAppender::GetFormatter() {
  MutexType::MutexLock lock(mutex_);
  return formatter_;
}

FileLogAppender::FileLogAppender(const std::string& filename) : filename_(filename) {
  Reopen();
}

void FileLogAppender::Log(std::shared_ptr<Logger> logger,
    LogLevel::Level level, LogEvent::Ptr event) {
  if (level >= level_) {
    uint64_t now = event->GetTime();
    if (now >= (lasttime_ + 3)) {
      Reopen();
      lasttime_ = now;
    }
    MutexType::MutexLock lock(mutex_);
    filestream_ << formatter_->Format(logger, level, event);
  }
}

bool FileLogAppender::Reopen() {
  MutexType::MutexLock lock(mutex_);
  if (filestream_) {
    filestream_.close();
  }
  filestream_.open(filename_);
  return !!filestream_;
}

std::string FileLogAppender::ToYamlString() {
  MutexType::MutexLock lock(mutex_);
  YAML::Node node;
  node["type"] = "FileLogAppender";
  node["file"] = filename_;
  if (level_ != LogLevel::UNKNOW) {
    node["level"] = LogLevel::ToString(level_);
  }
  if (has_formatter_ && formatter_) {
    node["formatter"] = formatter_->GetPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

void StdoutLogAppender::Log(std::shared_ptr<Logger> logger,
    LogLevel::Level level, LogEvent::Ptr event) {
  if (level >= level_) {
    MutexType::MutexLock lock(mutex_);
    std::cout << formatter_->Format(logger, level, event);
  }
}

std::string StdoutLogAppender::ToYamlString() {
  MutexType::MutexLock lock(mutex_);
  YAML::Node node;
  node["type"] = "StdoutLogAppender";
  if (level_ != LogLevel::UNKNOW) {
    node["level"] = LogLevel::ToString(level_);
  }
  if (has_formatter_ && formatter_) {
    node["formatter"] = formatter_->GetPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

LogFormatter::LogFormatter(const std::string& pattern) : pattern_(pattern) {
  Init();
}

std::string LogFormatter::Format(std::shared_ptr<Logger> logger,
      LogLevel::Level level, LogEvent::Ptr event) {
  std::stringstream ss;
  for (auto& item : items_) {
    item->Format(ss, logger, level, event);
  }
  return ss.str();
}

// %xxx %xxx{xxx} %%
// "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
void LogFormatter::Init() {
  // str, format, type
  // %d{%Y-%m-%d} -> (d, %Y-%m-%d, 1) d表示时间(自己定义),
  // 1表示没有问题，0表示错误或者要输出字符串
  std::vector<std::tuple<std::string, std::string, int>> vec;
  // nstr: 要输出的字符串
  std::string nstr;
  for (size_t i = 0; i < pattern_.size(); ++i) {
    if (pattern_[i] != '%') {
      nstr.append(1, pattern_[i]);
      continue;
    }
    // 我们就是要有一个%号的时候，%%->%
    if ((i + 1) < pattern_.size()) { //  %%d, %%Y
      if (pattern_[i + 1] == '%') {
        nstr.append(1, '%');
        continue;
      }
    }
    size_t n = i + 1;
    int fmt_status = 0; // 0为初始状态，1是子格式状态
    size_t fmt_begin = 0;
    std::string str, fmt; // str为要map中的key
    while (n < pattern_.size()) {
      // isalpha 判断是否是字母
      // 上一个str已经结束，不是字母(%nbnhhf )
      if (!fmt_status && (!isalpha(pattern_[n]) && pattern_[n] != '{' && pattern_[n] != '}')) {
        str = pattern_.substr(i + 1, n - i - 1);
        break;
      }
      if (fmt_status == 0) {
        if (pattern_[n] == '{') {
          str = pattern_.substr(i + 1, n - i - 1);
          fmt_status = 1; // 解析子格式
          fmt_begin = n;
          ++n;
          continue;
        }
      } else if (fmt_status == 1) {
        if (pattern_[n] == '}') {
          fmt = pattern_.substr(fmt_begin + 1, n - fmt_begin - 1);
          fmt_status = 0;
          ++n;
          break;
        }
      }
      ++n;
      if (n == pattern_.size()) {
        if (str.empty()) {
          str = pattern_.substr(i + 1);
        }
      }
    }
    if (fmt_status == 0) {
      if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, std::string(), 0));
        nstr.clear();
      }
      vec.push_back(std::make_tuple(str, fmt, 1));
      i = n - 1;
    } else if (fmt_status == 1) {
      std::cout << "patern parse error: " << pattern_ << " - " << pattern_.substr(i) << std::endl;
      error_ = true;
      vec.push_back(std::make_tuple("<<pattern>>", fmt, 0));
    }
  }
  if (!nstr.empty()) {
    vec.push_back(std::make_tuple(nstr, "", 0));
  }
  static std::map<std::string, std::function<FormatItem::Ptr(const std::string& str)>>
      s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::Ptr(new C(fmt));}}
    XX(m, MessageFormatItem),           // m:消息
    XX(p, LevelFormatItem),             // p:日志级别
    XX(r, ElapseFormatItem),            // r:累计毫秒数
    XX(c, NameFormatItem),              // c:日志名称
    XX(t, ThreadIdFormatItem),          // t:线程id
    XX(n, NewLineFormatItem),           // n:换行
    XX(d, DateTimeFormatItem),          // d:时间
    XX(f, FilenameFormatItem),          // f:文件名
    XX(l, LineFormatItem),              // l:行号
    XX(T, TabFormatItem),               // T:Tab
    XX(F, FiberIdFormatItem),           // F:协程id
    XX(N, ThreadNameFormatItem),        // N:线程名称
#undef XX
  };
  for (auto& i : vec) {
    if (std::get<2>(i) == 0) {
        items_.push_back(FormatItem::Ptr(new StringFormatItem(std::get<0>(i))));
    } else {
        auto it = s_format_items.find(std::get<0>(i));
        if (it == s_format_items.end()) {
          items_.push_back(FormatItem::Ptr(new StringFormatItem("<<error format %" + std::get<0>(i) + ">>")));
          error_ = true;
        } else {
          items_.push_back(it->second(std::get<1>(i)));
        }
    }
  }
}

LoggerManager::LoggerManager() {
  root_.reset(new Logger);
  root_->AddAppender(LogAppender::Ptr(new StdoutLogAppender));
  loggers_[root_->name_] = root_;
  Init();
}

Logger::Ptr LoggerManager::GetLogger(const std::string& name) {
  MutexType::MutexLock lock(mutex_);
  auto it = loggers_.find(name);
  if (it != loggers_.end()) {
    return it->second;
  }

  Logger::Ptr logger(new Logger(name));
  logger->root_ = root_;
  loggers_[name] = logger;
  return logger;
}

class LogAppenderDefine {
 public:
  int type_ = 0; // 1 File, 2 Stdout
  LogLevel::Level level_ = LogLevel::UNKNOW;
  std::string formatter_;
  std::string file_;

  bool operator==(const LogAppenderDefine& oth) const {
    return type_ == oth.type_ && level_ == oth.level_ && formatter_ == oth.formatter_
        && file_ == oth.file_;
  }
};

class LogDefine {
 public:
  std::string name_;
  LogLevel::Level level_ = LogLevel::UNKNOW;
  std::string formatter_;
  std::vector<LogAppenderDefine> appenders_;

  bool operator==(const LogDefine& oth) const {
    return name_ == oth.name_ && level_ == oth.level_ && formatter_ == oth.formatter_
        && appenders_ == oth.appenders_;
  }

  bool operator<(const LogDefine& oth) const {
    return name_ < oth.name_;
  }

  bool IsValid() const {
    return !name_.empty();
  }
};

template<>
class LexicalCast<std::string, LogDefine> {
 public:
  LogDefine operator()(const std::string& v) {
    YAML::Node n = YAML::Load(v);
    LogDefine ld;
    if (!n["name"].IsDefined()) {
      std::cout << "log config error: name is null, " << n << std::endl;
      throw std::logic_error("log config name is null");
    }
    ld.name_ = n["name"].as<std::string>();
    ld.level_ = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
    if (n["formatter"].IsDefined()) {
      ld.formatter_ = n["formatter"].as<std::string>();
    }

    if (n["appenders"].IsDefined()) {
      for (size_t x = 0; x < n["appenders"].size(); ++x) {
        auto a = n["appenders"][x];
        if (!a["type"].IsDefined()) {
          std::cout << "log config error: appender type is null, " << a
              << std::endl;
          continue;
        }
        std::string type = a["type"].as<std::string>();
        LogAppenderDefine lad;
        if (type == "FileLogAppender") {
          lad.type_ = 1;
          if (!a["file"].IsDefined()) {
            std::cout << "log config error: fileappender file is null, " << a
                << std::endl;
            continue;
          }
          lad.file_ = a["file"].as<std::string>();
          if (a["formatter"].IsDefined()) {
            lad.formatter_ = a["formatter"].as<std::string>();
          }
        } else if (type == "StdoutLogAppender") {
          lad.type_ = 2;
          if (a["formatter"].IsDefined()) {
            lad.formatter_ = a["formatter"].as<std::string>();
          }
        } else {
          std::cout << "log config error: appender type is invalid, " << a
              << std::endl;
          continue;
        }
        ld.appenders_.push_back(lad);
      }
    }
    return ld;
  }
};

template<>
class LexicalCast<LogDefine, std::string> {
 public:
  std::string operator()(const LogDefine& i) {
    YAML::Node n;
    n["name"] = i.name_;
    if (i.level_ != LogLevel::UNKNOW) {
      n["level"] = LogLevel::ToString(i.level_);
    }
    if (!i.formatter_.empty()) {
      n["formatter"] = i.formatter_;
    }

    for (auto& a : i.appenders_) {
      YAML::Node na;
      if (a.type_ == 1) {
        na["type"] = "FileLogAppender";
        na["file"] = a.file_;
      } else if (a.type_ == 2) {
        na["type"] = "StdoutLogAppender";
      }
      if (a.level_ != LogLevel::UNKNOW) {
        na["level"] = LogLevel::ToString(a.level_);
      }
      if (!a.formatter_.empty()) {
        na["formatter"] = a.formatter_;
      }
      n["appenders"].push_back(na);
    }
    std::stringstream ss;
    ss << n;
    return ss.str();
  }
};

sylar::ConfigVar<std::set<LogDefine>>::Ptr g_log_defines =
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

class LogIniter {
 public:
  LogIniter() {
    g_log_defines->AddListener([](const std::set<LogDefine>& old_value,
        const std::set<LogDefine>& new_value) {
        LOG_INFO(LOG_ROOT()) << "on_logger_conf_changed";
        for (auto& i : new_value) {
          auto it = old_value.find(i);
          sylar::Logger::Ptr logger;
          if (it == old_value.end()) {
            // 新增logger
            logger = LOG_NAME(i.name_);
          } else {
            if (!(i == *it)) {
              // 修改的logger
              logger = LOG_NAME(i.name_);
            } else {
              continue;
            }
          }
          logger->SetLevel(i.level_);
          if (!i.formatter_.empty()) {
            logger->SetFormatter(i.formatter_);
          }

          logger->ClearAppenders();
          for (auto& a : i.appenders_) {
            sylar::LogAppender::Ptr ap;
            if (a.type_ == 1) {
              ap.reset(new FileLogAppender(a.file_));
            } else if (a.type_ == 2) {
              ap.reset(new StdoutLogAppender);
            }
            ap->SetLevel(a.level_);
            if (!a.formatter_.empty()) {
              LogFormatter::Ptr fmt(new LogFormatter(a.formatter_));
              if (!fmt->IsError()) {
                ap->SetFormatter(fmt);
              } else {
                std::cout << "log.name=" << i.name_ << " appender type=" << a.type_
                    << " formatter=" << a.formatter_ << " is invalid" << std::endl;
              }
            }
            logger->AddAppender(ap);
          }
        }

        for (auto& i : old_value) {
          auto it = new_value.find(i);
          if (it == new_value.end()) {
            // 删除logger
            auto logger = LOG_NAME(i.name_);
            logger->SetLevel((LogLevel::Level)0);
            logger->ClearAppenders();
          }
        }
    });
  }
};

static LogIniter __log_init;

std::string LoggerManager::ToYamlString() {
  MutexType::MutexLock lock(mutex_);
  YAML::Node node;
  for (auto& i : loggers_) {
    node.push_back(YAML::Load(i.second->ToYamlString()));
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

void LoggerManager::Init() {
}

}  // namespace sylar

