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

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

using namespace llvm;

// Replace all uses of functions to be instrumented with a wrapper
// that calls the instrumentation library on either side of the call to
// the function. In a later pass, the call to the function will be inlined.
// The reason for this is because some LLVM passes hiccup when a call to an
// external function is made. A case in point here is Polly which originally
// motivated this approach.
//
class GenerateWrappersPass : public ModulePass {
public:
  static char ID;

protected:
  CFEContext& cfeContext;

public:
  GenerateWrappersPass()
      : ModulePass(ID), cfeContext(CFEContext::getSingleton()) {
    ;
  }

  virtual StringRef getPassName() const override {
    return "hwcinstr-wrappers";
  }

  virtual void getAnalysisUsage(AnalysisUsage&) const override {
    ;
  }

  bool addFunction(Module& mod,
                   const std::string& fname,
                   const std::vector<Type*>& paramTypes,
                   Type* retTy = nullptr) {
    LLVMContext& llvmContext = mod.getContext();
    if(not retTy)
      retTy = Type::getVoidTy(llvmContext);

    FunctionType* fty = FunctionType::get(retTy, paramTypes, false);
    Function* f = cast<Function>(mod.getOrInsertFunction(fname, fty));

    f->addFnAttr(Attribute::AttrKind::UWTable);
    f->addFnAttr(Attribute::AttrKind::AlwaysInline);
    f->addFnAttr(Attribute::AttrKind::NoUnwind);

    return true;
  }

  bool addAPIFunctionDecls(Module& mod) {
    bool changed = false;

    Type* fid = hwc::getType<FunctionID>(mod);
    Type* rid = hwc::getType<RegionID>(mod);

    changed |= addFunction(mod, hwc::getFuncEnterFunc(), {fid});
    changed |= addFunction(mod, hwc::getFuncExitFunc(), {fid});
    changed |= addFunction(mod, hwc::getFuncEnterRegion(), {rid});
    changed |= addFunction(mod, hwc::getFuncExitRegion(), {rid});

    return changed;
  }

  bool createWrapper(Function&f, IRBuilder<>& builder) {
    Module& mod = *f.getParent();
    LLVMContext& llvmContext = mod.getContext();
    const hwc::FEFuncMeta& meta = cfeContext.getFuncMeta(f);

    // Create the new function
    FunctionType* fty = f.getFunctionType();
    std::string fname = std::string(".hwcinstr.wrapper.") + f.getName().str();
    Function* wrapper = cast<Function>(mod.getOrInsertFunction(fname, fty));
    wrapper->copyAttributesFrom(&f);

    // The wrapper function is short and should probably get inlined anyway,
    // but just in case, force the issue. No reason to incur additional
    // function call overhead if it can be avoided
    wrapper->removeFnAttr(Attribute::AttrKind::NoInline);
    wrapper->addFnAttr(Attribute::AttrKind::AlwaysInline);

    // Replace all uses of the old function with the new one. Do this first
    // before adding the call to the original function in the wrapper
    f.replaceAllUsesWith(wrapper);

    // // We will want to inline the original function into the wrapper or we'll
    // // end up with additional function call overheads that we would like to
    // // reduce. But it's not clear if this attribute should be added here or
    // // elsewhere because we want this to be inlined only after any other
    // // optimizations have taken place or it will defeat the whole purpose of
    // // creating this wrapper
    // if(not f.hasFnAttribute(Attribute::AttrKind::NoInline))
    //   f.addFnAttr(Attribute::AttrKind::AlwaysInline);

    // Add the body of the function. This will consist of a call to enter
    // function, the call to the original function and a call to exit function
    BasicBlock* bbEntry = BasicBlock::Create(llvmContext, "", wrapper);
    BasicBlock* bbExit = BasicBlock::Create(llvmContext, "", wrapper);

    builder.SetInsertPoint(bbEntry);

    Function* enterFunc = mod.getFunction(hwc::getFuncEnterFunc());
    Value* enterArgs[] = { hwc::getConstant(meta.id, mod) };
    builder.CreateCall(enterFunc->getFunctionType(), enterFunc, enterArgs);

    std::vector<Value*> fargs;
    for(Argument& arg : wrapper->args())
      fargs.push_back(&arg);
    Value* ret = builder.CreateCall(fty, &f, fargs);

    Function* exitFunc = mod.getFunction(hwc::getFuncExitFunc());
    Value* exitArgs = { hwc::getConstant(meta.id, mod) };
    builder.CreateCall(exitFunc->getFunctionType(), exitFunc, exitArgs);

    builder.CreateBr(bbExit);

    builder.SetInsertPoint(bbExit);
    if(fty->getReturnType()->isVoidTy())
      builder.CreateRetVoid();
    else
      builder.CreateRet(ret);

    return true;
  }

  virtual bool runOnModule(Module& mod) override {
    bool changed = false;
    IRBuilder<> builder(mod.getContext());

    changed |= addAPIFunctionDecls(mod);
    for(Function& f : mod.functions()) {
      if(cfeContext.shouldInstrument(f)) {
        changed |= createWrapper(f, builder);
      }
    }

    return changed;
  }
};

char GenerateWrappersPass::ID = 0;

static void registerPass(const PassManagerBuilder&,
                         legacy::PassManagerBase& pm) {
  pm.add(new GenerateWrappersPass());
}

static RegisterStandardPasses
    registerEarly(PassManagerBuilder::EP_ModuleOptimizerEarly, registerPass);

static RegisterStandardPasses
    registerOpt0(PassManagerBuilder::EP_EnabledOnOptLevel0, registerPass);
