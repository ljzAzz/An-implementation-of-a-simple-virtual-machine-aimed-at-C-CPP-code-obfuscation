#pragma once
#include <elfio/elfio.hpp>
#include <fstream>
#include <filesystem>

bool CreateObjectFiles() {
    if (!llvm::sys::fs::exists("/home/ljz/XXXVMP/XXXClang/build/XXXins_new.bin") ||
        !llvm::sys::fs::exists("/home/ljz/XXXVMP/XXXClang/build/__init_global.bin")) {
        llvm::errs() << "Source data files don't exist!\n";
        abort();
        return false;
    }

    // 先将文件复制到临时文件
    std::string CopyNextCmd = "cp /home/ljz/XXXVMP/XXXClang/build/XXXins_new.bin /tmp/next";
    std::string CopyGlobalCmd = "cp /home/ljz/XXXVMP/XXXClang/build/__init_global.bin /tmp/global";

    int CopyResult1 = system(CopyNextCmd.c_str());
    int CopyResult2 = system(CopyGlobalCmd.c_str());
    
    if (CopyResult1 != 0 || CopyResult2 != 0) {
        llvm::errs() << "Failed to copy files to temporary location!\n";
        abort();
        return false;
    }
    if(std::filesystem::is_empty("/tmp/global")){
        auto file = std::ofstream("/home/ljz/XXXVMP/XXXClang/build/__init_global.bin");
        if(file.is_open()){
            file<<"11111111";
            file.close();
        }
        llvm::outs() << "__init_global.bin is empty\n";
    }
    if(std::filesystem::is_empty("/tmp/next")){
        auto file = std::ofstream("/home/ljz/XXXVMP/XXXClang/build/XXXins_new.bin");
        if(file.is_open()){
            file<<"11111111";
            file.close();
        }
        llvm::outs() << "XXXins_new.bin is empty\n";
    }


    std::string NextCmd = "objcopy -I binary -O elf64-x86-64 -B i386:x86-64 "
    "/tmp/next "
    "/home/ljz/XXXVMP/XXX/next.o";

    std::string GlobalCmd = "objcopy -I binary -O elf64-x86-64 -B i386:x86-64 "
          "/tmp/global "
          "/home/ljz/XXXVMP/XXX/global.o";

    
    int Result1 = system(NextCmd.c_str());
    int Result2 = system(GlobalCmd.c_str());
    
    if (Result1 != 0 || Result2 != 0) {
        llvm::errs() << "Failed to create object files!\n";
        return false;
    }
    
    system("rm /tmp/next /tmp/global");
    // system("rm /home/ljz/XXXVMP/XXX/next.o /home/ljz/XXXVMP/XXX/global.o");
    // system("rm /home/ljz/XXXVMP/XXXClang/build/XXXins_new.bin /home/ljz/XXXVMP/XXXClang/build/__init_global.bin");
    
    return true;
}
