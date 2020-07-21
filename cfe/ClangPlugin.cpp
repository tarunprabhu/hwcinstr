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

// Everything will have been taken care of in the "fake" compile, but the
// plugin still needs to return something valid from CreateASTConsumer
class NullConsumer : public ASTConsumer {
  ;
};

class HWCInstrAction : public PluginASTAction {
protected:
  virtual std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance& compiler, llvm::StringRef) override {
    CFEContext& cfeContext = CFEContext::getSingleton();
    const Conf& conf = cfeContext.getConf();

    // This is the usual ridiculous hack that you have to do to associate the
    // Clang AST structures with the LLVM IR. Essentially what this does is
    // parses the AST and creates a temporary LLVM module. The functions in the
    // module are then associated with a Decl. We have to do all this so we
    // know which LLVM function to instrument in the subsequent passes.
    // Doing it by demangling the function names is tricky because templated
    // functions sometimes cause problems. The same is true for thunks although
    // it is not clear that it even makes sense to try and instrument thunks
    // one way or another
    llvm::LLVMContext llvmContext;
    CodeGenerator* cg(CreateLLVMCodeGen(compiler.getDiagnostics(),
                                        "",
                                        compiler.getHeaderSearchOpts(),
                                        compiler.getPreprocessorOpts(),
                                        compiler.getCodeGenOpts(),
                                        llvmContext,
                                        nullptr));

    // Do this "fake" compile silently. If there are any compile errors, it
    // will just do nothing
    std::unique_ptr<DiagnosticConsumer> diagnosticsClient
        = compiler.getDiagnostics().takeClient();
    compiler.getDiagnostics().setClient(new clang::IgnoringDiagConsumer(),
                                        true);

    clang::Preprocessor& pp = compiler.getPreprocessor();
    pp.getBuiltinInfo().initializeBuiltins(pp.getIdentifierTable(),
                                           pp.getLangOpts());
    compiler.setASTConsumer(std::unique_ptr<ASTConsumer>(cg));
    compiler.createSema(getTranslationUnitKind(), nullptr);
    compiler.getDiagnosticClient().BeginSourceFile(compiler.getLangOpts(), &pp);
    ParseAST(compiler.getSema(),
             compiler.getFrontendOpts().ShowStats,
             compiler.getFrontendOpts().SkipFunctionBodies);
    compiler.getDiagnosticClient().EndSourceFile();

    // Before resetting the compiler state, collect everything that we need
    // The llvm::Module will be nullptr if there was an error during parsing
    if(llvm::Module* mod = cg->GetModule()) {
      for(llvm::Function& f : *mod) {
        const std::string& mangled = f.getName();
        if(auto* decl
           = cast_or_null<FunctionDecl>(cg->GetDeclForMangledName(mangled))) {
          const std::string& srcName = decl->getNameAsString();
          const std::string& qualName = decl->getQualifiedNameAsString();
          if(f.size() and conf.has(srcName))
            cfeContext.addFunction(
                mangled, srcName, qualName, conf.getCounters(srcName));
        }
      }
    }

    // Reset compiler state and proceed as normal
    // setASTConsumer(nullptr) will delete the CodeGenerator object created
    compiler.takeSema();
    compiler.setASTConsumer(nullptr);
    compiler.createPreprocessor(getTranslationUnitKind());
    compiler.createASTContext();
    compiler.getDiagnostics().setClient(diagnosticsClient.release(), true);

    return std::make_unique<NullConsumer>();
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
    //   // Example error handling.
    //   DiagnosticsEngine& D = CI.getDiagnostics();
    //   if(args[i] == "-an-error") {
    //     unsigned DiagID = D.getCustomDiagID(DiagnosticsEngine::Error,
    //                                         "invalid argument '%0'");
    //     D.Report(DiagID) << args[i];
    //     return false;
    //   } else if(args[i] == "-parse-template") {
    //     if(i + 1 >= e) {
    //       D.Report(D.getCustomDiagID(DiagnosticsEngine::Error,
    //                                  "missing -parse-template argument"));
    //       return false;
    //     }
    //     ++i;
    //     ParsedTemplates.insert(args[i]);
    //   }
    // }
    // if(!args.empty() && args[0] == "help")
    //   PrintHelp(llvm::errs());

    return true;
  }
  void PrintHelp(llvm::raw_ostream& os) {
    os << "Should print something helpful here\n";
  }
};

static FrontendPluginRegistry::Add<HWCInstrAction>
    X("hwcinstr", "Instrument functions with PAPI");

// static ParsedAttrInfoRegistry::Add<PAPIProfAttrInfo> X("papiprof", "");
