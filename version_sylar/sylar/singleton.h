#ifndef SYLAR_SINGLETON_H_
#define SYLAR_SINGLETON_H_
#pragma once

#include <memory>

namespace sylar {


/**
 * @brief 单例模式封装类
 * @details T 类型
 *          X 为了创造多个实例对应的Tag
 *          N 同一个Tag创造多个实例索引
 *          好像是非线程安全
 */
template<class T, class X = void, int N = 0>
class Singleton {
 public:
  static T* GetInstance() {
    static T v;
    return &v;
  }
};

template<class T, class X = void, int N = 0>
class SingletonPtr {
  static std::shared_ptr<T> GetInstance() {
    static std::shared_ptr<T> v(new T);
    return v;
  }
};

}

#endif  // SYLAR_SINGLETON_H_
