#pragma once
#include <cstdint>
#include <iostream>
#include <cstring>
#include <string>
enum OpCode : uint8_t
{
    _push_r1 = 0x01,
    _pop_r2 = 0x02,
    _add_rsp_r2 = 0x03,
    _sub_rsp_r2 = 0x04,
    _push_rbp = 0x05,
    _pop_rbp = 0x06,
    _push_rsp = 0x07,
    _pop_rsp = 0x08,
    _mov_rsp_rbp = 0x09,
    _mov_rx_ry = 0x0a,
    _mov_imm_rx = 0x0b,
    _mov_mem_rx = 0x0c,
    _mov_rx_mem = 0x0d,
    _mov_mem_mem = 0x0e,
    _mov_stack_rx = 0x0f,
    _mov_rx_stack = 0x10,
    _mov_stack_stack = 0x11,
    _add_imm_rx = 0x12,
    _add_rx_ry = 0x13,
    _sub_imm_rx = 0x14,
    _sub_rx_ry = 0x15,
    _mul_imm_rx = 0x16,
    _mul_rx_ry = 0x17,
    _sdiv_imm_rx = 0x18,
    _sdiv_rx_ry = 0x19,
    _jmp_r4 = 0x1c,
    _call_external = 0x1d,
    _call_internal = 0x1e,
    _jz_r5 = 0x1f,
    _jnz_r5 = 0x20,
    _jg_r6 = 0x21,
    _jl_r6 = 0x22,
    _jge_r7 = 0x23,
    _jle_r7 = 0x24,
    _ret_r0 = 0x25,
    _nop = 0x26,
    _icmp_sgt_imm_rx = 0x28,
    _icmp_sgt_rx_ry = 0x29,
    _push_pc = 0x2a,
    _pop_pc = 0x2b,
    _mov_rbp_rsp = 0x2c,
    _push_rx_with_size = 0x2d,
    _halt = 0x2e,
    _return_NRVO = 0x2f,//NRVO（具名返回值优化）
    _udiv_imm_rx = 0x30,
    _udiv_rx_ry = 0x31,
    _fadd_imm_rx = 0x32,
    _fadd_rx_ry = 0x33,
    _fsub_imm_rx = 0x34,
    _fsub_rx_ry = 0x35,
    _fmul_imm_rx = 0x36,
    _fmul_rx_ry = 0x37,
    _fdiv_imm_rx = 0x38,
    _fdiv_rx_ry = 0x39,
    _icmp_sge_imm_rx = 0x3a,
    _icmp_sge_rx_ry = 0x3b,
    _icmp_slt_imm_rx = 0x3c,
    _icmp_slt_rx_ry = 0x3d,
    _icmp_sle_imm_rx = 0x3e,
    _icmp_sle_rx_ry = 0x3f,
    _icmp_eq_imm_rx = 0x40,
    _icmp_eq_rx_ry = 0x41,
    _icmp_ne_imm_rx = 0x42,
    _icmp_ne_rx_ry = 0x43,
    _icmp_ugt_imm_rx = 0x44,
    _icmp_ugt_rx_ry = 0x45,
    _icmp_uge_imm_rx = 0x46,
    _icmp_uge_rx_ry = 0x47,
    _icmp_ult_imm_rx = 0x48,
    _icmp_ult_rx_ry = 0x49,
    _icmp_ule_imm_rx = 0x4a,
    _icmp_ule_rx_ry = 0x4b,
    _fcmp_oeq_imm_rx = 0x4c,
    _fcmp_oeq_rx_ry = 0x4d,
    _fcmp_ogt_imm_rx = 0x4e,
    _fcmp_ogt_rx_ry = 0x4f,
    _fcmp_oge_imm_rx = 0x50,
    _fcmp_oge_rx_ry = 0x51,
    _fcmp_olt_imm_rx = 0x52,
    _fcmp_olt_rx_ry = 0x53,
    _fcmp_ole_imm_rx = 0x54,
    _fcmp_ole_rx_ry = 0x55,
    _fcmp_one_imm_rx = 0x56,
    _fcmp_one_rx_ry = 0x57,
    _fcmp_ord_imm_rx = 0x58,
    _fcmp_ord_rx_ry = 0x59,
    _fcmp_uno_imm_rx = 0x5a,
    _fcmp_uno_rx_ry = 0x5b,
    _fcmp_ueq_imm_rx = 0x5c,
    _fcmp_ueq_rx_ry = 0x5d,
    _fcmp_ugt_imm_rx = 0x5e,
    _fcmp_ugt_rx_ry = 0x5f,
    _fcmp_uge_imm_rx = 0x60,
    _fcmp_uge_rx_ry = 0x61,
    _fcmp_ult_imm_rx = 0x62,
    _fcmp_ult_rx_ry = 0x63,
    _fcmp_ule_imm_rx = 0x64,
    _fcmp_ule_rx_ry = 0x65,
    _fcmp_une_rx_ry = 0x66,
    _fcmp_une_imm_rx = 0x67,
    _fcmp_true_imm_rx = 0x68,
    _fcmp_true_rx_ry = 0x69,
    _ret_to_native = 0x6a,
    _load_from_external = 0x6b,
};

