#include <clang/Driver/Driver.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Driver/Compilation.h>  // Compilation的定义
#include <clang/Driver/Job.h>  // 不加会有Command类型完整性问题
#include <llvm/Support/VirtualFileSystem.h> // 不加会有FileSystem类型完整性问题
#include <llvm/Support/FileSystem.h> // 不加会有FileSystem类型完整问题
#include <llvm/IRReader/IRReader.h> 
#include <llvm/IR/LLVMContext.h> 
#include <llvm/Support/SourceMgr.h> 
#include <llvm/IR/LegacyPassManager.h> 
#include <llvm/CodeGen/MachinePassManager.h>
#include "/home/ljz/XXXVMP/Vmp/Pass.h"
#include "GenStub.h"
#include "createSection.h"
#include <sstream>
#include <iomanip>
#include <filesystem>
using namespace clang;
std::hash<std::string> hashString;
//clang++-18 -c -fno-discard-value-names -Xclang -disable-llvm-passes -fplugin=./libExternCollectorPlugin.so  ../build/test.cpp
//opt-18 -load-pass-plugin=./VmpPass.so -passes=VmpPass test9.ll -o o.l

int main(int argc, const char **argv) {
    const char*env = std::getenv("DEBUG");
    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "]: " << (argv[i] ? argv[i] : "NULL") << std::endl;
    }
    
    // 初始化诊断引擎
    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
    TextDiagnosticPrinter *DiagClient = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
    IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
    DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

    // 创建clang Driver实例
    clang::driver::Driver Driver(
        "/usr/bin/clang++-18",
        "x86_64-unknown-linux-gnu",
        Diags,
        "Xlang LLVM compiler",  // 添加Driver标题
        nullptr // 指定VFS参数
    );
    llvm::SmallVector<char, 1> path;
    auto errs=llvm::sys::fs::current_path(path);
    
    llvm::outs() << "current path: " << path << "\n";

    std::string IRFile = "test.ll";
    for(auto c:llvm::sys::path::get_separator().str()){
        path.push_back(c);
    }
    for(auto c:IRFile){
        path.push_back(c);
    }
    std::string path_str(path.begin(),path.end());
    llvm::outs() << "path: " << path << "\n";
    //先生成IR
    SmallVector<const char *, 16> IR_Args;
    std::string input_file = argv[1];
    if(!std::filesystem::exists(input_file)){
        llvm::outs() << "input file not exist\n";
        return 0;
    }
    llvm::outs() << "input file: " << input_file << "\n";
    IR_Args.push_back("-c");
    IR_Args.push_back("-fplugin-arg-disable-llvm-passes");  // 添加自定义选项
    IR_Args.push_back("-fplugin=../../Cbuild/libExternCollectorPlugin.so");  // 添加插件处理外部函数和全局变量
    IR_Args.push_back("-fplugin=/home/ljz/XXXVMP/AttrProtected/build/libAttrPlugin.so"); // 添加插件处理自定义属性
    IR_Args.push_back("-fplugin=/home/ljz/XXXVMP/clangRewrite/build/libFunctionCallRewriter.so");
    IR_Args.push_back("-emit-llvm");
    // IR_Args.push_back("-O3");                // 最高级别优化,优化完不如不优化.....

    IR_Args.push_back("-S");


    IR_Args.push_back(input_file.c_str());
    IR_Args.push_back("-o");
    IR_Args.push_back(path_str.c_str());
    std::unique_ptr<driver::Compilation> CIR(Driver.BuildCompilation(IR_Args));
    clang::driver::JobList &IRJobs = CIR->getJobs();
    SmallVector<std::pair<int, const driver::Command *>, 5> FailingCommands;
    CIR->ExecuteJobs(IRJobs, FailingCommands);  // 执行编译作业
    auto &ResultIRFiles = CIR->getResultFiles();
    for(auto &File : FailingCommands) {
        llvm::outs() << "Failing command: " << File.first << "\n";
        File.second->Print(llvm::outs(),"\n",true);
    }
    for (auto &File : ResultIRFiles) {
        llvm::outs() << "Result file: " << File.first << "\n";
        llvm::outs() << "Result file path: " << File.second << "\n";
    }
    llvm::SMDiagnostic Err;
    llvm::LLVMContext Context;
    // 生成外部函数和变量的头文件
    GenStub();

    
    // 解析IR文件

    auto M = parseIRFile(path_str, Err, Context);

    if (!M) {
        llvm::errs() << "Error reading bitcode file.\n";
        Err.print(argv[0], llvm::errs());
        return 1;
    }
    // 运行自定义的Pass处理json生成stub和字节码
    llvm::ModulePassManager MPM;
    llvm::ModuleAnalysisManager MAM;
    llvm::VmpPass VmpPass;
    llvm::GenType GenType;

    llvm::errs() << "Before running GenType pass\n";
    GenType.run(*M, MAM);
    llvm::errs() << "After running GenType pass\n";

    llvm::errs() << "Before running VmpPass\n";
    VmpPass.run(*M, MAM);
    llvm::errs() << "After running VmpPass\n";

    

    if(!CreateObjectFiles()){
        std::cout << "Error creating object files.\n";
        abort();
        return 1;
    }
    // return 0;
    // 设置自定义参数
    SmallVector<const char *, 18> Args;
    Args.push_back("/usr/bin/clang++-18");
    Args.push_back("-shared");
    Args.push_back("-fPIC");

    Args.push_back("/home/ljz/XXXVMP/XXX/XXXVMPCore.cpp");
    if(env!=nullptr){
        Args.push_back("-DDBG");
    }
    // 添加目标文件
    Args.push_back("/home/ljz/XXXVMP/XXX/next.o");
    Args.push_back("/home/ljz/XXXVMP/XXX/global.o");
    Args.push_back("-o");
    Args.push_back("/home/ljz/XXXVMP/XXX/VMP.so");
    Args.push_back("-std=c++23");

    // 生成虚拟机的编译作业
    std::unique_ptr<driver::Compilation> C(Driver.BuildCompilation(Args));
    clang::driver::JobList &Jobs = C->getJobs();
    // 执行编译作业
    FailingCommands.clear();
    C->ExecuteJobs(Jobs, FailingCommands);  // 执行编译作业
    for(auto& failingCommand : FailingCommands) {
        llvm::outs() << "Failing command: " << failingCommand.first << "\n";
        failingCommand.second->Print(llvm::outs(),"\n",true);
    }
    //获取结果
    auto &ResultFiles = C->getResultFiles();
    for (auto &File : ResultFiles) {
        llvm::outs() << "Result file: " << File.first << "\n";
        llvm::outs() << "Result file path: " << File.second << "\n";
    }

    Args.clear();
    auto suffix_n = hashString(input_file);
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << suffix_n;
    
    std::string newFileName = input_file.substr(0, input_file.rfind(".")) + 
                             "_" + oss.str() + input_file.substr(input_file.rfind("."));
    if(!std::filesystem::exists(newFileName)){
        newFileName=input_file;
        Args.push_back("/usr/bin/clang++-18");
        Args.push_back("-DNOPROTECT");
        if(env!=nullptr){
            Args.push_back("-DDBG");
        }
        Args.push_back("/home/ljz/XXXVMP/XXX/XXXVMPCore.cpp");
        // 添加目标文件
        Args.push_back("/home/ljz/XXXVMP/XXX/next.o");
        Args.push_back("/home/ljz/XXXVMP/XXX/global.o");
        Args.push_back("-o");
        Args.push_back("/home/ljz/XXXVMP/XXX/VMP");
        Args.push_back("-std=c++23");
    }
    else{
        Args.push_back("/usr/bin/clang++-18");
        Args.push_back("-fplugin=/home/ljz/XXXVMP/AttrProtected/build/libAttrPlugin.so");
        if(env!=nullptr){
            Args.push_back("-DDBG");
        }
        Args.push_back(newFileName.c_str());
        // 添加目标文件
        Args.push_back("/home/ljz/XXXVMP/XXX/VMP.so");
    
        Args.push_back("-o");
        Args.push_back("/home/ljz/XXXVMP/XXX/VMP");
        Args.push_back("-std=c++23");
    }

    
    // 生成虚拟机的编译作业
    std::unique_ptr<driver::Compilation> CV(Driver.BuildCompilation(Args));
    clang::driver::JobList &JBS = CV->getJobs();
    // 执行编译作业
    FailingCommands.clear();
    CV->ExecuteJobs(JBS, FailingCommands);  // 执行编译作业
    //获取结果
    auto &RF = CV->getResultFiles();
    for (auto &File : RF) {
        llvm::outs() << "Result file: " << File.first << "\n";
        llvm::outs() << "Result file path: " << File.second << "\n";
    }
    for(auto& failingCommand : FailingCommands) {
        llvm::outs() << "Failing command: " << failingCommand.first << "\n";
        failingCommand.second->Print(llvm::outs(),"\n",true);
    }
    return 0;
}
