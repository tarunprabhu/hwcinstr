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

#ifndef HWC_TYPES_H
#define HWC_TYPES_H

#include <stdint.h>
#include <string>
#include <vector>

using FunctionID = uint64_t;
using RegionID = uint64_t;

// These are used in PAPI's API
using CounterID = int;
using CounterValue = long long;

using Time = long long;

namespace hwc {

// These contain the data that is needed for the runtime to do its work.
// The RT* types are easy to add as a constant in LLVM IR
// and accessible in it's raw form using pointers. The FE* types are easier
// to deal with in the frontend code
struct RTFuncMeta {
  FunctionID id;
  const CounterID* counters;
  unsigned numCounters;
  const char* srcName;
  const char* qualName;
};

struct FEFuncMeta {
  FunctionID id;
  const std::vector<CounterID> counters;
  std::string srcName;
  std::string qualName;

  FEFuncMeta(FunctionID id,
             const std::vector<CounterID>& counters,
             const std::string& srcName,
             const std::string& qualName)
      : id(id), counters(counters), srcName(srcName), qualName(qualName) {
    ;
  }
};

struct RTRegionMeta {
  RegionID id;
  const CounterID* counters;
  unsigned numCounters;
  const char* file;
  unsigned startLine;
  unsigned endLine;
};

struct FERegionMeta {
  RegionID id;
  std::vector<CounterID> counters;
  std::string file;
  unsigned startLine;
  unsigned endLine;

  FERegionMeta(RegionID id,
               const std::vector<CounterID>& counters,
               const std::string& file,
               unsigned startLine,
               unsigned endLine)
      : id(id), counters(counters), file(file), startLine(startLine),
        endLine(endLine) {
    ;
  }
};

} // namespace hwc

#endif // HWC_TYPES_H
