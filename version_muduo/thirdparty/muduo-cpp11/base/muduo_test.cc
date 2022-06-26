#include <functional>
#include <iostream>
#include <string>
#include "thirdparty/muduo-cpp11/base/Thread.h"

void PrintfFunc(int n) {
  for (int i = 0; i < n; ++i) {
    std::cout << i << " ";
  }
  std::cout << "\n";
}

int main(int argc, char** argv) {
  // muduo::Thread thread(std::bind(PrintfFunc, std::placeholders::_1));
  muduo::Thread thread(std::bind(PrintfFunc, 1));
  return 0;
}
