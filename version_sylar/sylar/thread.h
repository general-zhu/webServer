#ifndef SYLAR_THREAD_H_
#define SYLAR_THREAD_H_
#pragma once

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <string>
#include "mutex.h"
#include "noncopyable.h"

namespace sylar {

class Thread : private Noncopyable {
 public:
  typedef std::shared_ptr<Thread> Ptr;

  Thread(std::function<void()> cb, const std::string& name);
  ~Thread();
  const std::string& GetName() const { return name_; }
  pid_t GetId() const { return id_; }
  void Join();

  static Thread* GetThis();
  static const std::string& GetThreadName();
  // 对于主线程我们有时候也需要设置线程名字
  static void SetName(const std::string& name);
 private:
  // 为啥不能拷贝，因为互斥量和互斥信号量是不能拷贝的，拷贝会破坏其作用
  // Thread(const Thread&) = delete;
  // Thread(const Thread&&) = delete;
  // Thread& operator=(const Thread&) = delete;

  static void* Run(void* arg);

 private:
  pid_t id_ = -1;
  pthread_t thread_ = 0;
  std::function<void()> cb_;
  std::string name_;
  Semaphore semaphore_;
};

}  // namespace sylar

#endif  // SYLAR_THREAD_H_
