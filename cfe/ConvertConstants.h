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

#ifndef HWC_CONVERT_CONSTANTS_H
#define HWC_CONVERT_CONSTANTS_H

#include "ConvertTypes.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>

namespace hwc {

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
llvm::Constant* getConstant(T id, llvm::Module& mod) {
  return llvm::ConstantInt::get(getType<T>(mod), id);
}

template <typename T>
llvm::Constant* getConstant(const std::vector<T>& vec, llvm::Module& mod) {
  llvm::ArrayType* aty = llvm::ArrayType::get(getType<T>(mod), vec.size());
  std::vector<llvm::Constant*> cvec;
  for(const T& v : vec)
    cvec.push_back(getConstant(v, mod));
  return llvm::ConstantArray::get(aty, cvec);
}

llvm::Constant* getConstant(const std::string& str, llvm::Module& mod);
llvm::Constant* getConstant(const char* str, llvm::Module& mod);

} // namespace hwc

#endif // HWC_CONVERT_CONSTANTS_H