enum Reg : uint8_t
{
    r0 = 0,
    r1 = 1,
    r2 = 2,
    r3 = 3,
    r4 = 4,
    r5 = 5,
    r6 = 6,
    r7 = 7,
};

enum OpSize : uint8_t
{
    _8 = 0,
    _16 = 1,
    _32 = 2,
    _64 = 3,
};

enum OpType : uint8_t
{
    _sign_int = 0,
    _unsigned_int = 1,
    _float = 2,
    _double = 3,
};

#pragma pack(push, 1)
struct VMINST
{
    VMINST() = default;

    virtual size_t get_size() = 0;
    static size_t static_get_size()
    {
        return -1;
    }
    virtual uint8_t *get_data() = 0;
    virtual ~VMINST() = default;
    virtual void to_string() const = 0;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _PUSH_R1_Inst : public VMINST
{
    uint8_t inst = OpCode::_push_r1;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _PUSH_R1_Inst() { inst_data[0] = inst; }
    _PUSH_R1_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "push r1 " << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _POP_R2_Inst : public VMINST
{
    uint8_t inst = OpCode::_pop_r2;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }

    _POP_R2_Inst()
    {
        inst_data[0] = inst;
    }
    _POP_R2_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "pop r2" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _ADD_RSP_R2_Inst : public VMINST
{
    uint8_t inst = OpCode::_add_rsp_r2;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _ADD_RSP_R2_Inst()
    {
        inst_data[0] = inst;
    }
    _ADD_RSP_R2_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "add rsp r2 " << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _SUB_RSP_R2_Inst : public VMINST
{
    uint8_t inst = OpCode::_sub_rsp_r2;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _SUB_RSP_R2_Inst()
    {
        inst_data[0] = inst;
    }
    _SUB_RSP_R2_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "sub rsp r2 " << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _PUSH_RBP_Inst : public VMINST
{
    uint8_t inst = OpCode::_push_rbp;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _PUSH_RBP_Inst()
    {
        inst_data[0] = inst;
    }
    _PUSH_RBP_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "push rbp " << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _POP_RBP_Inst : public VMINST
{
    uint8_t inst = OpCode::_pop_rbp;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _POP_RBP_Inst()
    {
        inst_data[0] = inst;
    }
    _POP_RBP_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "pop rbp " << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _PUSH_RSP_Inst : public VMINST
{
    uint8_t inst = OpCode::_push_rsp;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _PUSH_RSP_Inst()
    {
        inst_data[0] = inst;
    }
    _PUSH_RSP_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "push rsp " << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _POP_RSP_Inst : public VMINST
{
    uint8_t inst = OpCode::_pop_rsp;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _POP_RSP_Inst()
    {
        inst_data[0] = inst;
    }
    _POP_RSP_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "pop rsp " << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MOV_RSP_RBP_Inst : public VMINST
{
    uint8_t inst = OpCode::_mov_rsp_rbp;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _MOV_RSP_RBP_Inst()
    {
        inst_data[0] = inst;
    }
    _MOV_RSP_RBP_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mov rsp rbp" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MOV_RX_RY_Inst : public VMINST
{
    uint8_t inst : 8 = OpCode::_mov_rx_ry;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _MOV_RX_RY_Inst(uint8_t src, uint8_t dst, uint8_t size) : src(src), dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = (src & 0x07) | ((dst & 0x38) << 3) | ((size & 0xc0) << 6);
    }
    _MOV_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        src = raw_data[1] & 0x07;
        dst = (raw_data[1] & 0x38) >> 3;
        size = (raw_data[1] & 0xc0) >> 6;
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mov r" << int(src) << "r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MOV_IMM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_mov_imm_rx;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _MOV_IMM_RX_Inst(uint8_t dst, uint8_t size, uint64_t imm1) : dst(dst), size(size), imm(imm1)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | (size << 3) | (nop << 5);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (imm1 >> (i * 8)) & 0xff;
        }
    }
    
    _MOV_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x07;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mov " << std::hex << imm << " r" << int(dst) << "" << std::dec << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MOV_STACK_STACK_Inst : public VMINST
{
    uint8_t inst = OpCode::_mov_stack_stack;
    int16_t src = 0; // offset
    int16_t dst = 0; // offset
    uint8_t size  = 0;
    bool push_or_write = 0; // 0: push 1: write
    uint8_t inst_data[6];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 7;
    }
    _MOV_STACK_STACK_Inst(int16_t src, int16_t dst, uint8_t size,bool push_or_write) : src(src), dst(dst), size(size),push_or_write(push_or_write)
    {
        inst_data[0] = inst;
        inst_data[1] = src & 0xff;
        inst_data[2] = (src >> 8) & 0xff;
        inst_data[3] = dst & 0xff;
        inst_data[4] = (dst >> 8) & 0xff;
        inst_data[5] = size;
        if(push_or_write)
            inst_data[6] = 1;
        else
            inst_data[6] = 0;
    }
    _MOV_STACK_STACK_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        src = raw_data[1] | (raw_data[2] << 8);
        dst = raw_data[3] | (raw_data[4] << 8);
        size = raw_data[5];
        push_or_write = raw_data[6];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mov stack[rbp - " << src << "] stack[rbp - " << dst << "]" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _ADD_IMM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_add_imm_rx;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _ADD_IMM_RX_Inst(uint8_t dst, uint8_t size, int64_t imm) :dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | (size << 3) | (nop << 5);
        this->imm = reinterpret_cast<uint64_t&>(imm);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (this->imm >> (i * 8)) & 0xff;
        }
    }
    _ADD_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x3;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "add " << imm << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _SUB_IMM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_sub_imm_rx;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _SUB_IMM_RX_Inst(uint8_t dst, uint8_t size, int64_t imm) : dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | (size << 3) | (nop << 5);
        this -> imm = reinterpret_cast<uint64_t&>(imm);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (this -> imm >> (i * 8)) & 0xff;
        }
    }
    _SUB_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = (raw_data[1] & 0x07);
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "sub " << imm << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MUL_IMM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_mul_imm_rx;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _MUL_IMM_RX_Inst(uint8_t dst, uint8_t size, int64_t imm) : dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | size << 3 | nop << 5;
        this -> imm = reinterpret_cast<uint64_t&>(imm);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (this->imm >> (i * 8)) & 0xff;
        }
    }
    _MUL_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x07;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mul " << imm << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _SDIV_IMM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_sdiv_imm_rx;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _SDIV_IMM_RX_Inst(uint8_t dst, uint8_t size, int64_t imm) : dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | (size << 3) | (nop << 5);
        this->imm = reinterpret_cast<uint64_t&>(imm);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (this->imm >> (i * 8)) & 0xff;
        }
    }
    _SDIV_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x07;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "sdiv " << imm << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)



