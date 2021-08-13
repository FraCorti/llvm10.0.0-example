#include "llvm/ADT/Optional.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/raw_ostream.h"
#include <clang/Frontend/CompilerInstance.h>
#include "clang/Sema/Sema.h"
#include <clang/Basic/DiagnosticOptions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/CodeGenOptions.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/Version.h"
#include "clang/Driver/Distro.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/Compression.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/Process.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Parse/ParseAST.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Config/config.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/SerializedDiagnosticPrinter.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Sema/CodeCompleteConsumer.h"
#include "clang/Serialization/ASTReader.h"
#include "clang/Serialization/GlobalModuleIndex.h"
#include "clang/Serialization/InMemoryModuleCache.h"
#include "llvm/Support/LockFileManager.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Timer.h"
#include <utility>
#import  <iostream>
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include <memory>
#include <string>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <unordered_map>

using namespace clang::driver;
using namespace clang;
using namespace llvm::opt;
using namespace llvm;
using namespace llvm::sys;

class Compiler {
private:
    LLVMContext context;
    clang::CompilerInstance compiler;
    std::unique_ptr<clang::CodeGenerator> CG;
    std::unordered_map<std::string, llvm::Module *> spoof_operators;
public:
    Compiler(clang::LangOptions LO,
             clang::CodeGenOptions CGO) {
        compiler.getLangOpts() = LO;
        compiler.getCodeGenOpts() = CGO;
        compiler.createDiagnostics();

        std::string TrStr = llvm::Triple::normalize(llvm::sys::getProcessTriple());
        llvm::Triple Tr(TrStr);
        Tr.setOS(Triple::Linux);
        Tr.setVendor(Triple::VendorType::PC);
        Tr.setEnvironment(Triple::EnvironmentType::GNU);
        compiler.getTargetOpts().Triple = Tr.getTriple();
        compiler.setTarget(clang::TargetInfo::CreateTargetInfo(
                compiler.getDiagnostics(),
                std::make_shared<clang::TargetOptions>(compiler.getTargetOpts())));

        compiler.createFileManager();
        compiler.createSourceManager(compiler.getFileManager());
        compiler.createPreprocessor(clang::TU_Prefix);
    }

    llvm::LLVMContext &getContext() {
        return context;
    }

    void init(const char *TestProgram, const std::string &name,
              std::unique_ptr<clang::ASTConsumer> Consumer = nullptr) {

        if (compiler.hasASTContext()) {
            compiler.resetAndLeakASTContext();
        }

        if (compiler.hasSema()) {
            compiler.resetAndLeakSema();
        }

        // compiler.resetAndLeakFileManager();
        // compiler.resetAndLeakPreprocessor();
        // compiler.resetAndLeakSourceManager();

        compiler.createASTContext();

        CG.reset(CreateLLVMCodeGen(compiler.getDiagnostics(),
                                   name,
                                   compiler.getHeaderSearchOpts(),
                                   compiler.getPreprocessorOpts(),
                                   compiler.getCodeGenOpts(),
                                   context));

        compiler.setASTConsumer(std::move(CG));
        compiler.createSema(clang::TU_Prefix, nullptr);

        clang::SourceManager &sm = compiler.getSourceManager();
        sm.setMainFileID(sm.createFileID(
                llvm::MemoryBuffer::getMemBuffer(TestProgram), clang::SrcMgr::C_User));
    }

    const llvm::Module *generateIR() {
        clang::ParseAST(compiler.getSema(), false, false);
        llvm::Module *module = static_cast<clang::CodeGenerator &>(compiler.getASTConsumer()).GetModule();
        /*for (const Function &function : module->getFunctionList()) {
            //spoof_operators.emplace(function.getName(), function)
            outs() << function.getName();
            outs() << function;
        } */
        return module;
    }
};

class ExecutionEngineSpoofs {
private:
    LLVMContext* context;
    std::unique_ptr<llvm::ExecutionEngine> executionEngine;
    std::unique_ptr<llvm::TargetMachine> targetMachine;
    std::unique_ptr<SMDiagnostic> error;
    const Target *machineConfiguration;

