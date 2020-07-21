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

#include "SymbolNames.h"
#include "API.h"

#define QUOTE_(s) #s
#define QUOTE(s) QUOTE_(s)

static const std::string symFuncMeta = QUOTE(HWC_GV_META_FUNC);
static const std::string symNumFuncMeta = QUOTE(HWC_GV_NUM_META_FUNC);
static const std::string symRegionMeta = QUOTE(HWC_GV_META_REGION);
static const std::string symNumRegionMeta = QUOTE(HWC_GV_NUM_META_REGION);
static const std::string funcEnterFunc = QUOTE(HWC_ENTER_FUNC);
static const std::string funcExitFunc = QUOTE(HWC_EXIT_FUNC);
static const std::string funcEnterRegion = QUOTE(HWC_ENTER_REGION);
static const std::string funcExitRegion = QUOTE(HWC_EXIT_REGION);

namespace hwc {

const std::string& getFuncEnterFunc() {
  return funcEnterFunc;
}

const std::string& getFuncExitFunc() {
  return funcExitFunc;
}

const std::string& getFuncEnterRegion() {
  return funcEnterRegion;
}

const std::string& getFuncExitRegion() {
  return funcExitRegion;
}

const std::string& getSymFuncMeta() {
  return symFuncMeta;
}

const std::string& getSymNumFuncMeta() {
  return symNumFuncMeta;
}

const std::string& getSymRegionMeta() {
  return symRegionMeta;
}

const std::string& getSymNumRegionMeta() {
  return symNumRegionMeta;
}

} // namespace hwc
