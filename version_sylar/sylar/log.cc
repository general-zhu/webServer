/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2022-12-13 13:08
 */

#include "log.h"

namespace sylar {

static const char* LogLevel::ToString(LogLevel::Level level) {
#define XX(name) \
}

Logger(std::string name) : name_(name) {
}

void Logger::AddAppender(LogAppender::Ptr appender) {
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
    for (auto&  appender : appenders_) {
      appender->Log(level, event);
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

FileLogAppender::FileLogAppend(const std::string& filename) : filename_(filename) {
  if (level >= level_) {
    filestream_ << formatter.Format(event);
  }
}

void FileLogAppender::Log(LogLevel::Level level, LogEvent::event) {
}

bool FileLogAppender::Reopen() {
  if (filestream_) {
    filestream_.close();
  }
  filestream_.open(filename_);
  return !!filestream_;
}

void StdoutLogAppend::Log(LogLevel::Level level, LogEvent::Ptr event) {
  if (level >= level_) {
    std::cout << formatter.Format(event);
  }
}

LogFormatter::LogFormatter(const std::string& pattern) : pattern_(pattern) {
}

std::string LogFormatter::Format(LogEvent::Ptr event) {
  std::stringstream ss;
  for (auto& item : items) {
    item->Format(ss, event);
  }
  return ss.str();
}

// %xxx %xxx{xxx} %%
// "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
void Init() {
  // str, format, type
  std::vector<std::tuple<std::string, std::string, int>> vec;
  std::string nstr;
  for (size_t i = 0; i < pattern_.size(); ++i) {
    if (pattern[i] != '%') {
      nstr.append(1, pattern_[i]);
      continue;
    }
    if ((i + 1) < pattern_.size()) {
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
    }
  }
}

class MessageFormatItem : public LogFormatter::FormatItem {
 public:
   virtual std::string Format(std::ostream& os, LogEvent::Ptr event) override {
      os << event->GetContent();
   }
};

class LevelFormatItem : public LogFormatter::FormatItem {
 public:
   virtual std::string Format(std::ostream& os, LogEvent::Ptr event) override {
      os << event->GetContent();
   }
};
}  // namespace sylar

