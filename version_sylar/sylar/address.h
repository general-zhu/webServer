#ifndef SYLAR_ADDRESS_H_
#define SYLAR_ADDRESS_H_
#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <map>

namespace sylar {

class IPAddress;

class Address {
 public:
  typedef std::shared_ptr<Address> Ptr;

  // 通过sockaddr指针创建Address
  static Address::Ptr Create(const sockaddr* addr, socklen_t addrlen);
  // 通过host(www.baidu.com)地址返回对应条件的所有Address
  static bool Lookup(std::vector<Address::Ptr>& result, const std::string& host,
      int family = AF_INET, int type = 0, int protocol = 0);
  // 通过host地址返回对应条件的任意一个Address
  static Address::Ptr LookupAny(const std::string& host, int family = AF_INET,
      int type = 0, int protocol = 0);
  // 通过host地址返回对应条件的任意一个IPAddress
  static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
      int family = AF_INET, int type = 0, int protocol = 0);
  // 返回本机所有网卡<网卡，地址， 子网掩码位数>
  static bool GetInterfaceAddresses(std::multimap<std::string,
      std::pair<Address::Ptr, uint32_t>>& result, int family = AF_INET);
  // 返回本机指定网卡的地址和子网掩码位数
  static bool GetInterfaceAddresses(std::vector<std::pair<Address::Ptr, uint32_t>>& result,
      const std::string& iface, int family = AF_INET);
  virtual ~Address() {}

  int GetFamily() const;
  virtual const sockaddr* GetAddr() const = 0;
  virtual sockaddr* GetAddr() = 0;
  virtual socklen_t GetAddrLen() const = 0;
  // 输出可读性地址
  virtual std::ostream& Insert(std::ostream& os) const = 0;
  std::string ToString() const;

  bool operator<(const Address& rhs) const;
  bool operator==(const Address& rhs) const;
  bool operator!=(const Address& rhs) const;
};

class IPAddress : public Address {
 public:
  typedef std::shared_ptr<IPAddress> Ptr;

  static IPAddress::Ptr Create(const char* address, uint16_t port = 0);
  // prefix_len 子网掩码位数
  virtual IPAddress::Ptr BroadcastAddress(uint32_t prefix_len) = 0;
  virtual IPAddress::Ptr NetwordAddress(uint32_t prefix_len) = 0;
  virtual IPAddress::Ptr SubnetMask(uint32_t prefix_len) = 0;

  virtual uint32_t GetPort() const = 0;
  virtual void SetPort(uint16_t v) = 0;
};

class IPv4Address : public IPAddress {
 public:
  typedef std::shared_ptr<IPv4Address> Ptr;

  static IPv4Address::Ptr Create(const char* address, uint16_t port = 0);
  IPv4Address(const sockaddr_in& address);
  IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

  const sockaddr* GetAddr() const override;
  sockaddr* GetAddr() override;
  socklen_t GetAddrLen() const override;
  std::ostream& Insert(std::ostream& os) const override;

  IPAddress::Ptr BroadcastAddress(uint32_t prefix_len) override;
  IPAddress::Ptr NetwordAddress(uint32_t prefix_len) override;
  IPAddress::Ptr SubnetMask(uint32_t prefix_len) override;

  uint32_t GetPort() const override;
  void SetPort(uint16_t v) override;

 private:
  sockaddr_in addr_;
};

class IPv6Address : public IPAddress {
 public:
  typedef std::shared_ptr<IPv6Address> Ptr;

  static IPv6Address::Ptr Create(const char* address, uint16_t port = 0);
  IPv6Address();
  IPv6Address(const sockaddr_in6& address);
  IPv6Address(const uint8_t address[16], uint16_t port = 0);

  const sockaddr* GetAddr() const override;
  sockaddr* GetAddr() override;
  socklen_t GetAddrLen() const override;
  std::ostream& Insert(std::ostream& os) const override;

  IPAddress::Ptr BroadcastAddress(uint32_t prefix_len) override;
  IPAddress::Ptr NetwordAddress(uint32_t prefix_len) override;
  IPAddress::Ptr SubnetMask(uint32_t prefix_len) override;

  uint32_t GetPort() const override;
  void SetPort(uint16_t v) override;

 private:
  sockaddr_in6 addr_;
};

class UnixAddress : public Address {
 public:
  typedef std::shared_ptr<UnixAddress> Ptr;

  UnixAddress();
  UnixAddress(const std::string& path);

  void SetAddrLen(uint32_t v);
  const sockaddr* GetAddr() const override;
  sockaddr* GetAddr() override;
  socklen_t GetAddrLen() const override;
  std::ostream& Insert(std::ostream& os) const override;
  std::string GetPath() const;

 private:
  sockaddr_un addr_;
  socklen_t length_;

};

class UnknownAddress : public Address {
 public:
  typedef std::shared_ptr<UnknownAddress> Ptr;

  UnknownAddress(int family);
  UnknownAddress(const sockaddr& addr);

  const sockaddr* GetAddr() const override;
  sockaddr* GetAddr() override;
  socklen_t GetAddrLen() const override;
  std::ostream& Insert(std::ostream& os) const override;

 private:
  sockaddr addr_;
};

std::ostream& operator<<(std::ostream& os, const Address& addr);

}  // namespace sylar
#endif  // SYLAR_ADDRESS_H_
