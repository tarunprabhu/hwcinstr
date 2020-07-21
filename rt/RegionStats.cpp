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

#include "RegionStats.h"
#include "common/Formatting.h"

RegionStats::RegionStats(RTContext& rt,
                         const std::vector<CounterID>& counters,
                         RegionID id,
                         const std::string& file,
                         unsigned startLine,
                         unsigned endLine)
    : Stats(rt, counters), id(id), file(file), startLine(startLine),
      endLine(endLine) {
  ;
}

std::ostream& RegionStats::print(std::ostream& os) const {
  os << tab(2) << quote(id) << ": {\n";

  if(file.length()) {
    if(startLine)
      os << tab(3) << quote("start") << ": "
         << quote(file + std::to_string(startLine)) << "\n";
    if(endLine)
      os << tab(3) << quote("end") << ": "
         << quote(file + std::to_string(endLine)) << "\n";
  }

  Stats::print(os) << "\n";
  os << tab(2) << "}\n";

  return os;
}
