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

#ifndef HWC_COMMON_PAPI_CONTEXT_H
#define HWC_COMMON_PAPI_CONTEXT_H

#include "Types.h"

#include <map>

// Singleton class keeping PAPI-specific library information that is easier
// to query than the regular PAPI API
class PAPIContext {
protected:
  std::map<std::string, CounterID> ids;

public:
  PAPIContext(bool readSymbols);
  PAPIContext(const PAPIContext&) = delete;
  PAPIContext(PAPIContext&&) = delete;

  bool isCounter(const std::string& name) const;
  CounterID getCounterID(const std::string& name) const;
  std::string getCounterShortDescr(CounterID id) const;
  std::string getCounterLongDescr(CounterID id) const;
};

#endif // HWC_COMMON_PAPI_CONTEXT_H
