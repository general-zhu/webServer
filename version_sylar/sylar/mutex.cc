/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-01-13 14:30
 */

#include "sylar/mutex.h"
#include "sylar/log.h"
#include "sylar/util.h"

namespace sylar {

static sylar::Logger::Ptr g_logger = LOG_NAME("system");

Semaphore::Semaphore(uint32_t count) {
  if (sem_init(&semaphore_, 0, count)) {
    throw std::logic_error("sem_init error");
  }
}

Semaphore::~Semaphore() {
  sem_destroy(&semaphore_);
}

void Semaphore::Wait() {
  if (sem_wait(&semaphore_)) {
    throw std::logic_error("sem_wait error");
  }
  // while循环是为了失败的时候重置
 //  while (true) {
 //    if (!sem_wait(&semaphore_)) {
 //      return;
 //    }
 //  }
}

void Semaphore::Notify() {
  if (sem_post(&semaphore_)) {
    throw std::logic_error("sem_post error");
  }
}

}  // namespace sylar
