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

#ifndef HWC_CONVERT_TYPES_H
#define HWC_CONVERT_TYPES_H

#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

namespace hwc {

template <typename T, std::enable_if_t<std::is_void<T>::value, int> = 0>
llvm::Type* getType(llvm::Module& mod) {
  return llvm::Type::getVoidTy(mod.getContext());
}

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
llvm::Type* getType(llvm::Module& mod) {
  return llvm::IntegerType::get(mod.getContext(), sizeof(T) * 8);
}

template <typename T, std::enable_if_t<std::is_pointer<T>::value, int> = 0>
llvm::Type* getType(llvm::Module& mod) {
  return getType<typename std::remove_pointer<T>::type>(mod)->getPointerTo();
}

} // namespace hwc

#endif // HWC_CONVERT_TYPES_H
