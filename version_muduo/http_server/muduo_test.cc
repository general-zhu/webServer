#include <functional>
#include <iostream>
#include <string>
#include <unistd.h>
#include "thirdparty/muduo/base/Thread.h"

void PrintfFunc(int n) {
  std::cout << "thread\n";
  for (int i = 0; i < n; ++i) {
    std::cout << i << " ";
  }
  std::cout << "\n";
}

int main(int argc, char** argv) {
  // muduo::Thread thread(std::bind(PrintfFunc, std::placeholders::_1));
  muduo::Thread thread(std::bind(PrintfFunc, 10));
  thread.start();
  std::cout << "main\n";
  sleep(1);
  return 0;
}
