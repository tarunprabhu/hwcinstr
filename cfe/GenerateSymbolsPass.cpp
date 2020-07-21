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

#include "CFEContext.h"
#include "ConvertTypes.h"
#include "ConvertConstants.h"
#include "common/SymbolNames.h"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

// Generates global variables containing any data that the runtime needs to
// do its work. Required things are the counters that must be recorded for
// each function and region. Optional things are source-level names that make
// the output useful
class GenerateSymbolsPass : public ModulePass {
public:
  static char ID;

protected:
  CFEContext& cfeContext;

public:
  GenerateSymbolsPass()
      : ModulePass(ID), cfeContext(CFEContext::getSingleton()) {
    ;
  }

  virtual StringRef getPassName() const override {
    return "hwcinstr-symbols";
  }

  virtual void getAnalysisUsage(AnalysisUsage& AU) const override {
    AU.setPreservesCFG();
  }

  StructType* createFuncMetaTy(Module& mod) {
    // struct FuncMeta {
    //   FunctionID id;
    //   const CounterID* counters;
    //   unsigned numCounters;
    //   const char* srcName;
    //   const char* qualName;
    // };
    Type* types[] = {hwc::getType<FunctionID>(mod),
                     hwc::getType<CounterID*>(mod),
                     hwc::getType<unsigned>(mod),
                     hwc::getType<const char*>(mod),
                     hwc::getType<const char*>(mod)};
    return StructType::create(types, "hwc::FuncMeta");
  }

  StructType* createRegionMetaTy(Module& mod) {
    // struct RegionMeta {
    //   RegionID id;
    //   const CounterID* counters;
    //   unsigned numCounters;
    //   const char* file;
    //   unsigned startLine;
    //   unsigned endLine;
    // };
    Type* types[] = {hwc::getType<FunctionID>(mod),
                     hwc::getType<CounterID*>(mod),
                     hwc::getType<unsigned>(mod),
                     hwc::getType<const char*>(mod),
                     hwc::getType<unsigned>(mod),
                     hwc::getType<unsigned>(mod)};
    return StructType::create(types, "hwc::RegionMeta");
  }

  Constant* getConstExpr(GlobalVariable* g) {
    Type* i32 = Type::getInt32Ty(g->getContext());
    Constant* zero = ConstantInt::get(i32, 0);
    Constant* indices[] = {zero, zero};
    return ConstantExpr::getGetElementPtr(
        g->getType()->getElementType(), g, indices, true);
  }

  Constant* processFunction(Module& mod,
                            const hwc::FEFuncMeta& meta,
                            StructType* metaTy) {
    auto* gSrc = cast<GlobalVariable>(hwc::getConstant(meta.srcName, mod));
    auto* gQual = cast<GlobalVariable>(hwc::getConstant(meta.qualName, mod));
    Constant* cCounters = hwc::getConstant(meta.counters, mod);
    auto* gCounters = new GlobalVariable(mod,
                                         cCounters->getType(),
                                         true,
                                         GlobalValue::PrivateLinkage,
                                         cCounters,
                                         ".hwc.counters");
    gCounters->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    Constant* fields[] = {hwc::getConstant(meta.id, mod),
                          getConstExpr(gCounters),
                          hwc::getConstant<unsigned>(meta.counters.size(), mod),
                          getConstExpr(gSrc),
                          getConstExpr(gQual)};
    return ConstantStruct::get(metaTy, fields);
  }

  bool processFunctions(Module& mod) {
    bool changed = false;

    StructType* metaTy = mod.getTypeByName("hwc::FuncMeta");
    if(not metaTy)
      metaTy = createFuncMetaTy(mod);

    std::vector<Constant*> meta;
    for(Function& f : mod.functions())
      if(cfeContext.shouldInstrument(f))
        meta.push_back(processFunction(mod, cfeContext.getFuncMeta(f), metaTy));

    ArrayType* aty = ArrayType::get(metaTy, meta.size());
    Constant* cMeta = ConstantArray::get(aty, meta);

    auto* gMeta = cast<GlobalVariable>(
        mod.getOrInsertGlobal(hwc::getSymFuncMeta(), cMeta->getType()));
    gMeta->setConstant(true);
    gMeta->setLinkage(GlobalValue::ExternalLinkage);
    gMeta->setInitializer(cMeta);

    auto* gNumMeta = cast<GlobalVariable>(mod.getOrInsertGlobal(
        hwc::getSymNumFuncMeta(), hwc::getType<unsigned>(mod)));
    gNumMeta->setConstant(true);
    gNumMeta->setLinkage(GlobalValue::ExternalLinkage);
    gNumMeta->setInitializer(hwc::getConstant<unsigned>(meta.size(), mod));

    return changed;
  }

  Constant* processRegion(Module& mod,
                          const hwc::FERegionMeta& meta,
                          StructType* metaTy) {
    auto* gFile = cast<GlobalVariable>(hwc::getConstant(meta.file, mod));
    Constant* cCounters = hwc::getConstant(meta.counters, mod);
    auto* gCounters = new GlobalVariable(mod,
                                         cCounters->getType(),
                                         true,
                                         GlobalValue::PrivateLinkage,
                                         cCounters,
                                         "");
    gCounters->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    Constant* fields[] = {hwc::getConstant(meta.id, mod),
                          getConstExpr(gCounters),
                          hwc::getConstant<unsigned>(meta.counters.size(), mod),
                          getConstExpr(gFile),
                          hwc::getConstant(meta.startLine, mod),
                          hwc::getConstant(meta.endLine, mod)};
    return ConstantStruct::get(metaTy, fields);
  }

  bool processRegions(Module& mod) {
    bool changed = false;

    StructType* metaTy = mod.getTypeByName("hwc::RegionMeta");
    if(not metaTy)
      metaTy = createRegionMetaTy(mod);

    std::vector<Constant*> meta;
    for(const hwc::FERegionMeta& region : cfeContext.getRegions())
      meta.push_back(processRegion(mod, region, metaTy));

    ArrayType* aty = ArrayType::get(metaTy, meta.size());
    Constant* cMeta = ConstantArray::get(aty, meta);

    auto* gMeta = cast<GlobalVariable>(
        mod.getOrInsertGlobal(hwc::getSymRegionMeta(), cMeta->getType()));
    gMeta->setConstant(true);
    gMeta->setLinkage(GlobalValue::ExternalLinkage);
    gMeta->setInitializer(cMeta);

    auto* gNumMeta = cast<GlobalVariable>(mod.getOrInsertGlobal(
        hwc::getSymNumRegionMeta(), hwc::getType<unsigned>(mod)));
    gNumMeta->setConstant(true);
    gNumMeta->setLinkage(GlobalValue::ExternalLinkage);
    gNumMeta->setInitializer(hwc::getConstant<unsigned>(meta.size(), mod));

    return changed;
  }

  virtual bool runOnModule(Module& mod) override {
    bool changed = false;

    changed |= processFunctions(mod);
    changed |= processRegions(mod);

    return changed;
  }
};

char GenerateSymbolsPass::ID = 0;

static void registerPass(const PassManagerBuilder&,
                         legacy::PassManagerBase& pm) {
  pm.add(new GenerateSymbolsPass());
}

static RegisterStandardPasses
    registerEarly(PassManagerBuilder::EP_ModuleOptimizerEarly, registerPass);

static RegisterStandardPasses
    registerOpt0(PassManagerBuilder::EP_EnabledOnOptLevel0, registerPass);
