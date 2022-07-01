#include "http_request.h"
#include <iostream>
#include <string>

int main() {
  http::HttpRequest req;
  const char* s = "/ get";
  const char* start = &s[0];
  const char* end = &s[1];
  req.SetPath(start, end);
  std::cout << "path_=" << req.GetPath() << "\n";
  return 0;
}
