#include <ifaddrs.h>
#include <sstream>
#include <netdb.h>
#include <utility>
#include "address.h"
#include "log.h"
#include "endian.h"

namespace sylar {

static sylar::Logger::Ptr g_logger = LOG_NAME("system");

template<class T>
static T CreateMask(uint32_t bits) {
  return (1 <<(sizeof(T) * 8 - bits)) - 1;
}

template<class T>
static uint32_t CountBytes(T value) {
  uint32_t result = 0;
  for (; value; ++result) {
    value &= value - 1;
  }
  return result;
}

Address::Ptr Address::LookupAny(const std::string& host, int family,
    int type, int protocol) {
  std::vector<Address::Ptr> result;
  if (Lookup(result, host, family, type, protocol)) {
    return result[0];
  }
  return nullptr;
}

std::shared_ptr<IPAddress> Address::LookupAnyIPAddress(const std::string& host,
      int family, int type, int protocol) {
  std::vector<Address::Ptr> result;
  if (Lookup(result, host, family, type, protocol)) {
    for (auto& i : result) {
      IPAddress::Ptr v = std::dynamic_pointer_cast<IPAddress>(i);
      if (v) {
        return v;
      }
    }
  }
  return nullptr;
}

bool Address::Lookup(std::vector<Address::Ptr>& result, const std::string& host,
    int family, int type, int protocol) {
  addrinfo hints, *results, *next;
  hints.ai_flags = 0;
  hints.ai_family = family;
  hints.ai_socktype = type;
  hints.ai_protocol = protocol;
  hints.ai_addrlen = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  std::string node;
  const char* service = NULL;

  // 检查 ipv6address service
  if (!host.empty() && host[0] == '[') {
    // -1 这个地方没有check out of range 只有正确的字符串才能识别[ipv6]:port
    const char* endipv6 = (const char*)memchr(host.c_str() + 1, ']', host.size() - 1);
    if (endipv6) {
      // TODO check out of range [ipv6]:port(service, http;https;ftp)
      if (*(endipv6 + 1) == ':') {
        service = endipv6 + 2;
      }
      node = host.substr(1, endipv6 - host.c_str() - 1);
    }
  }

  // 检查 node service
  if (node.empty()) {
    service = (const char*)memchr(host.c_str(), ':', host.size());
    if (service) {
      if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
        node = host.substr(0, service - host.c_str());
        ++service;
      }
    }
  }
  if (node.empty()) {
    node = host;
  }
  int error = getaddrinfo(node.c_str(), service, &hints, &results);
  if (error) {
    LOG_DEBUG(g_logger) << "Address::Lookup getaddrinfo(" << host << ", " << family << ", "
        << type << ") error=" << error << " errstr=" << strerror(errno);
    return false;
  }
  next = results;
  while (next) {
    result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
    next = next->ai_next;
  }
  freeaddrinfo(results);
  return !result.empty();
}

bool Address::GetInterfaceAddresses(std::multimap<std::string,
    std::pair<Address::Ptr, uint32_t>>& result, int family) {
  struct ifaddrs *next, *results;
  if (getifaddrs(&results) != 0) {
    LOG_DEBUG(g_logger) << "Address::GetInterfaceAddress getifaddrs"
      << " errno=" << errno << " errstr=" << strerror(errno);
    return false;
  }
  try {
    for (next = results; next; next = next->ifa_next) {
      Address::Ptr addr;
      uint32_t prefix_len = ~0u;
      if (family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
        continue;
      }
      switch(next->ifa_addr->sa_family) {
        case AF_INET:
          {
            addr = Create(next->ifa_addr, sizeof(sockaddr_in));
            uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
            prefix_len = CountBytes(netmask);
          }
          break;
        case AF_INET6:
          {
            addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
            in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
            prefix_len = 0;
            for (int i = 0; i < 16; ++i) {
              prefix_len += CountBytes(netmask.s6_addr[i]);
            }
          }
          break;
        default:
          LOG_ERROR(g_logger) << "GetInterfaceAddresses switch error";
          break;
      }
      if (addr) {
        result.insert(std::make_pair(next->ifa_name, std::make_pair(addr, prefix_len)));
      }
    }
  } catch(...) {
    LOG_ERROR(g_logger) << "Address::GetInterfaceAddress exception";
    freeifaddrs(results);
    return false;
  }
  freeifaddrs(results);
  return !result.empty();
}

