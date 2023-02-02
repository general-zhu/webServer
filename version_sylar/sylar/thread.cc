/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-01-13 14:30
 */

#include "sylar/thread.h"
#include "sylar/log.h"
#include "sylar/util.h"

namespace sylar {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

static sylar::Logger::Ptr g_logger = LOG_NAME("system");

Thread* Thread::GetThis() {
  return t_thread;
}

const std::string& Thread::GetThreadName() {
  return t_thread_name;
}

void Thread::SetName(const std::string& name) {
  if (t_thread) {
    t_thread->name_ = name;
  }
  t_thread_name = name;
}

Thread::Thread(std::function<void()> cb, const std::string& name) :
    cb_(cb),
    name_(name) {
  if (name.empty()) {
    name_ = "UNKNOW";
  }
  int rt = pthread_create(&thread_, nullptr, &Thread::Run, this);
  if (rt) {
    LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
      << " name=" << name;
    throw std::logic_error("pthread_create error");
  }
  semaphore_.Wait();  // 等待Run运行结束后在退出，这样就有序了
}

Thread::~Thread() {
  if (thread_) {
    pthread_detach(thread_);
  }
}

void Thread::Join() {
  if (thread_) {
    int rt = pthread_join(thread_, nullptr);
    if (rt) {
      LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
        << " name=" << name_;
      throw std::logic_error("pthread_join error");
    }
  }
  thread_ = 0;
}

void* Thread::Run(void* arg) {
  Thread* thread = static_cast<Thread*>(arg);
  t_thread = thread;
  t_thread_name = thread->name_;
  thread->id_ = sylar::GetThreadId();
  pthread_setname_np(pthread_self(), thread->name_.substr(0, 15).c_str());

  std::function<void()> cb;
  // swap是为了让这个函数的智能指针引用减一
  cb.swap(thread->cb_);

  thread->semaphore_.Notify();

  cb();
  return 0;
}

}  // namespace sylar