    Optional<Reloc::Model> RM;
    Optional<CodeModel::Model> CM;
    CodeGenOpt::Level OL;

    std::string CPU = "generic";
    std::string Features = "";
    llvm::TargetOptions opt;

public:
    ExecutionEngineSpoofs(std::string &mod, LLVMContext& context_) {
        context = &context_;
        OL = CodeGenOpt::Level::Aggressive;
        RM = Optional<Reloc::Model>();
        CM = Optional<CodeModel::Model>();
        error = std::make_unique<SMDiagnostic>();

        std::string TargetTriple = sys::getDefaultTargetTriple();
        std::string Error;
        machineConfiguration = TargetRegistry::lookupTarget(TargetTriple, Error);
        if (!machineConfiguration) {
            errs() << Error;
        }

        targetMachine = std::unique_ptr<llvm::TargetMachine>(
                machineConfiguration->createTargetMachine(TargetTriple, CPU, Features, opt, RM, CM, OL));

        executionEngine = std::unique_ptr<llvm::ExecutionEngine>(
                llvm::EngineBuilder(std::move(llvm::getLazyIRModule(MemoryBuffer::getMemBuffer(StringRef(mod)), *error,
                                                                    *context))).setEngineKind(
                        llvm::EngineKind::Interpreter).create());
    }

    void AddModule(std::unique_ptr<llvm::Module> M) {
        executionEngine->addModule(std::move(M));
    }

    void AddModule(llvm::Module* mod){
        std::string module;
        raw_string_ostream OS(module);
        OS << *mod;
        OS.flush();
        AddModule(module);
    }

    void AddModule(const std::string& mod) {
        std::unique_ptr<llvm::Module> module = llvm::getLazyIRModule(MemoryBuffer::getMemBuffer(StringRef(mod)), *error,
                                                                     *context);
        if (!module) {
            std::string what;
            llvm::raw_string_ostream os(what);
            error->print("error after ParseIR()", os);
            std::cout << what;
        } else {
            executionEngine->addModule(std::move(module));
        }
    }

    // TODO: add support for matrices operations and use
    llvm::GenericValue RunFunction(const std::string &functionName, GenericValue params[]) {
        ArrayRef<GenericValue> args = ArrayRef<GenericValue>(params, 2);
        Function *function = executionEngine->FindFunctionNamed(StringRef(functionName));
        return executionEngine->runFunction(function, args);
    }
};

clang::CodeGenOptions getCommonCodeGenOpts() {
    clang::CodeGenOptions CGOpts;
    CGOpts.StructPathTBAA = 1;
    CGOpts.OptimizationLevel = 3;
    return CGOpts;
}

int main() {

    InitializeAllTargets();
    InitializeAllTargetMCs();

    const char TestProgram[] = "float sum(float a, float b){return a+b;}\n";
    const char TestProgram1[] = "float subtract(float a, float b){return a-b;}\n";

    clang::LangOptions LO_tmp;
    Compiler compiler(LO_tmp, getCommonCodeGenOpts());
    compiler.init(TestProgram, "sum");
    const llvm::Module *sum = compiler.generateIR();

    std::string addition;
    raw_string_ostream OS(addition);
    OS << *sum;
    OS.flush();
    std::cout << addition << std::endl;

    compiler.init(TestProgram1, "subtraction");
    const llvm::Module *subtraction = compiler.generateIR();

    std::string Str1;
    raw_string_ostream OS1(Str1);
    OS1 << *subtraction;
    OS1.flush();
    std::cout << Str1 << std::endl;

    ExecutionEngineSpoofs executionEngine(addition, compiler.getContext());

    executionEngine.AddModule(addition);
    executionEngine.AddModule(Str1);

    llvm::GenericValue param1, param2;
    param1.FloatVal = 10;
    param2.FloatVal = 2;
    GenericValue params[] = {param1, param2};

    std::cout << param1.FloatVal << " + " << param2.FloatVal << " = "
              << executionEngine.RunFunction("sum", params).FloatVal << std::endl;

    std::cout << param1.FloatVal << " - " << param2.FloatVal << " = "
              << executionEngine.RunFunction("subtract", params).FloatVal << std::endl;

    return 0;
}