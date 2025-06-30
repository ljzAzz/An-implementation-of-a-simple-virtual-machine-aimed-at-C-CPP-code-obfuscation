#pragma once
#include "XXXInst.hpp"
#include <stack>
#include <vector>
#include <iostream>
#include <bitset>
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
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <fstream>
#include <unordered_map>
#include <variant>
#include <bitset>
#include "llvm/ADT/DenseMap.h"
#include <list>
#include <cstdint>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <llvm/Demangle/Demangle.h>
#include <nlohmann/json.hpp>
#include "Linker.hpp"
#include "XXXTypeGen.hpp"


extern class ExternalCollector ExCollector;
extern class MemoryManager Mem;
extern class Stack Stack;
extern std::unique_ptr<class FCTXOrder> FCTXOrder;
extern std::set<std::string> globalUsed;

class InsEmitter
{
    friend class FunCTX;
    friend class MemoryManager;
    friend class Stack;
    friend class Linker;

private:
    std::vector<uint8_t> buffer;

public:
    template <typename T>
    void emit(T V)
    {
        uint8_t *ptr = new uint8_t[sizeof(T)];             
        std::memcpy(ptr, &V, sizeof(T));                   
        buffer.insert(buffer.end(), ptr, ptr + sizeof(T)); 
        delete[] ptr;                                      
    }
    template <>
    void emit(VMINST *v)
    {
        v->to_string();
        // std::cout << v->get_size() << std::endl;
        uint8_t *ptr = new uint8_t[v->get_size()];
        std::memcpy(ptr, v->get_data(), v->get_size());
        buffer.insert(buffer.end(), ptr, ptr + v->get_size());
        std::cout << "[Transfor]: emit done" << std::endl;
        delete[] ptr;
    }
    uint32_t size() const
    {
        return buffer.size();
    }
    std::vector<uint8_t> get_buffer()
    {
        return buffer;
    }
    void dump()
    {
        std::ofstream out("ins_new.bin", std::ios::binary);
        out.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
        out.close();
        std::cout << "[Transfor]: ins_new.bin" << std::endl;
        // 二进制输出，4字节一个指令
        for (size_t i = 0; i < buffer.size(); i += 4)
        {
            std::bitset<32> bs;
            for (size_t j = 0; j < 4; ++j)
            {
                if (i + j < buffer.size())
                {
                    bs <<= 8;
                    bs |= buffer[i + j];
                }
            }
            std::cout << bs << std::endl; // 输出二进制表示的指令
        }
    }
};

class MemoryManager
{
private:
    std::vector<uint8_t> memory; // 保持原有内存存储
    std::vector<std::pair<uint32_t, uint32_t>> free_blocks;
    uint64_t free_start = 0;
public:
    MemoryManager()
    {
        memory.resize(1024 * 1024); // 初始化内存
        free_blocks.emplace_back(0, memory.size() - 1);
    }
    MemoryManager(MemoryManager &MM)
    {
        *this = MM;
    }
    void write(uint32_t addr,const uint8_t *data, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            memory[addr + i] = data[i];
        }
    }
    uint8_t *read(uint32_t addr, size_t size)
    {
        uint8_t *data = new uint8_t[size];
        for (size_t i = 0; i < size; i++)
        {
            data[i] = memory[addr + i];
        }
        return data;
    }
    template <typename T>
    T read_as(uint64_t addr)
    {
        T data;
        std::memcpy(&data, &memory[addr], sizeof(T));
        return data;
    }
    size_t size() const
    {
        return memory.size();
    }
    void resize(size_t size)
    {
        memory.resize(size);
    }
    uint64_t allocate(uint32_t size, uint32_t alignment)
    {
        auto aligned_start = free_start;
        free_start += size+1;
        return aligned_start;
        // for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it)
        // {
        //     auto &[block_start, block_end] = *it;

        //     // 计算对齐后的起始地址
        //     uint32_t aligned_start = (block_start + alignment - 1) & ~(alignment - 1);

        //     uint32_t aligned_end = aligned_start + size;

        //     if (aligned_end > block_end)
        //         continue;


        //     free_blocks.erase(it);
        //     // 处理前部剩余空间
        //     if (aligned_start > block_start)
        //     {
        //         free_blocks.emplace_back(block_start, aligned_start);
        //     }

        //     // 处理后部剩余空间
        //     if (aligned_end < block_end)
        //     {
        //         free_blocks.emplace_back(aligned_end, block_end);
        //     }

        //     // 合并相邻块（提升分配效率）
        //     merge_adjacent_blocks();

        //     return aligned_start;
        // }

        throw std::bad_alloc();
    }

    void dump()
    {
        // std::ofstream out("memory.bin", std::ios::binary);
        // out.write(reinterpret_cast<const char *>(memory.data()), memory.size());
        // out.close();
        // 格式化输出,打印不是空闲块的内容，16进制输出，包括地址和数据
        for (size_t i = 0; i < 64; i += 16)
        {
            std::cout << std::hex << std::setfill('0') << std::setw(8) << i << ": ";
            for (size_t j = 0; j < 16; ++j)
            {
                if (i + j < 64)
                {
                    std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(memory[i + j]) << " ";
                }
                else
                {
                    std::cout << "[Transfor]:    ";
                }
            }
            std::cout << std::dec << std::endl;
        }
    }

private:
    void merge_adjacent_blocks()
    {
        if (free_blocks.empty())
            return;

        // 按起始地址排序
        std::sort(free_blocks.begin(), free_blocks.end(),
                  [](const auto &a, const auto &b)
                  { return a.first < b.first; });

        // 合并相邻块
        for (size_t i = 1; i < free_blocks.size();)
        {
            if (free_blocks[i - 1].second == free_blocks[i].first)
            {
                free_blocks[i - 1].second = free_blocks[i].second;
                free_blocks.erase(free_blocks.begin() + i);
            }
            else
            {
                ++i;
            }
        }
    }
};

class Stack
{
private:
    std::array<uint8_t, 4096> stack;
    uintptr_t rsp;
    uintptr_t rbp;

public:
    Stack()
    {
        // 初始化栈
        for (size_t i = 0; i < stack.size(); ++i)
        {
            stack[i] = 0;
        }
        rsp = stack.size();
        rbp = rsp;
    }
    void push(uint8_t *data, size_t size)
    {
        if (rsp < size)
        {
            throw std::out_of_range("[Transfor]: Stack overflow");
        }
        rsp -= size;
        for (size_t i = 0; i < size; i++)
        {
            stack[rsp + i] = data[i];
        }
    }
    template <typename T>
    T pop()
    {
        if (rsp + sizeof(T) > stack.size())
        {
            throw std::underflow_error("[Transfor]: Stack underflow");
        }
        uint8_t *data = new uint8_t[sizeof(T)];
        //小端序
        std::memcpy(data, &stack[rsp], sizeof(T));
        T val=0;
        std::memcpy(&val, data, sizeof(T));
        rsp += sizeof(T);
        return val;
    }
    void write(int64_t offset, const uint8_t *data, size_t size)
    {
        if (rbp - offset > stack.size())
        {
            throw std::out_of_range("[Transfor]: Stack write out of bounds");
        }

        std::memcpy(&stack[rbp - offset], data, size);
    }
    uint8_t *read(int64_t offset, size_t size)
    {
        // std::cout<<"rbp is "<<rbp<<" offset is "<<offset<<" size is "<<size<<std::endl;
        if (rbp - offset > stack.size())
        {
            throw std::out_of_range("[Transfor]: Stack read out of bounds");
        }
        uint8_t *data = new uint8_t[size];
        std::memcpy(data, &stack[rbp - offset], size);
        return data;
    }
    template <typename T>
    T read_as(uint32_t offset)
    {
        T data;
        std::memcpy(&data, &stack[rbp - offset], sizeof(T));
        return data;
    }
    void set_rbp(uintptr_t value)
    {
        rbp = value;
    }
    void set_rsp(uintptr_t value)
    {
        rsp = value;
    }
    uintptr_t get_rbp()
    {
        return rbp;
    }
    uintptr_t get_rsp()
    {
        return rsp;
    }
    void dump()
    {
        std::cout << "[Transfor]: Stack dump:" << std::endl;
        // 从栈顶开始十六进制输出地址以及数据
        for (size_t i = 4000; i < stack.size(); i += 16)
        {
            std::cout << std::hex << std::setfill('0') << std::setw(8) << i << ": ";
            for (size_t j = 0; j < 16; ++j)
            {
                if (i + j < stack.size())
                {
                    std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(stack[i + j]) << " ";
                }
                else
                {
                    std::cout << "   ";
                }
            }
            std::cout << std::dec << std::endl;
        }
    }
};

class TypeDescriptor
{
public:
    virtual size_t size() const = 0;
    virtual uint32_t alignment() const = 0;
    virtual size_t member_offset(const std::string &member_name) const = 0;
};

// 基本类型描述符（如int）
class PrimitiveType : public TypeDescriptor
{
    size_t _size;
    uint32_t _align;

public:
    PrimitiveType(size_t s, uint32_t a) : _size(s), _align(a) {}
    size_t size() const override { return _size; }
    uint32_t alignment() const override { return _align; }
    size_t member_offset(const std::string &) const override
    {
        throw std::logic_error("[Transfor]: Primitive type has no members");
    }
};

// 结构体类型描述符
class StructType_ : public TypeDescriptor
{
    std::unordered_map<std::string, std::pair<size_t, std::shared_ptr<TypeDescriptor>>> members;
    size_t total_size;
    uint32_t struct_align;

public:
    void add_member(const std::string &name, std::shared_ptr<TypeDescriptor> type)
    {
        // 计算成员偏移（自动对齐）
        size_t offset = total_size;
        offset = (offset + type->alignment() - 1) / type->alignment() * type->alignment();
        members[name] = {offset, type};

        // 更新结构体总大小和对齐
        total_size = offset + type->size();
        struct_align = std::max(struct_align, type->alignment());
    }

    size_t size() const override
    {
        // 结构体总大小需对齐到最大成员对齐值
        return (total_size + struct_align - 1) / struct_align * struct_align;
    }
    uint32_t alignment() const override { return struct_align; }
    size_t member_offset(const std::string &name) const override
    {
        return members.at(name).first;
    }
};

class FunctionType_ : public TypeDescriptor
{
    std::vector<std::shared_ptr<TypeDescriptor>> params;
    std::shared_ptr<TypeDescriptor> ret_type;
    uint32_t align;
    bool CXXMember = false;

public:
    FunctionType_(std::vector<std::shared_ptr<TypeDescriptor>> params, std::shared_ptr<TypeDescriptor> ret_type, uint32_t align)
        : params(params), ret_type(ret_type), align(align) {}
    size_t size() const override { return 0; }
    uint32_t alignment() const override { return align; }
    size_t member_offset(const std::string &) const override
    {
        throw std::logic_error("[Transfor]: Function type has no members");
    }
};

class Variable
{
    friend class FunCTX;

private:
    std::shared_ptr<TypeDescriptor> type_;                                   // 类型描述符
    std::shared_ptr<uint8_t[]> storage_;                                     // 数据存储（根据类型分配）
    std::vector<std::pair<std::string, std::shared_ptr<Variable>>> members_; // 结构体成员

public:
    // 基本类型构造函数
    Variable(std::shared_ptr<TypeDescriptor> type, uint8_t *init_data = nullptr)
        : type_(type)
    {
        storage_.reset(new uint8_t[type->size()], std::default_delete<uint8_t[]>());
        if (init_data)
            memcpy(storage_.get(), static_cast<void *>(init_data), type->size());
    }

    // 结构体类型构造函数
    Variable(std::shared_ptr<class StructType_> struct_type)
        : type_(struct_type)
    {
        auto *st = static_cast<StructType_ *>(type_.get());
        storage_.reset(new uint8_t[st->size()], std::default_delete<uint8_t[]>());
    }
    const uint8_t *raw_data() const
    {
        return static_cast<const uint8_t *>(storage_.get());
    }
    auto type() const { return type_; }
    // 添加结构体成员
    void add_member(const std::string &name, std::shared_ptr<Variable> member_var)
    {
        auto *st = dynamic_cast<StructType_ *>(type_.get());
        if (!st)
            throw std::bad_cast();

        // 将成员变量数据拷贝到结构体存储区
        size_t offset = st->member_offset(name);
        memcpy(static_cast<uint8_t *>(storage_.get()) + offset,
               member_var->raw_data(),
               member_var->type()->size());

        members_.emplace_back(name, member_var);
    }

    // 访问成员变量
    std::shared_ptr<Variable> get_member(const std::string &name)
    {
        for (auto &[m_name, var] : members_)
        {
            if (m_name == name)
                return var;
        }
        return nullptr;
    }
};

class SymbolTable
{
    using StructType_ = class StructType_;
    friend class FunCTX;

private:
    struct VariableRecord
    {
        bool is_InMem = false;
        bool is_InReg = false;
        bool is_InStack = true;
        uint64_t base_addr = -1; // 变量基地址
        uint32_t reg_num = -1;
        int64_t stack_offset = -1;
        std::shared_ptr<Variable> var; // 变量对象
    };
    std::unordered_map<std::string, VariableRecord> symbols; // 按变量名存储

public:
    SymbolTable &operator=(const SymbolTable &)
    {
        return *this;
    }
    SymbolTable(){}
    SymbolTable(SymbolTable &ST)
    {
        // 每个函数都能有全局变量符号表的拷贝
        *this=ST;
    }
    // 注册变量（自动分配内存）
    void register_variable_to_mem(const std::string &name, std::shared_ptr<Variable> var)
    {
        uint64_t addr = Mem.allocate(var->type()->size(), var->type()->alignment());
        symbols[name] = {
            .is_InMem = true,
            .is_InReg = false,
            .is_InStack = false,
            .base_addr = addr,
            .var = var,
        };

        // 将初始化数据写入内存
        Mem.write(addr, var->raw_data(), var->type()->size());
    }
    void register_variable_to_stack(const std::string &name, std::shared_ptr<Variable> var)
    {
        int64_t offset = Stack.get_rbp() - Stack.get_rsp() + var->type()->size();
        Stack.set_rsp(Stack.get_rsp() - var->type()->size());
        symbols[name] = {
            .is_InMem = false,
            .is_InReg = false,
            .is_InStack = true,
            .stack_offset = offset,
            .var = var,
        };
    }

