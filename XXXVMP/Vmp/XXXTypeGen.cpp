#include "XXXTypeGen.hpp"
#include "XXXVMPCTX.h"

extern class MemoryManager Mem;
extern class Stack Stack;
extern class ExternalCollector ExCollector;
extern std::set<std::string> globalUsed;
void TypeCollector::addType(XXXType type)
{
    TypeSet.insert(type);
}

std::set<XXXType> TypeCollector::getTypeSet(){
    return TypeSet;
}

void TypeCollector::printTypeSet(){
    for(auto type:TypeSet){
        if(type.size==std::nullopt){
            std::cout << "[XXXTypeGen]: Type_Name: " << type.Type_name << " Size: " << "unknown" << " Align: " << "unknown" << std::endl;
        }else{
            std::cout << "[XXXTypeGen]: Type_Name: " << type.Type_name << " Size: " << type.size.value() << " Align: " << type.align.value() << std::endl;
        }
        for(auto t:type.Type){
            if(t.size==std::nullopt){
                std::cout << "[XXXTypeGen]: \tType: " << t.Type_name << " Size: " << "unknown" << " Align: " << "unknown" << std::endl;
            }else{
                std::cout << "[XXXTypeGen]: \tType: " << t.Type_name << " Size: " << t.size.value() << " Align: " << t.align.value() << std::endl;
            }
        }
    }
}
void TypeCollector::Gen(llvm::Module &M){
    int unname_count = 0;
    auto TypeInfo = M.getIdentifiedStructTypes();
    auto DL = M.getDataLayout();
    for(auto type:TypeInfo){
        XXXType newType;
        std::string type_name;
        llvm::raw_string_ostream OS(type_name);
        type->print(OS);
        OS.flush();
        newType.Type_name = type->getName().str().empty()?type_name:type->getName().str();

        for(auto element:type->elements()){
            std::string element_name;
            llvm::raw_string_ostream OS(element_name);
            auto struct_type = llvm::dyn_cast<llvm::StructType>(element);
            if(struct_type){
                std::string type_name;
                llvm::raw_string_ostream OS(type_name);
                struct_type->print(OS);
                OS.flush();
                element_name = struct_type->getName().str().empty()?type_name:struct_type->getName().str();
            }else{
                element->print(OS);
                OS.flush();
                
            }
            auto element_type = XXXType();
            element_type.Type_name = element_name;
            element_type.size = element->isSized()?std::optional<_TypeSize>(DL.getTypeAllocSize(element)):std::nullopt;
            element_type.align = DL.getABITypeAlign(element).value();
            newType.Type.push_back(element_type);
        }
        if(type->isSized()){
            newType.size = DL.getTypeAllocSize(type);
            auto align = DL.getABITypeAlign(type).value();
            newType.align = align;
        }else{
            newType.size = std::nullopt;
        }
        
        addType(newType);
    }
    for(auto &func:M.functions()){
        if(ExCollector.isUserFunction(func.getName().str())){
            for(auto &BB:func){
                for(auto &I:BB){
                    auto type = I.getType();
                    if(type->isStructTy()){
                        auto struct_type = llvm::dyn_cast<llvm::StructType>(type);
                        auto align = DL.getABITypeAlign(struct_type).value();
                        std::string type_name;
                        llvm::raw_string_ostream OS(type_name);
                        struct_type->print(OS);
                        OS.flush();
                        XXXType newType;
                        newType.Type_name = struct_type->getName().str().empty()?type_name:struct_type->getName().str();
                        auto size = DL.getTypeAllocSize(struct_type);
                        newType.size = size;
                        newType.align = align;
                        for(auto element:struct_type->elements()){
                            std::string element_name;
                            llvm::raw_string_ostream OS(element_name);
                            auto struct_type = llvm::dyn_cast<llvm::StructType>(element);
                            if(struct_type){
                                std::string type_name;
                                llvm::raw_string_ostream OS(type_name);
                                struct_type->print(OS);
                                OS.flush();
                                element_name = struct_type->getName().str().empty()?type_name:struct_type->getName().str();
                            }else{
                                element->print(OS);
                                OS.flush();
                            }
                            auto element_type = XXXType();
                            element_type.Type_name = element_name;
                            element_type.size = element->isSized()?std::optional<_TypeSize>(DL.getTypeAllocSize(element)):std::nullopt;
                            element_type.align = DL.getABITypeAlign(element).value();
                            newType.Type.push_back(element_type);
                        }
                        addType(newType);
                    }else {
                        XXXType newType;
                        std::string type_name;
                        llvm::raw_string_ostream OS(type_name);
                        type->print(OS);
                        OS.flush();
                        newType.Type_name = type_name;
                        
                        
                        if(type->isSized()){
                            newType.size = DL.getTypeAllocSize(type);
                            auto align = DL.getABITypeAlign(type).value();
                            newType.align = align;
                        }else{
                            newType.size = std::nullopt;
                            newType.align = std::nullopt;
                        }
                        addType(newType);
                    }
                }
            }
        }
    }
    for(auto &Global:M.globals()){
        auto name = Global.getName().str();
        if(ExCollector.is_v_external(name)||ExCollector.is_f_external(name)||name.find("InterFace")!=std::string::npos||name.find("llvm.")!=std::string::npos){
            continue;
        }
        auto type = Global.getType();
        if(type->isStructTy()){
            auto struct_type = llvm::dyn_cast<llvm::StructType>(type);
            auto align = DL.getABITypeAlign(struct_type).value();
            XXXType newType;
            std::string type_name;
            llvm::raw_string_ostream OS(type_name);
            if(struct_type->getName().str().empty()){
                type->print(OS);
                OS.flush();
            }else{
                type_name = struct_type->getName().str();
            }
            newType.Type_name = type_name;
            auto size = DL.getTypeAllocSize(struct_type);
            newType.size = size;
            newType.align = align;
            for(auto element:struct_type->elements()){
                std::string element_name;
                llvm::raw_string_ostream OS(element_name);
                auto struct_type = llvm::dyn_cast<llvm::StructType>(element);
                if(struct_type){
                    std::string _element_name;
                    llvm::raw_string_ostream OS(_element_name);
                    if(struct_type->getName().str().empty()){
                        element->print(OS);
                        OS.flush();
                    }else{
                        _element_name = struct_type->getName().str();
                    }
                    element_name = _element_name;
                }else{
                    element->print(OS);
                    OS.flush();
                }
                auto element_type = XXXType();
                element_type.Type_name = element_name;
                element_type.size = element->isSized()?std::optional<_TypeSize>(DL.getTypeAllocSize(element)):std::nullopt;
                element_type.align = DL.getABITypeAlign(element).value();
                newType.Type.push_back(element_type);
            }
            addType(newType);
        }else {
            XXXType newType;
            std::string type_name;
            llvm::raw_string_ostream OS(type_name);
            type->print(OS);
            OS.flush();
            newType.Type_name = type_name;
            
            
            if(type->isSized()){
                newType.size = DL.getTypeAllocSize(type);
                auto align = DL.getABITypeAlign(type).value();
                newType.align = align;
            }else{
                newType.size = std::nullopt;
                newType.align = std::nullopt;
            }
            addType(newType);
        }
    }
    printTypeSet();
}

