/**

 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-01-02 16:04
 */

#include "sylar/config.h"
#include "sylar/log.h"
#include <yaml-cpp/yaml.h>
#include <iostream>

#if 0
sylar::ConfigVar<int>::Ptr g_int_value_config = sylar::Config::Lookup("system.port", (int)8080,
    "system port");
sylar::ConfigVar<float>::Ptr g_int_valuex_config = sylar::Config::Lookup("system.port", (float)8080,
    "system port");
sylar::ConfigVar<float>::Ptr g_float_value_config = sylar::Config::Lookup("system.value",
    (float)10.2f, "system value");
sylar::ConfigVar<std::vector<int>>::Ptr g_int_vec_value_config =
    sylar::Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "system int vec");
sylar::ConfigVar<std::list<int>>::Ptr g_int_list_value_config =
    sylar::Config::Lookup("system.int_list", std::list<int>{1, 2}, "system int list");
sylar::ConfigVar<std::set<int>>::Ptr g_int_set_value_config =
    sylar::Config::Lookup("system.int_set", std::set<int>{1, 2}, "system int set");
sylar::ConfigVar<std::unordered_set<int>>::Ptr g_int_uset_value_config =
    sylar::Config::Lookup("system.int_uset", std::unordered_set<int>{1, 2}, "system int uset");
sylar::ConfigVar<std::map<std::string, int>>::Ptr g_str_int_map_value_config =
    sylar::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k", 2}}, "system str int map");
sylar::ConfigVar<std::unordered_map<std::string, int>>::Ptr g_str_int_umap_value_config =
    sylar::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k", 2}}, "system str int umap");

void PrintYaml(const YAML::Node& node, int level) {
  if (node.IsScalar()) {
    LOG_INFO(LOG_ROOT()) << std::string(level * 4, ' ')
        << node.Scalar() << " - " << node.Type() << " - " << level;
  } else if (node.IsNull()) {
    LOG_INFO(LOG_ROOT()) << std::string(level * 4, ' ')
        << "NULL - " << node.Type() << " - " << level;
  } else if (node.IsMap()) {
    for (auto it = node.begin(); it != node.end(); ++it) {
      LOG_INFO(LOG_ROOT()) << std::string(level * 4, ' ')
          << it->first << " - " << it->second.Type() << " - " << level;
      PrintYaml(it->second, level + 1);
    }
  } else if (node.IsSequence()) {
    for (size_t i = 0; i < node.size(); ++i) {
      LOG_INFO(LOG_ROOT()) << std::string(level * 4, ' ')
          << i << " - " << node[i].Type() << " - " << level;
      PrintYaml(node[i], level + 1);
    }
  }
}

void TestYaml() {
  YAML::Node root = YAML::LoadFile("/home/zhuchun/workspace/webServer"
      "/version_sylar/bin/conf/test.yml");
  PrintYaml(root, 0);
  //LOG_INFO(LOG_ROOT()) << root;
}