bool Address::GetInterfaceAddresses(std::vector<std::pair<Address::Ptr, uint32_t>>& result,
    const std::string& iface, int family) {
  if (iface.empty() || iface == "*") {
    if (family == AF_INET || family == AF_UNSPEC) {
      result.push_back(std::make_pair(Address::Ptr(new IPv4Address()), 0u));
    }
    if (family == AF_INET6 || family == AF_UNSPEC) {
      result.push_back(std::make_pair(Address::Ptr(new IPv6Address()), 0u));
    }
    return true;
  }
  std::multimap<std::string, std::pair<Address::Ptr, uint32_t>> results;
  if(!GetInterfaceAddresses(results, family)) {
    return false;
  }
  auto its = results.equal_range(iface);
  for (; its.first != its.second; ++its.first) {
    result.push_back(its.first->second);
  }
  return !result.empty();
}

int Address::GetFamily() const {
  return GetAddr()->sa_family;
}

std::string Address::ToString() const {
  std::stringstream ss;
  Insert(ss);
  return ss.str();
}

Address::Ptr Address::Create(const sockaddr* addr, socklen_t addrlen) {
  if (addr == nullptr) {
    return nullptr;
  }
  Address::Ptr result;
  switch(addr->sa_family) {
    case AF_INET:
      result.reset(new IPv4Address(*(const sockaddr_in*)addr));
      break;
    case AF_INET6:
      result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
      break;
    default:
      result.reset(new UnknownAddress(*addr));
      break;
  }
  return result;
}

bool Address::operator<(const Address& rhs) const {
  socklen_t minlen = std::min(GetAddrLen(), rhs.GetAddrLen());
  int result = memcmp(GetAddr(), rhs.GetAddr(), minlen);
  if (result < 0) {
    return true;
  } else if (result > 0) {
    return false;
  } else if (GetAddrLen() < rhs.GetAddrLen()) {
    return true;
  }
  return false;
}

bool Address::operator==(const Address& rhs) const {
  return GetAddrLen() == rhs.GetAddrLen()
      && memcmp(GetAddr(), rhs.GetAddr(), GetAddrLen()) == 0;
}

bool Address::operator!=(const Address& rhs) const {
  return !(*this == rhs);
}

IPAddress::Ptr IPAddress::Create(const char* address, uint16_t port) {
  addrinfo hints, *results;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_NUMERICHOST; // 以数字格式返回主机地址
  hints.ai_family = AF_UNSPEC;

  int error = getaddrinfo(address, NULL, &hints, &results);
  if (error) {
    LOG_DEBUG(g_logger) << "IPAddress::Create(" << address << ", " << port << ") error=" << error
        << " errno=" << errno << " errstr=" << strerror(errno);
    return nullptr;
  }
  try {
    IPAddress::Ptr result = std::dynamic_pointer_cast<IPAddress>(
        Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen));
    if (result) {
      result->SetPort(port);
    }
    freeaddrinfo(results);
    return result;
  } catch (...) {
    freeaddrinfo(results);
    return nullptr;
  }
}

IPv4Address::Ptr IPv4Address::Create(const char* address, uint16_t port) {
  IPv4Address::Ptr rt(new IPv4Address);
  rt->addr_.sin_port = ByteswapOnLittleEndian(port);
  int result = inet_pton(AF_INET, address, &rt->addr_.sin_addr);
  if (result <= 0) {
    LOG_DEBUG(g_logger) << "IPv4Address::Create(" << address << ", " << port << ") rt=" << result
        << " errno=" << errno << " errstr=" << strerror(errno);
    return nullptr;
  }
  return rt;
}

IPv4Address::IPv4Address(const sockaddr_in& address) {
  addr_ = address;
}

IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_port = ByteswapOnLittleEndian(port);
  addr_.sin_addr.s_addr = ByteswapOnLittleEndian(address);
}

const sockaddr* IPv4Address::GetAddr() const {
  return (sockaddr*)&addr_;
}

sockaddr* IPv4Address::GetAddr() {
 return (sockaddr*)&addr_;
}

socklen_t IPv4Address::GetAddrLen() const {
  return sizeof(addr_);
}

std::ostream& IPv4Address::Insert(std::ostream& os) const {
  uint32_t addr = ByteswapOnLittleEndian(addr_.sin_addr.s_addr);
  os << ((addr >> 24) & 0xff) << "."
      << ((addr >> 16) & 0xff) << "."
      << ((addr >> 8) & 0xff) << "."
      << (addr & 0xff);
  os << ":" << ByteswapOnLittleEndian(addr_.sin_port);
  return os;
}

IPAddress::Ptr IPv4Address::BroadcastAddress(uint32_t prefix_len) {
  if (prefix_len > 32) {
    return nullptr;
  }

  sockaddr_in baddr(addr_);
  baddr.sin_addr.s_addr |= ByteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return IPv4Address::Ptr(new IPv4Address(baddr));
}

IPAddress::Ptr IPv4Address::NetwordAddress(uint32_t prefix_len) {
  if (prefix_len > 32) {
    return nullptr;
  }
  sockaddr_in baddr(addr_);
  // 我觉得他写错了，因该是： 他这个是主机号
  // baddr.sin_addr.s_addr &= ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  baddr.sin_addr.s_addr &= ByteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return IPv4Address::Ptr(new IPv4Address(baddr));
}

IPAddress::Ptr IPv4Address::SubnetMask(uint32_t prefix_len) {
  sockaddr_in subnet;
  memset(&subnet, 0, sizeof(subnet));
  subnet.sin_family = AF_INET;
  subnet.sin_addr.s_addr = ~ByteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
  return IPv4Address::Ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::GetPort() const {
  return ByteswapOnLittleEndian(addr_.sin_port);
}

void IPv4Address::SetPort(uint16_t v) {
  addr_.sin_port = ByteswapOnLittleEndian(v);
}

IPv6Address::Ptr IPv6Address::Create(const char* address, uint16_t port) {
  IPv6Address::Ptr rt(new IPv6Address);
  rt->addr_.sin6_port = ByteswapOnLittleEndian(port);
  int result = inet_pton(AF_INET6, address, &rt->addr_.sin6_addr);
  if (result <= 0) {
    LOG_DEBUG(g_logger) << "IPv6Address::Create(" << address << ", " << port << ") rt=" << result
        << " errno=" << errno << " errstr=" << strerror(errno);
    return nullptr;
  }
  return rt;
}

IPv6Address::IPv6Address() {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& address) {
  addr_ = address;
}

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin6_family = AF_INET6;
  addr_.sin6_port = ByteswapOnLittleEndian(port);
  memcpy(&addr_.sin6_addr.s6_addr, address, 16);
}

const sockaddr* IPv6Address::GetAddr() const {
  return (sockaddr*)&addr_;
}

sockaddr* IPv6Address::GetAddr() {
  return (sockaddr*)&addr_;
}

socklen_t IPv6Address::GetAddrLen() const {
  return sizeof(addr_);
}

std::ostream& IPv6Address::Insert(std::ostream& os) const {
  os << "[";
  uint16_t* addr = (uint16_t*)addr_.sin6_addr.s6_addr;
  bool used_zeros = false;  // 只能有一次:: , 表示之前是否有::
  for (size_t i = 0; i < 8; ++i) {
    if (addr[i] == 0 && !used_zeros) {
      continue;
    }
    if (i && addr[i - 1] == 0 && !used_zeros) {
      os << ":";
      used_zeros = true;
    }
    if (i) {
      os << ":";
    }
    os << std::hex << (int)ByteswapOnLittleEndian(addr[i]) << std::dec;
  }
  if (!used_zeros && addr[7] == 0) {
    os << "::";
  }
  os << "]" << ByteswapOnLittleEndian(addr_.sin6_port);
  return os;
}

