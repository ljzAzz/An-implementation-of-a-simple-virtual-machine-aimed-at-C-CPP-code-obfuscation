#pragma once
#include <iostream>
#include <vector>
#include <stack>
#include <iomanip>
#include <unordered_map>
#include <fstream>
#include <string>
#include <bitset>
#include <cstring>
#include <memory>
#include <optional>
#include "/home/ljz/XXXVMP/Vmp/XXXInst.hpp"
#include "/home/ljz/XXXVMP/XXXClang/build/XXXstub.h"
#include <cmath>
#include <limits>
#include "AntiDebug.h"
#include "FakeBranch.hpp"
// #include "/home/ljz/XXXVMP/ELF/ELF_D.hpp"


#ifdef _DEBUG_ 
#define DEBUG_PRINT(x) std::cout << x << std::endl;
#else
#define DEBUG_PRINT(x)
#endif

#ifdef DBG
#define DBG_PRINT(x) x
#else
#define DBG_PRINT(x)
#endif


// 外部符号声明
extern "C" {
    extern const unsigned char _binary__tmp_next_start[];
    [[maybe_unused]]extern const unsigned char _binary__tmp_next_end[];
    extern const unsigned char _binary__tmp_global_start[];
    [[maybe_unused]]extern const unsigned char _binary__tmp_global_end[];
    extern const size_t _binary__tmp_next_size;
    extern const size_t _binary__tmp_global_size;
}
void load_section(uint8_t* data, size_t size, const unsigned char* start) {
    std::cout << "Section  size: " << size << " bytes\n";
    memcpy(data, start, size); 
}