void TestConfig() {
  LOG_INFO(LOG_ROOT()) << "before: " << g_int_value_config->GetValue();
  LOG_INFO(LOG_ROOT()) << "before: " << g_float_value_config->ToString();
#define XX(g_var, name, prefix) \
  { \
    auto& v = g_var->GetValue(); \
    for (auto& i : v) { \
      LOG_INFO(LOG_ROOT()) << #prefix " " #name ": " << i; \
    } \
    LOG_INFO(LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->ToString(); \
  }

#define XX_M(g_var, name, prefix) \
  { \
    auto& v = g_var->GetValue(); \
    for (auto& i : v) { \
      LOG_INFO(LOG_ROOT()) << #prefix " " #name ": { " << i.first << " - " << i.second << " }"; \
    } \
    LOG_INFO(LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->ToString(); \
  }

  XX(g_int_vec_value_config, int_vec, before);
  XX(g_int_list_value_config, int_list, before);
  XX(g_int_set_value_config, int_set, before);
  XX(g_int_uset_value_config, int_uset, before);
  XX_M(g_str_int_map_value_config, str_int_map, before);
  XX_M(g_str_int_umap_value_config, str_int_umap, before);

  YAML::Node root = YAML::LoadFile("/home/zhuchun/workspace/webServer"
      "/version_sylar/bin/conf/test.yml");
  sylar::Config::LoadFromYaml(root);

  LOG_INFO(LOG_ROOT()) << "after: " << g_int_value_config->GetValue();
  LOG_INFO(LOG_ROOT()) << "after: " << g_float_value_config->ToString();

  XX(g_int_vec_value_config, int_vec, after);
  XX(g_int_list_value_config, int_list, after);
  XX(g_int_set_value_config, int_set, after);
  XX(g_int_uset_value_config, int_uset, after);
  XX_M(g_str_int_map_value_config, str_int_map, after);
  XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

#endif

class Person {
 public:
  std::string name_;
  int age_ = 0;
  bool sex_ = false;

  std::string ToString() const {
    std::stringstream ss;
    ss << "[Person name=" << name_ << " age=" << age_ << " sex=" << sex_ << "]";
    return ss.str();
  }

  bool operator==(const Person& oth) const {
    return name_ == oth.name_ && age_ == oth.age_ && sex_ == oth.sex_;
  }
};

namespace sylar {
template<>
class LexicalCast<std::string, Person> {
 public:
  Person operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    Person p;
    p.name_ = node["name"].as<std::string>();
    p.age_ = node["age"].as<int>();
    p.sex_ = node["sex"].as<bool>();
    return p;
  }
};

template<>
class LexicalCast<Person, std::string> {
 public:
  std::string operator()(const Person& p) {
    YAML::Node node;
    node["name"] = p.name_;
    node["age"] = p.age_;
    node["sex"] = p.sex_;
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};
} // namespace sylar

sylar::ConfigVar<Person>::Ptr g_person = sylar::Config::Lookup("class.person", Person(),
    "class person");

void test_class() {
  LOG_INFO(LOG_ROOT()) << "before: " << g_person->GetValue().ToString() << " - " << g_person->ToString();

  g_person->AddListener([](const Person old_value, const Person new_value) {
    LOG_INFO(LOG_ROOT()) << "old_value=" << old_value.ToString()
      << " new_value=" << new_value.ToString();
  });
  YAML::Node root = YAML::LoadFile("/home/zhuchun/workspace/webServer"
      "/version_sylar/bin/conf/test.yml");
  sylar::Config::LoadFromYaml(root);

  LOG_INFO(LOG_ROOT()) << "after: " << g_person->GetValue().ToString() << " - " << g_person->ToString();

}

void test_log() {
  static sylar::Logger::Ptr system_log = LOG_NAME("system");
  LOG_INFO(system_log) << "hello system" << std::endl;

  std::cout << sylar::LoggerMgr::GetInstance()->ToYamlString() << std::endl;
  YAML::Node root = YAML::LoadFile("/home/zhuchun/workspace/webServer"
      "/version_sylar/bin/conf/log.yml");
  sylar::Config::LoadFromYaml(root);
  std::cout << "====================" << std::endl;
  std::cout << sylar::LoggerMgr::GetInstance()->ToYamlString() << std::endl;
  std::cout << "====================" << std::endl;
  std::cout << root << std::endl;
  LOG_INFO(system_log) << "hello system" << std::endl;

  system_log->SetFormatter("%d - %m%n");
  LOG_INFO(system_log) << "hello system" << std::endl;
}

int main() {
  // TestYaml();
  // TestConfig();
  // test_class();
  test_log();
  sylar::Config::Visit([](sylar::ConfigVarBase::Ptr var) {
    LOG_INFO(LOG_ROOT()) << "name=" << var->GetName()
        << " description=" << var->GetDescription()
        << " typename=" << var->GetTypeName()
        << " value=" << var->ToString();
  });
  return 0;
}
