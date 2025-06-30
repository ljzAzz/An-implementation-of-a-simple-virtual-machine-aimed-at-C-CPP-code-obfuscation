#include "Linker.hpp"

void Linker::link_entry(){
    added_f_code.insert(std::make_pair("main", linked_code.size()));
    linked_code.insert(linked_code.end(), fctxs["main"]->emitter.buffer.begin(), fctxs["main"]->emitter.buffer.end());
    
    auto jump_table = fctxs["main"]->jump_tables;
    auto offset = 0;
    for(auto jt : jump_table){
        if(added_f_code.find(jt.fname) != added_f_code.end()){
            uint8_t *call_addr = new uint8_t[8];
            //小端序
            for(int i = 0; i < 8; i++){
                call_addr[i] = (added_f_code[jt.fname] >> (i * 8)) & 0xff;
            }
            std::memcpy(linked_code.data() + jt.call_addr + offset, call_addr, 8);
            continue;
        }
        uint64_t called_addr = linked_code.size();
        added_f_code.insert(std::make_pair(jt.fname, linked_code.size()));
        linked_code.insert(linked_code.end(), fctxs[jt.fname]->emitter.buffer.begin(), fctxs[jt.fname]->emitter.buffer.end());
        uint8_t *call_addr = new uint8_t[8];
        //小端序
        for(int i = 0; i < 8; i++){
            call_addr[i] = (called_addr >> (i * 8)) & 0xff;
        }
        std::cout << "called_addr: " << called_addr << std::endl;
        std::cout << "call_addr: " << jt.call_addr << std::endl;
        std::memcpy(linked_code.data() + jt.call_addr + offset, call_addr, 8);
        offset += linked_code.size();
    }
}

void Linker::link_fctxs(){
    for(auto fctx : fctxs){
        if(fctx.first == "main"){
            continue;
        }
        if(added_f_code.find(fctx.first) != added_f_code.end()){
            continue;
        }else{
            added_f_code.insert(std::make_pair(fctx.first, linked_code.size()));
            linked_code.insert(linked_code.end(), fctx.second->emitter.buffer.begin(), fctx.second->emitter.buffer.end());
        }
        auto jump_table = fctx.second->jump_tables;
        auto offset = 0;
        for(auto jt : jump_table){
            if(added_f_code.find(jt.fname) != added_f_code.end()){
                uint8_t *call_addr = new uint8_t[8];
                //小端序
                for(int i = 0; i < 8; i++){
                    call_addr[i] = (added_f_code[jt.fname] >> (i * 8)) & 0xff;
                }
                std::memcpy(linked_code.data() + jt.call_addr + offset, call_addr, 8);
                continue;
            }
            added_f_code.insert(std::make_pair(fctx.first, linked_code.size()));
            linked_code.insert(linked_code.end(), fctx.second->emitter.buffer.begin(), fctx.second->emitter.buffer.end());
            uint64_t called_addr = linked_code.size();
            linked_code.insert(linked_code.end(), fctxs[jt.fname]->emitter.buffer.begin(), fctxs[jt.fname]->emitter.buffer.end());
            uint8_t *call_addr = new uint8_t[8];
            //小端序
            for(int i = 0; i < 8; i++){
                call_addr[i] = (called_addr >> (i * 8)) & 0xff;
            }
            std::memcpy(linked_code.data() + jt.call_addr + offset, call_addr, 8);
            offset += linked_code.size();
        }
    }
}
std::vector<uint8_t> Linker::get_linked_code(){
    return linked_code;
}