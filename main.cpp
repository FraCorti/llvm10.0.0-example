#include "clang/Driver/ToolChain.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/VersionTuple.h"
#include "clang/Basic/DebugInfoOptions.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Tool.h"
#include "clang/Driver/ToolChain.h"
#include "clang/Basic/DebugInfoOptions.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Types.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/raw_ostream.h"
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include "clang/Sema/Sema.h"
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/CodeGenOptions.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/ObjCRuntime.h"
#include "clang/Basic/Version.h"
#include "clang/Driver/Distro.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "clang/Driver/SanitizerArgs.h"
#include "clang/Driver/XRayArgs.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Compression.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/TargetParser.h"
#include "llvm/Support/YAMLParser.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Parse/ParseAST.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangStandard.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Stack.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/Version.h"
#include "clang/Config/config.h"
#include "clang/Frontend/ChainedDiagnosticConsumer.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/LogDiagnosticPrinter.h"
#include "clang/Frontend/SerializedDiagnosticPrinter.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/Utils.h"
#include "clang/Frontend/VerifyDiagnosticConsumer.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Sema/CodeCompleteConsumer.h"
#include "clang/Sema/Sema.h"
#include "clang/Serialization/ASTReader.h"
#include "clang/Serialization/GlobalModuleIndex.h"
#include "clang/Serialization/InMemoryModuleCache.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/BuryPointer.h"
#include "llvm/Support/CrashRecoveryContext.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/LockFileManager.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/TimeProfiler.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include <time.h>
#include <utility>
#import  <iostream>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <memory>
#include <string>
#include <utility>
#include <llvm/Support/CommandLine.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "llvm/ToolDrivers/llvm-dlltool/DlltoolDriver.h"
#include <unordered_map>
#include <fstream>

using namespace clang::driver;
using namespace clang;
using namespace llvm::opt;
using namespace llvm;
using namespace llvm::sys;

/*
struct MyASTConsumer;

static void test_codegen_fns(MyASTConsumer *my);

static bool test_codegen_fns_ran;

struct MyASTConsumer : public ASTConsumer {
    std::unique_ptr<CodeGenerator> Builder;
    std::vector<Decl *> toplevel_decls;

    MyASTConsumer(std::unique_ptr<CodeGenerator> Builder_in)
            : ASTConsumer(), Builder(std::move(Builder_in)) {
    }

    ~MyASTConsumer() {}

    void Initialize(ASTContext &Context) override;

    void HandleCXXStaticMemberVarInstantiation(VarDecl *VD) override;

    bool HandleTopLevelDecl(DeclGroupRef D) override;

    void HandleInlineFunctionDefinition(FunctionDecl *D) override;

    void HandleInterestingDecl(DeclGroupRef D) override;

    void HandleTranslationUnit(ASTContext &Ctx) override;

    void HandleTagDeclDefinition(TagDecl *D) override;

    void HandleTagDeclRequiredDefinition(const TagDecl *D) override;

    void HandleCXXImplicitFunctionInstantiation(FunctionDecl *D) override;

    void HandleTopLevelDeclInObjCContainer(DeclGroupRef D) override;

    void HandleImplicitImportDecl(ImportDecl *D) override;

    void CompleteTentativeDefinition(VarDecl *D) override;

    void AssignInheritanceModel(CXXRecordDecl *RD) override;

    void HandleVTable(CXXRecordDecl *RD) override;

    ASTMutationListener *GetASTMutationListener() override;

    ASTDeserializationListener *GetASTDeserializationListener() override;

    void PrintStats() override;

    bool shouldSkipFunctionBody(Decl *D) override;
};

void MyASTConsumer::Initialize(ASTContext &Context) {
    Builder->Initialize(Context);
}

bool MyASTConsumer::HandleTopLevelDecl(DeclGroupRef DG) {

    for (DeclGroupRef::iterator I = DG.begin(), E = DG.end(); I != E; ++I) {
        toplevel_decls.push_back(*I);
    }
    return Builder->HandleTopLevelDecl(DG);
}

void MyASTConsumer::HandleInlineFunctionDefinition(FunctionDecl *D) {
    Builder->HandleInlineFunctionDefinition(D);
}

void MyASTConsumer::HandleInterestingDecl(DeclGroupRef D) {
    Builder->HandleInterestingDecl(D);
}

void MyASTConsumer::HandleTranslationUnit(ASTContext &Context) {
    // HandleTranslationUnit can close the module
    Builder->HandleTranslationUnit(Context);
}

void MyASTConsumer::HandleTagDeclDefinition(TagDecl *D) {
    Builder->HandleTagDeclDefinition(D);
}

void MyASTConsumer::HandleTagDeclRequiredDefinition(const TagDecl *D) {
    Builder->HandleTagDeclRequiredDefinition(D);
}

void MyASTConsumer::HandleCXXImplicitFunctionInstantiation(FunctionDecl *D) {
    Builder->HandleCXXImplicitFunctionInstantiation(D);
}

void MyASTConsumer::HandleTopLevelDeclInObjCContainer(DeclGroupRef D) {
    Builder->HandleTopLevelDeclInObjCContainer(D);
}

void MyASTConsumer::HandleImplicitImportDecl(ImportDecl *D) {
    Builder->HandleImplicitImportDecl(D);
}

void MyASTConsumer::CompleteTentativeDefinition(VarDecl *D) {
    Builder->CompleteTentativeDefinition(D);
}

void MyASTConsumer::AssignInheritanceModel(CXXRecordDecl *RD) {
    Builder->AssignInheritanceModel(RD);
}

void MyASTConsumer::HandleCXXStaticMemberVarInstantiation(VarDecl *VD) {
    Builder->HandleCXXStaticMemberVarInstantiation(VD);
}

void MyASTConsumer::HandleVTable(CXXRecordDecl *RD) {
    Builder->HandleVTable(RD);
}

ASTMutationListener *MyASTConsumer::GetASTMutationListener() {
    return Builder->GetASTMutationListener();
}

ASTDeserializationListener *MyASTConsumer::GetASTDeserializationListener() {
    return Builder->GetASTDeserializationListener();
}

void MyASTConsumer::PrintStats() {
    Builder->PrintStats();
}

bool MyASTConsumer::shouldSkipFunctionBody(Decl *D) {
    return Builder->shouldSkipFunctionBody(D);
} */

