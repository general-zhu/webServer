/**
 * Copyright(c) All rights reserve/
 * Author: zhuchun@qq.com
 * Time:   2023-01-22 17:01
 */

#include "scheduler.h"
#include "log.h"
#include "macro.h"

namespace sylar {

static sylar::Logger::Ptr g_logger = LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
// 主调度协程
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name) : name_(name) {
  ASSERT(threads > 0);
  if (use_caller) {
    // 如果没有协程会创建主协程
    sylar::Fiber::GetThis();
    --threads;
    ASSERT(GetThis() == nullptr);  // 一个线程不能有两个协程调度器
    t_scheduler = this;
    root_fiber_.reset(new Fiber(std::bind(&Scheduler::Run, this), 0, true));
    sylar::Thread::SetName(name_);

    t_scheduler_fiber = root_fiber_.get();
    root_thread_ = sylar::GetThreadId();
    thread_ids_.push_back(root_thread_);
  } else {
    root_thread_ = -1;
  }
  thread_count_ = threads;
}

Scheduler::~Scheduler() {
  ASSERT(stopping_);
  if (GetThis() == this) {
    t_scheduler = nullptr;
  }
}

void Scheduler::Start() {
  MutexType::MutexLock lock(mutex_);
  if (!stopping_) {
    return;
  }
  stopping_ = false;
  ASSERT(threads_.empty());
  threads_.resize(thread_count_);
  for (size_t i = 0; i < thread_count_; ++i) {
    threads_[i].reset(new Thread(std::bind(&Scheduler::Run, this),
        name_ + "_" + std::to_string(i)));
    thread_ids_.push_back(threads_[i]->GetId());
  }
  lock.Unlock();
}

void Scheduler::Stop() {
  auto_stop_ = true;
  if (root_fiber_ && thread_count_ == 0 &&
        (root_fiber_->GetState() == Fiber::TERM || root_fiber_->GetState() == Fiber::INIT)) {
    LOG_INFO(g_logger) << this << " stopped";
    stopping_ = true;
    if (Stopping()) {
      return;
    }
  }
  // bool exit_on_this_fiber = false;
  if (root_thread_ != -1) {
    ASSERT(GetThis() == this);
  } else {
    ASSERT(GetThis() != this);
  }
  stopping_ = true;
  for (size_t i = 0; i < thread_count_; ++i) {
    Tickle();
  }
  if (root_fiber_) {
    Tickle();
  }
  if (root_fiber_) {
    if (!Stopping()) {
      root_fiber_->Call();
    }
  }
  std::vector<Thread::Ptr> thrs;
  {
    MutexType::MutexLock lock(mutex_);
    thrs.swap(threads_);
  }
  for (auto& i : thrs) {
    i->Join();
  }
}

void Scheduler::SetThis() {
  t_scheduler = this;
}

void Scheduler::Run() {
  LOG_DEBUG(g_logger) << name_ << " run";
  SetThis();
  if (sylar::GetThreadId() != root_thread_) {
    t_scheduler_fiber = Fiber::GetThis().get();
  }
  Fiber::Ptr idle_fiber(new Fiber(std::bind(&Scheduler::Idle, this)));
  Fiber::Ptr cb_fiber;
  FiberAndThread ft;
  while (true) {
    ft.Reset();
    bool tickle_me = false;  // false 表示需要唤醒其他线程去处理
    bool is_active = false;  // false表示没有任务去做
    {
      MutexType::MutexLock lock(mutex_);
      auto it = fibers_.begin();
      while (it != fibers_.end()) {
        // 该任务有了指定的thread_id 我们就不处理他
        if (it->thread_ != -1 && it->thread_ != sylar::GetThreadId()) {
          ++it;
          tickle_me = true;  // 唤醒让其他线程处理
          continue;
        }
        ASSERT(it->fiber_ || it->cb_);
        if (it->fiber_ && it->fiber_->GetState() == Fiber::EXEC) {
          ++it;
          continue;
        }
        ft = *it;
        fibers_.erase(it++);
        ++active_thread_count_;
        is_active = true;
        break;
      }
      tickle_me |= it != fibers_.end();
    }
    // 取出一个需要执行的任务放在ft上面, 为了防止意外每次处理任务都要进入唤醒处理，
    // 不用处理就打印一个日志
    if (tickle_me) {
      Tickle();
    }
    // 执行
    if (ft.fiber_ && (ft.fiber_->GetState() != Fiber::TERM &&
        ft.fiber_->GetState() != Fiber::EXCEPT)) {
      ft.fiber_->SwapIn();
      --active_thread_count_;
      if (ft.fiber_->GetState() == Fiber::READY) {
        // 如果前面执行了YeilToReady,就要重新执行
        Schedule(ft.fiber_);
      } else if (ft.fiber_->GetState() != Fiber::TERM &&
          ft.fiber_->GetState() != Fiber::EXCEPT) {
        ft.fiber_->state_ = Fiber::HOLD;
      }
      ft.Reset();
    } else if (ft.cb_) {
      if (cb_fiber) {
        cb_fiber->Reset(ft.cb_);
      } else {
        cb_fiber.reset(new Fiber(ft.cb_));
      }
      ft.Reset();
      cb_fiber->SwapIn();
      --active_thread_count_;
      if (cb_fiber->GetState() == Fiber::READY) {
        Schedule(cb_fiber);
        cb_fiber.reset();
      } else if (cb_fiber->GetState() == Fiber::EXCEPT ||
          cb_fiber->GetState() == Fiber::TERM) {
        cb_fiber->Reset(nullptr);
      } else {
        cb_fiber->state_ = Fiber::HOLD;
        cb_fiber.reset();
      }
    } else {
      if (is_active) {
        --active_thread_count_;
        continue;
      }
      if (idle_fiber->GetState() == Fiber::TERM) {
        LOG_INFO(g_logger) << "idle fiber term";
        break;
      }
      ++idle_thread_count_;
      idle_fiber->SwapIn();
      --idle_thread_count_;
      if (idle_fiber->GetState() != Fiber::TERM &&
          idle_fiber->GetState() != Fiber::EXCEPT) {
        idle_fiber->state_ = Fiber::HOLD;
      }
    }
  }
}

void Scheduler::Tickle() {
  LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::Stopping() {
  MutexType::MutexLock lock(mutex_);
  return auto_stop_ && stopping_ && fibers_.empty() && active_thread_count_ == 0;
}

void Scheduler::Idle() {
  LOG_INFO(g_logger) << "idle";
  while (!Stopping()) {
    sylar::Fiber::YieldToHold();
  }
}

Scheduler *Scheduler::GetThis() {
  return t_scheduler;
}

Fiber *Scheduler::GetMainFiber() {
  return t_scheduler_fiber;
}

}  // namespace sylar
