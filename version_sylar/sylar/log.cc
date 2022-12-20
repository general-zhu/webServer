/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2022-12-13 13:08
 */

#include "log.h"
#include <iostream>
#include <functional>
#include <time.h>

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
    os << logger->GetName();
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

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file,
      int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time)
    : file_(file),
      line_(line),
      elapse_(elapse),
      thread_id_(thread_id),
      fiber_id_(fiber_id),
      time_(time){

}

Logger::Logger(std::string name)
    : name_(name),
      level_(LogLevel::Level::DEBUG) {
  formatter_.reset(new LogFormatter(
      "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
  formatter_.reset(new LogFormatter( "%d [%p] <%f:%l>   %m %n"));
}

void Logger::AddAppender(LogAppender::Ptr appender) {
  if (!appender->GetFormatter()) {
    appender->SetFormatter(formatter_);
  }
  appenders_.push_back(appender);
}

void Logger::DelAppender(LogAppender::Ptr appender) {
  for (auto it = appenders_.begin(); it != appenders_.end(); ++it) {
    if (*it == appender) {
      appenders_.erase(it);
      break;
    }
  }
}

void Logger::Log(LogLevel::Level level, LogEvent::Ptr event) {
  if (level >= level_) {
    auto self = shared_from_this();
    for (auto&  appender : appenders_) {
      appender->Log(self, level, event);
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

FileLogAppender::FileLogAppender(const std::string& filename) : filename_(filename) {
}

void FileLogAppender::Log(std::shared_ptr<Logger> logger,
    LogLevel::Level level, LogEvent::Ptr event) {
  if (level >= level_) {
    filestream_ << formatter_->Format(logger, level, event);
  }
}

bool FileLogAppender::Reopen() {
  if (filestream_) {
    filestream_.close();
  }
  filestream_.open(filename_);
  return !!filestream_;
}

void StdoutLogAppender::Log(std::shared_ptr<Logger> logger,
    LogLevel::Level level, LogEvent::Ptr event) {
  if (level >= level_) {
    std::cout << formatter_->Format(logger, level, event);
  }
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
  // %d{%Y-%m-%d} -> (d, %Y-%m-%d, 1)
  std::vector<std::tuple<std::string, std::string, int>> vec;
  std::string nstr;
  for (size_t i = 0; i < pattern_.size(); ++i) {
    if (pattern_[i] != '%') {
      nstr.append(1, pattern_[i]);
      continue;
    }
    if ((i + 1) < pattern_.size()) { // %%(我猜是这种情况), %%d, %%Y
      if (pattern_[i + 1] == '%') {
        nstr.append(1, '%');
        continue;
      }
    }
    size_t n = i + 1;
    int fmt_status = 0;
    size_t fmt_begin = 0;
    std::string str, fmt;
    while (n < pattern_.size()) {
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
  //  XX(T, TabFormatItem),               // T:Tab
  //  XX(F, FiberIdFormatItem),           // F:协程id
  //  XX(N, ThreadNameFormatItem),        // N:线程名称
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

}  // namespace sylar