XXXType TypeCollector::getType(TypeName type_name){
    for(auto type:TypeSet){
        if(type.Type_name == type_name){
            return type;
        }else{
            for(auto t:type.Type){
                if(t.Type_name == type_name){
                    return t;
                }
            }
        }
    }
    return XXXType();
}

void GenSymbolTable::addSymbol(field_name f, _SymbolsTable s){
    SymbolTable_m.insert({f,s});
}

void GenSymbolTable::printSymbolTable(){
    for(auto symbol:SymbolTable_m){
        std::cout << "[XXXTypeGen]: Field Name: " << symbol.first << std::endl;
        for(auto s:symbol.second){
            std::cout << "[XXXTypeGen]: \tSymbol: "  << s.second.second.first << "  description: " << s.second.first << "  Size: " << ((s.second.second.second.size==std::nullopt)?"unkown":std::to_string(s.second.second.second.size.value())) << "  align: " << ((s.second.second.second.align==std::nullopt)?"unkown":std::to_string(s.second.second.second.align.value())) << "  addr: " << s.first << "  type: " << s.second.second.second.Type_name << std::endl;
        }
    }
}

void GenSymbolTable::addBB(llvm::BasicBlock* bb){
    BBs.push_back(bb);
}
void GenSymbolTable::Gen(llvm::Module &M, TypeCollector& tc){
    _SymbolsTable gnewTable;
    for(auto &func:M.functions()){
        if(ExCollector.isUserFunction(func.getName().str())){
            for(auto &bb:func){
                addBB(&bb);
            }
        }
    }
    for(auto &Global:M.globals()){
        auto global_name = Global.getName().str();
        if(ExCollector.is_v_external(global_name)||ExCollector.is_f_external(global_name)){
            continue;
        }
        bool used = false;
        for(auto bb:BBs){
            if(Global.isUsedInBasicBlock(bb)){
                used = true;
                break;
            }
        }
        if(!used){
            continue;
        }
        globalUsed.insert(global_name);
        auto type = Global.getType();
        std::string type_name;
        llvm::raw_string_ostream OS(type_name);
        type->print(OS);
        OS.flush();
        auto type_info = tc.getType(type_name);
        if(type_info.size==std::nullopt){
            continue;
        }else{
            auto DL = M.getDataLayout();
            auto actual_type = Global.getValueType();
            auto size = DL.getTypeAllocSize(actual_type);
            auto align = DL.getABITypeAlign(actual_type).value();
            type_info.size = size;
            type_info.align = align;
        }
        auto addr = Mem.allocate(type_info.size.value(),type_info.align.value());
        
        gnewTable.insert({addr,std::make_pair("global variable",std::make_pair(global_name,type_info))});
        
    }
    addSymbol("global",gnewTable);
    
    for(auto &func:M.functions()){
        auto offset = 0;
        _SymbolsTable newTable;
        if(ExCollector.isUserFunction(func.getName().str())){
            if(func.arg_size()==0){
                
            }else{
                
                auto argcount = 0;
                auto iterator = func.arg_end();
                do{
                    iterator--;
                    auto &arg = *iterator;
                    auto type = arg.getType();
                    std::string arg_name;
                    llvm::raw_string_ostream AOS(arg_name);
                    std::string type_name;
                    arg_name = arg.getName().str();
                    llvm::outs() << "[XXXTypeGen]: Type Name: " << arg_name << "\n";
                    if(arg_name.empty()){
                        arg.printAsOperand(AOS, false);
                        AOS.flush();
                    }
                    llvm::raw_string_ostream OS1(type_name);
                    if(type->isStructTy()){
                        type_name = llvm::dyn_cast<llvm::StructType>(type)->getName().str();
                    }else{
                        type->print(OS1);
                        OS1.flush();
                    }
                    auto type_info = tc.getType(type_name);
                    if(type_info.size==std::nullopt){
                        llvm::outs() << "[XXXTypeGen]: when handle arg "<<arg_name<<"  type_name is "<<type_info.Type_name<<" size is null\n";
                        abort();
                    }
                    auto addr = -16-offset;
                    offset += type_info.size.value();
                    newTable.insert({addr,std::make_pair("arg"+std::to_string(argcount++),std::make_pair(arg_name,type_info))});
                }while(iterator!=func.arg_begin());
            }

            offset = 0;
            for(auto &BB:func){
                for(auto &I:BB){
                    if(I.getType()->isVoidTy()){
                        continue;
                    }
                    bool mem = true;
                    std::string instNameWithAddr;
                    llvm::raw_string_ostream OS(instNameWithAddr);
                    I.printAsOperand(OS, false);
                    OS.flush();
                    std::cout << "[XXXTypeGen]: Inst Name: " << instNameWithAddr << std::endl;
                    auto type = I.getType();
                    std::string type_name;
                    
                    llvm::raw_string_ostream OS2(type_name);
                    
                    if(llvm::dyn_cast<llvm::AllocaInst>(&I)){
                        type = llvm::dyn_cast<llvm::AllocaInst>(&I)->getAllocatedType();
                        mem = false;
                    }else if(llvm::dyn_cast<llvm::LoadInst>(&I)){
                        type = llvm::dyn_cast<llvm::LoadInst>(&I)->getType();
                    }else if(llvm::dyn_cast<llvm::StoreInst>(&I)){
                        type = llvm::dyn_cast<llvm::StoreInst>(&I)->getType();
                    }
                    if(type->isStructTy()){
                        type_name = llvm::dyn_cast<llvm::StructType>(type)->getName().str();
                    }
                    if(type_name.empty()){
                        type->print(OS2);
                        OS2.flush();
                    }

                    auto type_info = tc.getType(type_name);
                    if(type_info.Type_name=="void"){
                        std::cout<<"[XXXTypeGen]: when handle "<<instNameWithAddr<<"  type_name is void"<<std::endl;
                        continue;
                    }
                    else if(type_info.Type_name.empty()){
                        std::cout<<"[XXXTypeGen]: when handle "<<instNameWithAddr<<"  type_name is empty"<<std::endl;
                        abort();
                    }else if(type_info.size==std::nullopt){
                        std::cout<<"[XXXTypeGen]: when handle "<<instNameWithAddr<<"  type_name is "<<type_info.Type_name<<" size is null"<<std::endl;
                        abort();
                    }
                    if(mem==false){
                        int64_t addr = offset + (type_info.size==std::nullopt?8:type_info.size.value());
                        newTable.insert({addr,std::make_pair("local variable",std::make_pair(instNameWithAddr,type_info))});
                        offset += type_info.size.value();
                    }else{
                        auto addr = Mem.allocate(type_info.size==std::nullopt?8:type_info.size.value(),type_info.align==std::nullopt?8:type_info.align.value());
                        std::cout<<instNameWithAddr<<"[XXXTypeGen]:  addr: "<<std::hex<<addr<<std::endl;
                        newTable.insert({addr,std::make_pair("variable",std::make_pair(instNameWithAddr,type_info))});
                    }

                }
            }
            
            addSymbol(func.getName().str(),newTable);
            mergeSymbolTable(field_name(func.getName().str()),field_name("global"));
        }
    }
    printSymbolTable();
}

_SymbolsTable GenSymbolTable::getSymbolTable(field_name func_name){
    return SymbolTable_m[func_name];
}

void GenSymbolTable::mergeSymbolTable(field_name s1,field_name s2){
    auto table1 = SymbolTable_m[s1];
    auto table2 = SymbolTable_m[s2];
    for(auto s:table2){
        table1.insert(s);
    }
    SymbolTable_m[s1] = table1;
}

