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

#include "FunctionStats.h"
#include "common/Formatting.h"

FunctionStats::FunctionStats(RTContext& rt,
                             const std::vector<CounterID>& counters,
                             FunctionID id,
                             const std::string& srcName,
                             const std::string& qualName)
    : Stats(rt, counters), id(id), srcName(srcName), qualName(qualName) {
  ;
}

std::ostream& FunctionStats::print(std::ostream& os) const {
  os << tab(2) << quote(id) << ": {\n";

  if(srcName.length())
    os << tab(3) << quote("source") << ": " << quote(srcName) << ",\n";

  if(qualName.size())
    os << tab(3) << quote("qualified") << ": " << quote(qualName) << ",\n";

  Stats::print(os) << "\n";
  os << tab(2) << "}";

  return os;
}
