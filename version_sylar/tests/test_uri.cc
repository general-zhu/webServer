#include "sylar/uri.h"
#include <iostream>

int main(int argc, char** argv) {
  //sylar::Uri::Ptr uri = sylar::Uri::Create("http://www.sylar.top/test/uri?id=100&name=sylar#frg");
  sylar::Uri::Ptr uri = sylar::Uri::Create("http://admin@www.sylar.top/test/中文/uri?id=100&name=sylar&vv=中文#frg中文");
  //sylar::Uri::Ptr uri = sylar::Uri::Create("http://www.sylar.top/test/uri");
  std::cout << uri->ToString() << std::endl;
  std::cout << "----------------\n";
  auto addr = uri->CreateAddress();
  std::cout << *addr << std::endl;
  return 0;
}