#pragma pack(push, 1)
struct _JMP_R4_Inst : public VMINST
{
    uint8_t inst = OpCode::_jmp_r4;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _JMP_R4_Inst()
    {
        inst_data[0] = inst;
    }
    _JMP_R4_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "jmp r4" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _CALL_EXTERNAL_Inst : public VMINST
{
    uint8_t inst = OpCode::_call_external;
    uint64_t addr=0;
    uint8_t params=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _CALL_EXTERNAL_Inst(uint64_t addr, uint8_t params) : addr(addr), params(params)
    {
        inst_data[0] = inst;
        std::memcpy(inst_data + 1, &addr, 8);
        inst_data[9] = params;
    }
    _CALL_EXTERNAL_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        std::memcpy(&addr, raw_data + 1, 8);
        params = raw_data[9];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        char arr[8];

        arr[0] = addr & 0xFF;
        arr[1] = (addr >> 8)  & 0xFF;
        arr[2] = (addr >> 16) & 0xFF;
        arr[3] = (addr >> 24) & 0xFF;
        arr[4] = (addr >> 32) & 0xFF;
        arr[5] = (addr >> 40) & 0xFF; 
        std::string cxxstr = std::string(arr);
        std::cout << "call external :" << cxxstr << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _CALL_INTERNAL_Inst : public VMINST
{
    uint8_t inst = OpCode::_call_internal;
    uint64_t addr=0;
    uint8_t inst_data[9];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 9;
    }
    _CALL_INTERNAL_Inst(uint64_t addr) : addr(addr)
    {
        inst_data[0] = inst;
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 1] = (addr >> (i * 8)) & 0xff;
        }
    }
    _CALL_INTERNAL_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        addr = 0;
        for (int i = 0; i < 8; i++)
        {
            addr |= raw_data[i + 1] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "call internal " << addr << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _RET_R0_Inst : public VMINST
{
    uint8_t inst = OpCode::_ret_r0;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _RET_R0_Inst()
    {
        inst_data[0] = inst;
    }
    _RET_R0_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "ret r0" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _JZ_R5_Inst : public VMINST
{
    uint8_t inst = OpCode::_jz_r5;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _JZ_R5_Inst()
    {
        inst_data[0] = inst;
    }
    _JZ_R5_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "jz r5" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _JNZ_R5_Inst : public VMINST
{
    uint8_t inst = OpCode::_jnz_r5;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _JNZ_R5_Inst()
    {
        inst_data[0] = inst;
    }
    _JNZ_R5_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "jnz r5" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _JL_R6_Inst : public VMINST
{
    uint8_t inst = OpCode::_jl_r6;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _JL_R6_Inst()
    {
        inst_data[0] = inst;
    }
    _JL_R6_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "jl r6" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _JLE_R7_Inst : public VMINST
{
    uint8_t inst = OpCode::_jle_r7;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _JLE_R7_Inst()
    {
        inst_data[0] = inst;
    }
    _JLE_R7_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "jle r7" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _JG_R6_Inst : public VMINST
{
    uint8_t inst = OpCode::_jg_r6;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _JG_R6_Inst()
    {
        inst_data[0] = inst;
    }
    _JG_R6_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "jg r6" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _JGE_R7_Inst : public VMINST
{
    uint8_t inst = OpCode::_jge_r7;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _JGE_R7_Inst()
    {
        inst_data[0] = inst;
    }
    _JGE_R7_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "jge r7" << std::endl;
    }
};
#pragma pack(pop)



#pragma pack(push, 1)
struct _MOV_MEM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_mov_mem_rx;
    uint64_t addr = 0;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _MOV_MEM_RX_Inst(uint64_t addr, uint8_t dst, uint8_t size) : addr(addr), dst(dst), size(size)
    {
        inst_data[0] = inst;
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 1] = (addr >> (i * 8)) & 0xff;
        }
        inst_data[9] = dst | (size << 3) | (nop << 5);
    }
    _MOV_MEM_RX_Inst(uint8_t *raw_data)
    {
        addr=0;
        inst = raw_data[0];
        for (int i = 0; i < 8; i++)
        {
            addr |= raw_data[i + 1] << (i * 8);
        }
        dst = raw_data[9] & 0x07;
        size = (raw_data[9] & 0x18) >> 3;
        nop = (raw_data[9] & 0xe0) >> 5;
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mov mem[" << addr << "] r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MOV_RX_MEM_Inst : public VMINST
{
    uint8_t inst = OpCode::_mov_rx_mem;
    uint64_t addr=0;
    uint8_t src : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _MOV_RX_MEM_Inst(uint64_t addr, uint8_t src, uint8_t size) : addr(addr), src(src), size(size)
    {
        inst_data[0] = inst;
        std::memcpy(inst_data + 1, &addr, 8);
        inst_data[9] = src | (size << 3) | (nop << 5);
    }
    _MOV_RX_MEM_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        std::memcpy(&addr, raw_data + 1, 8);
        src = raw_data[9] & 0x07;
        size = (raw_data[9] & 0x18) >> 3;
        nop = (raw_data[9] & 0xe0) >> 5;
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mov r" << int(src) << " mem[" << addr << "] " << inst << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MOV_MEM_MEM_Inst : public VMINST
{
    uint8_t inst = OpCode::_mov_mem_mem;
    uint64_t src;
    uint64_t dst;
    uint8_t size = 0;
    uint8_t inst_data[18];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 18;
    }
    _MOV_MEM_MEM_Inst(uint64_t src, uint64_t dst, uint8_t size) : src(src), dst(dst), size(size)
    {
        inst_data[0] = inst;
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 1] = (src >> (i * 8)) & 0xff;
        }
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 9] = (dst >> (i * 8)) & 0xff;
        }
        inst_data[17] = size;
    }
    _MOV_MEM_MEM_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        src = 0;
        dst = 0;
        for (int i = 0; i < 8; i++)
        {
            src |= raw_data[i + 1] << (i * 8);
        }
        for (int i = 0; i < 8; i++)
        {
            dst |= raw_data[i + 9] << (i * 8);
        }
        size = raw_data[17];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mov mem[" << src << "] mem[" << dst << "]" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MOV_STACK_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_mov_stack_rx;
    int16_t src = 0; // offset
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint8_t inst_data[4];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 4;
    }
    _MOV_STACK_RX_Inst(int64_t src, uint8_t dst, uint8_t size) : src(static_cast<int16_t>(src)), dst(dst), size(size)
    {
        auto usrc = static_cast<uint16_t>(src);
        inst_data[0] = inst;
        inst_data[1] = usrc & 0xff;
        inst_data[2] = (usrc >> 8) & 0xff;
        inst_data[3] = dst | (size << 3) | (nop << 5);
    }
    _MOV_STACK_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        src = raw_data[1] | (raw_data[2] << 8);
        dst = raw_data[3] & 0x07;
        size = (raw_data[3] & 0x18) >> 3;
        nop = (raw_data[3] & 0xe0) >> 5;
        src = static_cast<int16_t>(src);
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mov stack[rbp - " << src << "] r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MOV_RX_STACK_Inst : public VMINST
{
    uint8_t inst = OpCode::_mov_rx_stack;
    uint8_t src : 3 = 0;
    int16_t dst = 0; // offset
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint8_t inst_data[4];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 4;
    }
    _MOV_RX_STACK_Inst(uint8_t src, int16_t dst, uint8_t size) : src(src), dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = src | (dst << 3);
        inst_data[2] = (dst >> 5) & 0xff;
        inst_data[3] = (dst >> 13) & 0xff | (size << 3) | (nop << 5);
    }
    _MOV_RX_STACK_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        src = raw_data[1] & 0x07;
        dst = raw_data[1] >> 3 | (raw_data[2] << 5) | (raw_data[3] << 13);
        size = (raw_data[3] & 0x18) >> 3;
        nop = (raw_data[3] & 0xe0) >> 5;
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mov r" << int(src) << " stack[rbp - " << dst << "]" << std::endl;
    }
};
#pragma pack(pop)


