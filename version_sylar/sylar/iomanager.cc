/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-02-01 18:55
 */

#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <string.h>
#include "iomanager.h"
#include "macro.h"
#include "log.h"


namespace sylar {

static sylar::Logger::Ptr g_logger = LOG_NAME("system");

enum EpollCtlOp {
};

IOManager::FdContext::EventContext& IOManager::FdContext::GetContext(IOManager::Event event) {
  switch (event) {
    case IOManager::READ:
      return read_;
    case IOManager::WRITE:
      return write_;
    default:
      ASSERT2(false, "GetContext");
  }
  throw std::invalid_argument("GetContext invalid event");
}

void IOManager::FdContext::ResetContext(EventContext& ctx) {
  ctx.scheduler = nullptr;
  ctx.fiber.reset();
  ctx.cb = nullptr;
}

void IOManager::FdContext::TriggerEvent(IOManager::Event event) {
  ASSERT(events_ & event);
  events_ = (Event)(events_ & ~event);
  EventContext& ctx = GetContext(event);
  if (ctx.cb) {
    ctx.scheduler->Schedule(&ctx.cb);
  } else {
    ctx.scheduler->Schedule(&ctx.fiber);
  }
  ctx.scheduler = nullptr;
  return;
}

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name) :
    Scheduler(threads, use_caller, name) {
  epfd_ = epoll_create(5000);
  ASSERT(epfd_ > 0);
  int rt = pipe(tickle_fds_);
  ASSERT(!rt);
  epoll_event event;
  memset(&event, 0, sizeof(epoll_event));
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = tickle_fds_[0];
  rt = fcntl(tickle_fds_[0], F_SETFL, O_NONBLOCK);
  ASSERT(!rt);
  rt = epoll_ctl(epfd_, EPOLL_CTL_ADD, tickle_fds_[0], &event);
  ASSERT(!rt);
  ContextResize(32);
  Start();
}

IOManager::~IOManager() {
  Stop();
  close(epfd_);
  close(tickle_fds_[0]);
  close(tickle_fds_[1]);
  for (size_t i = 0; i < fd_contexts_.size(); ++i) {
    if (fd_contexts_[i]) {
      delete fd_contexts_[i];
    }
  }
}

void IOManager::ContextResize(size_t size) {
  fd_contexts_.resize(size);
  for (size_t i = 0; i < fd_contexts_.size(); ++i) {
    if (!fd_contexts_[i]) {
      fd_contexts_[i] = new FdContext;
      fd_contexts_[i]->fd_ = i;
    }
  }
}

int IOManager::AddEvent(int fd, Event event, std::function<void()> cb) {
  FdContext *fd_ctx = nullptr;
  RWMutexType::ReadLock lock(mutex_);
  if ((int)fd_contexts_.size() > fd) {
    fd_ctx = fd_contexts_[fd];
    lock.Unlock();
  } else {
    lock.Unlock();
    RWMutexType::WriteLock lock2(mutex_);
    ContextResize(fd * 1.5);
    fd_ctx = fd_contexts_[fd];
  }
  FdContext::MutexType::MutexLock lock2(fd_ctx->mutex_);
  // 事件已经加过了,则出现错误
  if (fd_ctx->events_ & event) {
    LOG_ERROR(g_logger) << " AddEvent assert fd=" << fd
        << " event=" << (EPOLL_EVENTS)event
        << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events_;
    ASSERT(!(fd_ctx->events_ & event));
  }
  int op = fd_ctx->events_ ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
  epoll_event epevent;
  epevent.events = EPOLLET | fd_ctx->events_ | event;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(epfd_, op, fd, &epevent);
  if (rt) {
    LOG_ERROR(g_logger) << "epoll_ctl(" << epfd_ << ", "
        << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
        << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
        << (EPOLL_EVENTS)fd_ctx->events_;
    return -1;
  }
  ++pending_event_count_;
  fd_ctx->events_ = (Event)(fd_ctx->events_ | event);
  FdContext::EventContext& event_ctx = fd_ctx->GetContext(event);
  ASSERT(!event_ctx.scheduler && !event_ctx.fiber && !event_ctx.cb);
  event_ctx.scheduler = Scheduler::GetThis();
  if (cb) {
    event_ctx.cb.swap(cb);
  } else {
    event_ctx.fiber = Fiber::GetThis();
    ASSERT2(event_ctx.fiber->GetState() == Fiber::EXEC, "state="
        << event_ctx.fiber->GetState());
  }
  return 0;
}

