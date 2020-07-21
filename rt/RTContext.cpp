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

#include "RTContext.h"
#include "common/Formatting.h"
#include "common/API.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

// These global variables contain the counters to record for each function and
// region and some source-level metadata to make that output easier to read
extern std::byte HWC_GV_META_FUNC __attribute__((weak));
extern std::byte HWC_GV_NUM_META_FUNC __attribute__((weak));
extern std::byte HWC_GV_META_REGION __attribute__((weak));
extern std::byte HWC_GV_NUM_META_REGION __attribute__((weak));

RTContext::RTContext() : papiContext(false) {
  if(const char* val = std::getenv("HWCINSTR"))
    output = val;
  readFuncMeta();
  readRegionMeta();
}

RTContext::~RTContext() {
  print();
}

const PAPIContext& RTContext::getPAPIContext() const {
  return papiContext;
}

void RTContext::readFuncMeta() {
  if(&HWC_GV_META_FUNC and &HWC_GV_NUM_META_FUNC) {
    unsigned numFuncs = *reinterpret_cast<unsigned*>(&HWC_GV_NUM_META_FUNC);
    const auto* funcMeta
        = reinterpret_cast<const hwc::RTFuncMeta*>(&HWC_GV_META_FUNC);
    for(unsigned i = 0; i < numFuncs; i++) {
      const hwc::RTFuncMeta& func = funcMeta[i];
      FunctionID id = func.id;
      std::vector<CounterID> counters(func.counters,
                                      &func.counters[func.numCounters]);
      std::string srcName = func.srcName;
      std::string qualName = func.qualName;
      funcs[id].reset(
          new FunctionStats(*this, counters, id, srcName, qualName));
    }
  }
}

void RTContext::readRegionMeta() {
  if(&HWC_GV_META_REGION and &HWC_GV_NUM_META_REGION) {
    unsigned numRegions = *reinterpret_cast<unsigned*>(&HWC_GV_NUM_META_REGION);
    const auto* regionMeta
        = reinterpret_cast<const hwc::RTRegionMeta*>(&HWC_GV_META_REGION);
    for(unsigned i = 0; i < numRegions; i++) {
      const hwc::RTRegionMeta& region = regionMeta[i];
      RegionID id = region.id;
      std::vector<CounterID> counters(region.counters,
                                      &region.counters[region.numCounters]);
      std::string file = region.file;
      unsigned startLine = region.startLine;
      unsigned endLine = region.endLine;
      regions[id].reset(
          new RegionStats(*this, counters, id, file, startLine, endLine));
    }
  }
}

bool RTContext::hasFunctionStats(FunctionID id) const {
  return funcs.find(id) != funcs.end();
}

FunctionStats& RTContext::getFunctionStats(FunctionID id) {
  return *funcs.at(id);
}

bool RTContext::hasRegionStats(RegionID id) const {
  return regions.find(id) != regions.end();
}

RegionStats& RTContext::getRegionStats(RegionID id) {
  return *regions.at(id);
}

std::ostream& RTContext::printFunctions(std::ostream& os) const {
  bool comma = false;

  os << tab(1) << "functions: {\n";
  for(const auto& i : funcs) {
    if(comma)
      os << ",\n";
    const FunctionStats& stats = *i.second;
    stats.print(os);
    comma = true;
  }
  os << "\n" << tab(1) << "}";

  return os;
}

std::ostream& RTContext::printRegions(std::ostream& os) const {
  bool comma = false;

  os << tab(1) << "regions: {\n";
  for(const auto& i : regions) {
    if(comma)
      os << ",\n";
    const RegionStats& stats = *i.second;
    stats.print(os);
    comma = true;
  }
  os << "\n" << tab(1) << "}";

  return os;
}

void RTContext::print(std::ostream& os) const {
  os << "{\n";
  if(funcs.size()) {
    printFunctions(os);
    if(regions.size()) {
      os << ",\n";
      printRegions(os);
    }
    os << "\n";
  }
  os << "}";
}

void RTContext::print() const {
  if(output.length()) {
    if(output == "-") {
      print(std::cout);
      std::cout << "\n";
    } else {
      std::ofstream of(output.c_str());
      if(of.is_open()) {
        print(of);
        of.close();
      }
    }
  }
}
