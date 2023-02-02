#ifndef SYLAR_FIBER_H_
#define SYLAR_FIBER_H_
#pragma once

#include <ucontext.h>
#include <memory>
#include <functional>
#include "sylar/thread.h"

namespace sylar {

class Scheduler;

class Fiber : public std::enable_shared_from_this<Fiber> {

  friend class Scheduler;

 public:
  typedef std::shared_ptr<Fiber> Ptr;

  enum State {
    INIT,    // 初始化状态
    HOLD,    // 暂停状态（阻塞）
    EXEC,    // 执行状态
    TERM,    // 结束状态
    READY,   // 可执行状态
    EXCEPT,  // 异常状态
  };

 private:
  // 不让拷贝就是留主线程去创建和调用
  Fiber();

 public:
  Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
  ~Fiber();

  // 重置协程函数，并重置状态
  // INIT, TERM
  void Reset(std::function<void()> cb);
  // 切换到当前协程执行
  void SwapIn();
  void Call();
  void Back();
  // 切换到后台执行
  void SwapOut();
  uint64_t GetId() const { return id_; }
  State GetState() const { return state_; }
  // 设置当前协程
  static void SetThis(Fiber *f);
  // 返回当前协程
  static Fiber::Ptr GetThis();
  // 协程切换到后台，并且设置为Ready状态
  static void YieldToReady();
  // 协程切换到后台，并且设置为Hold状态
  static void YieldToHold();
  // 总协程数
  static uint64_t TotalFibers();
  static void MainFunc();
  static void CallerMainFunc();
  static uint64_t GetFiberId();

 private:
  uint64_t id_ = 0;
  uint32_t stacksize_ = 0;
  State state_ = INIT;
  ucontext_t ctx_;
  void *stack_ = nullptr;
  std::function<void()> cb_;
};

}  // namespace sylar
#endif  // SYLAR_FIBER_H_
