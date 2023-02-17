#ifndef SYLAR_IOMANAGER_H_
#define SYLAR_IOMANAGER_H_
#pragma once

#include "scheduler.h"
#include "timer.h"

namespace sylar {

class IOManager : public Scheduler, public TimerManager{
 public:
  typedef std::shared_ptr<IOManager> Ptr;
  typedef RWMutex RWMutexType;

  enum Event {
    NONE = 0x0,
    // 读事件(EPOLLIN)
    READ = 0x1,
    // 写事件(EPOLLOUT)
    WRITE = 0x4,
  };

 private:
  class FdContext {
   public:
    typedef Mutex MutexType;

    struct EventContext {
      // 事件执行的调度器
      Scheduler *scheduler = nullptr;
      // 事件协程
      Fiber::Ptr fiber;
      // 事件回调函数
      std::function<void()> cb;
    };

    EventContext& GetContext(Event event);
    void ResetContext(EventContext& ctx);
    void TriggerEvent(Event event);

    // 事件关联的句柄
    int fd_;
    // 读事件上下文
    EventContext read_;
    // 写事件上下文
    EventContext write_;
    // 当前的事件
    Event events_ = NONE;
    MutexType mutex_;
  };

 public:
  IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
  ~IOManager();

  int AddEvent(int fd, Event event, std::function<void()> cb = nullptr);
  bool DelEvent(int fd, Event event);
  bool CancelEvent(int fd, Event event);
  bool CancelAll(int fd);

  static IOManager *GetThis();
 protected:
  void Tickle() override;
  bool Stopping() override;
  void Idle() override;
  void OnTimerInsertedAtFront() override;

  void ContextResize(size_t size);
  bool Stopping(uint64_t& timeout);

 private:
  // epoll 文件句柄
  int epfd_ = 0;
  // pipe 文件句柄
  int tickle_fds_[2];
  std::atomic<size_t> pending_event_count_ = {0};
  RWMutexType mutex_;
  // socket事件上下文的容器
  std::vector<FdContext*> fd_contexts_;
};

}  // namespace sylar
#endif  // SYLAR_IOMANAGER_H_