    void register_param_from_stack(const std::string &name, std::shared_ptr<Variable> param, int64_t offset)
    {
        symbols[name] = {
            .is_InMem = false,
            .is_InReg = false,
            .is_InStack = true,
            .stack_offset = offset,
            .var = param,
        };
    }

    void register_variable_to_reg(const std::string &name, std::shared_ptr<Variable> var, uint32_t reg_num)
    {
        symbols[name] = {
            .is_InMem = false,
            .is_InReg = true,
            .is_InStack = false,
            .reg_num = reg_num,
            .var = var,
        };
    }

    std::shared_ptr<VariableRecord> get_variable_record(const std::string &name)
    {
        if (symbols.find(name) != symbols.end())
        {
            return std::make_shared<VariableRecord>(symbols.at(name));
        }
        else
        {
            return nullptr;
        }
    }
    // 获取成员变量地址（复合类型支持）
    uint64_t get_member_address(const std::string &parent_var, const std::string &member)
    {
        auto &record = symbols.at(parent_var);
        auto member_var = record.var->get_member(member);
        if (!member_var)
            throw std::out_of_range("[Transfor]: Member not found");

        // 计算成员地址 = 基地址 + 成员偏移
        auto *struct_type = dynamic_cast<StructType_ *>(record.var->type().get());
        return record.base_addr + struct_type->member_offset(member);
    }

    void change_variable(const std::string &name, const uint8_t *data)
    {
        auto &record = symbols.at(name);
        if (record.is_InMem)
        {
            Mem.write(record.base_addr, data, record.var->type()->size());
        }
        else if (record.is_InStack)
        {
            Stack.write(record.stack_offset, data, record.var->type()->size());
        }
        else if (record.is_InReg)
        {
            // TODO
        }
    }
    uint8_t *get_variable(const std::string &name)
    {
        auto &record = symbols.at(name);
        if (record.is_InMem)
        {
            return Mem.read(record.base_addr, record.var->type()->size());
        }
        else if (record.is_InStack)
        {
            return Stack.read(record.stack_offset, record.var->type()->size());
        }
        else if (record.is_InReg)
        {
            uint8_t *data = new uint8_t;
            *data = static_cast<uint8_t>(record.reg_num);
            return data;
        }
        return nullptr;
    }

    void dump()
    {
        for (auto &symbol : symbols)
        {
            if (symbol.second.is_InMem)
            {
                std::cout << symbol.first << " : " << " is in memory, address is " << symbol.second.base_addr << std::endl;
            }
            else if (symbol.second.is_InStack)
            {
                std::cout << symbol.first << " : " << " is in stack, offset is " << (symbol.second.stack_offset) << std::endl;
            }
            else if (symbol.second.is_InReg)
            {
                std::cout << symbol.first << " : " << " is in register, register number is " << symbol.second.reg_num << std::endl;
            }
        }
    }
};

class FunCTX
{
    friend class Linker;

private:
    _SymbolsTable st;
    std::unordered_map<std::string, uint64_t> BBaddr;
    std::unordered_map<uint64_t, std::string> toInsBB;
    int64_t return_addr=0;
    std::set<class CXXFunCallInfo *> CXXFunCalls;
    std::set<std::string> CXXMemberFunctions;
    std::stack<llvm::Argument *> args;
    struct jump_table
    {
        std::string fname;
        uint64_t call_addr;
        int64_t called_offset;
    };
    std::vector<jump_table> jump_tables;
    SymbolTable symbol_table;
    class InsEmitter emitter;
    std::string Fname;
    size_t stack_size;
    std::vector<llvm::BasicBlock *> BBS;
    llvm::DataLayout &DL;
    std::optional<int> getAlign(llvm::Instruction *instruction)
    {
        std::optional<int> align = std::nullopt;
        if (llvm::LoadInst *LI = llvm::dyn_cast<llvm::LoadInst>(instruction))
        {
            llvm::outs() << "[Transfor]: Load alignment: " << LI->getAlign().value() << "\n";
            align = LI->getAlign().value();
        }
        else if (llvm::StoreInst *SI = llvm::dyn_cast<llvm::StoreInst>(instruction))
        {
            llvm::outs() << "[Transfor]: Store alignment: " << SI->getAlign().value() << "\n";
            align = SI->getAlign().value();
        }
        else if (llvm::AllocaInst *AI = llvm::dyn_cast<llvm::AllocaInst>(instruction))
        {
            llvm::outs() << "[Transfor]: Alloca alignment: " << AI->getAlign().value() << "\n";
            align = AI->getAlign().value();
        }
        return align;
    }
    // enum class ReturnType{addr,XXXType,description};
    // template<ReturnType R,typename T>
    // auto getSymbol(T t){
    //     std::string var_name;

    //     if constexpr (std::is_same_v<T,llvm::Instruction *>){
            
    //         llvm::raw_string_ostream OS(var_name);
    //         t->printAsOperand(OS, false); // 临时寄存器没名字，获取类似%1作为变量名称
    //         OS.flush();
    //     }else if constexpr (std::is_same_v<T,std::string>){
    //         var_name = t;
    //     }else if constexpr (std::is_same_v<T,llvm::Value *>){
    //         llvm::raw_string_ostream OS(var_name);
    //         t->printAsOperand(OS, false); // 临时寄存器没名字，获取类似%1作为变量名称
    //         OS.flush();
    //     }else{
    //         throw std::runtime_error("[Transfor]: getSymbolName error");
    //     }

    //     if constexpr (R==ReturnType::addr){
    //         for(auto &symbol:st){
    //             if(symbol.second.second.first == var_name){
    //                 return symbol.first;
    //             }
    //         }
    //         throw std::runtime_error("[Transfor]: getSymboladdr error");
    //     }else if constexpr (R==ReturnType::XXXType){
    //         for(auto &symbol:st){
    //             if(symbol.second.second.first == var_name){
    //                 return symbol.second.second.second;
    //             }
    //         }
    //         throw std::runtime_error("[Transfor]: getSymbolXXXType error");
    //     }else if constexpr (R==ReturnType::description){
    //         for(auto &symbol:st){
    //             if(symbol.second.second.first == var_name){
    //                 return symbol.second.first;
    //             }
    //         }
    //         throw std::runtime_error("[Transfor]: getSymbolDescription error");
    //     }

    // }

    bool isUserFunction(std::string fname);
    template <typename T, typename U>
    void handleCmpInst(llvm::Instruction *I)
    {
        auto op1 = llvm::dyn_cast<llvm::Value>(I->getOperand(0));
        auto op2 = llvm::dyn_cast<llvm::Value>(I->getOperand(1));
        if(llvm::isa<llvm::Constant>(op1)&&!llvm::isa<llvm::Constant>(op2)){
            auto actual_op1_value = llvm::dyn_cast<llvm::Constant>(op1);
            auto op1_size = DL.getTypeAllocSize(op1->getType());
            uint8_t rs=0;
            uint64_t op1_v=0;
            switch(op1_size)
            {
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }
            if(auto int_op1_value = llvm::dyn_cast<llvm::ConstantInt>(actual_op1_value))
            {
                auto op1_value = int_op1_value->getZExtValue();
                llvm::outs()<<"[Transfor]: op1 is "<<op1_value<<"\n";
                op1_v = op1_value;
                
            }else if(auto float_op1_value = llvm::dyn_cast<llvm::ConstantFP>(actual_op1_value))
            {
                llvm::APFloat fop = float_op1_value->getValueAPF();
                if(32 == fop.getSizeInBits(fop.getSemantics()))
                {
                    auto op1_value = float_op1_value->getValueAPF().convertToFloat();
                    llvm::outs()<<"[Transfor]: op1 is "<<op1_value<<"\n";
                    std::memcpy(&op1_v, &op1_value, sizeof(float));
                    llvm::outs()<<"[Transfor]: op1 is "<<op1_v<<"\n";
                }else if(64 == fop.getSizeInBits(fop.getSemantics()))
                {
                    auto op1_value = float_op1_value->getValueAPF().convertToDouble();
                    llvm::outs()<<"[Transfor]: op1 is "<<op1_value<<"\n";
                    std::memcpy(&op1_v, &op1_value, sizeof(double));
                    llvm::outs()<<"[Transfor]: op1 is "<<op1_v<<"\n";
                }
            }
            VMINST* MIRI = new _MOV_IMM_RX_Inst(1,rs,op1_v);
            emitter.emit(MIRI);
            auto op2_size = getSymbol(ReturnType::XXXType,op2).size.value();
            auto op2_description = getSymbol(ReturnType::description,op2);
            rs = 0;
            switch(op2_size){
                case 1:
                    rs = 0;
                case 2:
                    rs = 1;
                case 4:
                    rs = 2;
                case 8:
                    rs = 3;
                default:
                throw std::runtime_error("[Transfor]: Unknown CMP optype size");
            }
            if(op2_description=="local variable")
            {
                int64_t op_addr = getSymbol(ReturnType::addr,op2);
                VMINST *MSRI = new _MOV_STACK_RX_Inst(op_addr, 2, rs);
                emitter.emit(MSRI);
            }else if(op2_description=="variable")
            {
                int64_t op_addr = getSymbol(ReturnType::addr,op2);
                VMINST *MMRI = new _MOV_MEM_RX_Inst(op_addr, 2, rs);
                emitter.emit(MMRI);
            }
            rs = 0;
            auto res_size = getSymbol(ReturnType::XXXType,I).size.value();
            switch(res_size){
                case 1:
                    rs = 0;
                case 2:
                    rs = 1;
                case 4:
                    rs = 2;
                case 8:
                    rs = 3;
                default:
                throw std::runtime_error("[Transfor]: Unknown CMP optype size");
            }
            VMINST* cmp = new U(rs,1,2);
            emitter.emit(cmp);
        }
        else if(llvm::isa<llvm::Constant>(op2)&&!llvm::isa<llvm::Constant>(op1)){
            auto actual_op2_value = llvm::dyn_cast<llvm::Constant>(op2);
            auto op2_size = DL.getTypeAllocSize(op2->getType());
            uint8_t rs=0;
            uint64_t op2_v=0;
            switch(op2_size)
            {
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }
            if(auto int_op2_value = llvm::dyn_cast<llvm::ConstantInt>(actual_op2_value))
            {
                auto op2_value = int_op2_value->getZExtValue();
                llvm::outs()<<"[Transfor]: op1 is "<<op2_value<<"\n";
                op2_v = op2_value;
                
            }else if(auto float_op2_value = llvm::dyn_cast<llvm::ConstantFP>(actual_op2_value))
            {
                llvm::APFloat fop = float_op2_value->getValueAPF();
                if(32 == fop.getSizeInBits(fop.getSemantics()))
                {
                    auto op2_value = float_op2_value->getValueAPF().convertToFloat();
                    llvm::outs()<<"[Transfor]: op1 is "<<op2_value<<"\n";
                    std::memcpy(&op2_v, &op2_value, sizeof(float));
                    llvm::outs()<<"[Transfor]: op1 is "<<op2_v<<"\n";
                }else if(64 == fop.getSizeInBits(fop.getSemantics()))
                {
                    auto op2_value = float_op2_value->getValueAPF().convertToDouble();
                    llvm::outs()<<"[Transfor]: op1 is "<<op2_value<<"\n";
                    std::memcpy(&op2_v, &op2_value, sizeof(double));
                    llvm::outs()<<"[Transfor]: op1 is "<<op2_v<<"\n";
                }
            }
            auto op1_size = getSymbol(ReturnType::XXXType,op1).size.value();
            auto op1_description = getSymbol(ReturnType::description,op1);
            rs = 0;
            switch(op1_size){
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown CMP optype size");
            }
            if(op1_description=="local variable")
            {
                int64_t op_addr = getSymbol(ReturnType::addr,op1);
                VMINST *MSRI = new _MOV_STACK_RX_Inst(op_addr, 2, rs);
                emitter.emit(MSRI);
            }else if(op1_description=="variable")
            {
                int64_t op_addr = getSymbol(ReturnType::addr,op1);
                VMINST *MMRI = new _MOV_MEM_RX_Inst(op_addr, 2, rs);
                emitter.emit(MMRI);
            }
            rs = 0;
            auto res_size = getSymbol(ReturnType::XXXType,I).size.value();
            switch(res_size){
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown CMP optype size");
            }
            VMINST* cmp = new T(2,rs,op2_v);
            emitter.emit(cmp);
        }

    }
                    
