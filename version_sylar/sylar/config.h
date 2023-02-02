/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2022-12-26 14:06
 */

#ifndef SYLAR_CONFIG_H_
#define SYLAR_CONFIG_H_
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include "sylar/log.h"
#include "log.h"

namespace sylar {

class ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVarBase> Ptr;

  ConfigVarBase(const std::string& name, const std::string& description = "") :
      name_(name),
      description_(description) {
    std::transform(name_.begin(), name_.end(), name_.begin(), ::tolower);
  }
  virtual ~ConfigVarBase() {};
  const std::string& GetName() const { return name_; }
  const std::string& GetDescription() const { return description_; }
  virtual std::string ToString() = 0;
  virtual bool FromString(const std::string& val) = 0;
  virtual std::string GetTypeName() const = 0;

 protected:
  std::string name_;
  std::string description_;
};

// 类型转换模板类(F yuan类型， T 目标类型)
template<class F, class T>
class LexicalCast {
 public:
  T operator()(const F& v) {
    return boost::lexical_cast<T>(v);
  }
};

// 类型转换模板类片特化(YAML String 转换成 std::vector<T>)
template<class T>
class LexicalCast<std::string, std::vector<T>> {
 public:
  std::vector<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::vector<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

template<class T>
class LexicalCast<std::vector<T>, std::string> {
 public:
  std::string operator()(const std::vector<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template<class T>
class LexicalCast<std::string, std::list<T>> {
 public:
  std::list<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::list<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

template<class T>
class LexicalCast<std::list<T>, std::string> {
 public:
  std::string operator()(const std::list<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template<class T>
class LexicalCast<std::string, std::set<T>> {
 public:
  std::set<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::set<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

template<class T>
class LexicalCast<std::set<T>, std::string> {
 public:
  std::string operator()(const std::set<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template<class T>
class LexicalCast<std::string, std::unordered_set<T>> {
 public:
  std::unordered_set<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::unordered_set<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
 public:
  std::string operator()(const std::unordered_set<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template<class T>
class LexicalCast<std::string, std::map<std::string, T>> {
 public:
  std::map<std::string, T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::map<std::string, T> vec;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str("");
      ss << it->second;
      vec.insert(std::make_pair(it->first.Scalar(),
          LexicalCast<std::string, T>()(ss.str())));
    }
    return vec;
  }
};

template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
 public:
  std::string operator()(const std::map<std::string, T>& v) {
    YAML::Node node(YAML::NodeType::Map);
    for (auto& i : v) {
      node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
 public:
  std::unordered_map<std::string, T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::unordered_map<std::string, T> vec;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str("");
      ss << it->second;
      vec.insert(std::make_pair(it->first.Scalar(),
          LexicalCast<std::string, T>()(ss.str())));
    }
    return vec;
  }
};

template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
 public:
  std::string operator()(const std::unordered_map<std::string, T>& v) {
    YAML::Node node(YAML::NodeType::Map);
    for (auto& i : v) {
      node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

template<class T, class FromStr = LexicalCast<std::string, T>,
    class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
 public:
  typedef RWMutex RWMutexType;
  typedef std::shared_ptr<ConfigVar> Ptr;
  typedef std::function<void (const T& old_value, const T& new_value)> OnChangeCb;

  ConfigVar(const std::string& name, const T& default_value,
      const std::string& description = "") :
      ConfigVarBase(name, description),
      val_(default_value) {
  }

  std::string ToString() override {
    try {
      // return boost::lexical_cast<std::string>(val_);
      RWMutexType::ReadLock lock(mutex_);
      return ToStr()(val_);
    } catch (std::exception& e) {
      LOG_ERROR(LOG_ROOT()) << "ConfigVar::ToString exception" << e.what() << " convert: "
          << typeid(val_).name() << " to string";
    }
    return "";
  }

  bool FromString(const std::string& val) override {
    try {
      SetValue(FromStr()(val));
    } catch (std::exception& e) {
      LOG_ERROR(LOG_ROOT()) << "ConfigVar::ToString exception" << e.what()
          << " convert: string to " << typeid(val_).name() << " - " << val;
    }
    return false;
  }

  const T GetValue() {
    RWMutexType::ReadLock lock(mutex_);
    return val_;
  }

  void SetValue(const T& v) {
    {
      RWMutexType::ReadLock lock(mutex_);
      if (v == val_) {
        return;
      }
      for (auto& i : cbs_) {
        // 调用回调函数进行处理新值与旧值的关系
        i.second(val_, v);
      }
    }
    RWMutexType::WriteLock lock(mutex_);
    val_ = v;
  }

  std::string GetTypeName() const override { return typeid(T).name(); }

  uint64_t AddListener(OnChangeCb cb) {
    static uint64_t s_fun_id = 0;
    RWMutexType::WriteLock lock(mutex_);
    ++s_fun_id;
    cbs_[s_fun_id] = cb;
    return s_fun_id;
  }

  void DelListener(uint64_t key) {
    RWMutexType::WriteLock lock(mutex_);
    cbs_.erase(key);
  }

  OnChangeCb GetListener(uint64_t key) {
    RWMutexType::ReadLock lock(mutex_);
    auto it = cbs_.find(key);
    return it == cbs_.end() ? nullptr : it->second;
  }

  // 清理所有的回调函数
  void ClearListeners() {
    RWMutexType::WriteLock lock(mutex_);
    cbs_.clear();
  }
 private:
  RWMutexType mutex_;
  T val_;
  // 变更回调函数组， uint64_t key, 要求唯一，一般可以用hash
  std::map<uint64_t, OnChangeCb> cbs_;
};

class Config {
 public:
  typedef std::map<std::string, ConfigVarBase::Ptr> ConfigVarMap;
  typedef RWMutex RWMutexType;

  template<class T>
  static typename ConfigVar<T>::Ptr Lookup(const std::string& name, const T& default_value,
      const std::string& description = "") {
    RWMutexType::WriteLock lock(GetMutex());
    auto it = GetDatas().find(name);
    if (it != GetDatas().end()) {
      auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
      if (tmp) {
        LOG_INFO(LOG_ROOT()) << "Lookup name=" << name << " exists";
        return tmp;
      } else {
        LOG_ERROR(LOG_ROOT()) << "Lookup name=" << name << " exists but type not "
            << typeid(T).name() << "real_type=" << it->second->GetTypeName()
            <<  " " << it->second->ToString();
        return nullptr;
      }
    }
    if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
      LOG_ERROR(LOG_ROOT()) << "Lookup name invalid " << name;
      throw std::invalid_argument(name);
    }
    typename ConfigVar<T>::Ptr v(new ConfigVar<T>(name, default_value, description));
    GetDatas()[name] = v;
    return v;
  }

  template<class T>
  static typename ConfigVar<T>::Ptr Lookup(const std::string& name) {
    RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    if (it == GetDatas().end()) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
  }

  static void LoadFromYaml(const YAML::Node& root);
  static ConfigVarBase::Ptr LookupBase(const std::string& name);
  // 通过自己设置的回调函数遍历所有的配置项
  static void Visit(std::function<void(ConfigVarBase::Ptr)> cb);

 private:
  static ConfigVarMap& GetDatas() {
    static ConfigVarMap datas;
    return datas;
  }

  static RWMutexType& GetMutex() {
    static RWMutexType s_mutex;
    return s_mutex;
  }
};

}  // namespace sylar

#endif  // SYLAR_CONFIG_H_
