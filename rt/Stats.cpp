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

#include "Stats.h"
#include "RTContext.h"
#include "common/Formatting.h"

#include <iostream>

Stats::Stats(RTContext& rt, const std::vector<CounterID>& counters)
    : rt(rt), eventSet(PAPI_NULL), time(0), occurs(0), counters(counters),
      snapshot(counters.size(), 0), data(counters.size(), 0) {
  PAPI_create_eventset(&eventSet);
  for(CounterID event : counters)
    PAPI_add_event(eventSet, event);
}

Time Stats::tick() {
  auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::time_point_cast<std::chrono::nanoseconds>(now)
      .time_since_epoch()
      .count();
}

void Stats::start() {
  time -= tick();
  occurs += 1;
  if(counters.size()) {
    PAPI_start(eventSet);
    PAPI_read(eventSet, snapshot.data());
    for(unsigned i = 0; i < counters.size(); i++)
      data[i] -= snapshot[i];
  }
}

void Stats::stop() {
  time += tick();
  if(counters.size()) {
    PAPI_stop(eventSet, snapshot.data());
    for(unsigned i = 0; i < counters.size(); i++)
      data[i] += snapshot[i];
  }
}

CounterValue Stats::get(CounterID id) const {
  return data.at(id);
}

Time Stats::getTime() const {
  return time;
}

int64_t Stats::getOccurs() const {
  return occurs;
}

std::ostream& Stats::print(std::ostream& os) const {
  os << tab(3) << quote("Occurs") << ": " << occurs << ",\n";
  os << tab(3) << quote("Time") << ": " << time << ",\n";

  const PAPIContext& papiContext = rt.getPAPIContext();
  bool comma = false;
  for(unsigned i = 0; i < counters.size(); i++) {
    CounterID counter = counters[i];
    if(comma)
      os << ",\n";

    os << tab(3) << quote(papiContext.getCounterShortDescr(counter)) << ": "
       << data.at(i);
    comma = true;
  }

  return os;
}