class Memory
{
private:
    std::vector<uint8_t> memory;

public:
    Memory()
    {
        memory.resize(1024 * 1024);
    };
    void write(uint32_t addr, uint8_t *data, size_t size)
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
    uint8_t *getaddr(uint64_t addr)
    {
        return &memory[addr];
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
    void dump()
    {
        std::cout << "Memory dump:\n";
        // 十六进制打印地址以及数据
        for (size_t i = 0; i < 80; i += 16)
        {
            std::cout << "0x"<< std::hex << std::setfill('0') << std::setw(8) <<std::uppercase<< i << ": ";
            for (size_t j = 0; j < 16; ++j)
            {
                if (i + j < 80)
                {
                    std::cout << "0x"<< std::hex << std::setfill('0') << std::setw(2)<<std::uppercase << static_cast<int>(memory[i + j]) << " ";
                }
                else
                {
                    std::cout << "   ";
                }
            }
            std::cout <<std::nouppercase<< std::dec << std::endl;
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
            throw std::out_of_range("Stack overflow");
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
            std::cout << "rsp is " << rsp << " size is " << sizeof(T) << std::endl;
            throw std::underflow_error("Stack underflow");
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
            throw std::out_of_range("Stack write out of bounds");
        }

        std::memcpy(&stack[rbp - offset], data, size);
    }
    uint8_t *read(int64_t offset, size_t size)
    {
        // std::cout<<"[VMCORE]: rbp is "<<rbp<<" offset is "<<offset<<" size is "<<size<<std::endl;
        if (rbp - offset > stack.size())
        {
            throw std::out_of_range("Stack read out of bounds");
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
    uint8_t* getaddr(int64_t offset)
    {
        return &stack[rbp - offset];
    }
    void dump()
    {
        std::cout << "Stack dump:" << std::endl;
        // 从栈顶开始十六进制输出地址以及数据
        for (size_t i = 4000; i < stack.size(); i += 16)
        {
            std::cout <<"0x"<< std::hex << std::setfill('0') << std::setw(8) <<std::uppercase<< i << ": ";
            for (size_t j = 0; j < 16; ++j)
            {
                if (i + j < stack.size())
                {
                    std::cout << "0x"<<std::hex << std::setfill('0') << std::setw(2) <<std::uppercase<< static_cast<int>(stack[i + j]) << " ";
                }
                else
                {
                    std::cout << "   ";
                }
            }
            std::cout <<std::nouppercase<< std::dec << std::endl;
        }
    }
};

class XXXVMPCore
{
    friend class InterFace;
private:

    Memory mem;
    Stack stack;
    uint64_t R[8]={0};
    uint64_t PC;
    bool halt = false;
    int init_size = 0;
    std::optional<bool> cmp_flag = false;
    void* ret;

public:
    void Init();
    XXXVMPCore()
    {
        PC = 0;
    }
    template<typename T>
    bool almost_equal_combined(T a, T b, 
                              T rel_epsilon = std::numeric_limits<T>::epsilon() * 100,
                              T abs_epsilon = std::numeric_limits<T>::min() * 100) {
        if (a == b) return true;
                            
        T diff = std::fabs(a - b);
                            
        if (a == 0 || b == 0 || diff < abs_epsilon)
            return diff < abs_epsilon;
                            
        T abs_a = std::fabs(a);
        T abs_b = std::fabs(b);
        return diff <= ((abs_a < abs_b ? abs_b : abs_a) * rel_epsilon);
    }

    template<typename T, typename IntT>
    bool compare_float(IntT imm_val, IntT reg_val) {
        static_assert(sizeof(T) == sizeof(IntT), "Type size mismatch");
        
        union {
            IntT i;
            T f;
        } imm_conv, src_conv;
        
        imm_conv.i = imm_val;
        src_conv.i = reg_val;
        
        // std::cout << "src_value is " << src_conv.f << std::endl;
        // std::cout << "imm is " << imm_conv.f << std::endl;
        
        return almost_equal_combined<T>(src_conv.f, imm_conv.f);
    }

    void load_program()
    {
        // std::fstream file(program, std::ios::in | std::ios::binary);
        // if (!file.is_open())
        // {
        //     std::cerr << "Failed to open file: " << program << std::endl;
        //     return;
        // }
        // file.seekg(0, std::ios::end);
        // size_t size = file.tellg();
        // file.seekg(0, std::ios::beg);
        size_t size = _binary__tmp_next_end - _binary__tmp_next_start;

        uint8_t *data = new uint8_t[size];
        load_section(data, size, _binary__tmp_next_start);
        // file.read(reinterpret_cast<char *>(data), size);
        for(auto i = 0; i < size; i++){
            std::cout << "0x"<< std::hex << std::setfill('0') << std::setw(2) <<std::uppercase<< static_cast<int>(data[i]) << " ";
        }
        std::cout << std::nouppercase<<std::dec << std::endl;
        auto mem_size = mem.size();
        mem.resize(mem_size + size);
        mem.write(mem_size, data, size);
        //file.close();
    }
    void handle_PUSH_R1_Inst(_PUSH_R1_Inst& inst){
        stack.push(reinterpret_cast<uint8_t *>(&R[1]), sizeof(R[1]));
        DEBUG_PRINT(R[1]);
    }
    void handle_POP_R2_Inst(_POP_R2_Inst& inst){
        R[2] = stack.pop<uint64_t>();
        DEBUG_PRINT(R[2]);
    }
    void handle_ADD_RSP_R2_Inst(_ADD_RSP_R2_Inst& inst){
        stack.set_rsp(stack.get_rsp() + R[2]);
        DEBUG_PRINT(R[2]);
    }
    void handle_PUSH_RX_WITH_SIZE_Inst(_PUSH_RX_WITH_SIZE_Inst& inst){
        auto dst = int(inst.dst);
        auto size = inst.size;
        auto rs = 0;
        switch(size)
        {
            case 0:
                rs = 1;
                break;
            case 1:
                rs = 2;
                break;
            case 2:
                rs = 4;
                break;
            case 3:
                rs = 8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        stack.push(reinterpret_cast<uint8_t *>(&R[dst]), rs);
    }
    void  handle_SUB_RSP_R2_Inst(_SUB_RSP_R2_Inst& inst){
        stack.set_rsp(stack.get_rsp() - R[2]);
        DEBUG_PRINT(R[2]);
    }
    void handle_PUSH_PC_Inst(_PUSH_PC_Inst& inst){
        stack.push(reinterpret_cast<uint8_t *>(&PC), sizeof(PC));
        DEBUG_PRINT(PC);
    }
    void handle_POP_PC_Inst(_POP_PC_Inst& inst){
        PC = stack.pop<uint64_t>();
        PC += 10;
        DEBUG_PRINT(PC);
    }
    void handle_LOAD_FROM_EXTERNAL_Inst(_LOAD_FROM_EXTERNAL_Inst& inst){
        uint64_t addr = inst.offset;
        uint64_t dst = inst.addr;
        uint8_t _size = inst.size;
        int size = 0;
        switch(_size)
        {
            case 0:
                size = 1;
                break;
            case 1:
                size = 2;
                break;
            case 2:
                size = 4;
                break;
            case 3:
                size = 8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        char arr[8];

        arr[0] = addr & 0xFF;
        arr[1] = (addr >> 8)  & 0xFF;
        arr[2] = (addr >> 16) & 0xFF;
        arr[3] = (addr >> 24) & 0xFF;
        arr[4] = (addr >> 32) & 0xFF;
        arr[5] = (addr >> 40) & 0xFF; 
        arr[6] = (addr >> 48) & 0xFF;
        arr[7] = (addr >> 56) & 0xFF;
        std::string cxxstr = std::string(arr);
        DBG_PRINT(std::cout << "load external :" << cxxstr << std::endl);
        uint8_t *data =  new uint8_t[size];
        if(var_map.find(cxxstr+"_::res") == var_map.end()){
            DBG_PRINT(std::cout << "Error: " << cxxstr+"_::res" << " not found in var_map" << std::endl);
            abort();
        }
        DBG_PRINT(std::cout << "value :" << *reinterpret_cast<int*>(var_map[cxxstr+"_::res"]) << std::endl);
        std::memcpy(data, reinterpret_cast<void*>(var_map[cxxstr+"_::res"]), size);
        mem.write(dst, data, size);
    }
    void handle_MOV_RBP_RSP_Inst(_MOV_RBP_RSP_Inst& inst){
        stack.set_rsp(stack.get_rbp());
        DEBUG_PRINT(stack.get_rbp());
    }
    void handle_PUSH_RBP_Inst(_PUSH_RBP_Inst& Inst){
        uint64_t old_rbp = stack.get_rbp();
        // std::cout << "old_rbp: " << old_rbp << std::endl;
        stack.push(reinterpret_cast<uint8_t *>(&old_rbp), 8);
        DEBUG_PRINT(stack.get_rbp());
    }
    void handle_POP_RBP_Inst(_POP_RBP_Inst& Inst){
        stack.set_rbp(stack.pop<uintptr_t>());
        DEBUG_PRINT(stack.get_rbp());
    }
    void handle_PUSH_RSP_Inst(_PUSH_RSP_Inst& Inst){
        stack.push(reinterpret_cast<uint8_t *>(stack.get_rsp()), sizeof(stack.get_rsp()));
        DEBUG_PRINT(stack.get_rsp());
    }
    void handle_POP_RSP_Inst(_POP_RSP_Inst& Inst){
        stack.set_rsp(stack.pop<uintptr_t>());
        DEBUG_PRINT(stack.get_rsp());
    }
    void handle_MOV_RSP_RBP_Inst(_MOV_RSP_RBP_Inst& Inst){
        stack.set_rbp(stack.get_rsp());
        DEBUG_PRINT(stack.get_rbp());
    }
    void handle_MOV_RX_RY_Inst(_MOV_RX_RY_Inst& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);
        R[dst]=R[src];
        DEBUG_PRINT(R[src]);
        DEBUG_PRINT(R[dst]);
    }
    void handle_MOV_IMM_RX_Inst(_MOV_IMM_RX_Inst& inst){
        auto dst=static_cast<int>(inst.dst);
        R[dst]=inst.imm;
        DEBUG_PRINT(inst.imm);
        DEBUG_PRINT(R[dst]);
    }
    void handle_MOV_STACK_STACK_Inst(_MOV_STACK_STACK_Inst& inst){
        auto src=static_cast<int16_t>(inst.src);
        auto dst=static_cast<int16_t>(inst.dst);
        bool push_or_write = inst.push_or_write;
        int size=4;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:
                size=4;
                break;
            case 3:
                size=8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }


        auto data=stack.read(src-size, size);
        if(push_or_write){
            stack.write(dst, data, size);

        }else{
            stack.push(data, size);
        }
    }
    void handle_ADD_IMM_RX_Inst(_ADD_IMM_RX_Inst& inst){
        auto dst=static_cast<int>(inst.dst);
        auto imm=inst.imm;
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                R[dst]+=inst.imm;
                break;
            case 1:
                size=2;
                R[dst]+=inst.imm;
                break;
            case 2:
                size=4;
                R[dst]+=inst.imm;
                break;
            case 3:
                size=8;
                R[dst]+=inst.imm;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        DEBUG_PRINT(inst.imm);
        DEBUG_PRINT(R[dst]);

    }
    void handle_SUB_IMM_RX_Inst(_SUB_IMM_RX_Inst& inst){
        auto dst=static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:
                size=4;
                break;
            case 3:
                size=8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        uint64_t imm=inst.imm;
        R[dst]-=imm;
        DEBUG_PRINT(imm);
        DEBUG_PRINT(R[dst]);

    }
    void handle_MUL_IMM_RX_Inst(_MUL_IMM_RX_Inst& inst){
        auto dst=static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:
                size=4;
                break;
            case 3:
                size=8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        uint64_t imm=inst.imm;
        R[dst]*=imm;
        DEBUG_PRINT(imm);
        DEBUG_PRINT(R[dst]);
    }
    void handle_MUL_RX_RY_Inst(_MUL_RX_RY_Inst& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);

        auto size=static_cast<int>(inst.size);
        switch(size){
            case 0:
                //用位级操作确保数据类型正确
                R[dst]*=R[src];
                break;
            case 1:
                R[dst]*=R[src];
                break;
            case 2:
                R[dst]*=R[src];
                break;
            case 3:
                R[dst]*=R[src];
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid optype");
        }
        DEBUG_PRINT(R[src]);
        DEBUG_PRINT(R[dst]);
    }
    void handle_ADD_RX_RY_Inst(_ADD_RX_RY_Inst& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);

        auto size=static_cast<int>(inst.size);
        switch(size){
            case 0:
                //用位级操作确保数据类型正确
                R[dst]+=R[src];
                break;
            case 1:
                R[dst]+=R[src];
                break;
            case 2:
                R[dst]+=R[src];
                break;
            case 3:
                R[dst]+=R[src];
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid optype");
        }
        DEBUG_PRINT(R[src]);
        DEBUG_PRINT(R[dst]);
    }
    void handle_SUB_RX_RY_Inst(_SUB_RX_RY_Inst& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);

        auto size=static_cast<int>(inst.size);
        switch(size){
            case 0:
                //用位级操作确保数据类型正确
                R[dst]-=R[src];
                break;
            case 1:
                R[dst]-=R[src];
                break;
            case 2:
                R[dst]-=R[src];
                break;
            case 3:
                R[dst]-=R[src];
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid optype");
        }
        DEBUG_PRINT(R[src]);
        DEBUG_PRINT(R[dst]);
    }
    void handle_UDIV_IMM_RX_Inst(_UDIV_IMM_RX_Inst& inst){
        auto dst=static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:
                size=4;
                break;
            case 3:
                size=8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        uint64_t imm=0;
        std::memcpy(reinterpret_cast<void*>(&imm), reinterpret_cast<void*>(&inst.imm), size);
        // //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
        // std::cout<<"[VMCORE]: inst.imm is "<<inst.imm<<std::endl;
        uint64_t src_value = reinterpret_cast<uint64_t&>(R[dst]);
        uint64_t result = src_value / imm;
        R[dst] = reinterpret_cast<uint64_t&>(result);
        DEBUG_PRINT(imm);
        DEBUG_PRINT(R[dst]);
    }
    void handle_SDIV_IMM_RX_Inst(_SDIV_IMM_RX_Inst& inst){
        auto dst=static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:
                size=4;
                break;
            case 3:
                size=8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        int64_t imm=0;
        std::memcpy(reinterpret_cast<void*>(&imm), reinterpret_cast<void*>(&inst.imm), size);
        // //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
        // std::cout<<"[VMCORE]: inst.imm is "<<inst.imm<<std::endl;
        //转化为有符号数
        int64_t src_value = reinterpret_cast<int64_t&>(R[dst]);
        int64_t result = src_value / imm;
        R[dst] = reinterpret_cast<uint64_t&>(result);
        DEBUG_PRINT(imm);
        DEBUG_PRINT(R[dst]);
    }
    void handle_UDIV_RX_RY_Inst(_UDIV_RX_RY_Inst& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);

