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

// Calls to this will be added by the LLVM instrumentation passes

#include "common/API.h"
#include "FunctionStats.h"
#include "RTContext.h"
#include "RegionStats.h"

// Singleton global object that contains everything. It doesn't matter when
// this gets initialized because all the data needed for the initialization
// is saved in the object itself. The destructor will call the print method,
// so it is guaranteed to run after everything is done.
//
// FIXME: The only problem with this is that it is not thread-safe.
static RTContext rt;

extern "C" {

[[gnu::used]] void HWC_ENTER_FUNC(FunctionID id) {
  rt.getFunctionStats(id).start();
}

[[gnu::used]] void HWC_EXIT_FUNC(FunctionID id) {
  rt.getFunctionStats(id).stop();
}

[[gnu::used]] void HWC_ENTER_REGION(RegionID id) {
  rt.getRegionStats(id).start();
}

[[gnu::used]] void HWC_EXIT_REGION(RegionID id) {
  rt.getRegionStats(id).stop();
}

} // extern "C"