/// Describes the kind of translation unit being processed.
enum TranslationUnitKind {
    /// The translation unit is a complete translation unit.
    TU_Complete,

    /// The translation unit is a prefix to a translation unit, and is
    /// not complete.
    TU_Prefix,

    /// The translation unit is a module.
    TU_Module,

    /// The translation unit is a is a complete translation unit that we might
    /// incrementally extend later.
    TU_Incremental
};

struct Compiler {
private:
    LLVMContext context;
    clang::CompilerInstance compiler;
    std::unique_ptr<clang::CodeGenerator> CG;
    std::unordered_map<std::string, llvm::Module *> spoof_operators;
    SMDiagnostic diagnostic;
    unsigned PtrSize = 0;
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

        const clang::TargetInfo &TInfo = compiler.getTarget();
        PtrSize = TInfo.getPointerWidth(0) / 8;

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

        //compiler.resetAndLeakFileManager();
        // compiler.resetAndLeakPreprocessor();
        //compiler.resetAndLeakSourceManager();

        //if (!Consumer)
        //    Consumer = std::move(CG);

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

        for (const Function &function : module->getFunctionList()) {
            //spoof_operators.emplace(function.getName(), function)
            outs() << function.getName();
            outs() << function;
        }
        return module;
    }
};


class ExecutionEngineSpoofs {
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

    void AddModule(const std::string& mod) {
        executionEngine->addModule(std::move(llvm::getLazyIRModule(MemoryBuffer::getMemBuffer(StringRef(mod)), *error,
                                                                  *context)));
    }

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
    const llvm::Module *module = compiler.generateIR();

    std::string addition;
    raw_string_ostream OS(addition);
    OS << *module;
    OS.flush();
    std::cout << addition << std::endl;

    compiler.init(TestProgram1, "subtraction");
    const llvm::Module *module1 = compiler.generateIR();

    /// Print loaded mod function body for test purpose
    /*for (const Function &function : module->getFunctionList()) {
        outs() << function.getName();
        outs() << function;
    } */

    std::string Str1;
    raw_string_ostream OS1(Str1);
    OS1 << *module1;
    OS1.flush();
    std::cout << Str1 << std::endl;

    ExecutionEngineSpoofs executionEngine(addition, compiler.getContext());
    executionEngine.AddModule(addition);
    executionEngine.AddModule(Str1);

    llvm::GenericValue param1, param2;
    param1.FloatVal = 10;
    param2.FloatVal = 2;
    GenericValue params[] = {param1, param2};
    //executionEngine.RunFunction("sum", params);
    std::cout << param1.FloatVal << " + " << param2.FloatVal << " = " << executionEngine.RunFunction("sum", params).FloatVal << std::endl;

    //executionEngine.RunFunction("subtract", params);
    std::cout << param1.FloatVal << " - " << param2.FloatVal << " = " << executionEngine.RunFunction("subtract", params).FloatVal << std::endl;

    return 0;
}