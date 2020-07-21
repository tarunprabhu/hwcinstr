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

#ifndef HWC_RT_CONTEXT_H
#define HWC_RT_CONTEXT_H

#include "FunctionStats.h"
#include "RegionStats.h"
#include "common/PAPIContext.h"

#include <memory>

class RTContext {
protected:
  const PAPIContext papiContext;
  std::string output;
  std::map<FunctionID, std::unique_ptr<FunctionStats>> funcs;
  std::map<RegionID, std::unique_ptr<RegionStats>> regions;

protected:
  std::ostream& printFunctions(std::ostream& os) const;
  std::ostream& printRegions(std::ostream& os) const;

  void readFuncMeta();
  void readRegionMeta();
  void print(std::ostream& os) const;

public:
  RTContext();
  RTContext(const RTContext&) = delete;
  RTContext(RTContext&&) = delete;
  ~RTContext();

  const PAPIContext& getPAPIContext() const;

  std::string getSourceName(FunctionID id) const;
  std::string getQualifiedName(FunctionID id) const;
  std::string getRegionFile(RegionID id) const;
  unsigned getRegionStart(RegionID id) const;
  unsigned getRegionEnd(RegionID id) const;

  bool hasFunctionStats(FunctionID id) const;
  FunctionStats& getFunctionStats(FunctionID id);

  bool hasRegionStats(RegionID id) const;
  RegionStats& getRegionStats(RegionID id);

  void print() const;
};

#endif // HWC_RT_CONTEXT_H
