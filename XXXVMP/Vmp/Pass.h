#pragma once
// #include "generate.h"
// #include "VmpContext.h"
// #include "VmpCTX.h"

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "XXXVMPCTX.h"
#include "XXXTypeGen.hpp"

namespace llvm
{

    class VmpPass : public PassInfoMixin<VmpPass>
    {
    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
        static bool isRequired() { return true; }

    private:
        // RegUseInfo reg;
    };

    class GenType : public PassInfoMixin<GenType>
    {
    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
        static bool isRequired() { return true; }
    };

} // namespace llvm


