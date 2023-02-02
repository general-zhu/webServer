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
#include "iomanager.h"
#include "macro.h"
#include "log.h"


namespace sylar {

static sylar::Logger::Ptr g_logger = LOG_NAME("system");

IOManager::FdContext::EventContext& IOManager::FdContext::GetContext(IOManager::Event event) {
  switch (event) {
    case IOManager::Read:
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
  ctx.fiber.Reset();
  ctx.cb = nullptr;
}

void IOManager::FdContext::TriggerEvent(IOManager::Event event) {
  ASSERT(events & event);
  events_ = (Event)(events_ & ~event);
  EventContext& ctx = GetContext(event);
  if (ctx.cb) {
    ctx.scheduler->Scheduler(&ctx.cb);
  } else {
    ctx.scheduler->Scheduler(&ctx.fiber);
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
      fd_contexts_->fd_ = i;
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
  FdContext::MutexType::Lock lock2(fd_ctx->mutex_);
  // 事件已经加过了,则出现错误
  if (fd_ctx->event & event) {
    LOG_ERROR(g_logger) << " AddEvent assert fd=" << fd_
        << " event=" << (EPOLL_EVENTS)event
        << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events;
    ASSERT(!(fd_ctx->events & event));
  }
  int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
  epoll_event epevent;
  epevent.events = EPOLLET | fd_ctx->events | event;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(epfd_, op, fd, &epevent);
  if (rt) {
    LOG_ERROR(g_logger) << "epoll_ctl(" << epfd_ << ", "
        << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
        << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
        << (EPOLL_EVENTS)fd_ctx->events;
    return -1;
  }
  ++pending_event_count_;
  fd_ctx->events = (Event)(fd_ctx->events | event);
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
  FdContext::MutexType::Lock lock2(fd_ctx->mutex_);
  if (!fd_ctx->events & event) {
    return false;
  }
  Event new_events = (Event)(fd_ctx->events & ~event);
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
  fd_ctx->events = new_events;
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
  FdContext::MutexType::Lock lock2(fd_ctx->mutex_);
  if (!fd_ctx->events & event) {
    return false;
  }
  Event new_events = (Event)(fd_ctx->events & ~event);
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
  FdContext::MutexType::Lock lock2(fd_ctx->mutex_);
  if (!fd_ctx->events) {
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
  if (fd_ctx-events & READ) {
    fd_ctx->TriggerEvent(READ);
    --pending_event_count_;
  }
  if (fd_ctx->events & WRITE) {
    fd_ctx->TriggerEvent(WRITE);
    --pending_event_count_;
  }
  ASSERT(fd_ctx->events == 0);
  return true;
}

IOManager::IOManager *GetThis() {
  return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

}  // namespace sylar
