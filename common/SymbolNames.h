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

#ifndef HWC_COMMON_SYMBOL_NAMES_H
#define HWC_COMMON_SYMBOL_NAMES_H

#include <string>

namespace hwc {

// The names of the API functions as strings which get used when instrumenting
// the LLVM IR
const std::string& getFuncEnterFunc();
const std::string& getFuncExitFunc();
const std::string& getFuncEnterRegion();
const std::string& getFuncExitRegion();

// The function and region names and other metadata are saved as special
// symbols in the library/executable.
const std::string& getSymFuncMeta();
const std::string& getSymNumFuncMeta();
const std::string& getSymRegionMeta();
const std::string& getSymNumRegionMeta();

} // namespace hwc

#endif // HWC_COMMON_SYMBOL_NAMES_H
