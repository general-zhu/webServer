#include <atomic>
#include <iostream>
#include "log.h"
#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "scheduler.h"

namespace sylar {

static Logger::Ptr g_logger = LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

static thread_local Fiber* t_fiber = nullptr;
// 主协程
static thread_local Fiber::Ptr t_thread_fiber = nullptr;

static ConfigVar<uint32_t>::Ptr g_fiber_stack_size =
    Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

class MallocStackAllocator {
 public:
  static void *Alloc(size_t size) {
    return malloc(size);
  }

  static void Dealloc(void *vp, size_t size) {
    return free(vp);
  }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
  if (t_fiber) {
    return t_fiber->GetId();
  }
  return 0;
}

Fiber::Fiber() {
  state_ = EXEC;
  SetThis(this);

  if (getcontext(&ctx_)) {
    ASSERT2(false, "getcontext");
  }
  ++s_fiber_count;
  LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller) :
    id_(++s_fiber_id),
    cb_(cb) {
  ++s_fiber_count;
  stacksize_ = stacksize ? stacksize : g_fiber_stack_size->GetValue();
  stack_ = StackAllocator::Alloc(stacksize_);
  if (getcontext(&ctx_)) {
    ASSERT2(false, "getcontext");
  }
  ctx_.uc_link = nullptr;
  ctx_.uc_stack.ss_sp = stack_;
  ctx_.uc_stack.ss_size = stacksize_;
  if (!use_caller) {
    makecontext(&ctx_, &Fiber::MainFunc, 0);
  } else {
    makecontext(&ctx_, &Fiber::CallerMainFunc, 0);
  }
  LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << id_;
}

Fiber::~Fiber() {
  --s_fiber_count;
  if (stack_) {
    ASSERT(state_ == TERM || state_ == INIT);
    StackAllocator::Dealloc(stack_, stacksize_);
  } else {
    ASSERT(!cb_);
    ASSERT(state_ == EXEC);
    Fiber *cur = t_fiber;
    if (cur == this) {
      SetThis(nullptr);
    }
  }
  LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << id_;
}

// 重置协程函数，并重置状态
// INIT, TERM
void Fiber::Reset(std::function<void()> cb) {
  ASSERT(stack_);
  ASSERT(state_ == TERM || state_ == INIT || state_ == EXCEPT);
  cb_ = cb;
  if (getcontext(&ctx_)) {
    ASSERT2(false, "getcontext");
  }
  ctx_.uc_link = nullptr;
  ctx_.uc_stack.ss_sp = stack_;
  ctx_.uc_stack.ss_size = stacksize_;
  makecontext(&ctx_, &Fiber::MainFunc, 0);
  state_ = INIT;
}

void Fiber::Call() {
  SetThis(this);
  state_ = EXEC;
  if (swapcontext(&t_thread_fiber->ctx_, &ctx_)) {
    ASSERT2(false, "swapcontext");
  }
}

// 切换到当前协程执行
void Fiber::SwapIn() {
  SetThis(this);
  ASSERT(state_ != EXEC);
  state_ = EXEC;
  if (swapcontext(&Scheduler::GetMainFiber()->ctx_, &ctx_)) {
    ASSERT2(false, "swapcontext");
  }
}

void Fiber::Back() {
  SetThis(t_thread_fiber.get());
  if (swapcontext(&ctx_, &t_thread_fiber->ctx_)) {
    ASSERT2(false, "swapcontext");
  }
}

// 切换到后台执行,就是将主协程调出来
void Fiber::SwapOut() {
  SetThis(Scheduler::GetMainFiber());
  if (swapcontext(&ctx_, &Scheduler::GetMainFiber()->ctx_)) {
    ASSERT2(false, "swapcontext");
  }
}

// 设置当前协程执行
void Fiber::SetThis(Fiber *f) {
  t_fiber = f;
}

// 返回当前协程
Fiber::Ptr Fiber::GetThis() {
  if (t_fiber) {
    return t_fiber->shared_from_this();
  }
  Fiber::Ptr main_fiber(new Fiber);
  ASSERT(t_fiber == main_fiber.get());
  t_thread_fiber = main_fiber;
  // std::cout << t_thread_fiber.use_count() << std::endl;
  return t_fiber->shared_from_this();
}

// 协程切换到后台，并且设置为Ready状态
void Fiber::YieldToReady() {
  Fiber::Ptr cur = GetThis();
  cur->state_ = READY;
  cur->SwapOut();
}

// 协程切换到后台，并且设置为Ready状态
void Fiber::YieldToHold() {
  Fiber::Ptr cur = GetThis();
  cur->state_ = HOLD;
  cur->SwapOut();
}

// 总协程数
uint64_t TotalFibers() {
  return s_fiber_count;
}

void Fiber::MainFunc() {
  Fiber::Ptr cur = GetThis();
  ASSERT(cur);
  try {
    cur->cb_();
    cur->cb_ = nullptr;
    cur->state_ = TERM;
  } catch(std::exception& ex) {
    cur->state_ = EXCEPT;
    LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
        << " fiber_id=" << cur->GetId() << std::endl
        << sylar::BacktraceToString();
  } catch (...) {
    cur->state_ = EXCEPT;
    LOG_ERROR(g_logger) << "Fiber Except"
        << " fiber_id=" << cur->GetId() << std::endl
        << sylar::BacktraceToString();
  }

  // 执行结束后切换到主协程,因为contex切换不会对智能指针析构所以要人为的释放
  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->SwapOut();
  ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->GetId()));
}

void Fiber::CallerMainFunc() {
  Fiber::Ptr cur = GetThis();
  ASSERT(cur);
  try {
    cur->cb_();
    cur->cb_ = nullptr;
    cur->state_ = TERM;
  } catch(std::exception& ex) {
    cur->state_ = EXCEPT;
    LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
        << " fiber_id=" << cur->GetId() << std::endl
        << sylar::BacktraceToString();
  } catch (...) {
    cur->state_ = EXCEPT;
    LOG_ERROR(g_logger) << "Fiber Except"
        << " fiber_id=" << cur->GetId() << std::endl
        << sylar::BacktraceToString();
  }

  // 执行结束后切换到主协程,因为contex切换不会对智能指针析构所以要人为的释放
  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->Back();
  ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->GetId()));
}

}  // namespace sylar
