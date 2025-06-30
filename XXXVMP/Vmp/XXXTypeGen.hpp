#pragma once
#include <string>
#include <iostream>
#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>
#include <clang/AST/ParentMapContext.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>

using _TypeSize = int64_t;
using TypeName = std::string;
using _Align = int64_t;
class ExternalCollector;
struct XXXType{
    TypeName Type_name;
    std::vector<XXXType> Type;
    std::optional<_TypeSize> size=std::nullopt;
    std::optional<_Align> align=-1;
    bool operator < (const XXXType& type) const {
        return Type_name < type.Type_name;
    }
    bool operator == (const XXXType& type) const {
        return Type_name == type.Type_name;
    }
    bool operator > (const XXXType& type) const {
        return Type_name > type.Type_name;
    }
};

class TypeCollector
{
private:
    std::set<XXXType> TypeSet;
public:
    void Gen(llvm::Module &M);
    void addType(XXXType type);
    std::set<XXXType> getTypeSet();
    void printTypeSet();
    XXXType getType(TypeName type_name);
};

using description = std::string;
using field_name = std::string;
using var_name = std::string;
using addr = int64_t;
using _Symbols = std::pair<description, std::pair<var_name, XXXType>>;
using _SymbolsTable = std::multimap<addr, _Symbols>;
class GenSymbolTable
{
private:
    std::unordered_map<field_name, _SymbolsTable> SymbolTable_m;
    std::vector<llvm::BasicBlock*> BBs;
public:
    void Gen(llvm::Module &M, TypeCollector& typeCollector);
    void addBB(llvm::BasicBlock* bb);

    void addSymbol(field_name, _SymbolsTable);
    _SymbolsTable getSymbolTable(field_name);
    void mergeSymbolTable(field_name s1,field_name s2);
    void printSymbolTable();
};

#define getSymbol(R,t) getSymbolImpl<R>(t,*this)

enum class ReturnType{addr,XXXType,description};
template<ReturnType R,typename T,typename C>
auto getSymbolImpl(T t,C c){
    std::string var_name;
    std::string trap;
    llvm::raw_string_ostream TOS(trap);
    if constexpr (std::is_same_v<T,llvm::Instruction *>){
        
        llvm::raw_string_ostream OS(var_name);

        t->printAsOperand(OS, false); // 临时寄存器没名字，获取类似%1作为变量名称
        OS.flush(); 
        llvm::outs() << "[XXXTypeGen]: Inst Name: " << var_name << "\n";
        t->print(TOS);
        TOS.flush();
    }else if constexpr (std::is_same_v<T,std::string>){
        var_name = t;
    }else if constexpr (std::is_same_v<T,llvm::Value *>){
        llvm::raw_string_ostream OS(var_name);
        t->printAsOperand(OS, false); // 临时寄存器没名字，获取类似%1作为变量名称
        OS.flush();
        t->print(TOS);
        TOS.flush();
        llvm::outs() << "[XXXTypeGen]: Value Name: " << var_name << "\n"; 
    }else{
        throw std::runtime_error("[XXXTypeGen]: getSymbolName error: "+ trap);
    }

    if constexpr (R==ReturnType::addr){
        for(auto &symbol:c.get_st()){
            if(symbol.second.second.first == var_name){
                return symbol.first;
            }
        }
        throw std::runtime_error("[XXXTypeGen]: getSymboladdr error: "+ trap);
    }else if constexpr (R==ReturnType::XXXType){
        for(auto &symbol:c.get_st()){
            if(symbol.second.second.first == var_name){
                return symbol.second.second.second;
            }
        }
        // for(auto &symbol:c.get_st()){
        //     llvm::outs() << "[XXXTypeGen]: SymbolXXXType: " << symbol.second.second.first << " ";
            
        // }
        llvm::outs() << "\n";
        throw std::runtime_error("[XXXTypeGen]: getSymbolXXXType error: "+ trap);
    }else if constexpr (R==ReturnType::description){
        for(auto &symbol:c.get_st()){
            if(symbol.second.second.first == var_name){
                return symbol.second.first;
            }
        }
        throw std::runtime_error("[XXXTypeGen]: getSymbolDescription error: "+ trap);
    }

}


