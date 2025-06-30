#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#include "XXXVMPCTX.h"
constexpr const char *filename = "XXXins_new.bin";
class Linker
{
private:

    std::vector<uint8_t> linked_code;
    std::unordered_map<std::string,std::shared_ptr<class FunCTX>> fctxs;
    std::unordered_map<std::string,uint64_t> added_f_code;
public:
    Linker() = delete;
    Linker(std::unordered_map<std::string,std::shared_ptr<class FunCTX>> fctxs){
        this->fctxs = fctxs;
    };
    void link_entry();
    void link_fctxs();
    std::vector<uint8_t> get_linked_code();
    void dump(){
        std::ofstream out(filename,std::ios::binary);
        out.write((char*)linked_code.data(),linked_code.size());
        out.close();
    }
};