#pragma once
#include <iostream>
#include "/home/ljz/XXXVMP/Vmp/XXXInst.hpp"
enum fake_branch_opcode: uint16_t{
    FAKE_BRANCH_1=0x100,
    FAKE_BRANCH_2=0x101,
    FAKE_BRANCH_3=0x102,
    FAKE_BRANCH_4=0x103,
    FAKE_BRANCH_5=0x104,
    FAKE_BRANCH_6=0x105,
    FAKE_BRANCH_7=0x106,
    FAKE_BRANCH_8=0x107,
    FAKE_BRANCH_9=0x108,
    FAKE_BRANCH_10=0x109,
    FAKE_BRANCH_11=0x10A,
    FAKE_BRANCH_12=0x10B,
    FAKE_BRANCH_13=0x10C,
    FAKE_BRANCH_14=0x10D,
    FAKE_BRANCH_15=0x10E,
    FAKE_BRANCH_16=0x10F,
    FAKE_BRANCH_17=0x110,
    FAKE_BRANCH_18=0x111,
    FAKE_BRANCH_19=0x112,
    FAKE_BRANCH_20=0x113,
    FAKE_BRANCH_21=0x114,
    FAKE_BRANCH_22=0x115,
    FAKE_BRANCH_23=0x116,
    FAKE_BRANCH_24=0x117,
    FAKE_BRANCH_25=0x118,
    FAKE_BRANCH_26=0x119,
    FAKE_BRANCH_27=0x11A,
    FAKE_BRANCH_28=0x11B,
    FAKE_BRANCH_29=0x11C,
    FAKE_BRANCH_30=0x11D,
    FAKE_BRANCH_31=0x11E,
    FAKE_BRANCH_32=0x11F,
    FAKE_BRANCH_33=0x120,
    FAKE_BRANCH_34=0x121,
    FAKE_BRANCH_35=0x122,
    FAKE_BRANCH_36=0x123,
    FAKE_BRANCH_37=0x124,
    FAKE_BRANCH_38=0x125,
    FAKE_BRANCH_39=0x126,
    FAKE_BRANCH_40=0x127,
    FAKE_BRANCH_41=0x128,
    FAKE_BRANCH_42=0x129,
    FAKE_BRANCH_43=0x12A,
    FAKE_BRANCH_44=0x12B,
    FAKE_BRANCH_45=0x12C,
    FAKE_BRANCH_46=0x12D,
    FAKE_BRANCH_47=0x12E,
    FAKE_BRANCH_48=0x12F,
    FAKE_BRANCH_49=0x130,
    FAKE_BRANCH_50=0x131,
    FAKE_BRANCH_51=0x132,
    FAKE_BRANCH_52=0x133,
    FAKE_BRANCH_53=0x134,
    FAKE_BRANCH_54=0x135,
    FAKE_BRANCH_55=0x136,
    FAKE_BRANCH_56=0x137,
    FAKE_BRANCH_57=0x138,
    FAKE_BRANCH_58=0x139,
    FAKE_BRANCH_59=0x13A,
    FAKE_BRANCH_60=0x13B,
};
#pragma pack(push, 1)
template <fake_branch_opcode T>
struct FakeBranchInst : public VMINST
{
    uint16_t inst = T;
    uint8_t inst_data[2];
    size_t get_size() override
    {
        return static_get_size();
    }
    static size_t static_get_size()
    {
        return 2;
    }
    FakeBranchInst() { inst_data[0] = inst; }
    FakeBranchInst(uint8_t *raw_data)
    {
        inst = raw_data[0];
    }
    uint8_t *get_data() override
    {
        return inst_data;
    }
    void to_string() const override
    {
        std::cout << "fake " << std::endl;
    }
    
};
#pragma pack(pop)
template <fake_branch_opcode T>
void handle_FakeBranchInst(FakeBranchInst<T> &inst)
{
    if(inst.inst!= T)
    {
        std::cout << "Error: Invalid opcode" << std::endl;
        return;
    }else{
        std::cout << "Fake branch instruction executed" << std::endl;
        return;
    }
}