#pragma pack(push, 1)
struct _PUSH_PC_Inst: public VMINST
{
    uint8_t inst = OpCode::_push_pc;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _PUSH_PC_Inst()
    {
        inst_data[0] = inst;
    }
    _PUSH_PC_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "push pc" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _POP_PC_Inst: public VMINST
{
    uint8_t inst = OpCode::_pop_pc;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _POP_PC_Inst()
    {
        inst_data[0] = inst;
    }
    _POP_PC_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "pop pc" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MOV_RBP_RSP_Inst : public VMINST
{
    uint8_t inst = OpCode::_mov_rbp_rsp;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _MOV_RBP_RSP_Inst()
    {
        inst_data[0] = inst;
    }
    _MOV_RBP_RSP_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mov rbp rsp" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _PUSH_RX_WITH_SIZE_Inst : public VMINST
{
    uint8_t inst = OpCode::_push_rx_with_size;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _PUSH_RX_WITH_SIZE_Inst(uint8_t dst, uint8_t size) : dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | size << 3 | nop << 5;

    }
    _PUSH_RX_WITH_SIZE_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x07;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "push r1 with size " << int(size) << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _HALT_Inst : public VMINST
{
    uint8_t inst = OpCode::_halt;
    uint8_t inst_data[1];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 1;
    }
    _HALT_Inst()
    {
        inst_data[0] = inst;
    }
    _HALT_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "halt" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _ADD_RX_RY_Inst: public VMINST
{
    uint8_t inst = OpCode::_add_rx_ry;
    uint8_t size : 2 = 0;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _ADD_RX_RY_Inst(uint8_t size, uint8_t src, uint8_t dst) : size(size), src(src), dst(dst)
    {
        inst_data[0] = inst;
        inst_data[1] = size | src << 2 | dst << 5;
    }
    _ADD_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1] & 0x03;
        src = (raw_data[1] & 0x1c) >> 2;
        dst = (raw_data[1] & 0xe0) >> 5;

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "add r" << int(src) << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)


#pragma pack(push, 1)
struct _RETURN_NRVO_Inst : public VMINST
{
    uint8_t inst = OpCode::_return_NRVO;
    uint8_t size = 0;
    uint8_t s_or_m : 1 = 0;
    uint8_t nop:7 = 0;
    int64_t add_or_offset = 0;
    int64_t ret_addr = 0;
    uint8_t inst_data[19];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 19;
    }
    _RETURN_NRVO_Inst(uint8_t size, int64_t add_or_offset, int64_t ret_addr, uint8_t s_or_m) : size(size), add_or_offset(add_or_offset),ret_addr(ret_addr),s_or_m(s_or_m)
    {
        inst_data[0] = inst;
        inst_data[1] = size;
        inst_data[2] = s_or_m | nop << 7;
        std::memcpy(inst_data + 3, &add_or_offset, 8);
        std::memcpy(inst_data + 11, &ret_addr, 8);

    }
    _RETURN_NRVO_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1];
        s_or_m = raw_data[2] & 0x01;
        nop = raw_data[2] & 0xfe >> 7;
        std::memcpy(&add_or_offset, raw_data + 3, 8);
        std::memcpy(&ret_addr, raw_data + 11, 8);

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "return nrvo :";
        std::cout << " size: " << int(size);
        std::cout << ", s_or_m: " << int(s_or_m);
        std::cout << ", add_or_offset: " << add_or_offset;
        std::cout << ", ret_addr: " << ret_addr << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _SUB_RX_RY_Inst: public VMINST
{
    uint8_t inst = OpCode::_sub_rx_ry;
    uint8_t size : 2 = 0;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _SUB_RX_RY_Inst(uint8_t size, uint8_t src, uint8_t dst) : size(size), src(src), dst(dst)
    {
        inst_data[0] = inst;
        inst_data[1] = size | src << 2 | dst << 5;
    }
    _SUB_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1] & 0x03;
        src = raw_data[1] & 0x1c >> 2;
        dst = raw_data[1] & 0xe0 >> 5;

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "sub r" << int(src) << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _MUL_RX_RY_Inst: public VMINST
{
    uint8_t inst = OpCode::_mul_rx_ry;
    uint8_t size : 2 = 0;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _MUL_RX_RY_Inst(uint8_t size, uint8_t src, uint8_t dst) : size(size), src(src), dst(dst)
    {
        inst_data[0] = inst;
        inst_data[1] = size | src << 2 | dst << 5;
    }
    _MUL_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1] & 0x03;
        src = raw_data[1] & 0x1c >> 2;
        dst = raw_data[1] & 0xe0 >> 5;

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "mul r" << int(src) << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _SDIV_RX_RY_Inst: public VMINST
{
    uint8_t inst = OpCode::_sdiv_rx_ry;
    uint8_t size : 2 = 0;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _SDIV_RX_RY_Inst(uint8_t size, uint8_t src, uint8_t dst) : size(size), src(src), dst(dst)
    {
        inst_data[0] = inst;
        inst_data[1] = size | src << 2 | dst << 5;
    }
    _SDIV_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1] & 0x03;
        src = raw_data[1] & 0x1c >> 2;
        dst = raw_data[1] & 0xe0 >> 5;

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "sdiv r" << int(src) << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _UDIV_RX_RY_Inst: public VMINST
{
    uint8_t inst = OpCode::_udiv_rx_ry;
    uint8_t size : 2 = 0;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _UDIV_RX_RY_Inst(uint8_t size, uint8_t src, uint8_t dst) : size(size), src(src), dst(dst)
    {
        inst_data[0] = inst;
        inst_data[1] = size | src << 2 | dst << 5;
    }
    _UDIV_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1] & 0x03;
        src = raw_data[1] & 0x1c >> 2;
        dst = raw_data[1] & 0xe0 >> 5;

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "udiv r" << int(src) << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _UDIV_IMM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_udiv_imm_rx;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _UDIV_IMM_RX_Inst(uint8_t dst, uint8_t size, int64_t imm) : dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | (size << 3) | (nop << 5);
        this->imm = reinterpret_cast<uint64_t &>(imm);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (imm >> (i * 8)) & 0xff;
        }
    }
    _UDIV_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x07;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "udiv " << imm << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _FADD_RX_RY_Inst: public VMINST
{
    uint8_t inst = OpCode::_fadd_rx_ry;
    uint8_t size : 2 = 0;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _FADD_RX_RY_Inst(uint8_t size, uint8_t src, uint8_t dst) : size(size), src(src), dst(dst)
    {
        inst_data[0] = inst;
        inst_data[1] = size | src << 2 | dst << 5;
    }
    _FADD_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1] & 0x03;
        src = raw_data[1] & 0x1c >> 2;
        dst = raw_data[1] & 0xe0 >> 5;

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "fadd r" << int(src) << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _FADD_IMM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_fadd_imm_rx;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _FADD_IMM_RX_Inst(uint8_t dst, uint8_t size, int64_t imm) : dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | (size << 3) | (nop << 5);
        this->imm = reinterpret_cast<uint64_t &>(imm);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (imm >> (i * 8)) & 0xff;
        }
    }
    _FADD_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x07;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "fadd " << imm << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _FSUB_RX_RY_Inst: public VMINST
{
    uint8_t inst = OpCode::_fsub_rx_ry;
    uint8_t size : 2 = 0;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _FSUB_RX_RY_Inst(uint8_t size, uint8_t src, uint8_t dst) : size(size), src(src), dst(dst)
    {
        inst_data[0] = inst;
        inst_data[1] = size | src << 2 | dst << 5;
    }
    _FSUB_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1] & 0x03;
        src = raw_data[1] & 0x1c >> 2;
        dst = raw_data[1] & 0xe0 >> 5;

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "fsub r" << int(src) << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _FSUB_IMM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_fsub_imm_rx;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _FSUB_IMM_RX_Inst(uint8_t dst, uint8_t size, int64_t imm) : dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | (size << 3) | (nop << 5);
        this->imm = reinterpret_cast<uint64_t &>(imm);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (imm >> (i * 8)) & 0xff;
        }
    }
    _FSUB_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x07;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "fsub " << imm << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)


