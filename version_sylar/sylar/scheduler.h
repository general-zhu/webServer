#ifndef SYLAR_SCHEDULER_H_
#define SYLAR_SCHEDULER_H_
#pragma once

#include <memory>
#include <atomic>
#include <vector>
#include <list>
#include "fiber.h"
#include "mutex.h"
#include "thread.h"

namespace sylar {

class Scheduler {
 public:
  typedef std::shared_ptr<Scheduler> Ptr;
  typedef Mutex MutexType;

  // use_caller 是否使用当前调用线程
  Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
  virtual ~Scheduler();
  const std::string& GetName() const { return name_; };
  void Start();
  void Stop();

  static Scheduler *GetThis();
  static Fiber *GetMainFiber();

  // 调度协程：fc协程或者函数，thread协程执行的线程id，-1表示任意线程
  template<class FiberOrCb>
  void Schedule(FiberOrCb fc, int thread = -1) {
    bool need_tickle = false;
    {
      MutexType::MutexLock lock(mutex_);
      need_tickle = ScheduleNoLock(fc, thread);
    }

    if (need_tickle) {
      Tickle();
    }
  }

  template<class InputIterator>
  void Schedule(InputIterator begin, InputIterator end) {
    bool need_tickle = false;
    {
      MutexType::MutexLock lock(mutex_);
      while (begin != end) {
        need_tickle = ScheduleNoLock(&*begin, -1) || need_tickle;
        ++begin;
      }
    }
    if (need_tickle) {
      Tickle();
    }
  }
 protected:
  // 通知协程调度器有任务了
  virtual void Tickle();
  virtual bool Stopping();
  virtual void Idle(); // 没有任务的时候，不能让线程停下呀
  void Run();
  void SetThis();
  bool HasIdleThreads() const { return idle_thread_count_ > 0; }

 private:
  template<class FiberOrCb>
  bool ScheduleNoLock(FiberOrCb fc, int thread) {
    bool need_tickle = fibers_.empty();
    FiberAndThread ft(fc, thread);
    if (ft.fiber_ || ft.cb_) {
      fibers_.push_back(ft);
    }
    return need_tickle;
  }

  // 协程（线程）与执行函数
  class FiberAndThread {
  public:
    FiberAndThread(Fiber::Ptr f, int thr) :
        fiber_(f),
        thread_(thr) {
    }

    FiberAndThread(Fiber::Ptr *f, int thr) : thread_(thr) {
      fiber_.swap(*f);
    }

    FiberAndThread(std::function<void()> f, int thr) :
        cb_(f),
        thread_(thr) {
    }

    FiberAndThread(std::function<void()> *f, int thr) :
        thread_(thr) {
      cb_.swap(*f);
    }

    FiberAndThread() :
        thread_(-1) {
    }

    void Reset() {
      fiber_ = nullptr;
      cb_ = nullptr;
      thread_ = -1;
    }

    Fiber::Ptr fiber_;
    std::function<void()> cb_;
    int thread_;
  };
 private:
  MutexType mutex_;
  std::vector<Thread::Ptr> threads_;
  std::list<FiberAndThread> fibers_;
  Fiber::Ptr root_fiber_;
  std::string name_;

 protected:
  std::vector<int> thread_ids_;
  size_t thread_count_ = 0;
  std::atomic<size_t> active_thread_count_ = {0};
  std::atomic<size_t> idle_thread_count_ = {0};  // 空闲的线程个数
  bool stopping_ = true;
  bool auto_stop_ = false;
  int root_thread_ = 0;
};

}  // namespace sylar
#endif  // SYLAR_SCHEDULER_H_
