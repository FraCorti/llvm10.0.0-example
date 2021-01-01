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
#include <fstream>


using namespace llvm;
using namespace llvm::sys;

int main() {

    /// Open new LLVM context.
    std::unique_ptr<LLVMContext> context = std::make_unique<LLVMContext>();

    /// Create new diagnostic error manager
    std::unique_ptr<SMDiagnostic> error = std::make_unique<SMDiagnostic>();

    /// Get target hardware information in formato: CPU_TYPE-VENDOR-OPERATING_SYSTEM or CPU_TYPE-VENDOR-KERNEL-OPERATING_SYSTEM
    std::string TargetTriple = sys::getDefaultTargetTriple();

    /// Construct object and open file from disk
    std::ifstream test("../sum.bc", std::ifstream::in);
    if (!test) {
        std::cerr << "error after open stream " << std::endl;
        return 0;
    }

    /// Load ir/bitcode
    std::string ir((std::istreambuf_iterator<char>(test)), (std::istreambuf_iterator<char>()));

    /// Open new module and parse loaded ir/bitcode
    std::unique_ptr<Module> mod = llvm::getLazyIRModule(MemoryBuffer::getMemBuffer(StringRef(ir)), *error, *context);
    if (!mod) {
        std::string what;
        llvm::raw_string_ostream os(what);
        error->print("error after ParseIR()", os);
        std::cout << what;
    }

    /// Print loaded mod function body for test purpose
    for (Function &function : mod->getFunctionList()) {
        outs() << function.getName();
        outs() << function;
    }

    /// Initialize IR builder
    std::unique_ptr<IRBuilder<>> builder = std::make_unique<IRBuilder<>>(*context);

    // Create a new builder for the module.
    //std::unique_ptr<IRBuilder<>> Builder = std::make_unique<IRBuilder<>>(*TheContext);

    /// Access to all available target machines that LLVM is configured
    InitializeAllTargets();

    /// Access to all available target MC that LLVM is configured to support
    InitializeAllTargetMCs();

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target) {
        errs() << Error;
        return 1;
    }

    auto CPU = "generic";
    auto Features = "";

    /// Handle options for the target machine
    TargetOptions opt;

    auto RM = Optional<Reloc::Model>();
    auto CM = Optional<CodeModel::Model>();
    auto OL = CodeGenOpt::Level::Default; /// -O2 optimization
    std::unique_ptr<llvm::TargetMachine> TargetMachine(Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM, CM, OL));

    /// Setup current module for the target machine
    mod->setDataLayout(TargetMachine->createDataLayout());
    mod->setTargetTriple(TargetTriple);

    /// retrieve function called "name" from passed module in created ExecutionEngine object
    std::unique_ptr<ExecutionEngine> executionEngine(llvm::EngineBuilder(std::move(mod)).setEngineKind(
            llvm::EngineKind::Interpreter).create());

    Function *add = executionEngine->FindFunctionNamed(StringRef(std::string("_Z3sumff")));
    llvm::GenericValue param1, param2;
    param1.FloatVal = 5;
    param2.FloatVal = 2;
    GenericValue params[] = {param1, param2};
    ArrayRef<GenericValue> args = ArrayRef<GenericValue>(params, 2);
    GenericValue result = executionEngine->runFunction(add, args);
    std::cout << param1.FloatVal << " + " << param2.FloatVal << " = " << result.FloatVal << std::endl;

    return 0;
}