IPAddress::Ptr IPv6Address::BroadcastAddress(uint32_t prefix_len) {
  sockaddr_in6 baddr(addr_);
  baddr.sin6_addr.s6_addr[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len % 8);
  // 一个一个字节处理就不用转字节序了
  for (int i = prefix_len / 8 + 1; i < 16; ++i) {
    baddr.sin6_addr.s6_addr[i] = 0xff;
  }
  return IPv6Address::Ptr(new IPv6Address(baddr));
}

IPAddress::Ptr IPv6Address::NetwordAddress(uint32_t prefix_len) {
  sockaddr_in6 baddr(addr_);
  baddr.sin6_addr.s6_addr[prefix_len / 8] &= CreateMask<uint8_t>(prefix_len % 8);
  for (int i = prefix_len / 8 + 1; i < 16; ++i) {
    baddr.sin6_addr.s6_addr[i] = 0x00;
  }
  return IPv6Address::Ptr(new IPv6Address(baddr));
}

IPAddress::Ptr IPv6Address::SubnetMask(uint32_t prefix_len) {
  sockaddr_in6 subnet;
  memset(&subnet, 0, sizeof(subnet));
  subnet.sin6_family = AF_INET6;
  subnet.sin6_addr.s6_addr[prefix_len / 8] = ~CreateMask<uint8_t>(prefix_len % 8);
  for (uint32_t i = 0; i < prefix_len / 8; ++i) {
    subnet.sin6_addr.s6_addr[i] = 0xff;
  }
  return IPv6Address::Ptr(new IPv6Address(subnet));
}

uint32_t IPv6Address::GetPort() const {
  return ByteswapOnLittleEndian(addr_.sin6_port);
}

void IPv6Address::SetPort(uint16_t v) {
  addr_.sin6_port = ByteswapOnLittleEndian(v);
}

static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;

UnixAddress::UnixAddress() {
  memset(&addr_, 0 , sizeof(addr_));
  addr_.sun_family = AF_UNIX;
  length_ = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string& path) {
  memset(&addr_, 0 , sizeof(addr_));
  addr_.sun_family = AF_UNIX;
  length_ = path.size() + 1;

  if (!path.empty() && path[0] == '\0') {
    --length_;
  }
  if (length_ > sizeof(addr_.sun_path)) {
    throw std::logic_error("path too long");
  }
  memcpy(addr_.sun_path, path.c_str(), length_);
  length_ += offsetof(sockaddr_un, sun_path);
}

void UnixAddress::SetAddrLen(uint32_t v) {
  length_ = v;
}

const sockaddr* UnixAddress::GetAddr() const {
  return (sockaddr*)&addr_;
}

sockaddr* UnixAddress::GetAddr() {
  return (sockaddr*)&addr_;
}

socklen_t UnixAddress::GetAddrLen() const {
  return length_;
}
std::ostream& UnixAddress::Insert(std::ostream& os) const {
  if (length_ > offsetof(sockaddr_un, sun_path) && addr_.sun_path[0] == '\0') {
    return os << "\\0"
        << std::string(addr_.sun_path + 1, length_ - offsetof(sockaddr_un, sun_path) - 1);
  }
  return os << addr_.sun_path;
}

std::string UnixAddress::GetPath() const {
  return "";
}

UnknownAddress::UnknownAddress(int family) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr& addr) {
  addr_ = addr;
}

const sockaddr* UnknownAddress::GetAddr() const {
  return (sockaddr*)&addr_;
}

sockaddr* UnknownAddress::GetAddr() {
  return (sockaddr*)&addr_;
}

socklen_t UnknownAddress::GetAddrLen() const {
  return sizeof(addr_);
}

std::ostream& UnknownAddress::Insert(std::ostream& os) const {
  os << "[UnknownAddress family=" << addr_.sa_family << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Address& addr) {
  return addr.Insert(os);
}
}  // namespace sylar
