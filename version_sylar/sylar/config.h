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
#include <boost/lexical_cast.hpp>
#include "sylar/log.h"

namespace sylar {

class ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVarBase> Ptr;

  ConfigVarBase(const std::string& name, const std::string& description = "") :
      name_(name),
      description_(description) {
  }
  virtual ~ConfigVarBase() {};
  const std::string& GetName() const { return name_; }
  const std::string& GetDescription() const { return description_; }
  virtual std::string ToString() = 0;
  virtual bool FromString(const std::string& val) = 0;

 protected:
  std::string name_;
  std::string description_;
};

template<class T>
class ConfigVar : public ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVar> Ptr;

  ConfigVar(const std::string& name, const T& default_value, const std::string& description = "") :
      ConfigVarBase(name, description),
      val_(default_value) {
  }

  std::string ToString() override {
    try {
      return boost::lexical_cast<std::string>(val_);

    } catch (std::exception& e) {
      LOG_ERROR(LOG_ROOT()) << "ConfigVar::ToString exception" << e.what() << " convert: "
          << typeid(val_).name() << " to string";
    }
    return "";
  }

  bool FromString(const std::string& val) override {
    try {
      val_ = boost::lexical_cast<T>(val);
    } catch (std::exception& e) {
      LOG_ERROR(LOG_ROOT()) << "ConfigVar::ToString exception" << e.what()
          << " convert: string to " << typeid(val_).name();
    }
    return false;
  }
 private:
  T val_;
};

class Config {
 public:
  typedef std::map<std::string, ConfigVarBase::Ptr> configVarMap;

  template<class T>
  static typename ConfigVar<T>::Ptr Lookup(const std::string& name, const T& default_value,
      const std::string& description = "") {

  }

  template<class T>
  static typename ConfigVar<T>::Ptr Lookup(const std::string& name) {
    auto it = datas_.find(name);
    if (it == datas_.end()) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
  }
 private:
  static ConfigVarMap datas_;
};

}  // namespace sylar

#endif  // SYLAR_CONFIG_H_
