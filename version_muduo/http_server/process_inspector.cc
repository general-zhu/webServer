/**
 * Copyright(c) All rights reserved
 * Author: 244553899@qq.com
 * Time:   2022-06-27 21:02
 */

#include "http_server/process_inspector.h"
#include "thirdparty/muduo/base/ProcessInfo.h"
#include "thirdparty/muduo/base/FileUtil.h"

namespace http {
std::string ProcessInspector::username_ = muduo::ProcessInfo::username();

muduo::StringPiece next(muduo::StringPiece data) {
  const char* sp = static_cast<const char*>(::memchr(data.data(), ' ', data.size()));
  if (sp) {
    data.remove_prefix(static_cast<int>(sp + 1 - data.begin()));
    return data;
  }
  return "";
}

muduo::ProcessInfo::CpuTime GetCpuTime(muduo::StringPiece data) {
  muduo::ProcessInfo::CpuTime t;
  for (int i = 0; i < 10; ++i) {
    data = next(data);
  }
  long utime = strtol(data.data(), NULL, 10);
  data = next(data);
  long stime = strtol(data.data(), NULL, 10);
  const double hz = static_cast<double>(muduo::ProcessInfo::clockTicksPerSecond());
  t.userSeconds = static_cast<double>(utime) / hz;
  t.systemSeconds = static_cast<double>(stime) / hz;
  return t;
}

void ProcessInspector::RegisterCommands(Inspector* ins) {
  // TODO(zhuchun)
  // ins->Add("proc", "overview", ProcessInspector::Overview, "print basic overview.");
  ins->Add("proc", "pid", ProcessInspector::Pid, "print pid.");
  ins->Add("proc", "status", ProcessInspector::ProcStatus, "print /proc/self/status");
  ins->Add("proc", "threads", ProcessInspector::Threads, "list /proc/self/task.");
}

std::string ProcessInspector::Overview(HttpRequest::Method, const Inspector::ArgList&) {
  // TODO(zhuchun)
  std::string result;
  result.reserve(1024);
  muduo::Timestamp now = muduo::Timestamp::now();
  return "";
}

std::string ProcessInspector::Pid(HttpRequest::Method, const Inspector::ArgList&) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", muduo::ProcessInfo::pid());
  return buf;
}

std::string ProcessInspector::ProcStatus(HttpRequest::Method, const Inspector::ArgList&) {
  return muduo::ProcessInfo::procStatus();
}
/*
std::string ProcessInspector::OpenFiles(HttpRequest::Method, const Inspector::ArgList&) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", muduo::ProcessInfo::openedFiles());
  return buf;
}*/

std::string ProcessInspector::Threads(HttpRequest::Method, const Inspector::ArgList&) {
  std::vector<pid_t> threads = muduo::ProcessInfo::threads();
  std::string result = "  TID NAME             S    User Time  System Time\n";
  result.reserve(threads.size() * 64);
  std::string stat;
  for (pid_t tid : threads) {
    char buf[256];
    snprintf(buf, sizeof(buf), "/proc/%d/task/%d/stat", muduo::ProcessInfo::pid(), tid);
    if (muduo::FileUtil::readFile(buf, 65536, &stat) == 0) {
      muduo::StringPiece name = muduo::ProcessInfo::procname(stat);
      const char* rp = name.end();
      const char* state = rp + 2;
      *const_cast<char*>(rp) = '\0';
      muduo::StringPiece data(stat);
      data.remove_prefix(static_cast<int>(state - data.data() + 2));
      muduo::ProcessInfo::CpuTime t = GetCpuTime(data);
      snprintf(buf, sizeof buf, "%5d %-16s %c %12.3f %12.3f\n",
          tid, name.data(), *state, t.userSeconds, t.systemSeconds);
      result += buf;
    }
  }
  return result;
}
}  // namespace http