    template <typename T,typename U>
    void handleBasicOP(llvm::Instruction* I){
        auto op1 = llvm::dyn_cast<llvm::Value>(I->getOperand(0));
        auto op2 = llvm::dyn_cast<llvm::Value>(I->getOperand(1));
        auto res_size = getSymbol(ReturnType::XXXType,I).size.value();
        auto res_description = getSymbol(ReturnType::description,I);
        if(llvm::isa<llvm::Constant>(op1)&&llvm::isa<llvm::Constant>(op2)){
            auto actual_op1_value = llvm::dyn_cast<llvm::Constant>(op1);
            auto actual_op2_value = llvm::dyn_cast<llvm::Constant>(op2);
            auto op1_size = DL.getTypeAllocSize(op1->getType());
            auto op2_size = DL.getTypeAllocSize(op2->getType());
            uint8_t rs=0;
            switch(op1_size)
            {
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }
            uint64_t op1_v = 0;
            uint64_t op2_v = 0;
            if(auto int_op1_value = llvm::dyn_cast<llvm::ConstantInt>(actual_op1_value))
            {
                auto op1_value = int_op1_value->getZExtValue();
                llvm::outs()<<"[Transfor]: op1 is "<<op1_value<<"\n";
                op1_v = op1_value;
                
            }else if(auto float_op1_value = llvm::dyn_cast<llvm::ConstantFP>(actual_op1_value))
            {
                llvm::APFloat fop = float_op1_value->getValueAPF();
                if(32 == fop.getSizeInBits(fop.getSemantics()))
                {
                    auto op1_value = float_op1_value->getValueAPF().convertToFloat();
                    llvm::outs()<<"[Transfor]: op1 is "<<op1_value<<"\n";
                    std::memcpy(&op1_v, &op1_value, sizeof(float));
                    llvm::outs()<<"[Transfor]: op1 is "<<op1_v<<"\n";
                }else if(64 == fop.getSizeInBits(fop.getSemantics()))
                {
                    auto op1_value = float_op1_value->getValueAPF().convertToDouble();
                    llvm::outs()<<"[Transfor]: op1 is "<<op1_value<<"\n";
                    std::memcpy(&op1_v, &op1_value, sizeof(double));
                    llvm::outs()<<"[Transfor]: op1 is "<<op1_v<<"\n";
                }
            }
            if(auto int_op2_value = llvm::dyn_cast<llvm::ConstantInt>(actual_op2_value))
            {
                auto op2_value = int_op2_value->getZExtValue();
                llvm::outs()<<"[Transfor]: op2 is "<<op2_value<<"\n";
                op2_v = op2_value;
            }else if(auto float_op2_value = llvm::dyn_cast<llvm::ConstantFP>(actual_op2_value))
            {
                llvm::APFloat fop = float_op2_value->getValueAPF();
                if(32 == fop.getSizeInBits(fop.getSemantics()))
                {
                    auto op2_value = float_op2_value->getValueAPF().convertToFloat();
                    llvm::outs()<<"[Transfor]: op2 is "<<op2_value<<"\n";
                    std::memcpy(&op2_v, &op2_value, sizeof(float));
                    llvm::outs()<<"[Transfor]: op2 is "<<op2_v<<"\n";
                }else if(64 == fop.getSizeInBits(fop.getSemantics()))
                {
                    auto op2_value = float_op2_value->getValueAPF().convertToDouble();
                    llvm::outs()<<"[Transfor]: op2 is "<<op2_value<<"\n";
                    std::memcpy(&op2_v, &op2_value, sizeof(double));
                    llvm::outs()<<"[Transfor]: op2 is "<<op2_v<<"\n";
                }

            }
            VMINST *MIRI = new _MOV_IMM_RX_Inst(1, rs, op1_v);
            emitter.emit(MIRI);
            rs=0;
            switch(op2_size)
            {
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default: 
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }
            VMINST *MIRI2 = new _MOV_IMM_RX_Inst(2, rs, op2_v);
            emitter.emit(MIRI2);
            rs = 0;
            switch(res_size)
            {
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }

            VMINST *inst = new U(rs, 1, 2);
            emitter.emit(inst);

        }else if(llvm::isa<llvm::Constant>(op1)||llvm::isa<llvm::Constant>(op2)){
            uint64_t op_imm=0;
            int reg_size=0;
            std::string reg_description;
            llvm::Value *op=nullptr;
            if(llvm::isa<llvm::Constant>(op1)){
                op = llvm::dyn_cast<llvm::Constant>(op1);
                reg_size = getSymbol(ReturnType::XXXType,op2).size.value();
                reg_description = getSymbol(ReturnType::description,op2);
                if(auto int_op_imm = llvm::dyn_cast<llvm::ConstantInt>(op))
                {
                    int64_t op_imm_value = int_op_imm->getSExtValue();
                    llvm::outs()<<"[Transfor]: op_imm is "<<op_imm_value<<"\n";
                    op_imm = op_imm_value;
                }else if(auto float_op_imm = llvm::dyn_cast<llvm::ConstantFP>(op))
                {
                    llvm::APFloat fop = float_op_imm->getValueAPF();
                    if(32 == fop.getSizeInBits(fop.getSemantics()))
                    {
                        auto op_imm_value = float_op_imm->getValueAPF().convertToFloat();
                        llvm::outs()<<"[Transfor]: op_imm is "<<op_imm_value<<"\n";
                        std::memcpy(&op_imm, &op_imm_value, sizeof(float));
                        llvm::outs()<<"[Transfor]: op_imm is "<<op_imm<<"\n";
                    }else if(64 == fop.getSizeInBits(fop.getSemantics()))
                    {
                        auto op_imm_value = float_op_imm->getValueAPF().convertToDouble();
                        llvm::outs()<<"[Transfor]: op_imm is "<<op_imm_value<<"\n";
                        std::memcpy(&op_imm, &op_imm_value, sizeof(double));
                        llvm::outs()<<"[Transfor]: op_imm is "<<op_imm<<"\n";
                    }

                }
                op = op2;
            }else{
                op = llvm::dyn_cast<llvm::Constant>(op2);
                reg_size = getSymbol(ReturnType::XXXType,op1).size.value();
                reg_description = getSymbol(ReturnType::description,op1);
                if(auto int_op_imm = llvm::dyn_cast<llvm::ConstantInt>(op))
                {
                    int64_t op_imm_value = int_op_imm->getSExtValue();
                    llvm::outs()<<"[Transfor]: op_imm is "<<op_imm_value<<"\n";
                    op_imm = op_imm_value;
                }else if(auto float_op_imm = llvm::dyn_cast<llvm::ConstantFP>(op))
                {
                    llvm::APFloat fop = float_op_imm->getValueAPF();
                    if(32 == fop.getSizeInBits(fop.getSemantics()))
                    {
                        auto op_imm_value = float_op_imm->getValueAPF().convertToFloat();
                        llvm::outs()<<"[Transfor]: op_imm is "<<op_imm_value<<"\n";
                        std::memcpy(&op_imm, &op_imm_value, sizeof(float));
                        llvm::outs()<<"[Transfor]: op_imm is "<<op_imm<<"\n";
                    }else if(64 == fop.getSizeInBits(fop.getSemantics()))
                    {
                        auto op_imm_value = float_op_imm->getValueAPF().convertToDouble();
                        llvm::outs()<<"[Transfor]: op_imm is "<<op_imm_value<<"\n";
                        std::memcpy(&op_imm, &op_imm_value, sizeof(double));
                        llvm::outs()<<"[Transfor]: op_imm is "<<op_imm<<"\n";
                    }

                }
                
                op = op1;
            }
            
            uint8_t rs = 0;
            switch(reg_size)
            {
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }
            if(reg_description=="local variable")
            {
                int64_t op_addr = getSymbol(ReturnType::addr,op);
                VMINST *MSRI = new _MOV_STACK_RX_Inst(op_addr, 2, rs);
                emitter.emit(MSRI);
            }else if(reg_description=="variable")
            {
                int64_t op_addr = getSymbol(ReturnType::addr,op);
                VMINST *MSRI = new _MOV_MEM_RX_Inst(op_addr, 2, rs);
                emitter.emit(MSRI);
            }
            rs = 0;
            switch(res_size){
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }

            VMINST *inst = new T(2,rs,op_imm);
            emitter.emit(inst);
  
        }else {
            auto op1_description = getSymbol(ReturnType::description,op1);
            auto op2_description = getSymbol(ReturnType::description,op2);
            auto op1_addr = getSymbol(ReturnType::addr,op1);
            auto op2_addr = getSymbol(ReturnType::addr,op2);
            auto op1_size = getSymbol(ReturnType::XXXType,op1).size.value();
            auto op2_size = getSymbol(ReturnType::XXXType,op2).size.value();
            uint8_t rs = 0;
            switch(op1_size)
            {
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }
            if(op1_description=="local variable")
            {
                VMINST *MSRI = new _MOV_STACK_RX_Inst(op1_addr, 1, rs);
                emitter.emit(MSRI);
            }else{
                VMINST *MSRI = new _MOV_MEM_RX_Inst(op1_addr, 1, rs);
                emitter.emit(MSRI);
            }
            rs = 0;
            switch(op2_size)
            {
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }
            if(op2_description=="local variable")
            {
                VMINST *MSRI = new _MOV_STACK_RX_Inst(op2_addr, 2, rs);
                emitter.emit(MSRI);
            }else{
                VMINST *MSRI = new _MOV_MEM_RX_Inst(op2_addr, 2, rs);
                emitter.emit(MSRI);
            }
            rs = 0;
            switch(res_size)
            {
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }

            VMINST *inst = new U(rs, 1, 2);
            emitter.emit(inst);

        }
        uint8_t rs = 0;
        switch(res_size)
        {
            case 1:
                rs = 0;
                break;
            case 2:
                rs = 1;
                break;
            case 4:
                rs = 2;
                break;
            case 8:
                rs = 3;
                break;
            default:
                throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
        }
        if(res_description=="local variable")
        {
            auto res_addr = getSymbol(ReturnType::addr,I);
            VMINST *MSRI = new _MOV_RX_STACK_Inst(2,res_addr,rs);
            emitter.emit(MSRI);
        }else if(res_description=="variable")
        {
            auto res_addr = getSymbol(ReturnType::addr,I);
            VMINST *MSRI = new _MOV_RX_MEM_Inst(res_addr, 2, rs);
            emitter.emit(MSRI);
        }

    }
    void handleAlloca(llvm::Instruction *I)
    {
        if (auto *AI = llvm::dyn_cast<llvm::AllocaInst>(I))
        {
            llvm::Type *AllocType = AI->getAllocatedType();
            auto align = DL.getABITypeAlign(AllocType); // 用DL获取类型对齐，貌似一样
            llvm::outs() << "[Transfor]: Alloca type: " << AllocType << "\n";
            llvm::outs() << "[Transfor]: Alloca type alignment: " << align.value() << "\n";
            auto bytes = DL.getTypeAllocSize(AllocType);
            llvm::outs() << "[Transfor]: Alloca bytes: " << bytes << "\n";
            //下面换XXXTypeGen，不需要了
            // auto pdescriptor = std::make_shared<PrimitiveType>(bytes, align.value());
            // std::string instNameWithAddr;
            // llvm::raw_string_ostream OS(instNameWithAddr);
            // I->printAsOperand(OS, false); // 临时寄存器没名字，获取类似%1作为变量名称
            // OS.flush();
            // auto var = std::make_shared<Variable>(pdescriptor, nullptr);
            // symbol_table.register_variable_to_stack(instNameWithAddr, var);
            stack_size += bytes;
        }
    }

    void handleLoad(llvm::Instruction *I);

