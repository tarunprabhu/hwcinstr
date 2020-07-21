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

#ifndef HWC_API_H
#define HWC_API_H

#include "Types.h"

#define HWC_PREFIX hwcinstr

#define SYM_NAME_(prefix, fn) prefix##__##fn##_
#define SYM_NAME(prefix, fn) SYM_NAME_(prefix, fn)

#define FUNC_NAME(fn) SYM_NAME(HWC_PREFIX, fn)
#define GV_NAME(g) SYM_NAME(HWC_PREFIX, g)

#define HWC_GV_META_FUNC GV_NAME(gv_meta_func)
#define HWC_GV_NUM_META_FUNC GV_NAME(gv_num_meta_func)
#define HWC_GV_META_REGION GV_NAME(gv_meta_region)
#define HWC_GV_NUM_META_REGION GV_NAME(gv_num_meta_region)

#define HWC_ENTER_FUNC FUNC_NAME(enter_func)
#define HWC_EXIT_FUNC FUNC_NAME(exit_func)
#define HWC_ENTER_REGION FUNC_NAME(enter_region)
#define HWC_EXIT_REGION FUNC_NAME(exit_region)

extern "C" {

void HWC_ENTER_FUNC(FunctionID id);
void HWC_EXIT_FUNC(FunctionID id);
void HWC_ENTER_REGION(RegionID id);
void HWC_EXIT_REGION(RegionID id);

} // extern "C"

#endif // HWC_API_H
