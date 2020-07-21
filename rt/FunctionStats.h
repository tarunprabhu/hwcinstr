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

#ifndef HWC_FUNCTION_STATS_H
#define HWC_FUNCTION_STATS_H

#include "Stats.h"

class RTContext;

class FunctionStats : public Stats {
protected:
  FunctionID id;
  std::string srcName;
  std::string qualName;

public:
  FunctionStats(RTContext& rt,
                const std::vector<CounterID>& counters,
                FunctionID id,
                const std::string& srcName,
                const std::string& qualName);
  FunctionStats(const FunctionStats&) = delete;
  FunctionStats(FunctionStats&&) = delete;
  virtual ~FunctionStats() = default;

  virtual std::ostream& print(std::ostream& os) const override;
};

#endif // HWC_FUNCTION_STATS_H
