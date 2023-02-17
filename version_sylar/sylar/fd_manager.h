/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-02-06 17:17
 */

#ifndef SYLAR_FD_MANAGER_H_
#define SYLAR_FD_MANAGER_H_
#pragma once

#include <memory>
#include <vector>
#include "thread.h"
#include "singleton.h"

namespace sylar {

class FdCtx : public std::enable_shared_from_this<FdCtx> {
 public:
  typedef std::shared_ptr<FdCtx> Ptr;
  FdCtx(int fd);
  ~FdCtx();

  bool Init();
  bool IsInit() const { return is_init_; }
  bool IsSocket() const { return is_socket_; }
  bool IsClose() const { return is_closed_; }

  void SetUserNonblock(bool v) { user_nonblock_ = v; }
  bool GetUserNonblock() const { return user_nonblock_; }
  void SetSysNonblock(bool v) { sys_nonblock_ = v; }
  bool GetSysNonblock() const { return sys_nonblock_; }
  void SetTimeout(int type, uint64_t v);
  uint64_t GetTimeout(int type);
 private:
  bool is_init_ : 1;
  bool is_socket_ : 1;
  bool sys_nonblock_ : 1;
  bool user_nonblock_ : 1;
  bool is_closed_ : 1;
  int fd_;
  uint64_t recv_timeout_;
  uint64_t send_timeout_;
};

class FdManager {
 public:
  typedef RWMutex RWMutexType;

  FdManager();
  FdCtx::Ptr Get(int fd, bool auto_create = false);
  void Del(int fd);

 private:
  RWMutexType mutex_;
  std::vector<FdCtx::Ptr> datas_;
};

typedef Singleton<FdManager> FdMgr;
}  // namespace sylar

#endif  // SYLAR_FD_MANAGER_H_