bool IOManager::DelEvent(int fd, Event event) {
  RWMutexType::ReadLock lock(mutex_);
  if ((int)fd_contexts_.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = fd_contexts_[fd];
  lock.Unlock();
  FdContext::MutexType::MutexLock lock2(fd_ctx->mutex_);
  if (!(fd_ctx->events_ & event)) {
    return false;
  }
  Event new_events = (Event)(fd_ctx->events_ & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(epfd_, op, fd, &epevent);
  if (rt) {
    LOG_ERROR(g_logger) << "epoll_ctl(" << epfd_ << ", "
        << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
        << rt << " (" << errno << ") (" << strerror(errno) << ")";
    return false;
  }
  --pending_event_count_;
  fd_ctx->events_ = new_events;
  FdContext::EventContext& event_ctx = fd_ctx->GetContext(event);
  fd_ctx->ResetContext(event_ctx);
  return true;
}

bool IOManager::CancelEvent(int fd, Event event) {
  RWMutexType::ReadLock lock(mutex_);
  if ((int)fd_contexts_.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = fd_contexts_[fd];
  lock.Unlock();
  FdContext::MutexType::MutexLock lock2(fd_ctx->mutex_);
  if (!(fd_ctx->events_ & event)) {
    return false;
  }
  Event new_events = (Event)(fd_ctx->events_ & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(epfd_, op, fd, &epevent);
  if (rt) {
    LOG_ERROR(g_logger) << "epoll_ctl(" << epfd_ << ", "
        << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
        << rt << " (" << errno << ") (" << strerror(errno) << ")";
    return false;
  }
  --pending_event_count_;
  fd_ctx->TriggerEvent(event);
  return true;
}

bool IOManager::CancelAll(int fd) {
  RWMutexType::ReadLock lock(mutex_);
  if ((int)fd_contexts_.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = fd_contexts_[fd];
  lock.Unlock();
  FdContext::MutexType::MutexLock lock2(fd_ctx->mutex_);
  if (!fd_ctx->events_) {
    return false;
  }
  int op = EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = 0;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(epfd_, op, fd, &epevent);
  if (rt) {
    LOG_ERROR(g_logger) << "epoll_ctl(" << epfd_ << ", "
        << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
        << rt << " (" << errno << ") (" << strerror(errno) << ")";
    return false;
  }
  if (fd_ctx->events_ & READ) {
    fd_ctx->TriggerEvent(READ);
    --pending_event_count_;
  }
  if (fd_ctx->events_ & WRITE) {
    fd_ctx->TriggerEvent(WRITE);
    --pending_event_count_;
  }
  ASSERT(fd_ctx->events_ == 0);
  return true;
}

IOManager *IOManager::GetThis() {
  return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::Tickle() {
  if (!HasIdleThreads()) {
    return;
  }
  int rt = write(tickle_fds_[1], "T", 1);
  ASSERT(rt == 1);
}

bool IOManager::Stopping(uint64_t& timeout) {
  timeout = GetNextTimer();
  return timeout == ~0ull && pending_event_count_ == 0 && Scheduler::Stopping();
}

bool IOManager::Stopping() {
  uint64_t timeout = 0;
  return Stopping(timeout);
}

// 协程什么事情都不干就会陷入idle
void IOManager::Idle() {
  LOG_DEBUG(g_logger) << "idle";
  const uint64_t MAX_EVENTS = 255;
  epoll_event *events = new epoll_event[MAX_EVENTS]();
  std::shared_ptr<epoll_event> shared_events(events, [](epoll_event *ptr) {
      delete[] ptr;});
  while (true) {
    uint64_t next_timeout = 0;
    if (Stopping(next_timeout)) {
      LOG_INFO(g_logger) << "name=" << GetName() << " idle stopping exit";
      break;
    }
    int rt = 0;
    do {
      static const int MAX_TIMEOUT = 3000;
      if (next_timeout != ~0ull) {
        next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
      } else {
        next_timeout = MAX_TIMEOUT;
      }
      rt = epoll_wait(epfd_, events, MAX_EVENTS, (int)next_timeout);
      if (rt < 0 && errno == EINTR) {
        // 再跑一次循环
      } else {
        break;
      }
    } while (true);
    std::vector<std::function<void()>> cbs;
    ListExpiredCb(cbs);
    if (!cbs.empty()) {
      Schedule(cbs.begin(), cbs.end());
      cbs.clear();
    }
    for (int i = 0; i < rt; ++i) {
      epoll_event& event = events[i];
      if (event.data.fd == tickle_fds_[0]) {
        uint8_t dummy[255];
        while (read(tickle_fds_[0], dummy, sizeof(dummy)) > 0);
        continue;
      }
      FdContext *fd_ctx = (FdContext*)event.data.ptr;
      FdContext::MutexType::MutexLock lock(fd_ctx->mutex_);
      if (event.events & (EPOLLERR | EPOLLHUP)) {
        event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events_;
      }
      int real_events = NONE;
      if (event.events & EPOLLIN) {
        real_events |= READ;
      }
      if (event.events & EPOLLOUT) {
        real_events |= WRITE;
      }
      if ((fd_ctx->events_ & real_events) == NONE) {
        continue;
      }

      int left_events = (fd_ctx->events_ & ~real_events);
      int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
      event.events = EPOLLET | left_events;
      int rt2 = epoll_ctl(epfd_, op, fd_ctx->fd_, &event);
      if (rt2) {
        LOG_ERROR(g_logger) << "epoll_ctl(" << epfd_ << ", "
            << (EpollCtlOp)op << ", " << fd_ctx->fd_ << ", " << (EPOLL_EVENTS)event.events << "):"
            << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
        continue;
      }
      if (real_events & READ) {
        fd_ctx->TriggerEvent(READ);
        --pending_event_count_;
      }
      if (real_events & WRITE) {
        fd_ctx->TriggerEvent(WRITE);
        --pending_event_count_;
      }
    }
    Fiber::Ptr cur = Fiber::GetThis();
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->SwapOut();

  }
}

void IOManager::OnTimerInsertedAtFront() {
  Tickle();
}
}  // namespace sylar
