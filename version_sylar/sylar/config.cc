/**
 * Copyright(c) All rights reserved
 * Author: zhuchun@qq.com
 * Time:   2023-01-02 16:15
 */

#include "sylar/config.h"
#include <list>

namespace sylar {

ConfigVarBase::Ptr Config::LookupBase(const std::string& name) {
  auto it = GetDatas().find(name);
  return it == GetDatas().end() ? nullptr : it->second;
}

//"A.B", 10
//A:
//  B: 10
//  C: str
static void ListAllMember(const std::string& prefix, const YAML::Node& node,
    std::list<std::pair<std::string, const YAML::Node>>& output) {
  if(prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")
      != std::string::npos) {
    LOG_ERROR(LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
    return;
  }
  output.push_back(std::make_pair(prefix, node));
  if (node.IsMap()) {
    for (auto it = node.begin(); it != node.end(); ++it) {
      ListAllMember(prefix.empty() ? it->first.Scalar()
          : prefix + "." + it->first.Scalar(), it->second, output);
    }
  }
}

void Config::LoadFromYaml(const YAML::Node& root) {
  std::list<std::pair<std::string, const YAML::Node>> all_node;
  ListAllMember("", root, all_node);
  for (auto& i : all_node) {
    std::string key = i.first;
    if (key.empty()) {
      continue;
    }
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    ConfigVarBase::Ptr var = LookupBase(key);
    if (var) {
      if (i.second.IsScalar()) {
        var->FromString(i.second.Scalar());
      } else {
        std::stringstream ss;
        ss << i.second;
        var->FromString(ss.str());
      }
    }
  }
}

}  // namespace sylar
