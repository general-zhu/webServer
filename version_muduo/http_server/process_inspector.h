#ifndef HTTP_SERVER_PROCESS_INSPECTOR_H_
#define HTTP_SERVER_PROCESS_INSPECTOR_H_
#pragma once

#include "http_server/inspector.h"

namespace http {
class ProcessInspector {
 public:
  void RegisterCommands(Inspector* ins);
  static std::string Overview(HttpRequest::Method, const Inspector::ArgList&);
  static std::string Pid(HttpRequest::Method, const Inspector::ArgList&);
  static std::string ProcStatus(HttpRequest::Method, const Inspector::ArgList&);
  static std::string OpenedFiles(HttpRequest::Method, const Inspector::ArgList&);
  static std::string Threads(HttpRequest::Method, const Inspector::ArgList&);
  static std::string username_;
};

}  // namespace http
#endif  // HTTP_SERVER_PROCESS_INSPECTOR_H_
