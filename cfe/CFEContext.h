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

#ifndef HWC_CFE_CONTEXT_H
#define HWC_CFE_CONTEXT_H

#include "common/Conf.h"
#include "common/PAPIContext.h"
#include "common/Types.h"

#include <clang/AST/DeclCXX.h>

#include <llvm/IR/Function.h>

// Class that contains all the data that will be collected by the Clang plugin
// and used by the LLVM pass
class CFEContext {
protected:
  const PAPIContext papiContext;
  Conf conf;
  std::map<std::string, hwc::FEFuncMeta> funcs;
  std::vector<hwc::FERegionMeta> regions;

public:
  using region_iterator = decltype(regions)::const_iterator;
  using region_range = llvm::iterator_range<region_iterator>;

protected:
  const clang::FunctionDecl* getDecl(llvm::Function& f) const;

public:
  CFEContext();
  CFEContext(const CFEContext&) = delete;
  CFEContext(CFEContext&&) = delete;

  void addFunction(const std::string& mangled,
                   const std::string& srcName,
                   const std::string& qualName,
                   const std::vector<CounterID>& counters);
  void addRegion(const std::string& file,
                 unsigned start,
                 unsigned end,
                 const std::vector<CounterID>& counters);
  llvm::LLVMContext& getLLVMContext();

  Conf& getConf();
  const Conf& getConf() const;
  const PAPIContext& getPAPIContext() const;
  bool shouldInstrument(llvm::Function& f) const;
  const hwc::FEFuncMeta& getFuncMeta(llvm::Function& f) const;

  region_range getRegions() const;

public:
  static CFEContext& getSingleton();
};

#endif // HWC_CFE_CONTEXT_H
