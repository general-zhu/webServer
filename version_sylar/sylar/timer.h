/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-02-05 21:43
 */

#ifndef SYLAR_TIMER_H_
#define SYLAR_TIMER_H_
#pragma once

#include <memory>
#include <set>
#include <vector>
#include "mutex.h"

namespace sylar {

class TimerManager;

class Timer : public std::enable_shared_from_this<Timer> {
  friend class TimerManager;
 public:
  typedef std::shared_ptr<Timer> Ptr;

  bool Cancel();
  bool Refresh();
  bool Reset(uint64_t ms, bool from_now);

 private:
  Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager);
  Timer(uint64_t next);
  class Comparator {
   public:
    bool operator()(const Timer::Ptr& lhs, const Timer::Ptr& rhs) const;
  };

 private:
  bool recurring_ = false;
  // 执行周期
  uint64_t ms_ = 0;
  // 精确的执行时间
  uint64_t next_ = 0;
  std::function<void()> cb_;
  TimerManager* manager_ = nullptr;
};

class TimerManager {
  friend class Timer;
 public:
  typedef RWMutex RWMutexType;

  TimerManager();
  virtual ~TimerManager();

  Timer::Ptr AddTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);
  Timer::Ptr AddConditionTimer(uint64_t ms, std::function<void()> cb,
      std::weak_ptr<void> weak_cond, bool recurring = false);
  uint64_t GetNextTimer();
  void ListExpiredCb(std::vector<std::function<void()>>& cbs);
  bool HasTimer();

 protected:
  virtual void OnTimerInsertedAtFront() = 0;
  void AddTimer(Timer::Ptr val, RWMutexType::WriteLock& lock);

 private:
  bool DetectClockRollover(uint64_t now_ms);

 private:
  RWMutexType mutex_;
  std::set<Timer::Ptr, Timer::Comparator> timers_;
  // 是否触发OnTimerInsertedAtFront
  bool tickled_ = false;
  uint64_t previous_time_ = 0;
};
}  // namespace sylar
#endif  // SYLAR_TIMER_H_
