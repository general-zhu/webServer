#include "sylar/address.h"
#include "sylar/log.h"

sylar::Logger::Ptr g_logger = LOG_ROOT();

void Test() {
  std::vector<sylar::Address::Ptr> addrs;
  bool v = sylar::Address::Lookup(addrs, "www.baidu.com");
  if (!v) {
    LOG_ERROR(g_logger) << "lookup fail";
    return;
  }
  for (size_t i = 0; i < addrs.size(); ++i) {
    LOG_INFO(g_logger) << i << " - " << addrs[i]->ToString();
  }
}

void IfaceTest() {
  std::multimap<std::string, std::pair<sylar::Address::Ptr, uint32_t>> results;
  bool v = sylar::Address::GetInterfaceAddresses(results);
  if (!v) {
    LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
    return;
  }
  for (auto&i : results) {
    LOG_INFO(g_logger) << i.first << " - " << i.second.first->ToString() << " - " << i.second.second;
  }
}

void IPv4Test() {
  // auto addr = sylar::IPAddress::Create("www.baidu.com");
  auto addr = sylar::IPAddress::Create("127.0.0.8");
  if (addr) {
    LOG_INFO(g_logger) << addr->ToString();
  }
}

int main() {
  // Test();
  // IfaceTest();
  IPv4Test();
  return 0;
}
