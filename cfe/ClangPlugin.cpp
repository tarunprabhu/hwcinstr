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
#include "../common/Conf.h"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Attr.h>
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Sema/ParsedAttr.h>
#include <clang/Sema/Sema.h>
#include <clang/Sema/SemaDiagnostic.h>

#include <llvm/IR/Module.h>

using namespace clang;

// The consumer is a wrapper around a code generator which generates an
// LLVM module. The functions in that module are then associated with the
// Clang Decl's. The Decl's can't be kept around until the final LLVM module
// is generated and instrumented because the ASTContext object containing the
// Decl's will have been deleted by then. So everything has to be associated
// after the translation unit has been handled by the code generator
class Consumer : public ASTConsumer {
protected:
  llvm::LLVMContext llvmContext;
  CodeGenerator& cg;
  std::unique_ptr<CodeGenerator> pcg;

public:
  explicit Consumer(CompilerInstance& compiler)
      : cg(*CreateLLVMCodeGen(compiler.getDiagnostics(),
                              "",
                              compiler.getHeaderSearchOpts(),
                              compiler.getPreprocessorOpts(),
                              compiler.getCodeGenOpts(),
                              llvmContext,
                              nullptr)) {
    pcg.reset(&cg);
  }

  virtual void HandleTranslationUnit(ASTContext& astContext) override {
    cg.HandleTranslationUnit(astContext);

    CFEContext& cfeContext = CFEContext::getSingleton();
    const Conf& conf = cfeContext.getConf();

    // If there was an error during code generation the llvm::Module will be
    // null
    if(llvm::Module* mod = cg.GetModule()) {
      for(llvm::Function& f : *mod) {
        const std::string& mangled = f.getName();
        if(auto* decl
           = cast_or_null<FunctionDecl>(cg.GetDeclForMangledName(mangled))) {
          const std::string& srcName = decl->getNameAsString();
          const std::string& qualName = decl->getQualifiedNameAsString();
          if(f.size() and conf.has(srcName))
            cfeContext.addFunction(
                mangled, srcName, qualName, conf.getCounters(srcName));
        }
      }
    }
  }

  virtual void Initialize(ASTContext& astContext) override {
    return cg.Initialize(astContext);
  }

  virtual bool HandleTopLevelDecl(DeclGroupRef g) override {
    return cg.HandleTopLevelDecl(g);
  }

  virtual void HandleInlineFunctionDefinition(FunctionDecl* f) override {
    return cg.HandleInlineFunctionDefinition(f);
  }

  virtual void HandleInterestingDecl(DeclGroupRef g) override {
    return cg.HandleInterestingDecl(g);
  }

  virtual void HandleTagDeclDefinition(TagDecl* tag) override {
    return cg.HandleTagDeclDefinition(tag);
  }

  virtual void HandleTagDeclRequiredDefinition(const TagDecl* tag) override {
    return cg.HandleTagDeclRequiredDefinition(tag);
  }

  virtual void
  HandleCXXImplicitFunctionInstantiation(FunctionDecl* f) override {
    return cg.HandleCXXImplicitFunctionInstantiation(f);
  }

  virtual void CompleteTentativeDefinition(VarDecl* var) override {
    return cg.CompleteTentativeDefinition(var);
  }

  virtual void AssignInheritanceModel(CXXRecordDecl* record) override {
    return cg.AssignInheritanceModel(record);
  }

  virtual void HandleCXXStaticMemberVarInstantiation(VarDecl* var) override {
    return cg.HandleCXXStaticMemberVarInstantiation(var);
  }

  virtual void HandleVTable(CXXRecordDecl* record) override {
    return cg.HandleVTable(record);
  }
};

class HWCInstrAction : public PluginASTAction {
protected:
  virtual std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance& compiler, llvm::StringRef) override {
    return std::make_unique<Consumer>(compiler);
  }

  // Automatically run the plugin after the main AST action
  PluginASTAction::ActionType getActionType() override {
    return PluginASTAction::AddBeforeMainAction;
  }

  bool ParseArgs(const CompilerInstance& compiler,
                 const std::vector<std::string>& args) override {
    CFEContext& cfeContext = CFEContext::getSingleton();
    DiagnosticsEngine& diag = compiler.getDiagnostics();
    for(unsigned i = 0; i < args.size(); i++) {
      if(args[i] == "-conf") {
        const std::string& file = args[i + 1];
        if((i + 1) >= args.size()) {
          unsigned id
              = diag.getCustomDiagID(DiagnosticsEngine::Error,
                                     "hwcinstr: Required argument for -conf");
          diag.Report(id);
          return false;
        }

        Conf& conf = cfeContext.getConf();
        if(not conf.parse(file)) {
          unsigned id
              = diag.getCustomDiagID(DiagnosticsEngine::Error,
                                     "hwcinstr: Could not parse config file");
          diag.Report(id);
          return false;
        }
        i += 1;
      } else if(args[i] == "-help") {
        PrintHelp(llvm::errs());
        return false;
      } else {
        unsigned id = diag.getCustomDiagID(DiagnosticsEngine::Error,
                                           "Unknown argument: '%0'");
        diag.Report(id) << args[i];
        return false;
      }
    }

    return true;
  }
  void PrintHelp(llvm::raw_ostream& os) {
    os << "Should print something helpful here\n";
  }
};

static FrontendPluginRegistry::Add<HWCInstrAction>
    X("hwcinstr", "Instrument functions with PAPI");
