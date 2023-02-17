/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-02-05 21:43
 */

#include "timer.h"
#include "util.h"

namespace sylar {

bool Timer::Comparator::operator()(const Timer::Ptr& lhs, const Timer::Ptr& rhs) const {
  if (!lhs && !rhs) {
    return false;
  }
  if (!lhs) {
    return true;
  }
  if (!rhs) {
    return false;
  }
  if (lhs->next_ < rhs->next_) {
    return true;
  }
  if (rhs->next_ < lhs->next_) {
    return false;
  }
  return lhs.get() < rhs.get();
}

Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager) :
    recurring_(recurring),
    ms_(ms),
    cb_(cb),
    manager_(manager) {
  next_ = sylar::GetCurrentMS() + ms_;
}

Timer::Timer(uint64_t next) : next_(next) {
}

bool Timer::Cancel() {
  TimerManager::RWMutexType::WriteLock lock(manager_->mutex_);
  if (cb_) {
    cb_ = nullptr;
    auto it = manager_->timers_.find(shared_from_this());
    manager_->timers_.erase(it);
    return true;
  }
  return false;
}

bool Timer::Refresh() {
  TimerManager::RWMutexType::WriteLock lock(manager_->mutex_);
  if (!cb_) {
    return false;
  }
  auto it = manager_->timers_.find(shared_from_this());
  if (it == manager_->timers_.end()) {
    return false;
  }
  manager_->timers_.erase(it);
  next_ = sylar::GetCurrentMS() + ms_;
  manager_->timers_.insert(shared_from_this());
  return true;
}

bool Timer::Reset(uint64_t ms, bool from_now) {
  if (ms == ms_ && !from_now) {
    return true;
  }
  TimerManager::RWMutexType::WriteLock lock(manager_->mutex_);
  if (!cb_) {
    return false;
  }
  auto it = manager_->timers_.find(shared_from_this());
  if (it == manager_->timers_.end()) {
    return false;
  }
  manager_->timers_.erase(it);
  uint64_t start = 0;
  if (from_now) {
    start = sylar::GetCurrentMS();
  } else {
    start = next_ - ms_;
  }
  ms_ = ms;
  next_ = start + ms;
  manager_->AddTimer(shared_from_this(), lock);
  return true;
}

TimerManager::TimerManager() {
  previous_time_ = sylar::GetCurrentMS();
}

TimerManager::~TimerManager() {
}

void TimerManager::AddTimer(Timer::Ptr val, RWMutexType::WriteLock& lock) {
  auto it = timers_.insert(val).first;
  bool at_front = (it == timers_.begin()) && !tickled_;
  if (at_front) {
    tickled_ = true;
  }
  lock.Unlock();
  if (at_front) {
    OnTimerInsertedAtFront();
  }
}

Timer::Ptr TimerManager::AddTimer(uint64_t ms, std::function<void()> cb, bool recurring) {
  Timer::Ptr timer(new Timer(ms, cb, recurring, this));
  RWMutexType::WriteLock lock(mutex_);
  AddTimer(timer, lock);
  return timer;
}

static void Ontimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
  std::shared_ptr<void> tmp = weak_cond.lock();
  if (tmp) {
    cb();
  }
}

Timer::Ptr TimerManager::AddConditionTimer(uint64_t ms, std::function<void()> cb,
    std::weak_ptr<void> weak_cond, bool recurring) {
  return AddTimer(ms, std::bind(&Ontimer, weak_cond, cb), recurring);
}

uint64_t TimerManager::GetNextTimer() {
  RWMutexType::ReadLock lock(mutex_);
  tickled_ = false;
  if (timers_.empty()) {
    return ~0ull;
  }
  const Timer::Ptr& next = *timers_.begin();
  uint64_t now_ms = sylar::GetCurrentMS();
  if (now_ms >= next->next_) {
    return 0;
  } else {
    return next->next_- now_ms;
  }
}

void TimerManager::ListExpiredCb(std::vector<std::function<void()>>& cbs) {
  uint64_t now_ms = sylar::GetCurrentMS();
  std::vector<Timer::Ptr> expired;
  {
    RWMutexType::ReadLock lock(mutex_);
    if (timers_.empty()) {
      return;
    }
  }
  RWMutexType::WriteLock lock(mutex_);
  if (timers_.empty()) {
    return;
  }
  bool rollover = DetectClockRollover(now_ms);
  if (!rollover && ((*timers_.begin())->next_ > now_ms)) {
    return;
  }
  Timer::Ptr now_timer(new Timer(now_ms));
  auto it = rollover ? timers_.end() : timers_.lower_bound(now_timer);
  while (it != timers_.end() && (*it)->next_ == now_ms) {
    ++it;
  }
  expired.insert(expired.begin(), timers_.begin(), it);
  timers_.erase(timers_.begin(), it);
  cbs.reserve(expired.size());
  for (auto& timer : expired) {
    cbs.push_back(timer->cb_);
    if (timer->recurring_) {
      timer->next_ = now_ms + timer->ms_;
      timers_.insert(timer);
    } else {
      timer->cb_ = nullptr;
    }
  }
}

bool TimerManager::DetectClockRollover(uint64_t now_ms) {
  bool rollover = false;
  if (now_ms < previous_time_ && now_ms < (previous_time_ - 60 * 60 * 1000)) {
    rollover = true;
  }
  previous_time_ = now_ms;
  return rollover;
}

bool TimerManager::HasTimer() {
  RWMutexType::ReadLock lock(mutex_);
  return !timers_.empty();
}
}  // namespace sylar