        auto size=static_cast<int>(inst.size);
        switch(size){
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid optype");
        }
        uint64_t src_value = reinterpret_cast<uint64_t&>(R[src]);
        uint64_t dst_value = reinterpret_cast<uint64_t&>(R[dst]);
        uint64_t result = dst_value / src_value;
        R[dst] = reinterpret_cast<uint64_t&>(result);
        DEBUG_PRINT(R[src]);
        DEBUG_PRINT(R[dst]);
    }
    void handle_SDIV_RX_RY_Inst(_SDIV_RX_RY_Inst& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);

        auto size=static_cast<int>(inst.size);
        switch(size){
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid optype");
        }
        //转化为有符号数
        int64_t src_value = reinterpret_cast<int64_t&>(R[src]);
        int64_t dst_value = reinterpret_cast<int64_t&>(R[dst]);
        int64_t result = dst_value / src_value;
        R[dst] = reinterpret_cast<uint64_t&>(result);
        DEBUG_PRINT(R[src]);
        DEBUG_PRINT(R[dst]);
    }
    void handle_FADD_IMM_RX_Inst(_FADD_IMM_RX_Inst& inst){
        auto dst=static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:{
                size=4;
                float f1=0;
                float f2=0;
                std::memcpy(reinterpret_cast<void*>(&f1), reinterpret_cast<void*>(&inst.imm), size);
                std::memcpy(reinterpret_cast<void*>(&f2), reinterpret_cast<void*>(&R[dst]), size);
                float result = f2 + f1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                // std::cout<<"[VMCORE]: f1 is "<<f1<<std::endl;
                // std::cout<<"[VMCORE]: f2 is "<<f2<<std::endl;
                // std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            case 3:{
                size=8;
                double d1=0;
                double d2=0;
                std::memcpy(reinterpret_cast<void*>(&d1), reinterpret_cast<void*>(&inst.imm), size);
                std::memcpy(reinterpret_cast<void*>(&d2), reinterpret_cast<void*>(&R[dst]), size);
                double result = d2 + d1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                // std::cout<<"[VMCORE]: d1 is "<<d1<<std::endl;
                // std::cout<<"[VMCORE]: d2 is "<<d2<<std::endl;
                // std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
    }
    void handle_FADD_RX_RY_Inst(_FADD_RX_RY_Inst& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);

        auto size=static_cast<int>(inst.size);
        switch(size){
            case 0:
                break;
            case 1:
                break;
            case 2:{
                float f1=0;
                float f2=0;
                std::memcpy(reinterpret_cast<void*>(&f1), reinterpret_cast<void*>(&R[src]), size);
                std::memcpy(reinterpret_cast<void*>(&f2), reinterpret_cast<void*>(&R[dst]), size);
                float result = f2 + f1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                // std::cout<<"[VMCORE]: f1 is "<<f1<<std::endl;
                // std::cout<<"[VMCORE]: f2 is "<<f2<<std::endl;
                // std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            case 3:{
                double d1=0;
                double d2=0;
                std::memcpy(reinterpret_cast<void*>(&d1), reinterpret_cast<void*>(&R[src]), size);
                std::memcpy(reinterpret_cast<void*>(&d2), reinterpret_cast<void*>(&R[dst]), size);
                double result = d2 + d1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                // std::cout<<"[VMCORE]: d1 is "<<d1<<std::endl;
                // std::cout<<"[VMCORE]: d2 is "<<d2<<std::endl;
                // std::cout<<"[VMCORE]: result is "<<result<<std::endl;

                break;
            }
            default:
                throw std::runtime_error("[VMCORE]: Invalid optype");
        }
    }
    void handle_FSUB_IMM_RX_Inst(_FSUB_IMM_RX_Inst& inst){
        auto dst=static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:{
                size=4;
                float f1=0;
                float f2=0;
                std::memcpy(reinterpret_cast<void*>(&f1), reinterpret_cast<void*>(&inst.imm), size);
                std::memcpy(reinterpret_cast<void*>(&f2), reinterpret_cast<void*>(&R[dst]), size);
                float result = f2 - f1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                // std::cout<<"[VMCORE]: f1 is "<<f1<<std::endl;
                // std::cout<<"[VMCORE]: f2 is "<<f2<<std::endl;
                // std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            case 3:{
                size=8;
                double d1=0;
                double d2=0;
                std::memcpy(reinterpret_cast<void*>(&d1), reinterpret_cast<void*>(&inst.imm), size);
                std::memcpy(reinterpret_cast<void*>(&d2), reinterpret_cast<void*>(&R[dst]), size);
                double result = d2 - d1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                // std::cout<<"[VMCORE]: d1 is "<<d1<<std::endl;
                // std::cout<<"[VMCORE]: d2 is "<<d2<<std::endl;
                // std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
    }
    void handle_FSUB_RX_RY_Inst(_FSUB_RX_RY_Inst& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);

        auto size=static_cast<int>(inst.size);
        switch(size){
            case 0:
                break;
            case 1:
                break;
            case 2:{
                float f1=0;
                float f2=0;
                std::memcpy(reinterpret_cast<void*>(&f1), reinterpret_cast<void*>(&R[src]), size);
                std::memcpy(reinterpret_cast<void*>(&f2), reinterpret_cast<void*>(&R[dst]), size);
                float result = f2 - f1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                //std::cout<<"[VMCORE]: f1 is "<<f1<<std::endl;
                //std::cout<<"[VMCORE]: f2 is "<<f2<<std::endl;
                //std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            case 3:{
                double d1=0;
                double d2=0;
                std::memcpy(reinterpret_cast<void*>(&d1), reinterpret_cast<void*>(&R[src]), size);
                std::memcpy(reinterpret_cast<void*>(&d2), reinterpret_cast<void*>(&R[dst]), size);
                double result = d2 - d1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                //std::cout<<"[VMCORE]: d1 is "<<d1<<std::endl;
                //std::cout<<"[VMCORE]: d2 is "<<d2<<std::endl;
                //std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                
                break;
            }
            default:
                throw std::runtime_error("[VMCORE]: Invalid optype");
        }
    }
    void handle_FMUL_IMM_RX_Inst(_FMUL_IMM_RX_Inst& inst){
        auto dst=static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:{
                size=4;
                float f1=0;
                float f2=0;
                std::memcpy(reinterpret_cast<void*>(&f1), reinterpret_cast<void*>(&inst.imm), size);
                std::memcpy(reinterpret_cast<void*>(&f2), reinterpret_cast<void*>(&R[dst]), size);
                float result = f2 * f1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                //std::cout<<"[VMCORE]: f1 is "<<f1<<std::endl;
                //std::cout<<"[VMCORE]: f2 is "<<f2<<std::endl;
                //std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            case 3:{
                size=8;
                double d1=0;
                double d2=0;
                std::memcpy(reinterpret_cast<void*>(&d1), reinterpret_cast<void*>(&inst.imm), size);
                std::memcpy(reinterpret_cast<void*>(&d2), reinterpret_cast<void*>(&R[dst]), size);
                double result = d2 * d1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                //std::cout<<"[VMCORE]: d1 is "<<d1<<std::endl;
                //std::cout<<"[VMCORE]: d2 is "<<d2<<std::endl;
                //std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
    }
    void handle_FMUL_RX_RY_Inst(_FMUL_RX_RY_Inst& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);

        auto size=static_cast<int>(inst.size);
        switch(size){
            case 0:
                break;
            case 1:
                break;
            case 2:{
                float f1=0;
                float f2=0;
                std::memcpy(reinterpret_cast<void*>(&f1), reinterpret_cast<void*>(&R[src]), size);
                std::memcpy(reinterpret_cast<void*>(&f2), reinterpret_cast<void*>(&R[dst]), size);
                float result = f2 * f1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                //std::cout<<"[VMCORE]: f1 is "<<f1<<std::endl;
                //std::cout<<"[VMCORE]: f2 is "<<f2<<std::endl;
                //std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            case 3:{
                double d1=0;
                double d2=0;
                std::memcpy(reinterpret_cast<void*>(&d1), reinterpret_cast<void*>(&R[src]), size);
                std::memcpy(reinterpret_cast<void*>(&d2), reinterpret_cast<void*>(&R[dst]), size);
                double result = d2 * d1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                //std::cout<<"[VMCORE]: d1 is "<<d1<<std::endl;
                //std::cout<<"[VMCORE]: d2 is "<<d2<<std::endl;
                //std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                
                break;
            }
            default:
                throw std::runtime_error("[VMCORE]: Invalid optype");
        }
    }
    void handle_FDIV_IMM_RX_Inst(_FDIV_IMM_RX_Inst& inst){
        auto dst=static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:{
                size=4;
                float f1=0;
                float f2=0;
                std::memcpy(reinterpret_cast<void*>(&f1), reinterpret_cast<void*>(&inst.imm), size);
                std::memcpy(reinterpret_cast<void*>(&f2), reinterpret_cast<void*>(&R[dst]), size);
                float result = f2 / f1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                //std::cout<<"[VMCORE]: f1 is "<<f1<<std::endl;
                //std::cout<<"[VMCORE]: f2 is "<<f2<<std::endl;
                //std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            case 3:{
                size=8;
                double d1=0;
                double d2=0;
                std::memcpy(reinterpret_cast<void*>(&d1), reinterpret_cast<void*>(&inst.imm), size);
                std::memcpy(reinterpret_cast<void*>(&d2), reinterpret_cast<void*>(&R[dst]), size);
                double result = d2 / d1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                //std::cout<<"[VMCORE]: d1 is "<<d1<<std::endl;
                //std::cout<<"[VMCORE]: d2 is "<<d2<<std::endl;
                //std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
    }
    void handle_FDIV_RX_RY_Inst(_FDIV_RX_RY_Inst& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);

        auto size=static_cast<int>(inst.size);
        switch(size){
            case 0:
                break;
            case 1:
                break;
            case 2:{
                float f1=0;
                float f2=0;
                std::memcpy(reinterpret_cast<void*>(&f1), reinterpret_cast<void*>(&R[src]), size);
                std::memcpy(reinterpret_cast<void*>(&f2), reinterpret_cast<void*>(&R[dst]), size);
                float result = f2 / f1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                //std::cout<<"[VMCORE]: f1 is "<<f1<<std::endl;
                //std::cout<<"[VMCORE]: f2 is "<<f2<<std::endl;
                //std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                break;
            }
            case 3:{
                double d1=0;
                double d2=0;
                std::memcpy(reinterpret_cast<void*>(&d1), reinterpret_cast<void*>(&R[src]), size);
                std::memcpy(reinterpret_cast<void*>(&d2), reinterpret_cast<void*>(&R[dst]), size);
                double result = d2 / d1;
                R[dst]=0;
                std::memcpy(reinterpret_cast<void*>(&R[dst]), reinterpret_cast<void*>(&result), size);
                //std::cout<<"[VMCORE]: d1 is "<<d1<<std::endl;
                //std::cout<<"[VMCORE]: d2 is "<<d2<<std::endl;
                //std::cout<<"[VMCORE]: result is "<<result<<std::endl;
                
                break;
            }
            default:
                throw std::runtime_error("[VMCORE]: Invalid optype");
        }
    }
    template<OpCode op>
    void handle_CMP_IMM_RX_Inst(_CMP_IMM_RX_Inst<op>& inst){
        auto dst=static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:
                size=4;
                break;
            case 3:
                size=8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        switch (op) {
            case OpCode::_icmp_eq_imm_rx:{
                //std::cout<<"[VMCORE]: ICMP_EQ"<<std::endl;
                uint64_t imm=inst.imm;
                uint64_t src_value = 0;
                for(auto i=0;i<size*8;i++){
                    src_value|=(R[dst]&(1<<i));
                }
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
                if(src_value==imm){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_ne_imm_rx:{
                //std::cout<<"[VMCORE]: ICMP_NE"<<std::endl;
                uint64_t imm=inst.imm;
                uint64_t src_value = 0;
                for(auto i=0;i<size*8;i++){
                    src_value|=(R[dst]&(1<<i));
                }
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
                if(src_value!=imm){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_ugt_imm_rx:{
                //std::cout<<"[VMCORE]: ICMP_UGT"<<std::endl;
                uint64_t imm=inst.imm;
                uint64_t src_value = 0;
                for(auto i=0;i<size*8;i++){
                    src_value|=(R[dst]&(1<<i));
                }
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
                if(src_value>imm){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_uge_imm_rx:{
                //std::cout<<"[VMCORE]: ICMP_UGE"<<std::endl;
                uint64_t imm=inst.imm;
                uint64_t src_value = 0;
                for(auto i=0;i<size*8;i++){
                    src_value|=(R[dst]&(1<<i));
                }
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
                if(src_value>=imm){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_ult_imm_rx:{
                //std::cout<<"[VMCORE]: ICMP_ULT"<<std::endl;
                uint64_t imm=inst.imm;
                uint64_t src_value = 0;
                for(auto i=0;i<size*8;i++){
                    src_value|=(R[dst]&(1<<i));
                }
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
                if(src_value<imm){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_ule_imm_rx:{
                //std::cout<<"[VMCORE]: ICMP_ULE"<<std::endl;
                uint64_t imm=inst.imm;
                uint64_t src_value = 0;
                for(auto i=0;i<size*8;i++){
                    src_value|=(R[dst]&(1<<i));
                }
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
                if(src_value<=imm){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_sgt_imm_rx:{
                //std::cout<<"[VMCORE]: ICMP_SGT"<<std::endl;
                int64_t imm=inst.imm;
                int64_t src_value = 0;
                for(auto i=0;i<size*8;i++){
                    src_value|=(R[dst]&(1<<i));
                }
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
                if(src_value>imm){
                    cmp_flag=true;
                    R[dst]=1;
                }else{
                    cmp_flag=false;
                    R[dst]=0;
                }
                break;
            }
            case OpCode::_icmp_sge_imm_rx:{
                //std::cout<<"[VMCORE]: ICMP_SGE"<<std::endl;
                int64_t imm=inst.imm;
                int64_t src_value = 0;
                for(auto i=0;i<size*8;i++){
                    src_value|=(R[dst]&(1<<i));
                }
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
                if(src_value>=imm){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_slt_imm_rx:{
                //std::cout<<"[VMCORE]: ICMP_SLT"<<std::endl;
                int64_t imm=inst.imm;
                int64_t src_value = 0;
                for(auto i=0;i<size*8;i++){
                    src_value|=(R[dst]&(1<<i));
                }
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
                if(src_value<imm){
                    cmp_flag=true;
                    R[dst]=1;
                }else{
                    cmp_flag=false;
                    R[dst]=0;
                }
                break;
            }
            case OpCode::_icmp_sle_imm_rx:{
                //std::cout<<"[VMCORE]: ICMP_SLE"<<std::endl;
                int64_t imm=inst.imm;
                int64_t src_value = 0;
                for(auto i=0;i<size*8;i++){
                    src_value|=(R[dst]&(1<<i));
                }
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: imm is "<<imm<<std::endl;
                if(src_value<=imm){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_oeq_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_OEQ"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_ogt_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_OGT"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_oge_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_OGE"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_olt_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_OLT"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_ole_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_OLE"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_one_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_ONE"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_ueq_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_UEQ"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_ugt_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_UGT"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_uge_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_UGE"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_ult_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_ULT"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_ule_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_ULE"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_une_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_UNE"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            case OpCode::_fcmp_true_imm_rx:{
                //std::cout<<"[VMCORE]: FCMP_TRUE"<<std::endl;
                if(size == 4){
                    if(compare_float<float,uint32_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }else if(size == 8 ){
                    if(compare_float<double,uint64_t>(inst.imm,R[dst])){
                        cmp_flag=true;
                    }else{
                        cmp_flag=false;
                    }
                }
                break;
            }
            default:
                throw std::runtime_error("[VMCORE]: Invalid cmp opcode");
        }
        DEBUG_PRINT(R[dst]);

    }
    void handle_JMP_R4_Inst(_JMP_R4_Inst& inst){
        PC=R[4]+1024*1024+init_size-1;
    }
    void handle_CALL_EXTERNAL_Inst(_CALL_EXTERNAL_Inst& inst){
        uint64_t addr = inst.addr;
        int param_nums = inst.params;
        char arr[8];

        arr[0] = addr & 0xFF;
        arr[1] = (addr >> 8)  & 0xFF;
        arr[2] = (addr >> 16) & 0xFF;
        arr[3] = (addr >> 24) & 0xFF;
        arr[4] = (addr >> 32) & 0xFF;
        arr[5] = (addr >> 40) & 0xFF; 
        arr[6] = (addr >> 48) & 0xFF;
        arr[7] = (addr >> 56) & 0xFF;
        std::string cxxstr = std::string(arr);
        // std::cout<<"[VMCORE]: cxxstr "<<cxxstr<<std::endl;
        uint8_t* old_rbp_ptr = stack.read(0,8);
        uint64_t old_rbp = *reinterpret_cast<uint64_t*>(old_rbp_ptr);
        // std::cout<<"[VMCORE]: old_rbp "<<old_rbp<<std::endl;
        // std::cout<<"[VMCORE]: stack.get_rbp() "<<stack.get_rbp()<<std::endl;
        // stack.dump();
        // mem.dump(); 
        int64_t rbp_offset = -old_rbp+stack.get_rbp();
        for(int i=param_nums-1;i>=0;i--){

            int param_size=static_cast<int>(*stack.read(-17, 8));
            DBG_PRINT(std::cout<<"[VMCORE]: param_size: "<<param_size<<std::endl);
            int bool_ =  0;
            std::memcpy(reinterpret_cast<void*>(&bool_), stack.read(-16, 1), 1);
            DBG_PRINT(std::cout<<"[VMCORE]: bool_ "<<bool_<<std::endl);
            uint8_t* addr_or_offset=stack.read(-8,8);
            uint64_t ao = 0;
            for(int i=0;i<8;i++){
                ao |= (uint64_t)addr_or_offset[i] << (i*8);
            }
            uint8_t* param_data=new uint8_t[param_size];
            if(int(bool_)==1){
                int64_t offset=0;
                std::memcpy(reinterpret_cast<void*>(&offset), addr_or_offset, 8);
                
                if(i==param_nums-1&&cxxstr.starts_with("t")){
                    uint8_t* addr = stack.getaddr(offset);
                    var_map[cxxstr+"_::addr"]=addr;
                    DBG_PRINT(std::cout<<"[VMCORE]: addr_ "<<addr<<std::endl);
                    std::memcpy(var_map[cxxstr+"_::addr"], addr, sizeof(var_map[cxxstr+"_::addr"]));
                    continue;
                }
                param_data = stack.read(rbp_offset+offset, param_size);
            }else{
                uint64_t addr=0;
                std::memcpy(reinterpret_cast<void*>(&addr), addr_or_offset, 8);
                
                if(i==param_nums-1&&cxxstr.starts_with("t")){
                    uint8_t* addr_ = mem.getaddr(addr);
                    var_map[cxxstr+"_::addr"]=addr_;
                    DBG_PRINT(std::cout<<"[VMCORE]: addr_ "<<addr_<<std::endl);
                    std::memcpy(var_map[cxxstr+"_::addr"], addr_, sizeof(var_map[cxxstr+"_::addr"]));
                    continue;
                }
                param_data = mem.read(addr, param_size);
            }
            
            auto actual_param_name = cxxstr+"_"+"::"+"p"+std::to_string(i);
            // std::cout<<"[VMCORE]: actual_param_name "<<actual_param_name<<" cxxstr "<<cxxstr<<" i "<<i<<" param_size "<<param_size<<std::endl;
            std::memcpy(reinterpret_cast<void*>(var_map[actual_param_name]), param_data, param_size);

        }

        func_map[cxxstr]();
        // if(cxxstr == "f90"){
        //     DBG_PRINT(std::cout<<"[VMCORE]: f90_::res is "<<*reinterpret_cast<int*>(var_map[cxxstr+"_::res"])<<std::endl);
        //     DBG_PRINT(std::cout<<"[VMCORE]: *t86_::res[0] is "<<(*t86_::res)[0]<<std::endl);
        // }
        for(auto i=param_nums-1;i>=0;i--){
            if(cxxstr.starts_with("t")&&i==param_nums-1){
                continue;
            }
            auto actual_param_name = cxxstr+"_"+"::"+"p"+std::to_string(i);
            int param_size=static_cast<int>(*stack.read(-17, 8));
            int bool_ =  0;
            std::memcpy(reinterpret_cast<void*>(&bool_), stack.read(-16, 1), 1);
            uint8_t* addr_or_offset=stack.read(-8, 8);
            uint64_t ao = 0;
            for(int i=0;i<8;i++){
                ao |= (uint64_t)addr_or_offset[i] << (i*8);
            }
            if(bool_){
                int64_t offset=0;
                std::memcpy(reinterpret_cast<void*>(&offset), addr_or_offset, 8);
                uint8_t* param_data=new uint8_t[param_size];
                std::memcpy(param_data, reinterpret_cast<void*>(var_map[actual_param_name]), param_size);
                DBG_PRINT(std::cout<<"[VMCORE]: param_data "<<*reinterpret_cast<int*>(param_data)<<std::endl);
                stack.write(rbp_offset+offset, param_data, param_size);
            }else{
                uint64_t addr=0;
                std::memcpy(reinterpret_cast<void*>(&addr), addr_or_offset, 8);
                uint8_t* param_data=new uint8_t[param_size];
                std::memcpy(param_data, reinterpret_cast<void*>(var_map[actual_param_name]), param_size);
                DBG_PRINT(std::cout<<"[VMCORE]: param_data "<<*reinterpret_cast<int*>(param_data)<<std::endl);
                mem.write(addr, param_data, param_size);
            }
        }
    }
    void handle_CALL_INTERNAL_Inst(_CALL_INTERNAL_Inst& inst){
        auto call_addr=inst.addr;

        uint8_t* data=new uint8_t[8];
        for(auto i=0;i<8;i++){
            data[i]=(call_addr>>(i*8))&0xff;
        }
        std::memcpy(&call_addr, data, 8);
        PC=call_addr+1024*1024+init_size;
        DEBUG_PRINT(PC);
    }
    void hanle_RET_R0_Inst(_RET_R0_Inst& inst){
        return;
    }
    void handle_JZ_R5_Inst(_JZ_R5_Inst& inst){
        if(cmp_flag.value()==false){
            PC=R[5]+1024*1024+init_size-1;
            // std::cout<<"[VMCORE]: JZ TO "<<PC<<std::endl;
            cmp_flag=std::nullopt;
        }
    }
    void handle_JNZ_R5_Inst(_JNZ_R5_Inst& inst){
        if(cmp_flag.value()==true){
            PC=R[5]+1024*1024+init_size-1;
            // std::cout<<"[VMCORE]: JNZ TO "<<PC<<std::endl;
            cmp_flag=std::nullopt;
        }
    }
    void handle_JL_R6_Inst(_JL_R6_Inst& inst){
        if(cmp_flag.value()==true){
            PC=R[6]+1024*1024+init_size-1;
            // std::cout<<"[VMCORE]: JNZ TO "<<PC<<std::endl;
            cmp_flag=std::nullopt;
        }
    }
    void handle_JLE_R7_Inst(_JLE_R7_Inst& inst){
        if(cmp_flag.value()==true){
            PC=R[7]+1024*1024+init_size-1;
            // std::cout<<"[VMCORE]: JNZ TO "<<PC<<std::endl;
            cmp_flag=std::nullopt;
        }
    }
    void handle_JG_R6_Inst(_JG_R6_Inst& inst){
        if(cmp_flag.value()==true){
            PC=R[6]+1024*1024+init_size-1;
            // std::cout<<"[VMCORE]: JNZ TO "<<PC<<std::endl;
            cmp_flag=std::nullopt;
        }
    }
    void handle_JGE_R7_Inst(_JGE_R7_Inst& inst){
        if(cmp_flag.value()==true){
            PC=R[7]+1024*1024+init_size-1;
            // std::cout<<"[VMCORE]: JNZ TO "<<PC<<std::endl;
            cmp_flag=std::nullopt;
        }
    }
    void handle_RET_TO_NATIVE_Inst(_RET_TO_NATIVE_Inst& inst){
        uint64_t addr = inst.offset;
        int size = inst.size;
        uint8_t* data=new uint8_t[size];
        data=mem.read(addr,size);
        std::memcpy(this->ret,data,size);
    }
    template<OpCode op>
    void handle_CMP_RX_RY_Inst(_CMP_RX_RY_Inst<op>& inst){
        auto src=static_cast<int>(inst.src);
        auto dst=static_cast<int>(inst.dst);

        auto size=static_cast<int>(inst.size);
        switch (op) {
            case OpCode::_icmp_eq_rx_ry:{
                //std::cout<<"[VMCORE]: ICMP_EQ"<<std::endl;
                uint64_t src_value = reinterpret_cast<uint64_t&>(R[src]);
                uint64_t dst_value = reinterpret_cast<uint64_t&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value==dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_ne_rx_ry:{
                //std::cout<<"[VMCORE]: ICMP_NE"<<std::endl;
                uint64_t src_value = reinterpret_cast<uint64_t&>(R[src]);
                uint64_t dst_value = reinterpret_cast<uint64_t&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value!=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_sgt_rx_ry:{
                //std::cout<<"[VMCORE]: ICMP_SGT"<<std::endl;
                int64_t src_value = reinterpret_cast<int64_t&>(R[src]);
                int64_t dst_value = reinterpret_cast<int64_t&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value>dst_value){
                    cmp_flag=true;
                    R[dst]=1;
                }else{
                    cmp_flag=false;
                    R[dst]=0;
                }
                break;
            }
            case OpCode::_icmp_sge_rx_ry:{
                //std::cout<<"[VMCORE]: ICMP_SGE"<<std::endl;
                int64_t src_value = reinterpret_cast<int64_t&>(R[src]);
                int64_t dst_value = reinterpret_cast<int64_t&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value>=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_slt_rx_ry:{
                //std::cout<<"[VMCORE]: ICMP_SLT"<<std::endl;
                int64_t src_value = reinterpret_cast<int64_t&>(R[src]);
                int64_t dst_value = reinterpret_cast<int64_t&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value<dst_value){
                    cmp_flag=true;
                    R[dst]=1;
                }else{
                    cmp_flag=false;
                    R[dst]=0;
                }
                break;
            }
            case OpCode::_icmp_sle_rx_ry:{
                //std::cout<<"[VMCORE]: ICMP_SLE"<<std::endl;
                int64_t src_value = reinterpret_cast<int64_t&>(R[src]);
                int64_t dst_value = reinterpret_cast<int64_t&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value<=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_ugt_rx_ry:{
                //std::cout<<"[VMCORE]: ICMP_UGT"<<std::endl;
                uint64_t src_value = reinterpret_cast<uint64_t&>(R[src]);
                uint64_t dst_value = reinterpret_cast<uint64_t&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value>dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_uge_rx_ry:{
                //std::cout<<"[VMCORE]: ICMP_UGE"<<std::endl;
                uint64_t src_value = reinterpret_cast<uint64_t&>(R[src]);
                uint64_t dst_value = reinterpret_cast<uint64_t&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value>=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_ult_rx_ry:{
                //std::cout<<"[VMCORE]: ICMP_ULT"<<std::endl;
                uint64_t src_value = reinterpret_cast<uint64_t&>(R[src]);
                uint64_t dst_value = reinterpret_cast<uint64_t&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value<dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_icmp_ule_rx_ry:{
                //std::cout<<"[VMCORE]: ICMP_ULE"<<std::endl;
                uint64_t src_value = reinterpret_cast<uint64_t&>(R[src]);
                uint64_t dst_value = reinterpret_cast<uint64_t&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value<=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_oeq_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_OEQ"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value==dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_ogt_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_OGT"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value>dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_oge_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_OGE"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value>=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_olt_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_OLT"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value<dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_ole_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_OLE"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value<=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_one_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_ONE"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value!=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_ueq_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_UEQ"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value==dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_ugt_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_UGT"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value>dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_uge_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_UGE"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value>=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_ult_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_ULT"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value<dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_ule_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_ULE"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value<=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_une_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_UNE"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value!=dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            case OpCode::_fcmp_true_rx_ry:{
                //std::cout<<"[VMCORE]: FCMP_TRUE"<<std::endl;
                float src_value = reinterpret_cast<float&>(R[src]);
                float dst_value = reinterpret_cast<float&>(R[dst]);
                //std::cout<<"[VMCORE]: src_value is "<<src_value<<std::endl;
                //std::cout<<"[VMCORE]: dst_value is "<<dst_value<<std::endl;
                if(src_value==dst_value){
                    cmp_flag=true;
                }else{
                    cmp_flag=false;
                }
                break;
            }
            default:
                throw std::runtime_error("[VMCORE]: Invalid cmp opcode");
        }
    }
    void handle_MOV_MEM_RX_Inst(_MOV_MEM_RX_Inst& inst){
        auto addr=inst.addr;
        //std::cout<<"[VMCORE]: addr is "<<addr<<std::endl;
        auto dst = static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:
                size=4;
                break;
            case 3:
                size=8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        auto data=mem.read(addr, size);
        std::memcpy(&R[dst], data, size);
    }
    void handle_MOV_RX_MEM_Inst(_MOV_RX_MEM_Inst& inst){
        auto addr=inst.addr;
        auto src = static_cast<int>(inst.src);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:
                size=4;
                break;
            case 3:
                size=8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        uint8_t *data=new uint8_t[size];
        std::memcpy(data, reinterpret_cast<uint8_t*>(&R[src]), size);
        mem.write(addr, data, size);
        DEBUG_PRINT(R[src]);
        DEBUG_PRINT(addr);
    }
    void handle_MOV_MEM_MEM_Inst(_MOV_MEM_MEM_Inst& inst){
        auto src=inst.src;
        auto dst=inst.dst;
        auto size = inst.size;
        DBG_PRINT(std::cout<<"[VMCORE]: MMMI size is "<<static_cast<int>(size)<<std::endl);
        auto data=mem.read(src, static_cast<int>(size));
        mem.write(dst, data, static_cast<int>(size));
    }
    void handle_MOV_STACK_RX_Inst(_MOV_STACK_RX_Inst& inst){
        auto src = static_cast<int16_t>(inst.src);
        auto dst = static_cast<int>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:
                size=4;
                break;
            case 3:
                size=8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
        }
        auto data=stack.read(src, size);
        std::memcpy(&R[dst], data, size);
    }
    void handle_MOV_RX_STACK_Inst(_MOV_RX_STACK_Inst& inst){
        auto src = static_cast<int>(inst.src);
        auto dst = static_cast<int16_t>(inst.dst);
        auto size=-1;
        switch(static_cast<int>(inst.size)){
            case 0:
                size=1;
                break;
            case 1:
                size=2;
                break;
            case 2:
                size=4;
                break;
            case 3:
                size=8;
                break;
            default:
                throw std::runtime_error("[VMCORE]: Invalid size");
                break;
        }
        uint8_t* data = new uint8_t[size];
        std::memcpy(data, reinterpret_cast<uint8_t*>(&R[src]), size);
        stack.write(dst, data, size);
        DEBUG_PRINT(R[src]);
    }
    void handle_HALT_Inst(_HALT_Inst& inst){
        halt=true;
    }
    void handle_RETURN_NRVO_Inst(_RETURN_NRVO_Inst& inst){
        int size = inst.size;
        uint64_t ret = 0;
        int64_t add_or_offset = inst.add_or_offset;
        int64_t return_addr = inst.ret_addr;
        uint8_t s_or_m = inst.s_or_m;
        if(inst.ret_addr == 0){
            return;
        }
        std::memcpy(reinterpret_cast<uint8_t *>(&ret), stack.read(return_addr,8), 8);
        //std::cout<<"[VMCORE]: ret is "<<ret<<std::endl;
        //std::cout<<"[VMCORE]: rbp is "<<stack.get_rbp()<<std::endl;
        if(s_or_m == 0){
            mem.write(ret, stack.read(add_or_offset,size), size);
        }else{
            mem.write(ret, mem.read(add_or_offset,size), size);
        }
    }

    void load_initGlobal(){
        //改成从文件头读取字节码，可以加密字节码再存储
        // const char* initGlobal="/home/ljz/XXXVMP/XXXClang/build/__init_global.bin";
        // std::fstream file(initGlobal, std::ios::in | std::ios::binary);
        // if (!file.is_open())
        // {
        //     std::cerr << "Failed to open file: " << initGlobal << std::endl;
        //     return;
        // }
        // file.seekg(0, std::ios::end);
        // size_t size = file.tellg();
        // file.seekg(0, std::ios::beg);

        size_t size =_binary__tmp_global_end - _binary__tmp_global_start;
        std::cout << "initGlobal size is " << size << std::endl;
        uint8_t *data = new uint8_t[size];
        // file.read(reinterpret_cast<char *>(data), size);

        load_section(data, size,_binary__tmp_global_start);
        for(auto i = 0; i < size; i++){
            std::cout <<"0x"<< std::hex << std::setfill('0') << std::setw(2) <<std::uppercase<< static_cast<int>(data[i]) << " ";
        }
        std::cout << std::nouppercase<<std::dec << std::endl;
        auto mem_size = mem.size();
        mem.resize(mem_size + size);
        mem.write(mem_size, data, size);
        PC = mem_size;
        init_size = size;
        //file.close();
        delete[] data;
    }
    void run()
    {

        while (PC < mem.size()&&!halt)
        {
            // start_comprehensive_detection();
   
            // 读取指令
            auto opcode = mem.read(PC, 1);
            //用宏或者模板函数可以简化，但是不想写，就这样先
            switch (*opcode)
            {
                case 0x0b:{
                    auto inst_size=_MOV_IMM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MOV_IMM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MOV_IMM_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x10:{
                    auto inst_size=_MOV_RX_STACK_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MOV_RX_STACK_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MOV_RX_STACK_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x04:{
                    auto inst_size = _SUB_RSP_R2_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _SUB_RSP_R2_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_SUB_RSP_R2_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x0d:{
                    auto inst_size=_MOV_RX_MEM_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MOV_RX_MEM_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MOV_RX_MEM_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x0f:{
                    auto inst_size=_MOV_STACK_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MOV_STACK_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MOV_STACK_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x25:{
                    auto inst_size=_RET_R0_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _RET_R0_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    hanle_RET_R0_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x2a:{
                    auto inst_size=_PUSH_PC_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _PUSH_PC_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_PUSH_PC_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x2b:{
                    auto inst_size=_POP_PC_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _POP_PC_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_POP_PC_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x2c:{
                    auto inst_size=_MOV_RBP_RSP_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MOV_RBP_RSP_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MOV_RBP_RSP_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x1e:{
                    auto inst_size=_CALL_INTERNAL_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _CALL_INTERNAL_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CALL_INTERNAL_Inst(inst);
                    break;
                }
                case 0x0e:{
                    auto inst_size=_MOV_MEM_MEM_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MOV_MEM_MEM_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MOV_MEM_MEM_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x06:{
                    auto inst_size=_POP_RBP_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _POP_RBP_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_POP_RBP_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x05:{
                    auto inst_size=_PUSH_RBP_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _PUSH_RBP_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_PUSH_RBP_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x01:{
                    auto inst_size=_PUSH_R1_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _PUSH_R1_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_PUSH_R1_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x0c:{
                    auto inst_size=_MOV_MEM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MOV_MEM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MOV_MEM_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x09:{
                    auto inst_size = _MOV_RSP_RBP_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MOV_RSP_RBP_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MOV_RSP_RBP_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x2d:{
                    auto inst_size=_PUSH_RX_WITH_SIZE_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _PUSH_RX_WITH_SIZE_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_PUSH_RX_WITH_SIZE_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case  0x2e:{
                    auto inst_size=_HALT_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _HALT_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_HALT_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x12:{
                    auto inst_size = _ADD_IMM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _ADD_IMM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_ADD_IMM_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x13:{
                    auto inst_size = _ADD_RX_RY_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _ADD_RX_RY_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_ADD_RX_RY_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x15:{
                    auto inst_size = _SUB_RX_RY_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _SUB_RX_RY_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_SUB_RX_RY_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x16:{
                    auto inst_size = _MUL_IMM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MUL_IMM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MUL_IMM_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x17:{
                    auto inst_size = _MUL_RX_RY_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MUL_RX_RY_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MUL_RX_RY_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x18:{
                    auto inst_size = _SDIV_IMM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _SDIV_IMM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_SDIV_IMM_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x19:{
                    auto inst_size = _SDIV_RX_RY_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _SDIV_RX_RY_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_SDIV_RX_RY_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x1d:{
                    auto inst_size = _CALL_EXTERNAL_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _CALL_EXTERNAL_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CALL_EXTERNAL_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x07:{
                    auto inst_size = _PUSH_RSP_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _PUSH_RSP_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_PUSH_RSP_Inst(inst);
                    PC+=inst_size;
                    break;

                }
                case 0x11:{
                    auto inst_size = _MOV_STACK_STACK_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MOV_STACK_STACK_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MOV_STACK_STACK_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x2f:{
                    auto inst_size = _RETURN_NRVO_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _RETURN_NRVO_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_RETURN_NRVO_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x14:{
                    auto inst_size = _SUB_IMM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _SUB_IMM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_SUB_IMM_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x32:{
                    auto inst_size = _FADD_IMM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _FADD_IMM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FADD_IMM_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x33:{
                    auto inst_size = _FADD_RX_RY_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _FADD_RX_RY_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FADD_RX_RY_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x34:{
                    auto inst_size = _FSUB_IMM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _FSUB_IMM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FSUB_IMM_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x35:{
                    auto inst_size = _FSUB_RX_RY_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _FSUB_RX_RY_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FSUB_RX_RY_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x36:{
                    auto inst_size = _FMUL_IMM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _FMUL_IMM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FMUL_IMM_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x37:{
                    auto inst_size = _FMUL_RX_RY_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _FMUL_RX_RY_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FMUL_RX_RY_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x38:{
                    auto inst_size = _FDIV_IMM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _FDIV_IMM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FDIV_IMM_RX_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x39:{
                    auto inst_size = _FDIV_RX_RY_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _FDIV_RX_RY_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FDIV_RX_RY_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x28:{
                    auto inst_size = _CMP_IMM_RX_Inst<OpCode::_icmp_sgt_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_icmp_sgt_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_icmp_sgt_imm_rx>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x1f:{
                    auto inst_size = _JZ_R5_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _JZ_R5_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_JZ_R5_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x20:{
                    auto inst_size = _JNZ_R5_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _JNZ_R5_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_JNZ_R5_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x1c:{
                    auto inst_size = _JMP_R4_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _JMP_R4_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_JMP_R4_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x3c:{
                    auto inst_size = _CMP_IMM_RX_Inst<OpCode::_icmp_slt_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_icmp_slt_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_icmp_slt_imm_rx>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x02:{
                    auto inst_size = _POP_R2_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _POP_R2_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_POP_R2_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x03:{
                    auto inst_size = _ADD_RSP_R2_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _ADD_RSP_R2_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_ADD_RSP_R2_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x08:{
                    auto inst_size = _POP_RSP_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _POP_RSP_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_POP_RSP_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x0a:{
                    auto inst_size = _MOV_RX_RY_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = _MOV_RX_RY_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_MOV_RX_RY_Inst(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x21:{
                    auto inst_szie = _JG_R6_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _JG_R6_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_JG_R6_Inst(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x22:{
                    auto inst_szie = _JL_R6_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _JL_R6_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_JL_R6_Inst(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x23:{
                    auto inst_szie = _JGE_R7_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _JGE_R7_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_JGE_R7_Inst(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x24:{
                    auto inst_szie = _JLE_R7_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _JLE_R7_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_JLE_R7_Inst(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x29:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_icmp_sgt_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_icmp_sgt_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_icmp_sgt_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x30:{
                    auto inst_szie = _UDIV_IMM_RX_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _UDIV_IMM_RX_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_UDIV_IMM_RX_Inst(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x31:{
                    auto inst_szie = _UDIV_RX_RY_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _UDIV_RX_RY_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_UDIV_RX_RY_Inst(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x3a:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_icmp_sge_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_icmp_sge_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_icmp_sge_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x3b:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_icmp_sge_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_icmp_sge_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_icmp_sge_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x3d:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_icmp_slt_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_icmp_slt_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_icmp_slt_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x3e:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_icmp_sle_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_icmp_sle_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_icmp_sle_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x3f:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_icmp_sle_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_icmp_sle_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_icmp_sle_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x40:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_icmp_eq_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_icmp_eq_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_icmp_eq_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x41:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_icmp_eq_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_icmp_eq_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_icmp_eq_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x42:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_icmp_ne_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_icmp_ne_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_icmp_ne_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x43:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_icmp_ne_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_icmp_ne_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_icmp_ne_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x44:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_icmp_ugt_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_icmp_ugt_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_icmp_ugt_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x45:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_icmp_ugt_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_icmp_ugt_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_icmp_ugt_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x46:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_icmp_uge_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_icmp_uge_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_icmp_uge_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x47:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_icmp_uge_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_icmp_uge_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_icmp_uge_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x48:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_icmp_ult_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_icmp_ult_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_icmp_ult_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x49:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_icmp_ult_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_icmp_ult_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_icmp_ult_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x4a:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_icmp_ule_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_icmp_ule_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_icmp_ule_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x4b:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_icmp_ule_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_icmp_ule_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_icmp_ule_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x4c:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_oeq_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_oeq_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_oeq_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x4d:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_oeq_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_oeq_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_oeq_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x4e:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_ogt_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_ogt_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_ogt_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x4f:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_ogt_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_ogt_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_ogt_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x50:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_oge_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_oge_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_oge_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x51:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_oge_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_oge_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_oge_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x52:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_olt_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_olt_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_olt_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x53:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_olt_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_olt_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_olt_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x54:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_ole_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_ole_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_ole_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x55:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_ole_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_ole_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_ole_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x56:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_one_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_one_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_one_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x57:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_one_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_one_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_one_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x58:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_ord_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_ord_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_ord_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x59:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_ord_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_ord_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_ord_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x5a:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_uno_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_uno_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_uno_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x5b:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_uno_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_uno_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_uno_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x5c:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_ueq_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_ueq_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_ueq_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x5d:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_ueq_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_ueq_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_ueq_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x5e:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_ugt_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_ugt_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_ugt_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x5f:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_ugt_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_ugt_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_ugt_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x60:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_uge_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_uge_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_uge_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x61:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_uge_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_uge_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_uge_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x62:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_ult_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_ult_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_ult_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x63:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_ult_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_ult_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_ult_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x64:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_ule_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_ule_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_ule_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x65:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_ule_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_ule_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_ule_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x66:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_une_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_une_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_une_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x67:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_une_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_une_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_une_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x68:{
                    auto inst_szie = _CMP_IMM_RX_Inst<OpCode::_fcmp_true_imm_rx>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_IMM_RX_Inst<OpCode::_fcmp_true_imm_rx>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_IMM_RX_Inst<OpCode::_fcmp_true_imm_rx>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x69:{
                    auto inst_szie = _CMP_RX_RY_Inst<OpCode::_fcmp_true_rx_ry>::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _CMP_RX_RY_Inst<OpCode::_fcmp_true_rx_ry>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_CMP_RX_RY_Inst<OpCode::_fcmp_true_rx_ry>(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x6a:{
                    auto inst_szie = _RET_TO_NATIVE_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _RET_TO_NATIVE_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_RET_TO_NATIVE_Inst(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x6b:{
                    auto inst_szie = _LOAD_FROM_EXTERNAL_Inst::static_get_size();
                    auto raw_data = mem.read(PC, inst_szie);
                    auto inst = _LOAD_FROM_EXTERNAL_Inst(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_LOAD_FROM_EXTERNAL_Inst(inst);
                    PC+=inst_szie;
                    break;
                }
                case 0x100:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_1>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_1>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_1>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x101:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_2>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_2>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_2>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x102:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_3>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_3>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_3>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x103:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_4>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_4>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_4>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x104:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_5>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_5>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_5>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x105:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_6>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_6>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_6>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x106:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_7>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_7>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_7>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x107:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_8>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_8>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_8>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x108:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_9>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_9>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_9>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x109:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_10>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_10>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_10>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x10a:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_11>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_11>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_11>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x10b:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_12>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_12>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_12>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x10c:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_13>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_13>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_13>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x10d:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_14>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_14>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_14>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x10e:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_15>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_15>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_15>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x10f:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_16>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_16>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_16>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x110:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_17>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_17>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_17>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x111:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_18>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_18>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_18>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x112:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_19>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_19>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_19>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x113:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_20>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_20>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_20>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x114:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_21>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_21>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_21>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x115:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_22>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_22>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_22>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x116:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_23>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_23>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_23>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x117:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_24>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_24>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_24>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x118:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_25>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_25>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_25>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x119:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_26>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_26>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_26>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x11a:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_27>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_27>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_27>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x11b:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_28>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_28>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_28>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x11c:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_29>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_29>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_29>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x11d:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_30>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_30>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_30>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x11e:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_31>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_31>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_31>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x11f:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_32>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_32>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_32>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x120:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_33>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_33>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_33>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x121:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_34>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_34>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_34>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x122:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_35>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_35>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_35>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x123:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_36>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_36>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_36>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x124:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_37>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_37>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_37>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x125:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_38>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_38>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_38>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x126:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_39>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_39>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_39>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x127:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_40>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_40>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_40>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x128:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_41>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_41>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_41>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x129:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_42>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_42>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_42>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x12a:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_43>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_43>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_43>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x12b:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_44>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_44>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_44>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x12c:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_45>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_45>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_45>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x12d:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_46>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_46>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_46>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x12e:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_47>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_47>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_47>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x12f:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_48>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_48>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_48>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x130:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_49>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_49>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_49>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x131:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_50>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_50>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_50>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x132:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_51>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_51>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_51>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x133:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_52>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_52>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_52>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x134:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_53>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_53>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_53>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x135:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_54>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_54>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_54>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x136:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_55>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_55>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_55>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x137:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_56>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_56>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_56>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x138:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_57>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_57>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_57>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x139:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_58>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_58>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_58>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x13a:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_59>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_59>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_59>(inst);
                    PC+=inst_size;
                    break;
                }
                case 0x13b:{
                    auto inst_size = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_60>::static_get_size();
                    auto raw_data = mem.read(PC, inst_size);
                    auto inst = FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_60>(raw_data);
                    DBG_PRINT(inst.to_string());
                    handle_FakeBranchInst<fake_branch_opcode::FAKE_BRANCH_60>(inst);
                    PC+=inst_size;
                    break;
                }

                default:
                    std::cout << "Unknown opcode: " << std::hex << static_cast<int>(*opcode) << std::dec << std::endl;
                    std::cout << "res ins: " <<std::endl;
                    while(PC<mem.size()){
                        auto opcode = mem.read(PC, 1);
                        std::cout  << std::hex << static_cast<int>(*opcode) << std::dec << " ";
                        PC++;
                    }
                    std::cout<<std::endl;
                    return;
            }
        }
    }
    void dump()
    {
        mem.dump();
        stack.dump();
    }
};