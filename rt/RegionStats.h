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

#ifndef HWC_REGION_STATS_H
#define HWC_REGION_STATS_H

#include "Stats.h"

class RTContext;

class RegionStats : public Stats {
protected:
  RegionID id;
  std::string file;
  unsigned startLine;
  unsigned endLine;

public:
  RegionStats(RTContext& rt,
              const std::vector<CounterID>& counters,
              RegionID id,
              const std::string& file,
              unsigned startLine,
              unsigned endLine);
  RegionStats(const RegionStats&) = delete;
  RegionStats(RegionStats&&) = delete;
  virtual ~RegionStats() = default;

  virtual std::ostream& print(std::ostream& os) const override;
};

#endif // HWC_REGION_STATS_H