    void handleRET(llvm::Instruction *I);
    void handleStore(llvm::Instruction *inst)
    {

        if (auto *St = llvm::dyn_cast<llvm::StoreInst>(inst))
        {
            llvm::outs() << "[Transfor]: Store instruction\n";
            auto OP1 = St->getOperand(0);

            if (llvm::isa<llvm::Constant>(OP1))
            {
                uint64_t *value = new uint64_t;
                *value = 0;
                if (auto *CFP = llvm::dyn_cast<llvm::ConstantFP>(OP1))
                {
                    llvm::APFloat Val = CFP->getValueAPF();
                    // 转换为标准浮点类型
                    if (Val.getSizeInBits(Val.getSemantics()) == 32)
                    {
                        auto fvalue = CFP->getValueAPF().convertToFloat();
                        std::memcpy(reinterpret_cast<uint8_t *>(value), reinterpret_cast<const uint8_t *>(&fvalue), 4);
                        std::cout << "[Transfor]: Store value: " << std::hex << *value << std::dec << "\n";
                        // 处理32位float
                    }
                    else
                    {
                        auto dvalue = CFP->getValueAPF().convertToDouble();
                        std::memcpy(reinterpret_cast<uint8_t *>(value), reinterpret_cast<const uint8_t *>(&dvalue), 8);
                        std::cout << "[Transfor]: Store value: " << std::hex << *value << std::dec << "\n";
                        // 处理64位double
                    }
                }
                else if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(OP1))
                {
                    // 处理整数常量
                    uint64_t IntVal = CI->getSExtValue();
                    std::memcpy(reinterpret_cast<uint8_t *>(value), reinterpret_cast<uint8_t *>(&IntVal), 8);
                    std::cout << "[Transfor]: Store value: " << std::hex << *value << std::dec << "\n";
                }
                std::string instNameWithAddr;
                llvm::raw_string_ostream OS(instNameWithAddr);
                St->getOperand(1)->printAsOperand(OS, false);
                OS.flush();
                llvm::outs() << "[Transfor]: Store value: " << *value << "\n";
                llvm::outs() << "[Transfor]: Store to: " << instNameWithAddr << "\n";
                // symbol_table.change_variable(instNameWithAddr, reinterpret_cast<uint8_t *>(value));
                symbol_table.dump();

                auto size = getSymbol(ReturnType::XXXType,instNameWithAddr).size.value();
                llvm::outs() << "[Transfor]: Store size: " << size << "\n";
                VMINST *inst_src;
                VMINST *inst_dst;
                // std::string instNameWithAddr1;
                // llvm::raw_string_ostream OS1(instNameWithAddr1);
                // St->getOperand(1)->printAsOperand(OS1, false);
                // OS1.flush();
                // auto op1_record = symbol_table.get_variable_record(instNameWithAddr1);
                auto op1_addr = getSymbol(ReturnType::addr,St->getOperand(1));
                auto op1_description = getSymbol(ReturnType::description,St->getOperand(1));
                if (size <= 8)
                {
                    while (size != 1 && (size % 2 != 0 && size % 4 != 0 && size % 8 != 0))
                    {
                        size++;
                    }
                    uint8_t rs = 0;
                    if (size == 1)
                        rs = 0;
                    if (size == 2)
                        rs = 1;
                    if (size == 4)
                        rs = 2;
                    if (size == 8)
                        rs = 3;

                    if (op1_description=="local variable")
                    {
                        _MOV_IMM_RX_Inst *MIRI = new _MOV_IMM_RX_Inst(1, rs, *value);
                        inst_src = MIRI;
                        _MOV_RX_STACK_Inst *MRRI = new _MOV_RX_STACK_Inst(1, op1_addr, rs);
                        inst_dst = MRRI;
                        if (inst_src)
                            emitter.emit(inst_src);
                        if (inst_dst)
                            emitter.emit(inst_dst);
                    }
                    else
                    {
                        _MOV_IMM_RX_Inst *MIRI = new _MOV_IMM_RX_Inst(1, rs, *value);
                        inst_src = MIRI;
                        _MOV_RX_MEM_Inst *MRMI = new _MOV_RX_MEM_Inst(op1_addr, 1, rs);
                        inst_dst = MRMI;
                        if (inst_src)
                            emitter.emit(inst_src);
                        if (inst_dst)
                            emitter.emit(inst_dst);
                    }
                }
                else if (size > 8)
                {
                    auto petch = size / 8;
                    auto rem = size % 8;
                    auto offset = 0;
                    // 如果大于8字节，需要分段加载，小端序
                    if (op1_description!="local variable")
                    {
                        for (auto i = 0; i < petch; i++)
                        {
                            auto value_ = *reinterpret_cast<uint64_t *>(reinterpret_cast<uint8_t *>(value) + offset);
                            _MOV_IMM_RX_Inst *MIRI = new _MOV_IMM_RX_Inst(1, 3, value_);
                            inst_src = MIRI;
                            _MOV_RX_MEM_Inst *MRMI = new _MOV_RX_MEM_Inst(op1_addr + offset, 1, 3);
                            inst_dst = MRMI;
                            offset += 8;
                            if (inst_src)
                                emitter.emit(inst_src);
                            if (inst_dst)
                                emitter.emit(inst_dst);
                        }
                        if (rem != 0)
                        {
                            uint8_t rs = 0;
                            while (rem != 1 && (rem % 2 != 0 && rem % 4 != 0 && rem % 8 != 0))
                            {
                                rem++;
                            }
                            if (rem == 1)
                                rs = 0;
                            if (rem == 2)
                                rs = 1;
                            if (rem == 4)
                                rs = 2;
                            if (rem == 8)
                                rs = 3;
                            auto value_ = *reinterpret_cast<uint64_t *>(reinterpret_cast<uint8_t *>(value) + offset);
                            _MOV_IMM_RX_Inst *MIRI = new _MOV_IMM_RX_Inst(1, rs, value_);
                            inst_src = MIRI;
                            _MOV_RX_MEM_Inst *MRMI = new _MOV_RX_MEM_Inst(op1_addr + offset, 1, rs);
                            inst_dst = MRMI;
                            if (inst_src)
                                emitter.emit(inst_src);
                            if (inst_dst)
                                emitter.emit(inst_dst);
                        }
                    }
                    else
                    {
                        for (auto i = 0; i < petch; i++)
                        {
                            auto value_ = *reinterpret_cast<uint64_t *>(reinterpret_cast<uint8_t *>(value) + offset);
                            _MOV_IMM_RX_Inst *MIRI = new _MOV_IMM_RX_Inst(1, 3, value_);
                            inst_src = MIRI;
                            _MOV_RX_STACK_Inst *MRRI = new _MOV_RX_STACK_Inst(1, op1_addr + offset, 3);
                            inst_dst = MRRI;
                            offset += 8;
                            if (inst_src)
                                emitter.emit(inst_src);
                            if (inst_dst)
                                emitter.emit(inst_dst);
                        }
                        if (rem != 0)
                        {
                            uint8_t rs = 0;
                            while (rem != 1 && (rem % 2 != 0 && rem % 4 != 0 && rem % 8 != 0))
                            {
                                rem++;
                            }
                            if (rem == 1)
                                rs = 0;
                            if (rem == 2)
                                rs = 1;
                            if (rem == 4)
                                rs = 2;
                            if (rem == 8)
                                rs = 3;
                            auto value_ = *reinterpret_cast<uint64_t *>(reinterpret_cast<uint8_t *>(value) + offset);
                            _MOV_IMM_RX_Inst *MIRI = new _MOV_IMM_RX_Inst(1, rs, value_);
                            inst_src = MIRI;
                            _MOV_RX_STACK_Inst *MRRI = new _MOV_RX_STACK_Inst(1, op1_addr + offset, 3);
                            inst_dst = MRRI;
                            if (inst_src)
                                emitter.emit(inst_src);
                            if (inst_dst)
                                emitter.emit(inst_dst);
                        }
                    }
                }
                delete value;
                value = nullptr;
            }
            else
            {
                // std::string instNameWithAddr;
                // llvm::raw_string_ostream OS(instNameWithAddr);
                // St->getOperand(0)->printAsOperand(OS, false);
                // OS.flush();
                // // auto data = symbol_table.get_variable(instNameWithAddr);
                // auto op1_record = symbol_table.get_variable_record(instNameWithAddr);
                // llvm::outs() << "[Transfor]: op1_record: " << instNameWithAddr << "\n";

                // instNameWithAddr.clear();
                // St->getOperand(1)->printAsOperand(OS, false);
                // symbol_table.change_variable(instNameWithAddr, data);
                VMINST *inst_src = nullptr;
                VMINST *inst_dst = nullptr;

                // auto op2_record = symbol_table.get_variable_record(instNameWithAddr);
                // if (op1_record == nullptr)
                // {
                //     llvm::outs() << "[Transfor]: op1_record is nullptr\n";
                // }
                // if (op2_record == nullptr)
                // {
                //     llvm::outs() << "[Transfor]: op2_record is nullptr\n";
                // }
                auto op1_type = getSymbol(ReturnType::XXXType,St->getOperand(0));
                auto op2_type = getSymbol(ReturnType::XXXType,St->getOperand(1));
                auto op1_description = getSymbol(ReturnType::description,St->getOperand(0));
                auto op2_description = getSymbol(ReturnType::description,St->getOperand(1));
                auto op1_addr = getSymbol(ReturnType::addr,St->getOperand(0));
                auto op2_addr = getSymbol(ReturnType::addr,St->getOperand(1));
                if (auto size = op1_type.size.value(); size <= 8)
                {
                    while (size != 1 && (size % 2 != 0 && size % 4 != 0 && size % 8 != 0))
                    {
                        size++;
                    }
                    uint8_t rs = 0;
                    if (size == 1)
                        rs = 0;
                    if (size == 2)
                        rs = 1;
                    if (size == 4)
                        rs = 2;
                    if (size == 8)
                        rs = 3;
                    if (op1_description=="variable"||!op1_description.starts_with("arg"))
                    {
                        if (Fname != "main")
                        {
                            llvm::outs() << "[Transfor]: op1_record->stack_offset = " << op1_addr << "\n";
                        }
                        _MOV_MEM_RX_Inst *MMRI = new _MOV_MEM_RX_Inst(op1_addr, 0, rs);
                        inst_src = MMRI;
                        _MOV_RX_STACK_Inst *MRSI = new _MOV_RX_STACK_Inst(0, op2_addr, rs);
                        inst_dst = MRSI;
                        if (inst_src)
                            emitter.emit(inst_src);
                        if (inst_dst)
                            emitter.emit(inst_dst);
                    }
                    else
                    {
                        _MOV_STACK_STACK_Inst *MSSI = new _MOV_STACK_STACK_Inst(op1_addr, op2_addr, rs, 1);
                        inst_src = MSSI;
                        if (inst_src)
                            emitter.emit(inst_src);
                        if (inst_dst)
                            emitter.emit(inst_dst);
                    }
                }
                else if (size > 8)
                {
                    auto petch = size / 8;
                    auto rem = size % 8;
                    auto offset = 0;
                    // 如果大于8字节，需要分段加载，小端序
                    if (op1_description=="local variable")
                    {
                        for (auto i = 0; i < petch; i++)
                        {
                            _MOV_STACK_STACK_Inst *MSSI = new _MOV_STACK_STACK_Inst(op1_addr + offset, op2_addr + offset, 3, 1);
                            inst_src = MSSI;
                            offset += 8;
                            if (inst_src)
                                emitter.emit(inst_src);
                            if (inst_dst)
                                emitter.emit(inst_dst);
                        }
                        if (rem != 0)
                        {
                            uint8_t rs = 0;
                            while (rem != 1 && (rem % 2 != 0 && rem % 4 != 0 && rem % 8 != 0))
                            {
                                rem++;
                            }
                            if (rem == 1)
                                rs = 0;
                            if (rem == 2)
                                rs = 1;
                            if (rem == 4)
                                rs = 2;
                            if (rem == 8)
                                rs = 3;
                            _MOV_STACK_STACK_Inst *MSSI = new _MOV_STACK_STACK_Inst(op1_addr + offset, op2_addr + offset, rs, 1);
                            inst_src = MSSI;
                            if (inst_src)
                                emitter.emit(inst_src);
                            if (inst_dst)
                                emitter.emit(inst_dst);
                        }
                    }
                    else
                    {
                        for (auto i = 0; i < petch; i++)
                        {
                            _MOV_MEM_RX_Inst *MSRI = new _MOV_MEM_RX_Inst(op1_addr + offset, 0, 3);
                            inst_src = MSRI;
                            _MOV_RX_STACK_Inst *MRRI = new _MOV_RX_STACK_Inst(0, op2_addr + offset, 3);
                            inst_dst = MRRI;
                            offset += 8;
                            if (inst_src)
                                emitter.emit(inst_src);
                            if (inst_dst)
                                emitter.emit(inst_dst);
                        }
                        if (rem != 0)
                        {
                            uint8_t rs = 0;
                            while (rem != 1 && (rem % 2 != 0 && rem % 4 != 0 && rem % 8 != 0))
                            {
                                rem++;
                            }
                            if (rem == 1)
                                rs = 0;
                            if (rem == 2)
                                rs = 1;
                            if (rem == 4)
                                rs = 2;
                            if (rem == 8)
                                rs = 3;
                            _MOV_MEM_RX_Inst *MSRI = new _MOV_MEM_RX_Inst(op1_addr + offset, 0, rs);
                            inst_src = MSRI;
                            _MOV_RX_STACK_Inst *MRRI = new _MOV_RX_STACK_Inst(0, op2_addr + offset, rs);
                            inst_dst = MRRI;
                            if (inst_src)
                                emitter.emit(inst_src);
                            if (inst_dst)
                                emitter.emit(inst_dst);
                        }
                    }
                }
            }
        }
    }

    void handleSext(llvm::Instruction *I)   //vector用到这个指令，暂时不处理了
    {
        if (auto *SI = llvm::dyn_cast<llvm::SExtInst>(I))
        {
            auto op1 = SI->getOperand(0);
            auto op1_size = getSymbol(ReturnType::XXXType,op1).size.value();
            auto op1_description = getSymbol(ReturnType::description,op1);
            auto op1_addr = getSymbol(ReturnType::addr,op1);
            auto res_size = getSymbol(ReturnType::XXXType,I).size.value();
            auto res_description = getSymbol(ReturnType::description,I);
            auto res_addr = getSymbol(ReturnType::addr,I);
            uint8_t rs = 0;
            switch(res_size)
            {
                case 1:
                    rs = 0;
                    break;
                case 2:
                    rs = 1;
                    break;
                case 4:
                    rs = 2;
                    break;
                case 8:
                    rs = 3;
                    break;
                default:
                    throw std::runtime_error("[Transfor]: Unknown UnSignedOPType size");
            }
            if(op1_description=="local variable")
            {
                VMINST *MSRI = new _MOV_STACK_RX_Inst(op1_addr, 2, rs);
                emitter.emit(MSRI);
            }else{
                VMINST *MSRI = new _MOV_MEM_RX_Inst(op1_addr, 2, rs);
                emitter.emit(MSRI);
            }
            
        }
    }
    bool isExternalFunction(std::string mangled_name);
    void handleCall(llvm::Instruction *I)
    {

        if (auto *CI = llvm::dyn_cast<llvm::CallInst>(I))
        {

            auto callFname = CI->getCalledFunction()->getName().str();
            // auto record = symbol_table.get_variable_record(callFname);
            if (1)
            {
                if (isExternalFunction(callFname))  //call外部函数
                {
                    handleExternalFCall(CI);
                }else if(!isUserFunction(callFname)){   //两者都非，说明信息收集出错，直接返回
                    llvm::outs()<<"[Transfor]: Call function is not user function neither extern function\n";
                    return;
                }
                else
                {
                    // std::string instNameWithAddr;
                    // llvm::raw_string_ostream OS(instNameWithAddr);
                    // auto *callValue = llvm::dyn_cast<llvm::Value>(CI);
                    // callValue->printAsOperand(OS, false);
                    // llvm::outs() << "[Transfor]: Call Instruction: " << instNameWithAddr << "\n";
                    // auto size = DL.getTypeAllocSize(callValue->getType());
                    // auto align = DL.getABITypeAlign(callValue->getType());
                    // llvm::outs() << "[Transfor]: Call Instruction result size: " << size << "\n";
                    // auto pdescriptor = std::make_shared<PrimitiveType>(size, align.value());
                    // auto var = std::make_shared<Variable>(pdescriptor, nullptr);
                    // symbol_table.register_variable_to_mem(instNameWithAddr, var);
                    // auto addr = symbol_table.get_variable_record(instNameWithAddr)->base_addr;
                    if(I->getType()->isVoidTy()){
                        llvm::outs() << "[Transfor]: Call Instruction result is void\n";

                    }else{
                        auto addr = getSymbol(ReturnType::addr,I);
                        llvm::outs() << "[Transfor]: Call Instruction result addr: " << addr << "\n";
    
                        VMINST *inst_ = new _MOV_IMM_RX_Inst(1, 3, addr);
                        emitter.emit(inst_);
                        VMINST* _inst = new _PUSH_R1_Inst();
                        
                        emitter.emit(_inst);
                    }

                    Stack.set_rsp(Stack.get_rsp() - 8);
                    // OS.flush();
                    auto argcount = 0;
                    for (auto &arg : CI->args())
                    {
                        std::string instNameWithAddr;
                        llvm::raw_string_ostream OS(instNameWithAddr);
                        auto *argValue = llvm::dyn_cast<llvm::Value>(&arg);
                        argValue->printAsOperand(OS, false);
                        llvm::outs() << "[Transfor]: Call arg: " << instNameWithAddr << "\n";
                        OS.flush();
                        if(!argValue->getName().str().empty()){
                            instNameWithAddr=argValue->getName().str();
                        }   
                        auto description = getSymbol(ReturnType::description,argValue);
                        auto size = getSymbol(ReturnType::XXXType,instNameWithAddr).size.value();
                        auto addr = getSymbol(ReturnType::addr,instNameWithAddr);
                        llvm::outs()<<instNameWithAddr<<" size is "<<size<<"\n";
                        // auto data = symbol_table.get_variable(instNameWithAddr);
                        // stack.push(data, size);
                        VMINST *inst_src = nullptr;
                        VMINST *inst_dst = nullptr;
                        auto petch = size / 8;
                        auto rem = size % 8;
                        auto offset = 0;
                        llvm::outs() << "[Transfor]: petch:" << petch << " rem:" << rem << " offset:" << offset << "\n";
                        // 如果大于8字节，需要分段加载，小端序
                        if (description!="local variable")
                        {
                            if (petch != 0)
                            {
                                for (auto i = 0; i < petch; i++)
                                {
                                    _MOV_MEM_RX_Inst *MSRI = new _MOV_MEM_RX_Inst(addr + offset, 1, 3);
                                    inst_src = MSRI;
                                    _PUSH_RX_WITH_SIZE_Inst *PR1I = new _PUSH_RX_WITH_SIZE_Inst(1, 3);
                                    Stack.set_rsp(Stack.get_rsp() - 8);
                                    inst_dst = PR1I;
                                    offset += 8;
                                    if (inst_src)
                                        emitter.emit(inst_src);
                                    if (inst_dst)
                                        emitter.emit(inst_dst);
                                }
                            }
                            if (rem != 0)
                            {
                                uint8_t rs = 0;
                                while (rem != 1 && (rem % 2 != 0 && rem % 4 != 0 && rem % 8 != 0))
                                {
                                    rem++;
                                }
                                if (rem == 1)
                                    rs = 0;
                                if (rem == 2)
                                    rs = 1;
                                if (rem == 4)
                                    rs = 2;
                                if (rem == 8)
                                    rs = 3;
                                _MOV_MEM_RX_Inst *MSRI = new _MOV_MEM_RX_Inst(addr + offset, 1, rs);
                                llvm::outs() << "[Transfor]: op1_record->base_addr + offset:" << addr + offset << "\n";
                                inst_src = MSRI;
                                _PUSH_RX_WITH_SIZE_Inst *PR1I = new _PUSH_RX_WITH_SIZE_Inst(1, rs);
                                Stack.set_rsp(Stack.get_rsp() - rs);
                                inst_dst = PR1I;
                                if (inst_src)
                                    emitter.emit(inst_src);
                                if (inst_dst)
                                    emitter.emit(inst_dst);
                            }
                        }
                        else
                        {
                            if (petch != 0)
                            {
                                for (auto i = 0; i < petch; i++)
                                {
                                    _PUSH_RX_WITH_SIZE_Inst *PR1I = new _PUSH_RX_WITH_SIZE_Inst(1, 3);
                                    Stack.set_rsp(Stack.get_rsp() - 8);
                                    inst_dst = PR1I;
                                    _MOV_STACK_RX_Inst *MSRI = new _MOV_STACK_RX_Inst(addr + offset, 1, 3);
                                    inst_src = MSRI;
                                    offset += 8;
                                    if (inst_src)
                                        emitter.emit(inst_src);
                                    if (inst_dst)
                                        emitter.emit(inst_dst);
                                }
                            }
                            if (rem != 0)
                            {
                                uint8_t rs = 0;
                                while (rem != 1 && (rem % 2 != 0 && rem % 4 != 0 && rem % 8 != 0))
                                {
                                    rem++;
                                }
                                if (rem == 1)
                                    rs = 0;
                                if (rem == 2)
                                    rs = 1;
                                if (rem == 4)
                                    rs = 2;
                                if (rem == 8)
                                    rs = 3;
                                _PUSH_RX_WITH_SIZE_Inst *PR1I = new _PUSH_RX_WITH_SIZE_Inst(1, rs);
                                
                                inst_dst = PR1I;
                                _MOV_STACK_RX_Inst *MSRI = new _MOV_STACK_RX_Inst(addr + offset, 1, rs);
                                llvm::outs() << "[Transfor]: op1_record->stack_offset - offset:" << addr + offset << "\n";
                                inst_src = MSRI;
                                if (inst_src)
                                    emitter.emit(inst_src);
                                if (inst_dst)
                                    emitter.emit(inst_dst);
                                Stack.set_rsp(Stack.get_rsp() - rs);
                            }
                        }
                    }
                    if (isExternalFunction(CI->getCalledFunction()->getName().str()))
                    {
                        llvm::outs() << "[Transfor]: Call External Function,unsupported\n";
                        abort();
                    }
                    else
                    {
                        auto old_rbp = Stack.get_rbp();
                        Stack.push(reinterpret_cast<uint8_t *>(&old_rbp), 8);
                        Stack.set_rbp(Stack.get_rsp());
                        VMINST *PRBPI = new _PUSH_RBP_Inst();
                        emitter.emit(PRBPI);
                        Stack.set_rsp(Stack.get_rsp() - 8);
                        VMINST *PPI = new _PUSH_PC_Inst();
                        emitter.emit(PPI);
                        VMINST *MRBPI = new _MOV_RSP_RBP_Inst();
                        emitter.emit(MRBPI);
                        Stack.set_rbp(Stack.get_rsp());
                        VMINST *CII = new _CALL_INTERNAL_Inst(uint64_t(0));
                        emitter.emit(CII);
                        llvm::outs() << "[Transfor]: untill now call code size :" << emitter.size() << "\n";
                        jump_table jt{
                            .fname = CI->getCalledFunction()->getName().str(),
                            .call_addr = emitter.size() - 8,
                            .called_offset = 0,
                        };
                        jump_tables.push_back(jt);
                    }
                }
            }
        }
    }

    void handleBinaryOperator(llvm::Instruction *inst)
    {
        if (llvm::BinaryOperator *BO = llvm::dyn_cast<llvm::BinaryOperator>(inst))
        {
            llvm::outs() << "[Transfor]: Binary Operator\n";
        }
        else
        {
            return;
        }
        switch (inst->getOpcode())
        {
        case llvm::Instruction::Add:
            handleAdd(inst);
            break;
        case llvm::Instruction::FAdd:
            handleFAdd(inst);
            break;
        case llvm::Instruction::Sub:
            handleSub(inst);
            break;
        case llvm::Instruction::FSub:
            handleFSub(inst);
            break;           
        case llvm::Instruction::Mul:
            handleMul(inst);
            break;
        case llvm::Instruction::FMul:
            handleFMul(inst);
            break;
        case llvm::Instruction::UDiv:
            handleUDiv(inst);
            break;
        case llvm::Instruction::SDiv:
            handleSDiv(inst);
            break;
        case llvm::Instruction::FDiv:
            handleFDiv(inst);
            break;
        case llvm::Instruction::URem:
            handleURem(inst);
            break;
        case llvm::Instruction::SRem:
            handleSRem(inst);
            break;
        case llvm::Instruction::FRem:
            handleFRem(inst);
            break;  
        case llvm::Instruction::And:
            handleAnd(inst);
            break;
        case llvm::Instruction::Or:
            handleOr(inst);
            break;
        case llvm::Instruction::Xor:
            handleXor(inst);
            break;
        case llvm::Instruction::Shl:
            handleShl(inst);
            break;
        case llvm::Instruction::LShr:
            handleLShr(inst);
            break;
        case llvm::Instruction::AShr:
            handleAShr(inst);
            break;
        case llvm::Instruction::ICmp:
            llvm::outs() << "[Transfor]: ICmp instruction\n";
            handleICmp(inst);
            break;
        case llvm::Instruction::FCmp:
            handleFCmp(inst);
            break;
        default:
            llvm::outs() << "[Transfor]: unsupported binary operator\n";
            break;
        }
    }

    // void handleAdd(llvm::Instruction *inst)
    // {
    //     // auto value = llvm::dyn_cast<llvm::Value>(inst);
    //     // auto type = inst->getType();
    //     // auto size = DL.getTypeSizeInBits(type).getKnownMinValue() / 8;
    //     llvm::outs() << "[Transfor]: Add instruction\n";
    //     // llvm::outs() << "[Transfor]: Add size: " << size << "\n";
    //     // auto align = DL.getABITypeAlign(type).value();
    //     // llvm::outs() << "[Transfor]: Add align: " << align << "\n";
    //     // auto pd = std::make_shared<PrimitiveType>(size, align);
    //     // uint8_t *data = new uint8_t[size];
    //     // *data = 0;
    //     // auto var = std::make_shared<Variable>(pd, data);
    //     // std::string instNameWithAddr;
    //     // llvm::raw_string_ostream OS(instNameWithAddr);
    //     // inst->printAsOperand(OS, false);
    //     // OS.flush();
    //     auto size = getSymbol(ReturnType::XXXType,inst).size.value();
    //     auto I_description = getSymbol(ReturnType::description,inst);
    //     auto I_addr = getSymbol(ReturnType::addr,inst);
    //     auto rs = 0;
    //     switch (size)
    //     {
    //     case 1:
    //         rs = 0;
    //         break;
    //     case 2:
    //         rs = 1;
    //         break;
    //     case 4:
    //         rs = 2;
    //         break;
    //     case 8:
    //         rs = 3;
    //         break;
    //     default:
    //         break;
    //     }
    //     // symbol_table.register_variable_to_mem(instNameWithAddr, var);
    //     // auto addr_test = symbol_table.get_variable_record(instNameWithAddr)->base_addr;
    //     auto addr_test = getSymbol(ReturnType::addr,inst);
    //     llvm::outs() << "[Transfor]: Add addr: " << addr_test << "\n";
    //     auto op1 = llvm::dyn_cast<llvm::Value>(inst->getOperand(0));
    //     auto op2 = llvm::dyn_cast<llvm::Value>(inst->getOperand(1));
    //     if (llvm::isa<llvm::ConstantInt>(op1) && llvm::isa<llvm::ConstantInt>(op2))
    //     {
    //         auto op1_value = llvm::dyn_cast<llvm::ConstantInt>(op1)->getSExtValue();
    //         auto op2_value = llvm::dyn_cast<llvm::ConstantInt>(op2)->getSExtValue();
    //         VMINST *MIR1 = new _MOV_IMM_RX_Inst(1, rs, op1_value);
    //         VMINST *MIR2 = new _MOV_IMM_RX_Inst(2, rs, op2_value);
    //         emitter.emit(MIR1);
    //         emitter.emit(MIR2);

    //         VMINST *ADD = new _ADD_RX_RY_Inst(rs, 1, 2);
    //         emitter.emit(ADD);

    //         // auto addr = symbol_table.get_variable_record(instNameWithAddr)->base_addr;
    //         VMINST *MRR = new _MOV_RX_MEM_Inst(addr_test, 2, rs);
    //         emitter.emit(MRR);
    //         return;
    //     }
    //     else if (llvm::isa<llvm::ConstantInt>(op1) || llvm::isa<llvm::ConstantInt>(op2))
    //     {
    //         uint64_t value = 0;
    //         // std::shared_ptr<SymbolTable::VariableRecord> record = nullptr;
    //         description description_m;
    //         addr addr_m;
    //         if (llvm::isa<llvm::ConstantInt>(op1))
    //         {
    //             auto data = llvm::dyn_cast<llvm::ConstantInt>(op1)->getSExtValue();
    //             std::memcpy(reinterpret_cast<uint8_t *>(&value), reinterpret_cast<uint8_t *>(&data), 8);
    //             // std::string op2_name;
    //             // llvm::raw_string_ostream OS(op2_name);
    //             // inst->getOperand(1)->printAsOperand(OS, false);
    //             // llvm::outs() << "[Transfor]: op2_name: " << op2_name << "\n";
    //             // record = symbol_table.get_variable_record(op2_name);
    //             description_m = getSymbol(ReturnType::description,inst->getOperand(1));
    //             addr_m = getSymbol(ReturnType::addr,inst->getOperand(1));
    //         }
    //         else
    //         {
    //             auto data = llvm::dyn_cast<llvm::ConstantInt>(op2)->getSExtValue();
    //             std::memcpy(reinterpret_cast<uint8_t *>(&value), reinterpret_cast<uint8_t *>(&data), 8);
    //             // std::string op1_name;
    //             // llvm::raw_string_ostream OS(op1_name);
    //             // inst->getOperand(0)->printAsOperand(OS, false);
    //             // llvm::outs() << "[Transfor]: op1_name: " << op1_name << "\n";
    //             // record = symbol_table.get_variable_record(op1_name);
    //             description_m = getSymbol(ReturnType::description,inst->getOperand(0));
    //             addr_m = getSymbol(ReturnType::addr,inst->getOperand(0));
    //         }
    //         if (description_m=="local variable")
    //         {
    //             VMINST *MSRX = new _MOV_STACK_RX_Inst(addr_m, 2, rs);
    //             VMINST *ADD = new _ADD_IMM_RX_Inst(2, rs, value);
    //             emitter.emit(MSRX);
    //             emitter.emit(ADD);
    //         }
    //         else if (description_m=="variable")
    //         {
    //             VMINST *MMMI = new _MOV_MEM_RX_Inst(addr_m, 2, rs);
    //             VMINST *ADD = new _ADD_IMM_RX_Inst(2, rs, value);
    //             emitter.emit(MMMI);
    //             emitter.emit(ADD);
    //         }
    //         // auto res_record = symbol_table.get_variable_record(instNameWithAddr);
    //         if (I_description=="local variable")
    //         {
    //             VMINST *MRXS = new _MOV_RX_STACK_Inst(2, I_addr, rs);
    //             emitter.emit(MRXS);
    //         }
    //         else
    //         {
    //             VMINST *MRXM = new _MOV_RX_MEM_Inst(I_addr, 2, rs);
    //             llvm::outs() << "[Transfor]: res_record->base_addr: " << I_addr << "\n";
    //             emitter.emit(MRXM);
    //         }
    //         return;
    //     }
    //     else
    //     {
    //         // std::string op1_name;
    //         // std::string op2_name;
    //         // llvm::raw_string_ostream OS1(op1_name);
    //         // llvm::raw_string_ostream OS2(op2_name);
    //         // inst->getOperand(0)->print(OS1);
    //         // inst->getOperand(1)->print(OS2);

    //         // auto op1_record = symbol_table.get_variable_record(op1_name);
    //         // auto op2_record = symbol_table.get_variable_record(op2_name);
    //         auto op1_description = getSymbol(ReturnType::description,inst->getOperand(0));
    //         auto op2_description = getSymbol(ReturnType::description,inst->getOperand(1));
    //         auto op1_addr = getSymbol(ReturnType::addr,inst->getOperand(0));
    //         auto op2_addr = getSymbol(ReturnType::addr,inst->getOperand(1));

    //         VMINST *op1 = nullptr;
    //         VMINST *op2 = nullptr;
    //         if (op1_description=="local variable")
    //         {
    //             op1 = new _MOV_STACK_RX_Inst(op1_addr, 1, rs);
    //         }
    //         else
    //         {
    //             op1 = new _MOV_MEM_RX_Inst(op1_addr, 1, rs);
    //         }
    //         if (op2_description=="local variable")
    //         {
    //             op2 = new _MOV_STACK_RX_Inst(op2_addr, 2, rs);
    //         }
    //         else
    //         {
    //             op2 = new _MOV_MEM_RX_Inst(op2_addr, 2, rs);
    //         }
    //         emitter.emit(op1);
    //         emitter.emit(op2);
    //         VMINST *ADD = new _ADD_RX_RY_Inst(rs, 1, 2);
    //         emitter.emit(ADD);
    //         // auto res_record = symbol_table.get_variable_record(instNameWithAddr);
    //         if (I_description=="local variable")
    //         {
    //             VMINST *MRX = new _MOV_RX_STACK_Inst(2, I_addr, rs);
    //             emitter.emit(MRX);
    //         }
    //         else
    //         {
    //             VMINST *MRXM = new _MOV_RX_MEM_Inst(I_addr, 2, rs);
    //             emitter.emit(MRXM);
    //         }
    //         return;
    //     }
    // }
    void handleAdd(llvm::Instruction *inst)
    {
        handleBasicOP<_ADD_IMM_RX_Inst,_ADD_RX_RY_Inst>(inst);
    }
    void handleSub(llvm::Instruction *inst)
    {
        handleBasicOP<_SUB_IMM_RX_Inst,_SUB_RX_RY_Inst>(inst);
    }
    void handleMul(llvm::Instruction *inst)
    {
        handleBasicOP<_MUL_IMM_RX_Inst,_MUL_RX_RY_Inst>(inst);
    }
    void handleUDiv(llvm::Instruction *inst)
    {
        handleBasicOP<_UDIV_IMM_RX_Inst,_UDIV_RX_RY_Inst>(inst);
    }
    void handleSDiv(llvm::Instruction *inst)
    {
        handleBasicOP<_SDIV_IMM_RX_Inst,_SDIV_RX_RY_Inst>(inst);
    }
    void handleURem(llvm::Instruction *inst)
    {
        // todo
    }
    void handleSRem(llvm::Instruction *inst)
    {
        // todo
    }
    void handleAnd(llvm::Instruction *inst)
    {
        // todo
    }
    void handleOr(llvm::Instruction *inst)
    {
        // todo
    }
    void handleXor(llvm::Instruction *inst)
    {
        // todo
    }
    void handleShl(llvm::Instruction *inst)
    {
        // todo
    }
    void handleLShr(llvm::Instruction *inst)
    {
        // todo
    }
    void handleAShr(llvm::Instruction *inst)
    {
        // todo
    }
    void handleFAdd(llvm::Instruction *inst)
    {
        handleBasicOP<_FADD_IMM_RX_Inst,_FADD_RX_RY_Inst>(inst);
    }
    void handleFSub(llvm::Instruction *inst)
    {
        handleBasicOP<_FSUB_IMM_RX_Inst,_FSUB_RX_RY_Inst>(inst);
    }
    void handleFMul(llvm::Instruction *inst)
    {
        handleBasicOP<_FMUL_IMM_RX_Inst,_FMUL_RX_RY_Inst>(inst);
    }
    void handleFDiv(llvm::Instruction *inst)
    {
        handleBasicOP<_FDIV_IMM_RX_Inst,_FDIV_RX_RY_Inst>(inst);
    }
    void handleFRem(llvm::Instruction *inst)
    {
        // todo
    }
    void handleBranch(llvm::Instruction *inst)
    {
        if (llvm::BranchInst *BI = llvm::dyn_cast<llvm::BranchInst>(inst))
        {
            if (BI->isConditional())
            {
                llvm::outs() << "[Transfor]: Conditional Branch\n";
                auto *cond = BI->getCondition();
                auto *trueDest = BI->getSuccessor(0);
                auto *falseDest = BI->getSuccessor(1);
                std::string trueDestName;
                std::string falseDestName;
                llvm::raw_string_ostream OS(trueDestName);
                trueDest->printAsOperand(OS, false);
                OS.flush();
                llvm::raw_string_ostream OS2(falseDestName);
                falseDest->printAsOperand(OS2, false);
                OS2.flush();
                auto trueDestAddr = BBaddr[trueDestName];
                auto falseDestAddr = BBaddr[falseDestName];
                VMINST *MOV = new _MOV_IMM_RX_Inst(5, 3, trueDestAddr);
                emitter.emit(MOV);
                toInsBB[emitter.size()-8]=trueDestName;
                VMINST *JMP_T = new _JNZ_R5_Inst();
                emitter.emit(JMP_T);
                VMINST *MOV2 = new _MOV_IMM_RX_Inst(5, 3, falseDestAddr);
                emitter.emit(MOV2);
                toInsBB[emitter.size()-8]=falseDestName;
                VMINST *JMP_F = new _JZ_R5_Inst();
                emitter.emit(JMP_F);
            }
            else
            {
                llvm::outs() << "[Transfor]: Unconditional Branch\n";
                auto *dest = BI->getSuccessor(0);
                std::string destName;
                llvm::raw_string_ostream OS(destName);
                dest->printAsOperand(OS, false);
                OS.flush();
                auto destAddr = BBaddr[destName];
                VMINST *MOV = new _MOV_IMM_RX_Inst(4, 3, destAddr);
                emitter.emit(MOV);
                toInsBB[emitter.size()-8]=destName;
                VMINST *JMP = new _JMP_R4_Inst();
                emitter.emit(JMP);
            }
        }
    }
    void handleICmp(llvm::Instruction *inst)
    {
        if (llvm::ICmpInst *ICmp = llvm::dyn_cast<llvm::ICmpInst>(inst))
        {
            auto Predicate = ICmp->getPredicate();
            switch (Predicate)
            {
            case llvm::ICmpInst::ICMP_EQ:{
                llvm::outs() << "[Transfor]: ICMP_EQ\n";
                constexpr auto op1 = OpCode::_icmp_eq_imm_rx;
                constexpr auto op2 = OpCode::_icmp_eq_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
                break;
            }
            case llvm::ICmpInst::ICMP_NE:{
                llvm::outs() << "[Transfor]: ICMP_NE\n";
                constexpr auto op1 = OpCode::_icmp_ne_imm_rx;
                constexpr auto op2 = OpCode::_icmp_ne_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
                break;
            }
            case llvm::ICmpInst::ICMP_UGT:{
                llvm::outs() << "[Transfor]: ICMP_UGT\n";
                constexpr auto op1 = OpCode::_icmp_ugt_imm_rx;
                constexpr auto op2 = OpCode::_icmp_ugt_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
                break;
            }
            case llvm::ICmpInst::ICMP_UGE:{
                llvm::outs() << "[Transfor]: ICMP_UGE\n";
                constexpr auto op1 = OpCode::_icmp_uge_imm_rx;
                constexpr auto op2 = OpCode::_icmp_uge_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
                break;
            }
            case llvm::ICmpInst::ICMP_ULT:{
                llvm::outs() << "[Transfor]: ICMP_ULT\n";
                constexpr auto op1 = OpCode::_icmp_ult_imm_rx;
                constexpr auto op2 = OpCode::_icmp_ult_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
                break;
            }
            case llvm::ICmpInst::ICMP_ULE:{
                llvm::outs() << "[Transfor]: ICMP_ULE\n";
                constexpr auto op1 = OpCode::_icmp_ule_imm_rx;
                constexpr auto op2 = OpCode::_icmp_ule_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
                break;
            }
            case llvm::ICmpInst::ICMP_SGT:{
                llvm::outs() << "[Transfor]: ICMP_SGT\n";
                constexpr auto op1 = OpCode::_icmp_sgt_imm_rx;
                constexpr auto op2 = OpCode::_icmp_sgt_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
                break;
            }
            case llvm::ICmpInst::ICMP_SGE:{
                llvm::outs() << "[Transfor]: ICMP_SGE\n";
                constexpr auto op1 = OpCode::_icmp_sge_imm_rx;
                constexpr auto op2 = OpCode::_icmp_sge_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
                break;
            }
            case llvm::ICmpInst::ICMP_SLT:{
                llvm::outs() << "[Transfor]: ICMP_SLT\n";
                constexpr auto op1 = OpCode::_icmp_slt_imm_rx;
                constexpr auto op2 = OpCode::_icmp_slt_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
                break;
            }
            case llvm::ICmpInst::ICMP_SLE:{
                llvm::outs() << "[Transfor]: ICMP_SLE\n";
                constexpr auto op1 = OpCode::_icmp_sle_imm_rx;
                constexpr auto op2 = OpCode::_icmp_sle_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
                break;
            }
            default:
                llvm::outs() << "[Transfor]: unsupported ICmp Predicate\n";
                abort();
                break;
            }
            
        }
    }

    void handleFCmp(llvm::Instruction *inst)
    {
        if(auto *FCMPI=llvm::dyn_cast<llvm::FCmpInst>(inst)){
            auto Predicate = FCMPI->getPredicate();
            switch (Predicate)
            {
            case llvm::FCmpInst::FCMP_OEQ:{
                llvm::outs()<<"[Transfor]: FCMP_OEQ\n";
                constexpr auto op1 = OpCode::_fcmp_oeq_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_oeq_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_OGT:{
                llvm::outs()<<"[Transfor]: FCMP_OGT\n";
                constexpr auto op1 = OpCode::_fcmp_ogt_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_ogt_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_OGE:{
                llvm::outs()<<"[Transfor]: FCMP_OGE\n";
                constexpr auto op1 = OpCode::_fcmp_oge_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_oge_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_OLT:{
                llvm::outs()<<"[Transfor]: FCMP_OLT\n";
                constexpr auto op1 = OpCode::_fcmp_olt_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_olt_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_OLE:{
                llvm::outs()<<"[Transfor]: FCMP_OLE\n";
                constexpr auto op1 = OpCode::_fcmp_ole_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_ole_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_ONE:{
                llvm::outs()<<"[Transfor]: FCMP_ONE\n";
                constexpr auto op1 = OpCode::_fcmp_one_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_one_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_ORD:{
                llvm::outs()<<"[Transfor]: FCMP_ORD\n";
                constexpr auto op1 = OpCode::_fcmp_ord_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_ord_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_UNO:{
                llvm::outs()<<"[Transfor]: FCMP_UNO\n";
                constexpr auto op1 = OpCode::_fcmp_uno_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_uno_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_UEQ:{
                llvm::outs()<<"[Transfor]: FCMP_UEQ\n";
                constexpr auto op1 = OpCode::_fcmp_ueq_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_ueq_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_UGT:{
                llvm::outs()<<"[Transfor]: FCMP_UGT\n";
                constexpr auto op1 = OpCode::_fcmp_ugt_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_ugt_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_UGE:{
                llvm::outs()<<"[Transfor]: FCMP_UGE\n";
                constexpr auto op1 = OpCode::_fcmp_uge_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_uge_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_ULT:{
                llvm::outs()<<"[Transfor]: FCMP_ULT\n";
                constexpr auto op1 = OpCode::_fcmp_ult_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_ult_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_ULE:{
                llvm::outs()<<"[Transfor]: FCMP_ULE\n";
                constexpr auto op1 = OpCode::_fcmp_ule_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_ule_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_UNE:{
                llvm::outs()<<"[Transfor]: FCMP_UNE\n";
                constexpr auto op1 = OpCode::_fcmp_une_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_une_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            case llvm::FCmpInst::FCMP_TRUE:{
                llvm::outs()<<"[Transfor]: FCMP_TRUE\n";
                constexpr auto op1 = OpCode::_fcmp_true_imm_rx;
                constexpr auto op2 = OpCode::_fcmp_true_rx_ry;
                handleCmpInst<_CMP_IMM_RX_Inst<op1>,_CMP_RX_RY_Inst<op2>>(inst);
            }
            default:
                llvm::outs()<<"[Transfor]: unsupported FCmp Predicate\n";
                abort();
                break;
            }

        }
    }

    bool handleInvoke(llvm::BasicBlock* bb,llvm::Instruction *inst)
    {
        if (llvm::InvokeInst *II = llvm::dyn_cast<llvm::InvokeInst>(inst))
        {
            llvm::InvokeInst* clone = llvm::dyn_cast<llvm::InvokeInst>(inst->clone());
            llvm::outs() << "[Transfor]: Invoke instruction\n";
            
            // 获取被调用的函数
            llvm::Value* calledValue = II->getCalledOperand();
            llvm::Function* calledFunction = II->getCalledFunction();
            
            if (!calledFunction) {
                llvm::outs() << "[Transfor]: Invoke function is not a direct call\n";
                return false;
            }
            
            // 收集参数
            llvm::SmallVector<llvm::Value *, 8> args(II->arg_begin(), II->arg_end());
            std::string return_value;
            llvm::raw_string_ostream OS(return_value);
            if(!II->getType()->isVoidTy()){
                II->printAsOperand(OS, false);
                OS.flush();
            }
            
            // 创建CallInst，但不指定插入点（稍后会替换）
            llvm::CallInst *CI = llvm::CallInst::Create(
                calledFunction->getFunctionType(),  // 函数类型
                calledValue,                        // 被调用的值
                args                               // 参数列表
                                       // 使用与invoke相同的名称
            );
            // llvm::Value* valuename = llvm::dyn_cast<llvm::Value>(inst);
            // llvm::ValueName* name = llvm::dyn_cast<llvm::ValueName>(valuename);
            // CI->setValueName(name);
            // 复制调用约定和属性
            CI->setCallingConv(II->getCallingConv());
            CI->setAttributes(II->getAttributes());
            
            // 复制元数据
            CI->copyMetadata(*II);
            
            CI->setDebugLoc(II->getDebugLoc());
            // 创建一个无条件分支到正常目标
            llvm::BasicBlock *normalDest = II->getNormalDest();
            llvm::BranchInst *BI = llvm::BranchInst::Create(normalDest);
            // II->eraseFromParent();
            // llvm::ReplaceInstWithInst(inst, CI);
            // II->print(llvm::errs());
            CI = llvm::changeToCall(II);
            CI->print(llvm::errs());
            llvm::outs() << "[Transfor]: Generated Call instruction:\n";

            // BI->insertAfter(CI);
            
            // 打印生成的Call指令
            

            llvm::outs() << "[Transfor]: \n";
            // 转为call指令处理
            handleCall(CI);
            
        }
        return true;
    }
    
    
    void handleExternalFCall(llvm::CallInst *inst);
    void handleCXXMemberCall(llvm::CallInst *inst);
    CXXFunCallInfo* get_existed_CXXCallInfo(CXXFunCallInfo* info);
public:
    _SymbolsTable get_st(){
        return st;
    }
    void insBBlabel(){
        for(auto &tib:toInsBB){
            auto label = tib.second;
            auto insaddr = tib.first;
            std::cout << "[Transfor]: label: " << label << " ";
            std::cout << "[Transfor]: insaddr: " << insaddr << "\n";
            auto addr = BBaddr[label];
            std::cout << "[Transfor]: addr: " << addr << "\n";
            uint8_t* little_end_addr = new uint8_t[8];
            for(auto i=0;i<8;i++){
                little_end_addr[i]=(addr>>(i*8))&0xFF;
            }
            std::memcpy(emitter.buffer.data()+insaddr,little_end_addr,8);
            
        }
    }
    FunCTX(llvm::DataLayout &DL) : DL(DL), stack_size(0) {}
    FunCTX(SymbolTable &GST, llvm::DataLayout &DL) : symbol_table(GST), DL(DL), stack_size(0) {}
    FunCTX(_SymbolsTable ST_, llvm::DataLayout &DL) : st(ST_), DL(DL), stack_size(0) {}
    SymbolTable &get_symbol_table_ref()
    {
        return symbol_table;
    }
    void setFunctionName(std::string name)
    {
        this->Fname = name;
    }
    void addBB(llvm::BasicBlock *bb)
    {
        BBS.push_back(bb);
    }
    void allocate()
    {
        handleFParams();
        for(auto &bb:BBS){
            for (auto &inst : *bb)
            {
                handleAlloca(&inst);
            }
        }
    }
    void emitAlloca()
    {
        // VMINST inst{.opcode = OpCode::_SUB, .num = OpNum::_64, .obj = OpObj::_Rsp, .obj_dst = OpObj::_Nop, .mode = OpMode::_Imm, .mode_dst = OpMode::_Nop};
        // emitter.emit(inst);
        // emitter.emit(static_cast<uint64_t>(stack_size));
        VMINST *src = new _MOV_IMM_RX_Inst(2, 3, stack_size);
        VMINST *dst = new _SUB_RSP_R2_Inst();
        emitter.emit(src);
        emitter.emit(dst);
    }
    void createVar();
    void assign()
    {
        for(auto &bb:BBS){
            for (auto &inst : *bb)
            {
                if (auto *St = llvm::dyn_cast<llvm::StoreInst>(&inst))
                {
                    if (auto OP1 = St->getOperand(0))
                    {
                        if (llvm::isa<llvm::Constant>(OP1))
                        {
                            auto Cst = llvm::dyn_cast<llvm::Constant>(OP1);
                            auto value = Cst->getUniqueInteger().getLimitedValue();
                            uint8_t *value_t = nullptr;
    
                            std::string instNameWithAddr;
                            llvm::raw_string_ostream OS(instNameWithAddr);
                            St->getOperand(1)->printAsOperand(OS, false);
                            OS.flush();
                            llvm::outs() << "[Transfor]: Store value: " << value << "\n";
                            llvm::outs() << "[Transfor]: Store to: " << instNameWithAddr << "\n";
                            // symbol_table.change_variable(instNameWithAddr, value_t);
                            symbol_table.dump();
                        }
                    }
                }
            }
        }
    }
    bool insertCXXCall(CXXFunCallInfo* info,std::string);
    void addParams(llvm::Argument *arg)
    {
        this->args.push(arg);
    }
    void handleFParams();
    void InitFCTXorder();

    void dump()
    {
        symbol_table.dump();
        emitter.dump();
    }

    auto get_jump_table()
    {
        return jump_tables;
    }

    ~FunCTX()
    {
    }
};

class ExternalCollector
{
    friend class FunCTX;
private:
    std::string ProtectedName="main";
    _SymbolsTable st;
    std::set<std::string> CXXMemberFunctions;
    std::set<std::string> CXXConstructors;
    std::set<std::string> target_vars;
    std::set<std::string> target_functions;
    std::set<std::string> User_Function;
    std::unordered_map<std::string,std::string> name_key;
    InsEmitter emitter;
    constexpr const static char *jsonFile = "/home/ljz/XXXVMP/XXXClang/build/XXXlib.json";
    SymbolTable symbol_table;
    // struct pair_hash {
    //     template <class T1, class T2>
    //     std::size_t operator() (const std::pair<T1, T2> &pair) const {
    //         return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    //     }
    // };
    struct GlobalVar
    {
        std::string mangled_name;
        std::string demangled_name;
        size_t size = -1;
        uint64_t address;
    };
    std::vector<GlobalVar> external_global_vars;
    struct ExternalFunctionSignature
    {
        std::string return_type;
        std::string mangled_name;
        std::string demangled_name;
        std::string args;
        uint64_t address = 0;
    };
    std::vector<ExternalFunctionSignature> external_functions;
    std::string demangle(const std::string &name)
    {
        char *demangled = llvm::itaniumDemangle(name.c_str());
        if (!demangled)
            return name;
        std::string result(demangled);
        std::free(demangled);
        return result;
    }

public:
    std::string getPName(){
        return ProtectedName;
    }
    _SymbolsTable get_st(){
        return st;
    }
    ExternalCollector(SymbolTable &symbol_table) : symbol_table(symbol_table)
    {
        std::ifstream jsonFile("/home/ljz/XXXVMP/XXXClang/build/XXXlib.json");
        nlohmann::json json;
        jsonFile >> json;
        for (auto &var : json["Var"])
        {
            target_vars.insert(var["mangledName"].dump().substr(1, var["mangledName"].dump().size() - 2));
            name_key[var["mangledName"].dump().substr(1, var["mangledName"].dump().size() - 2)]=var["key"].dump().substr(1, var["key"].dump().size() - 2);
        }
        for (auto &func : json["Func"])
        {
            target_functions.insert(func["mangledName"].dump().substr(1, func["mangledName"].dump().size() - 2));
            if(func["isCXXConstructor"]=="true"){
                CXXMemberFunctions.insert(func["mangledName"].dump().substr(1, func["mangledName"].dump().size() - 2));
                CXXConstructors.insert(func["mangledName"].dump().substr(1, func["mangledName"].dump().size() - 2));
                llvm::outs() << "[Transfor]: CXXMemberFunctions:" << func["mangledName"].dump().substr(1, func["mangledName"].dump().size() - 2) << "\n";
            }
            if (func["isCXXClassMember"] == "true")
            {
                CXXMemberFunctions.insert(func["mangledName"].dump().substr(1, func["mangledName"].dump().size() - 2));
                llvm::outs() << "[Transfor]: CXXMemberFunctions:" << func["mangledName"].dump().substr(1, func["mangledName"].dump().size() - 2) << "\n";
            }
            name_key[func["mangledName"].dump().substr(1, func["mangledName"].dump().size() - 2)]=func["key"].dump().substr(1, func["key"].dump().size() - 2);
        }
        for(auto &func : json["UserFunc"]){
            if(func["name"]!="main"){
                User_Function.insert(func["mangledName"].dump().substr(1, func["mangledName"].dump().size() - 2));
            }else{
                User_Function.insert("main");
            }
        }
        for(auto &tf:json["Native"]){
            ProtectedName=tf["mangleName"].dump().substr(1, tf["mangleName"].dump().size() - 2);
        }
    }
    void set_st(_SymbolsTable st_){
        st=st_;
    }
    bool isUserFunction(std::string func_name){
        if(User_Function.find(func_name)==User_Function.end()){
            return false;
        }
        return true;
    }
    bool is_v_external(std::string var_name)
    {
        if (target_vars.find(var_name) == target_vars.end())
        {
            return false;
        }
        return true;
    }
    bool is_f_external(std::string func_name)
    {
        if (target_functions.find(func_name) == target_functions.end())
        {
            return false;
        }
        return true;
    }
    void set_symbol_table(SymbolTable &symbol_table)
    {
        this->symbol_table = symbol_table;
    }
    void add_external_global_var(std::string var_name)
    {
        if (target_vars.find(var_name) == target_vars.end())
        {
            return;
        }
        auto demangled = demangle(var_name);
        GlobalVar var;
        var.mangled_name = var_name;
        var.demangled_name = demangled;
        var.size = -1;
        var.address = -1;
        external_global_vars.push_back(var);
    }
    bool isCXXConstructor(std::string mangled_name)
    {
        if (CXXConstructors.find(mangled_name) == CXXConstructors.end())
        {
            return false;
        }
        return true;
    }
    // bool isCXXDestructor(std::string mangled_name)
    // {
    //     if (CXXConstructors.find(mangled_name) == CXXConstructors.end())
    //     {
    //         return false;
    //     }
    //     return true;
    // }
    bool isCXXMemberFunction(std::string mangled_name)
    {
        if (CXXMemberFunctions.find(mangled_name) == CXXMemberFunctions.end())
        {
            return false;
        }
        return true;
    }
    ExternalFunctionSignature analyze_function(std::string mangled_name)
    {
        auto demangled = demangle(mangled_name);
        llvm::outs() << "[Transfor]: Demangled name: " << demangled << "\n";
        auto space_pos = demangled.find(" ");
        auto return_type = demangled.substr(0, space_pos);
        auto name_pos = demangled.find("(");
        auto name = demangled.substr(space_pos + 1, name_pos - space_pos - 1);
        auto args = demangled.substr(name_pos + 1, demangled.size() - name_pos - 2);
        return {.return_type = return_type, .mangled_name = mangled_name, .demangled_name = name, .args = args};
    }
    void add_external_function(ExternalFunctionSignature signature)
    {
        if (target_functions.find(signature.mangled_name) == target_functions.end())
        {
            return;
        }
        external_functions.push_back(signature);
    }
    void dump()
    {
        llvm::outs() << "[Transfor]: External Global Variables:\n";
        for (auto var_name : external_global_vars)
        {
            llvm::outs() << var_name.mangled_name << " == " << var_name.demangled_name << "\n";
        }
        llvm::outs() << "[Transfor]: External Functions:\n";
        for (auto signature : external_functions)
        {
            llvm::outs() << signature.mangled_name << " == ";
            llvm::outs() << signature.return_type << " " << signature.demangled_name << "(" << signature.args << ")\n";
        }
    }

    void record_func_to_mem()
    {
        for (auto &signature : external_functions)
        {
            llvm::outs() << "[Transfor]: Record " << signature.mangled_name << " to memory\n";
            auto pdescriptor = std::make_shared<PrimitiveType>(8, 8);
            auto var = std::make_shared<Variable>(pdescriptor, reinterpret_cast<uint8_t *>(new uint64_t(0)));
            symbol_table.register_variable_to_mem(signature.mangled_name, var);
            auto addr = symbol_table.get_variable_record(signature.mangled_name)->base_addr;
            llvm::outs() << "[Transfor]: Address: " << addr << "\n";
            signature.address = addr;
        }
    }

    void record_var_to_mem(GlobalVar &var_name, uint8_t *data)
    {

        llvm::outs() << "[Transfor]: Record " << var_name.mangled_name << " to memory\n";
        auto pdescriptor = std::make_shared<PrimitiveType>(var_name.size, 8);
        auto var = std::make_shared<Variable>(pdescriptor, data);
        symbol_table.register_variable_to_mem(var_name.mangled_name, var);
        auto addr = symbol_table.get_variable_record(var_name.mangled_name)->base_addr;
        llvm::outs() << "[Transfor]: Address: " << addr << "\n";
        var_name.address = addr;
    }

    void write_as_json()
    {
        std::ifstream file(jsonFile);
        if (!file.is_open())
        {
            llvm::outs() << "[Transfor]: Failed to open file: " << jsonFile << "\n";
            return;
        }
        nlohmann::ordered_json json;
        file >> json;
        file.close();
        for (auto var_name : external_global_vars)
        {
            for (auto &item : json["Var"])
            {
                if (item["mangledName"] == var_name.mangled_name)
                {
                    item["address"] = var_name.address;
                    break;
                }
            }
        }
        for (auto signature : external_functions)
        {
            for (auto &item : json["Func"])
            {
                if (item["mangledName"] == signature.mangled_name)
                {
                    item["address"] = signature.address;
                    break;
                }
            }
        }
        std::ofstream file2(jsonFile);
        if (!file2.is_open())
        {
            llvm::outs() << "[Transfor]: Failed to open file: " << jsonFile << "\n";
            return;
        }
        file2 << json.dump(4);
        file2.close();
    }

    void change_variable(const std::string &name, const uint8_t *data)
    {
        symbol_table.change_variable(name, data);
    }

    void handleInitGV(llvm::GlobalVariable &G)
    {
        for (auto &item : external_global_vars)
        {
            if (item.mangled_name == G.getName().str())
            {
                return;
            }
        }
        int size_ =0;
        if (G.hasInitializer() && !G.use_empty())
        {
            if(G.getSection() == "llvm.metadata"||G.getName().str().find("InterFace")!= std::string::npos||G.getName().str().find("llvm.")!= std::string::npos){
                llvm::outs() << "[Transfor]: llvm.metadata\n";
                return;
            }
            if(globalUsed.find(G.getName().str())==globalUsed.end()){
                llvm::outs() << "[Transfor]: Global not used\n";
                return;
            }
            llvm::outs() << "[Transfor]: handle global variable: " << G.getName() << "\n";
            uint8_t *value = new uint8_t;
            *value = 0;
            if (auto *CFP = llvm::dyn_cast<llvm::ConstantFP>(G.getInitializer()))
            {
                llvm::APFloat Val = CFP->getValueAPF();
                llvm::outs() << "[Transfor]: value: " << Val.convertToFloat() << "\n";
                // 转换为标准浮点类型
                if (Val.getSizeInBits(Val.getSemantics()) == 32)
                {
                    llvm::APInt intBits = Val.bitcastToAPInt();
                    std::memcpy(reinterpret_cast<uint8_t *>(value), reinterpret_cast<const uint8_t *>(intBits.getRawData()), 4);
                    std::cout << "[Transfor]: value: " << *value << "\n";
                    // 处理32位float
                    size_ = 4;
                }
                else
                {
                    llvm::outs() << "[Transfor]: value: " << Val.convertToDouble() << "\n";
                    llvm::APInt intBits = Val.bitcastToAPInt();
                    std::memcpy(reinterpret_cast<uint8_t *>(value), reinterpret_cast<const uint8_t *>(intBits.getRawData()), 8);
                    std::cout << "[Transfor]: value: " << *value << "\n";
                    // 处理64位double
                    size_ = 8;
                }
            }
            else if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(G.getInitializer()))
            {
                llvm::outs() << "[Transfor]: value: " << CI->getSExtValue() << "\n";
                // 处理整数常量
                uint64_t IntVal = CI->getSExtValue();
                std::memcpy(reinterpret_cast<uint8_t *>(value), reinterpret_cast<uint8_t *>(&IntVal), 8);
                std::cout << "[Transfor]: value: " << *value << "\n";
                size_ = 8;
            }
            else if (auto *CA = llvm::dyn_cast<llvm::ConstantArray>(G.getInitializer()))
            {
                llvm::outs() << "[Transfor]: value: " << *CA << "\n";
                std::cout << "[Transfor]: Unsupported global variable type:ConstantArray\n";
                return;
            }
            else if (auto *CS = llvm::dyn_cast<llvm::ConstantStruct>(G.getInitializer()))
            {
                llvm::outs() << "[Transfor]: value: " << *CS << "\n";
                std::cout << "[Transfor]: Unsupported global variable type:ConstantStruct\n";
                return;
            }
            else if (auto *CV = llvm::dyn_cast<llvm::ConstantVector>(G.getInitializer()))
            {
                llvm::outs() << "[Transfor]: value: " << *CV << "\n";
                std::cout << "[Transfor]: Unsupported global variable type:ConstantVector\n";
                return;
            }
            else if (auto *CU = llvm::dyn_cast<llvm::UndefValue>(G.getInitializer()))
            {
                llvm::outs() << "[Transfor]: value: " << *CU << "\n";
                std::cout << "[Transfor]: Unsupported global variable type:UndefValue\n";
                return;
            }
            else if (auto *CV = llvm::dyn_cast<llvm::ConstantPointerNull>(G.getInitializer()))
            {
                llvm::outs() << "[Transfor]: value: " << *CV << "\n";
                std::cout << "[Transfor]: Unsupported global variable type:ConstantPointerNull\n";
                return;
            }
            else if (auto *CV = llvm::dyn_cast<llvm::ConstantAggregateZero>(G.getInitializer()))
            {
                llvm::outs() << "[Transfor]: value: " << *CV << "\n";
                std::cout << "[Transfor]: Unsupported global variable type:ConstantAggregateZero\n";
                return;
            }
            else if (auto *CV = llvm::dyn_cast<llvm::ConstantDataArray>(G.getInitializer()))
            {
                llvm::outs() << "[Transfor]: value: " << *CV << "\n";
                std::cout << "[Transfor]: global variable type:ConstantDataArray\n";
                auto string = CV->getAsCString();
                auto size = CV->getNumElements() * CV->getElementType()->getPrimitiveSizeInBits() / 8;
                size_ = size;
                std::memcpy(reinterpret_cast<uint8_t *>(value), reinterpret_cast<const uint8_t *>(string.data()), size);
                
                llvm::outs() << "[Transfor]: size: " << size << "\n";
                llvm::outs() << "[Transfor]: value: " << string << "\n";
            }
            else if (auto *CV = llvm::dyn_cast<llvm::ConstantDataVector>(G.getInitializer()))
            {
                llvm::outs() << "[Transfor]: value: " << *CV << "\n";
                std::cout << "[Transfor]: Unsupported global variable type:ConstantDataVector\n";
            }
            else if (auto *CV = llvm::dyn_cast<llvm::ConstantDataSequential>(G.getInitializer()))
            {
                llvm::outs() << "[Transfor]: value: " << *CV << "\n";
                std::cout << "[Transfor]: Unsupported global variable type:ConstantDataSequential\n";
                return;
            }
            else
            {
                llvm::outs() << G.getInitializer()->getValueID() << " " << " " << *G.getInitializer() << "\n";
                std::cout << "[Transfor]: Unsupported global variable type\n";
                return;
            }
            if (size_ != -1)
            {
                auto size = size_;
                // auto pdescriptor = std::make_shared<PrimitiveType>(size, 8);
                // auto Var = std::make_shared<Variable>(pdescriptor, value);
                // symbol_table.register_variable_to_mem(G.getName().str(), Var);
                // auto record = symbol_table.get_variable_record(G.getName().str());
                for(auto s:st){
                    std::cout << "[Transfor]: \tSymbol: "  << s.second.second.first << "  description: " << s.second.first << "  Size: " << ((s.second.second.second.size==std::nullopt)?"unkown":std::to_string(s.second.second.second.size.value())) << "  align: " << ((s.second.second.second.align==std::nullopt)?"unkown":std::to_string(s.second.second.second.align.value())) << "  addr: " << s.first << "  type: " << s.second.second.second.Type_name << std::endl;
                }
                llvm::outs() << "[Transfor]: global variable: " << G.getName().str() << " size: " << size_ << "\n";
                int64_t addr;
                try{
                    addr = getSymbol(ReturnType::addr,G.getName().str());
                }catch (std::exception &e){
                    llvm::outs() << "[Transfor]: getSymbol error: " << e.what() << "\n";
                    return;
                }catch (...) {
                    llvm::outs() << "[Transfor]: Unknown exception caught\n";
                    abort();
                    return;
                }
                auto per = size / 8;
                auto rem = size % 8;
                auto offset = 0;
                auto rs = 0;
                if (per != 0)
                {
                    for (auto i = 0; i < per; i++)
                    {
                        uint64_t _data = 0;
                        std::memcpy(reinterpret_cast<uint8_t *>(&_data), value + offset, 8);
                        VMINST *MIRI = new _MOV_IMM_RX_Inst(1, 3, _data);
                        VMINST *MRMI = new _MOV_RX_MEM_Inst(addr + offset, 1, 3);
                        emitter.emit(MIRI);
                        emitter.emit(MRMI);
                        offset += 8;
                    }
                }
                if (rem >= 4)
                {
                    uint64_t _data = 0;
                    std::memcpy(reinterpret_cast<uint8_t *>(&_data), value + offset, 4);
                    VMINST *MIRI = new _MOV_IMM_RX_Inst(1, 2, _data);
                    VMINST *MRMI = new _MOV_RX_MEM_Inst(addr + offset, 1, 2);
                    emitter.emit(MIRI);
                    emitter.emit(MRMI);
                    offset += 4;
                    rem -= 4;
                }
                if (rem >= 2)
                {
                    uint64_t _data = 0;
                    std::memcpy(reinterpret_cast<uint8_t *>(&_data), value + offset, 2);
                    VMINST *MIRI = new _MOV_IMM_RX_Inst(1, 1, _data);
                    VMINST *MRMI = new _MOV_RX_MEM_Inst(addr + offset, 1, 1);
                    emitter.emit(MIRI);
                    emitter.emit(MRMI);
                    offset += 2;
                    rem -= 2;
                }
                if (rem >= 1)
                {
                    uint64_t _data = 0;
                    std::memcpy(reinterpret_cast<uint8_t *>(&_data), value + offset, 1);
                    VMINST *MIRI = new _MOV_IMM_RX_Inst(1, 0, _data);
                    VMINST *MRMI = new _MOV_RX_MEM_Inst(addr + offset, 1, 0);
                    emitter.emit(MIRI);
                    emitter.emit(MRMI);
                    offset += 1;
                    rem -= 1;
                }
            }

            llvm::outs() << "[Transfor]: global variable: " << G.getName().str() << " size: " << size_ << "\n";
        }
    }
    void dump_target_functions()
    {
        for (auto &func : target_functions)
        {
            llvm::outs() << func << "\n";
        }
    }
    void dump_bin(){
        const char* initGloabl = "__init_global.bin";
        std::ofstream file(initGloabl, std::ios::binary);
        std::vector<uint8_t> data = emitter.get_buffer();
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
        llvm::outs() << "[Transfor]: successfully dump initGloabl.bin\n";
    }

    std::string get_key(std::string name){
        return name_key[name];
    }
};

class FCTXOrder
{
private:
    std::once_flag init_flag;
    std::unordered_map<std::string, std::shared_ptr<FunCTX>> fctxs;
    std::vector<std::string> fctxs_order;
    std::string main = "main";


public:
    FCTXOrder()
    {
        main=ExCollector.getPName();
    }

    void add_fctx(std::string name, std::shared_ptr<FunCTX> fctx)
    {
        llvm::outs() << "[Transfor]: Add FCTX: " << name << "\n";
        if (fctx == nullptr)
        {
            llvm::outs() << "[Transfor]: FCTX is nullptr\n";
            return;
        }
        fctxs[name] = fctx;
        llvm::outs() << "[Transfor]: Add FCTX: " << name << " success\n";
    }
    void add_fctx_order(std::string name)
    {
        if (ExCollector.is_f_external(name)&&!ExCollector.isUserFunction(name))
        {
            llvm::outs() << "[Transfor]: External Function: " << name << " SKIP \n";
            return;
        }
        llvm::outs() << "[Transfor]: Add FCTX order: " << name << "\n";
        fctxs_order.push_back(name);
    }
    void init_fctx_order()
    {
        fctxs_order.push_back(main);
        if (fctxs.find(main) == fctxs.end())
        {
            llvm::outs() << "[Transfor]: Main function not found\n";
            return;
        }
        else
        {
            fctxs[main]->InitFCTXorder();
        }
    }
    void build();
};

class CXXFunCallInfo
{
    friend class GenStubMap;
private:
    const char *jsonFile = "/home/ljz/XXXVMP/XXXClang/build/XXXlib.json";
    const char *stubFile = "/home/ljz/XXXVMP/XXXClang/build/XXXstub.h";
    bool isCXXConstructor_m = false;                                            //判断是否是构造函数
    std::unordered_map<int, std::string> params;                                //收集实参与它在参数列表中的顺序（从左往右）
    std::unordered_map<int, std::string> otherCXXCall_result;                   //参数是否是其他外部函数调用的返回值，如果是则会被加入到这个map中
    std::unordered_map<std::string,int> order_inst_map;                         //收集实参与它在参数列表中的顺序（从左往右），存入map中
    std::unordered_map<int,std::pair<std::string,int>> other_order_inst_map;    //收集使用到的其他CXXFunCallInfo实例的order_inst_map存入到这个map中
    std::set<int> order;
    int param_nums = 0;
    std::string mangled_name;
    std::string signature;
    std::string key_scope;
    std::set<std::string> instNameWithAddr;
    std::string return_;
    std::string result;
    bool ret_is_ref = false;
    std::unordered_map<int, std::string> gen_params;
    std::vector<std::string> gen_params_stub;
    bool isVoidReturn = false;
public:

    CXXFunCallInfo(std::string mangled_name,std::string signature)
    {
        this->mangled_name = mangled_name;
        this->signature = signature;
    }
    void setCXXConstructor(bool isCXXConstructor)
    {
        this->isCXXConstructor_m = isCXXConstructor;
    }
    bool isCXXConstructor()
    {
        return isCXXConstructor_m;
    }
    bool isVoidReturnFunc()
    {
        return isVoidReturn;
    }
    void setVoidReturnFunc(bool isVoidReturn)
    {
        this->isVoidReturn = isVoidReturn;
    }
    std::string get_mangled_name()
    {
        return mangled_name;
    }
    void set_key_scope(std::string key_scope)
    {
        this->key_scope = key_scope;
    }
    void set_instNameWithAddr(std::string instNameWithAddr)
    {
        this->instNameWithAddr.insert(instNameWithAddr);
    }
    std::string get_sig(){
        return signature;
    }
    int get_order_from_inst(std::string n){
        return order_inst_map[n];
    }
    void add_order_inst_map(std::string n,int order){
        order_inst_map[n] = order;
    }
    std::string get_key_scope()
    {
        return key_scope;
    }
    void add_other_oi_map(std::string n,int order,int this_order){
        other_order_inst_map.insert({this_order,std::make_pair(n,order)});
    }
    std::unordered_map<std::string,int> get_order_inst_map(){
        return order_inst_map;
    }
    std::set<std::string> get_instNameWithAddr()
    {
        return instNameWithAddr;
    }
    std::string get_return()
    {
        return return_;
    }
    std::string get_gen_params(int order)
    {
        return gen_params[order];
    }
    std::string get_gen_params_stub(int order)
    {
        return gen_params_stub[order];
    }
    int get_param_nums()
    {
        return param_nums - order.size();
    }
    void add_param(int order, std::string param)
    {
        params.insert({order, param});
        this->order.insert(order);
    }
    void add_otherCXXCall_result(int order, std::string result)
    {
        otherCXXCall_result.insert({order, result});
        this->order.insert(order);
    }
    void buildCXXConstructor(std::string clas_key_scope);
    void build();
    void buildCXXMemberFunction(std::string clas_key_scope);
    
};

class GenStubMap{
    private:
    inline static std::set<CXXFunCallInfo*> cxx_fun_call_infos;
    static constexpr char  stubFile[] = "/home/ljz/XXXVMP/XXXClang/build/XXXstub.h";
    public:
    static void gen(){
        std::fstream stub(stubFile, std::ios::app);
        if (!stub.is_open())
        {
            llvm::outs() << "[Transfor]: Failed to open file: " << stubFile << "\n";
            return;
        }
        stub << "\n";
        stub << "std::unordered_map<std::string, void*> var_map = {\n";
        for(auto it:cxx_fun_call_infos){
            for(int i=0;i<it->gen_params_stub.size();i++){
                if(it->gen_params_stub[i].ends_with(std::string(" * p")+std::to_string(i))){
                    stub << "\t{\"" << it->key_scope + "_::p" + std::to_string(i) + "\", " << it->key_scope << "_::p" + std::to_string(i) << "},\n";
                }
                else{
                    stub << "\t{\"" << it->key_scope + "_::p" + std::to_string(i) + "\", " << "&"<< it->key_scope << "_::p" + std::to_string(i) << "},\n";
                }
            }
            if(it->isCXXConstructor()){
                stub << "\t{\"" << it->key_scope + "_::addr"+"\", "+ it->key_scope + "_::addr" << "},\n";
            }
            if(!it->isVoidReturnFunc()){
                stub << "\t{\"" << it->key_scope + "_::res" + "\", reinterpret_cast<void*>(" << it->key_scope << "_::res)" << "},\n";
            }
        }
        stub << "};\n";
        stub << "std::unordered_map<std::string, std::function<void()>> func_map = {\n";
        for(auto it:cxx_fun_call_infos){
            stub << "\t{\"" << it->key_scope << "\", " << it->key_scope << "_::" << it->key_scope << "_instance},\n";
        }

        stub << "};\n";
        stub.close();
    }

    static void add_cxx_fun_call_info(CXXFunCallInfo* cxx_fun_call_info){
        for(auto it:cxx_fun_call_infos){
            if(it->key_scope == cxx_fun_call_info->key_scope){
                return;
            }
        }
        cxx_fun_call_infos.insert(cxx_fun_call_info);
    }
};
