// Copyright 2020 Tarun Prabhu
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef HWC_COMMON_CONF_H
#define HWC_COMMON_CONF_H

#include "PAPIContext.h"
#include "Types.h"

#include <map>
#include <vector>
#include <yaml.h>

class YAMLNode;
class YAMLList;
class YAMLMap;
class YAMLScalar;

// Parses the conf file that specifies the functions and regions to be
// instrumented and the counters to record for them
class Conf {
protected:
  const PAPIContext& papiContext;
  yaml_parser_t parser;
  std::map<std::string, std::vector<CounterID>> funcs;
  // TODO: Support regions

protected:
  bool check(const YAMLNode* node);
  YAMLNode* consume(yaml_event_t& event, YAMLNode* curr);
  YAMLNode* fail(const std::string& msg = "");
  YAMLNode* parseList(YAMLList* list);
  YAMLNode* parseMap(YAMLMap* map);
  YAMLNode* parse(YAMLNode* curr = nullptr);

public:
  Conf(const PAPIContext& papiContext);
  Conf(const Conf&) = delete;
  Conf(Conf&&) = delete;

  bool parse(const std::string& file);

  bool has(const std::string& func) const;
  const std::vector<CounterID>& getCounters(const std::string& func) const;
};

#endif // HWC_COMMON_CONF_H
