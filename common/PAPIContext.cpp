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

#include "PAPIContext.h"

#include <papi.h>

#include <iostream>

static std::string getKey(const std::string& name) {
  if(name.find("PAPI") == 0)
    return name;
  return "PAPI_" + name;
}

PAPIContext::PAPIContext(bool readSymbols) {
  PAPI_library_init(PAPI_VER_CURRENT);

  if(readSymbols) {
    CounterID i = 0 | PAPI_PRESET_MASK;
    if(PAPI_enum_event(&i, PAPI_ENUM_FIRST) == PAPI_OK) {
      do {
        PAPI_event_info_t info;
        if(PAPI_get_event_info(i, &info) == PAPI_OK)
          ids[info.symbol] = i;
      } while(PAPI_enum_event(&i, PAPI_ENUM_ALL) == PAPI_OK);
    }
  }
}

bool PAPIContext::isCounter(const std::string& name) const {
  return ids.find(getKey(name)) != ids.end();
}

CounterID PAPIContext::getCounterID(const std::string& name) const {
  return ids.at(getKey(name));
}

std::string PAPIContext::getCounterShortDescr(CounterID id) const {
  PAPI_event_info_t info;
  PAPI_get_event_info(id, &info);
  return info.short_descr;
}

std::string PAPIContext::getCounterLongDescr(CounterID id) const {
  PAPI_event_info_t info;
  PAPI_get_event_info(id, &info);
  return info.long_descr;
}
