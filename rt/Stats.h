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

#ifndef HWC_STATS_H
#define HWC_STATS_H

#include "common/Types.h"

#include <papi.h>

#include <chrono>
#include <map>
#include <vector>

class RTContext;

class Stats {
protected:
  RTContext& rt;

  // The PAPI event set that is used to tell PAPI which counters to record
  int eventSet;

  // The start of an interval of time being measured
  Time time;

  // The number of times the function was called or the number of times the
  // region was entered
  int64_t occurs;

  // The counters to record for this object
  const std::vector<CounterID> counters;

  // Temporary array used when reading counters
  std::vector<CounterValue> snapshot;

  // The actual counter data
  std::vector<CounterValue> data;

protected:
  Time tick();

public:
  Stats(RTContext& papiContext, const std::vector<CounterID>& counters);
  Stats(const Stats&) = delete;
  Stats(const Stats&&) = delete;
  virtual ~Stats() = default;

  void start();
  void stop();
  CounterValue get(CounterID) const;
  Time getTime() const;
  int64_t getOccurs() const;

  virtual std::ostream& print(std::ostream& os) const;
};

#endif // HWC_STATS_H