#pragma pack(push, 1)
struct _FMUL_RX_RY_Inst: public VMINST
{
    uint8_t inst = OpCode::_fmul_rx_ry;
    uint8_t size : 2 = 0;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _FMUL_RX_RY_Inst(uint8_t size, uint8_t src, uint8_t dst) : size(size), src(src), dst(dst)
    {
        inst_data[0] = inst;
        inst_data[1] = size | src << 2 | dst << 5;
    }
    _FMUL_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1] & 0x03;
        src = raw_data[1] & 0x1c >> 2;
        dst = raw_data[1] & 0xe0 >> 5;

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "fmul r" << int(src) << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _FMUL_IMM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_fmul_imm_rx;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _FMUL_IMM_RX_Inst(uint8_t dst, uint8_t size, int64_t imm) : dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | (size << 3) | (nop << 5);
        this->imm = reinterpret_cast<uint64_t &>(imm);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (imm >> (i * 8)) & 0xff;
        }
    }
    _FMUL_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x07;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "fmul " << imm << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _FDIV_RX_RY_Inst: public VMINST
{
    uint8_t inst = OpCode::_fdiv_rx_ry;
    uint8_t size : 2 = 0;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _FDIV_RX_RY_Inst(uint8_t size, uint8_t src, uint8_t dst) : size(size), src(src), dst(dst)
    {
        inst_data[0] = inst;
        inst_data[1] = size | src << 2 | dst << 5;
    }
    _FDIV_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1] & 0x03;
        src = raw_data[1] & 0x1c >> 2;
        dst = raw_data[1] & 0xe0 >> 5;

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "fdiv r" << int(src) << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _FDIV_IMM_RX_Inst : public VMINST
{
    uint8_t inst = OpCode::_fdiv_imm_rx;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _FDIV_IMM_RX_Inst(uint8_t dst, uint8_t size, int64_t imm) : dst(dst), size(size)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | (size << 3) | (nop << 5);
        this->imm = reinterpret_cast<uint64_t &>(imm);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (imm >> (i * 8)) & 0xff;
        }
    }
    _FDIV_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x07;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "fdiv " << imm << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
