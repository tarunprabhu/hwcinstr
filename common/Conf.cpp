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

#include "Conf.h"
#include "Formatting.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <set>

template <class Iterator>
class IteratorRange {
public:
  Iterator itBegin;
  Iterator itEnd;

public:
  IteratorRange(Iterator itBegin, Iterator itEnd)
      : itBegin(itBegin), itEnd(itEnd) {
    ;
  }

  Iterator begin() const {
    return begin;
  }

  Iterator end() const {
    return end;
  }
};

template <class BaseIter>
class DerefIter : public BaseIter {
public:
  using value_type = typename BaseIter::value_type::element_type;
  using pointer = value_type*;
  using reference = value_type&;

  DerefIter(const BaseIter& other) : BaseIter(other) {
    ;
  }

  reference operator*() const {
    return *(this->BaseIter::operator*());
  }

  pointer operator->() const {
    return this->BaseIter::operator*().get();
  }
};

class YAMLNode {
public:
  enum Kind {
    List,
    Map,
    Scalar,
  };

protected:
  Kind kind;

protected:
  YAMLNode(Kind kind) : kind(kind) {
    ;
  }

public:
  YAMLNode(const YAMLNode&) = delete;
  YAMLNode(YAMLNode&&) = delete;
  virtual ~YAMLNode() = default;

  Kind getKind() const {
    return kind;
  }

  virtual std::ostream& print(std::ostream& os, unsigned depth) const = 0;
};

class YAMLList : public YAMLNode {
protected:
  std::vector<std::unique_ptr<YAMLNode>> data;

  using DataIterator = DerefIter<decltype(data)::const_iterator>;
  using DataRange = IteratorRange<DataIterator>;

public:
  YAMLList() : YAMLNode(YAMLNode::List) {
    ;
  }

  YAMLList(const YAMLList&) = delete;
  YAMLList(YAMLList&&) = delete;
  virtual ~YAMLList() = default;

  void add(YAMLNode* node) {
    data.emplace_back(node);
  }

  DataIterator begin() const {
    return data.begin();
  }

  DataIterator end() const {
    return data.end();
  }

  virtual std::ostream& print(std::ostream& os, unsigned depth) const override {
    os << "\n";
    for(const auto& node : data) {
      os << indent(depth) << " - ";
      node->print(os, depth + 4);
      os << "\n";
    }
    return os;
  }
};

class YAMLMap : public YAMLNode {
protected:
  std::map<std::string, std::unique_ptr<YAMLNode>> data;

public:
  using DataIterator = decltype(data)::const_iterator;
  using DataRange = IteratorRange<DataIterator>;

public:
  YAMLMap() : YAMLNode(YAMLNode::Map) {
    ;
  }

  YAMLMap(const YAMLMap&) = delete;
  YAMLMap(YAMLMap&&) = delete;
  virtual ~YAMLMap() = default;

  void add(const std::string& key, YAMLNode* value) {
    data[key].reset(value);
  }

  bool has(const std::string& key) const {
    return data.find(key) != data.end();
  }

  const YAMLNode* get(const std::string& key) const {
    return data.at(key).get();
  }

  DataIterator begin() const {
    return data.begin();
  }

  DataIterator end() const {
    return data.end();
  }

  virtual std::ostream& print(std::ostream& os, unsigned depth) const override {
    for(const auto& i : data) {
      const std::string& key = i.first;
      const std::unique_ptr<YAMLNode>& val = i.second;
      os << indent(depth) << key << ":";
      val->print(os, depth + 2);
      os << "\n";
    }
    return os;
  }
};

class YAMLScalar : public YAMLNode {
protected:
  std::string val;

public:
  YAMLScalar(yaml_char_t* val)
      : YAMLNode(YAMLNode::Scalar), val(reinterpret_cast<const char*>(val)) {
    ;
  }

  YAMLScalar(const YAMLScalar&) = delete;
  YAMLScalar(YAMLScalar&&) = delete;
  virtual ~YAMLScalar() = default;

  const std::string& get() const {
    return val;
  }

  virtual std::ostream& print(std::ostream& os, unsigned) const override {
    os << val;
    return os;
  }
};

Conf::Conf(const PAPIContext& papiContext) : papiContext(papiContext) {
  ;
}

