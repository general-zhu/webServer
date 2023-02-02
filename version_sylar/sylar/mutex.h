#ifndef SYLAR_MUTEX_H_
#define SYLAR_MUTEX_H_
#pragma once

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <string>
#include <semaphore.h>
#include <atomic>

namespace sylar {

// 信号量
class Semaphore {
 public:
  Semaphore(uint32_t count = 0);
  ~Semaphore();
  void Wait();
  void Notify();

 private:
  Semaphore(const Semaphore&) = delete;
  Semaphore(const Semaphore&&) = delete;
  Semaphore& operator=(const Semaphore&) = delete;
 private:
  sem_t semaphore_;
};

template<class T>
class ScopedLockImpl {
 public:
  ScopedLockImpl(T& mutex) : mutex_(mutex) {
    mutex_.Lock();
    locked_ = true;
  }

  ~ScopedLockImpl() {
    Unlock();
  }

  void Lock() {
    if (!locked_) {
      mutex_.Lock();
      locked_ = true;
    }
  }

  void Unlock() {
    if (locked_) {
      mutex_.Unlock();
      locked_ = false;
    }
  }

 private:
  T& mutex_;
  bool locked_;
};

template<class T>
class ReadScopedLockImpl {
 public:
  ReadScopedLockImpl(T& mutex) : mutex_(mutex) {
    mutex_.RdLock();
    locked_ = true;
  }

  ~ReadScopedLockImpl() {
    Unlock();
  }

  void Lock() {
    if (!locked_) {
      mutex_.Rdock();
      locked_ = true;
    }
  }

  void Unlock() {
    if (locked_) {
      mutex_.Unlock();
      locked_ = false;
    }
  }

 private:
  T& mutex_;
  bool locked_;
};

template<class T>
class WriteScopedLockImpl {
 public:
  WriteScopedLockImpl(T& mutex) : mutex_(mutex) {
    mutex_.WrLock();
    locked_ = true;
  }

  ~WriteScopedLockImpl() {
    Unlock();
  }

  void Lock() {
    if (!locked_) {
      mutex_.Wrock();
      locked_ = true;
    }
  }

  void Unlock() {
    if (locked_) {
      mutex_.Unlock();
      locked_ = false;
    }
  }

 private:
  T& mutex_;
  bool locked_;
};

class Mutex {
 public:
  typedef ScopedLockImpl<Mutex> MutexLock;

  Mutex() {
    pthread_mutex_init(&mutex_, nullptr);
  }

  ~Mutex() {
    pthread_mutex_destroy(&mutex_);
  }

  void Lock() {
    pthread_mutex_lock(&mutex_);
  }

  void Unlock() {
    pthread_mutex_unlock(&mutex_);
  }

 private:
  pthread_mutex_t mutex_;
};

class NullMutex {
 public:
  typedef ScopedLockImpl<NullMutex> MutexLock;

  NullMutex() {}
  ~NullMutex() {}
  void Lock() {}
  void Unlock() {}
};

class RWMutex {
 public:
   typedef ReadScopedLockImpl<RWMutex> ReadLock;
   typedef WriteScopedLockImpl<RWMutex> WriteLock;

  RWMutex() {
    pthread_rwlock_init(&lock_, nullptr);
  }

  ~RWMutex() {
    pthread_rwlock_destroy(&lock_);
  }

  void RdLock() {
    pthread_rwlock_rdlock(&lock_);
  }

  void WrLock() {
    pthread_rwlock_wrlock(&lock_);
  }

  void Unlock() {
    pthread_rwlock_unlock(&lock_);
  }
 private:
  pthread_rwlock_t lock_;
};

class NullRWMutex {
 public:
  typedef ReadScopedLockImpl<NullRWMutex> ReadLock;
  typedef WriteScopedLockImpl<NullRWMutex> WriteLock;

  NullRWMutex() {}
  ~NullRWMutex() {}
  void RdLock() {}
  void WrLock() {}
  void Unlock() {}
};

class Spinlock {
 public:
  typedef ScopedLockImpl<Spinlock> MutexLock;

  Spinlock() {
    pthread_spin_init(&mutex_, 0);
  }

  ~Spinlock() {
    pthread_spin_destroy(&mutex_);
  }

  void Lock() {
    pthread_spin_lock(&mutex_);
  }

  void Unlock() {
    pthread_spin_unlock(&mutex_);
  }

 private:
  pthread_spinlock_t mutex_;

};

class CASLock {
 public:
  typedef ScopedLockImpl<CASLock> MutexLock;

  CASLock() {
    mutex_.clear();
  }

  ~CASLock() {
  }

  void Lock() {
    while(std::atomic_flag_test_and_set_explicit(&mutex_, std::memory_order_acquire));
  }

  void Unlock() {
    std::atomic_flag_clear_explicit(&mutex_, std::memory_order_release);
  }

 private:
  volatile std::atomic_flag mutex_;
};

}  // namespace sylar

#endif  // SYLAR_MUTEX_H_