template <OpCode cmp=OpCode::_nop>
struct _CMP_IMM_RX_Inst : public VMINST
{
    uint8_t inst = cmp;
    uint8_t dst : 3 = 0;
    uint8_t size : 2 = 0;
    uint8_t nop : 3 = 0;
    uint64_t imm=0;
    uint8_t inst_data[10];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _CMP_IMM_RX_Inst(uint8_t dst, uint8_t size, uint64_t imm) : dst(dst), size(size), imm(imm)
    {
        inst_data[0] = inst;
        inst_data[1] = dst | (size << 3) | (nop << 5);
        for (int i = 0; i < 8; i++)
        {
            inst_data[i + 2] = (imm >> (i * 8)) & 0xff;
        }
    }
    _CMP_IMM_RX_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        dst = raw_data[1] & 0x07;
        size = (raw_data[1] & 0x18) >> 3;
        nop = (raw_data[1] & 0xe0) >> 5;
        imm = 0;
        for (int i = 0; i < 8; i++)
        {
            imm |= (uint64_t)raw_data[i + 2] << (i * 8);
        }
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "cmp " << imm << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
template <OpCode cmp=OpCode::_nop>
struct _CMP_RX_RY_Inst : public VMINST
{
    uint8_t inst = cmp;
    uint8_t size : 2 = 0;
    uint8_t src : 3 = 0;
    uint8_t dst : 3 = 0;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    _CMP_RX_RY_Inst(uint8_t size, uint8_t src, uint8_t dst) : size(size), src(src), dst(dst)
    {
        inst_data[0] = inst;
        inst_data[1] = size | src << 2 | dst << 5;
    }
    _CMP_RX_RY_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        size = raw_data[1] & 0x03;
        src = raw_data[1] & 0x1c >> 2;
        dst = raw_data[1] & 0xe0 >> 5;

    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "cmp r" << int(src) << " r" << int(dst) << "" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _RET_TO_NATIVE_Inst : public VMINST
{
    uint8_t inst = OpCode::_ret_to_native;
    uint8_t inst_data[10];
    uint64_t offset = 0;
    uint8_t size = 0;
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 10;
    }
    _RET_TO_NATIVE_Inst(uint64_t offset,uint8_t size)
    {
        inst_data[0] = inst;
        for(int i = 0; i < 8; i++)
        {
            inst_data[i + 1] = (offset >> (i * 8)) & 0xff;
        }
        inst_data[9] = size;
    }