YAMLNode* Conf::consume(yaml_event_t& event, YAMLNode* curr) {
  switch(event.type) {
  case YAML_STREAM_START_EVENT:
    return parse();
  case YAML_DOCUMENT_START_EVENT:
    return parse();
  case YAML_SEQUENCE_START_EVENT:
    return parseList(new YAMLList());
  case YAML_MAPPING_START_EVENT:
    return parseMap(new YAMLMap());
  case YAML_SCALAR_EVENT:
    return new YAMLScalar(event.data.scalar.value);
  case YAML_DOCUMENT_END_EVENT:
    return parse(curr);
  case YAML_STREAM_END_EVENT:
    return curr;
  case YAML_NO_EVENT:
    return parse(curr);
  default:
    std::cerr << "Unexpected parser event: " << event.type << "\n";
    return nullptr;
  }
}

YAMLNode* Conf::parseMap(YAMLMap* map) {
  yaml_event_t event;
  yaml_parser_parse(&parser, &event);
  while(event.type != YAML_MAPPING_END_EVENT) {
    std::string key;
    if(event.type == YAML_SCALAR_EVENT)
      key = reinterpret_cast<const char*>(event.data.scalar.value);
    else
      return fail("Could not find scalar key");

    if(YAMLNode* val = parse(map))
      map->add(key, val);
    else
      return fail();

    yaml_parser_parse(&parser, &event);
  }

  // return parse(static_cast<YAMLNode*>(map));
  return map;
}

YAMLNode* Conf::parseList(YAMLList* list) {
  yaml_event_t event;
  yaml_parser_parse(&parser, &event);
  while(event.type != YAML_SEQUENCE_END_EVENT) {
    if(YAMLNode* val = consume(event, list))
      list->add(val);
    else
      return fail("Could not parse list element");
    yaml_parser_parse(&parser, &event);
  }

  // return parse(static_cast<YAMLNode*>(list));
  return list;
}

YAMLNode* Conf::parse(YAMLNode* curr) {
  yaml_event_t event;
  if(not yaml_parser_parse(&parser, &event))
    return fail("Parse error");
  return consume(event, curr);
}

YAMLNode* Conf::fail(const std::string& msg) {
  if(msg.length())
    std::cerr << msg << "\n";
  yaml_parser_delete(&parser);
  return nullptr;
}

bool Conf::parse(const std::string& file) {
  yaml_parser_initialize(&parser);

  FILE* fp = fopen(file.c_str(), "r");
  if(not fp)
    return fail("Could not open file: " + file);

  yaml_parser_set_input_file(&parser, fp);
  if(YAMLNode* root = parse()) {
    if(not check(root))
      return fail();

    std::vector<CounterID> counters;
    const YAMLMap* map = static_cast<const YAMLMap*>(root);
    if(map->has("counters")) {
      const YAMLList* list = static_cast<const YAMLList*>(map->get("counters"));
      for(const YAMLNode& elem : *list)
        counters.push_back(papiContext.getCounterID(
            static_cast<const YAMLScalar&>(elem).get()));
    }

    if(map->has("functions")) {
      const YAMLList* fns
          = static_cast<const YAMLList*>(map->get("functions"));
      for(const YAMLNode& elem : *fns)
        funcs[static_cast<const YAMLScalar&>(elem).get()] = counters;
    }

    delete root;
  } else {
    fail();
    return false;
  }
  return true;
}

bool Conf::check(const YAMLNode* root) {
  if(root->getKind() != YAMLNode::Map)
    return fail("Root of the config file should be a map");

  const YAMLMap* map = static_cast<const YAMLMap*>(root);
  std::set<std::string> keys = {"counters", "functions", "regions"};
  for(const auto& i : *map) {
    const std::string& key = i.first;
    if(keys.find(key) == keys.end())
      return fail("Unexpected key at top level: " + key);
  }

  if(map->has("counters")) {
    const YAMLNode* node = map->get("counters");
    if(node->getKind() != YAMLNode::List)
      return fail("Counters must be a list");

    for(const YAMLNode& elem : *static_cast<const YAMLList*>(node)) {
      if(elem.getKind() != YAMLNode::Scalar)
        return fail("Counter element must be a scalar");
      const YAMLScalar& scalar = static_cast<const YAMLScalar&>(elem);
      if(not papiContext.isCounter(scalar.get()))
        return fail("Counter element is not a PAPI counter: " + scalar.get());
    }
  }

  if(map->has("functions")) {
    const YAMLNode* node = map->get("functions");
    if(node->getKind() != YAMLNode::List)
      return fail("Function must be a list");
    for(const YAMLNode& elem : *static_cast<const YAMLList*>(node)) {
      if(elem.getKind() != YAMLNode::Scalar)
        return fail("Functions element must be a scalar");
    }
  }

  return true;
}

bool Conf::has(const std::string& func) const {
  return funcs.find(func) != funcs.end();
}

const std::vector<CounterID>& Conf::getCounters(const std::string& func) const {
  return funcs.at(func);
}