    _RET_TO_NATIVE_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        offset = 0;
        for (int i = 0; i < 8; i++)
        {
            offset |= (uint64_t)raw_data[i + 1] << (i * 8);
        }
        size = raw_data[9];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "ret mem[" << offset << "] size: " << int(size) << " to native" << std::endl;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _LOAD_FROM_EXTERNAL_Inst : public VMINST
{
    uint8_t inst = OpCode::_load_from_external;
    uint8_t inst_data[18];

    uint64_t offset = 0;
    uint64_t addr = 0;
    uint8_t size = 0;
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 18;
    }
    _LOAD_FROM_EXTERNAL_Inst(uint64_t offset,uint64_t addr,uint8_t size)
    {
        inst_data[0] = inst; 
        for(int i = 0; i < 8; i++)
        {
            inst_data[i + 1] = (offset >> (i * 8)) & 0xff;
        }
        for(int i = 0; i < 8; i++)
        {
            inst_data[i + 9] = (addr >> (i * 8)) & 0xff;
        }
        inst_data[17] = size;

    }
    _LOAD_FROM_EXTERNAL_Inst(uint8_t *raw_data)
    {
        inst = raw_data[0];
        offset = 0;
        for (int i = 0; i < 8; i++)
        {
            offset |= (uint64_t)raw_data[i + 1] << (i * 8);
        }
        addr = 0;
        for (int i = 0; i < 8; i++)
        {
            addr |= (uint64_t)raw_data[i + 9] << (i * 8);
        }
        size = raw_data[17];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        char arr[8];

        arr[0] = offset & 0xFF;
        arr[1] = (offset >> 8)  & 0xFF;
        arr[2] = (offset >> 16) & 0xFF;
        arr[3] = (offset >> 24) & 0xFF;
        arr[4] = (offset >> 32) & 0xFF;
        arr[5] = (offset >> 40) & 0xFF; 
        arr[6] = (offset >> 48) & 0xFF;
        arr[7] = (offset >> 56) & 0xFF;
        std::string cxxstr = std::string(arr);
        std::cout << "load from external[" << cxxstr << "_::res] size: " << int(size) << std::endl;
    }
};
#pragma pack(pop)