#include "XXXVMPCTX.h"
extern bool RETURN;

void FunCTX::handleLoad(llvm::Instruction *I)
{
    llvm::outs() << "[Transfor]: LoadInst\n";
    if (llvm::LoadInst *LI = llvm::dyn_cast<llvm::LoadInst>(I))
    {

        llvm::Type *LoadType = LI->getType();
        auto DLLoadType = DL.getABITypeAlign(LoadType); // 用DL获取类型对齐，貌似一样
        llvm::outs() << "[Transfor]: Load type: " << LoadType << "\n";
        llvm::outs() << "[Transfor]: Load type alignment: " << DLLoadType.value() << "\n";
        auto op1 = LI->getOperand(0);
        // std::string op1_name;
        // llvm::raw_string_ostream OS(op1_name);
        // op1->printAsOperand(OS, false); // 临时寄存器没名字，获取类似%1作为变量名称
        // OS.flush();
        // auto op1_record = symbol_table.get_variable_record(op1_name);
        if (llvm::isa<llvm::Constant>(op1))
        {
            llvm::outs() << "[Transfor]: Load type is constant\n";
        }
        else
        {

            // llvm::outs() << "[Transfor]: Load type is pointer\n";
            // auto size_m = DL.getTypeSizeInBits(LoadType) / 8;
            // llvm::outs() << "[Transfor]: Load size: " << size_m << "\n";

            // std::string instNameWithAddr;
            // llvm::raw_string_ostream OS(instNameWithAddr);

            // auto pdescriptor = std::make_shared<PrimitiveType>(size_m, getAlign(I).value());
            // auto var = std::make_shared<Variable>(pdescriptor, nullptr);
            // I->printAsOperand(OS, false);
            // symbol_table.register_variable_to_mem(instNameWithAddr, var);
            // OS.flush();
            
            VMINST *inst_src = nullptr;
            VMINST *inst_dst = nullptr;
            std::string instNameWithAddr;
            llvm::raw_string_ostream OS(instNameWithAddr);
            std::string cxxstr;
            op1->printAsOperand(OS, false);

            // auto op2_record = symbol_table.get_variable_record(instNameWithAddr);
            auto size = getSymbol(ReturnType::XXXType,I).size.value();
            auto petch = size / 8;
            auto rem = size % 8;
            llvm::outs()<<"[Transfor]: LoadInst name is ";
            I->printAsOperand(llvm::outs(),false);
            llvm::outs()<<"[Transfor]: , size is "<<size<<"\n";
            auto I_description= getSymbol(ReturnType::description,I);
            auto op_description = getSymbol(ReturnType::description,op1);
            auto op_offset = getSymbol(ReturnType::addr,op1);
            auto I_addr = getSymbol(ReturnType::addr,I);
            for(const auto& it: this->CXXFunCalls){
                auto set = it->get_instNameWithAddr();
                auto map = it->get_order_inst_map();
                if(set.find(instNameWithAddr)!=set.end()){
                    if(map.find(instNameWithAddr)==map.end()){
                        cxxstr = it->get_key_scope();
                    }
                }
            }
            if(!cxxstr.empty()){
                uint8_t _size = 2;
                switch (size)
                {
                case 1:
                    _size = 0;
                    break;
                case 2:
                    _size = 1;
                    break;
                case 4:
                    _size = 2;
                    break;
                case 8:
                    _size = 3;
                    break;
                default:
                    std::runtime_error("[Transfor]: LoadInst size error");
                    break;
                }
                uint64_t _addr = 0;
                std::memcpy(&_addr, cxxstr.data(), cxxstr.size());
                _LOAD_FROM_EXTERNAL_Inst *LFEI = new _LOAD_FROM_EXTERNAL_Inst(_addr, static_cast<uint64_t>(I_addr), _size);
                inst_src = LFEI;
                if (inst_src)
                    emitter.emit(inst_src);
                return;
            }
            if (op_description=="local variable")
            {
                for(auto i = 0; i < petch; i++)
                {
                    auto offset = i * 8;
                    auto rs = op_offset + offset;
                    auto rd = I_addr + offset;
                    llvm::outs()<<"[Transfor]: rs is "<<rs<<"  rd is "<<rd<<"\n";
                    _MOV_STACK_RX_Inst *MSRI = new _MOV_STACK_RX_Inst(rs, 1, 3);
                    _MOV_RX_MEM_Inst *MRMI = new _MOV_RX_MEM_Inst(rd, 1, 3);
                    inst_src = MSRI;
                    inst_dst = MRMI;
                    if (inst_src)
                        emitter.emit(inst_src);
                    if (inst_dst)
                        emitter.emit(inst_dst);
                }
                if (rem>=4)
                {
                    auto rs = op_offset + petch * 8;
                    auto rd = I_addr + petch * 8;
                    _MOV_STACK_RX_Inst *MSMI = new _MOV_STACK_RX_Inst(rs, 1, 2);
                    _MOV_RX_MEM_Inst *MRMI = new _MOV_RX_MEM_Inst(rd, 1, 2);
                    inst_src = MSMI;
                    inst_dst = MRMI;
                    if (inst_src)
                        emitter.emit(inst_src);
                    if (inst_dst)
                        emitter.emit(inst_dst);
                    rem-=4;
                }
                if (rem>=2)
                {
                    auto rs = op_offset + petch * 8 + 4;
                    auto rd = I_addr + petch * 8 + 4;
                    _MOV_STACK_RX_Inst *MSMI = new _MOV_STACK_RX_Inst(rs, 1, 1);
                    _MOV_RX_MEM_Inst *MRMI = new _MOV_RX_MEM_Inst(rd, 1, 1);
                    inst_src = MSMI;
                    inst_dst = MRMI;
                    if (inst_src)
                        emitter.emit(inst_src);
                    if (inst_dst)
                        emitter.emit(inst_dst);
                    rem-=2;
                }
                if (rem>=1)
                {
                    auto rs = op_offset + petch * 8 + 4 + 2;
                    auto rd = I_addr + petch * 8 + 4 + 2;
                    _MOV_STACK_RX_Inst *MSMI = new _MOV_STACK_RX_Inst(rs, 1, 0);
                    _MOV_RX_MEM_Inst *MRMI = new _MOV_RX_MEM_Inst(rd, 1, 0);
                    inst_src = MSMI;
                    inst_dst = MRMI;
                    if (inst_src)
                        emitter.emit(inst_src);
                    if (inst_dst)
                        emitter.emit(inst_dst);
                    rem-=1;
                }
            }
            else if (op_description=="variable")  //本来想省事的，想在好像省出事来了，这里就先这样吧，改一下解释器那边的处理
            {
                uint8_t _size = static_cast<uint8_t>(size);
                _MOV_MEM_MEM_Inst *MMMI = new _MOV_MEM_MEM_Inst(static_cast<uint64_t>(op_offset), static_cast<uint64_t>(I_addr), _size);
                llvm::outs()<<"[Transfor]: MMMI is ";
                
                inst_src = MMMI;
                inst_src->to_string();
                llvm::outs()<<"\n";
                if (inst_src)
                    emitter.emit(inst_src);
                if (inst_dst)
                    emitter.emit(inst_dst);
            }
        }
        
    }
}

void FunCTX::handleFParams()
{
    if(this->Fname==ExCollector.getPName()&&this->Fname!="main"){
        llvm::outs() << "protected function\n";
        VMINST* MIRX = new _MOV_IMM_RX_Inst(2, 3, 28);
        VMINST* SRR = new _SUB_RSP_R2_Inst();
        VMINST* MRR = new _MOV_RSP_RBP_Inst();
        emitter.emit(MIRX);
        emitter.emit(SRR);
        emitter.emit(MRR);
        return;

    }
    if (args.empty())
    {
        llvm::outs() << "no args\n";
        return;
    }
    auto offset = 0;
    while (!args.empty())
    {
        auto arg = args.top();
        args.pop();
        std::string instNameWithAddr;
        llvm::raw_string_ostream OS(instNameWithAddr);
        arg->printAsOperand(OS, false);
        OS.flush();
        llvm::outs() << "arg name is " << instNameWithAddr << "\n";
        auto arg_value = llvm::dyn_cast<llvm::Value>(arg);
        auto arg_type = arg_value->getType();
        auto arg_size = DL.getTypeSizeInBits(arg_type) / 8;
        auto align = DL.getABITypeAlign(arg_type).value();
        llvm::outs() << "arg type is " << arg_type << "\n";
        llvm::outs() << "arg size is " << arg_size << "\n";
        llvm::outs() << "arg align is " << align << "\n";
        auto pdescriptor = std::make_shared<PrimitiveType>(arg_size, align);
        // auto data = stack.read(-static_cast<int64_t>(arg_size), arg_size);
        auto var = std::make_shared<Variable>(pdescriptor, nullptr);
        symbol_table.register_param_from_stack(instNameWithAddr, var, -8 - 8 - offset);
        offset += arg_size;
    }
    return_addr = -8 - 8 - offset;
    llvm::outs() << "return addr: " << return_addr << "\n";
}
void FunCTX::handleRET(llvm::Instruction *I)
{
    if (llvm::ReturnInst *RI = llvm::dyn_cast<llvm::ReturnInst>(I))
    {
        if(this->Fname==ExCollector.getPName()&&this->Fname!="main"){
            llvm::outs() << "ReturnInst in main function\n";
            VMINST *RTN;
            auto retop = RI->getReturnValue();
            if(auto* R=llvm::dyn_cast<llvm::Constant>(retop);!R){
                llvm::outs() << "ReturnInst with value\n";
                auto op_description= getSymbol(ReturnType::description,retop);
                auto op_addr = getSymbol(ReturnType::addr,retop);
                auto size = getSymbol(ReturnType::XXXType,retop).size.value();
                if (op_description=="local variable")
                {
                    RTN = new _RET_TO_NATIVE_Inst(op_addr, size);
                    emitter.emit(RTN);
                }
                else if (op_description=="variable")
                {
                    RTN = new _RET_TO_NATIVE_Inst(op_addr, size);

                    emitter.emit(RTN);
                }
                delete RTN;
                return;
            }
        }
        llvm::outs() << "ReturnInst\n";
        auto op1 = RI->getReturnValue();
        VMINST *inst_src;
        VMINST *inst_dst;
        if (auto *CST = llvm::dyn_cast<llvm::Constant>(op1);!CST)
        {
            llvm::outs() << "ReturnInst with value\n";
            // std::string instNameWithAddr;
            // llvm::raw_string_ostream OS(instNameWithAddr);
            // op1->printAsOperand(OS, false);
            // OS.flush();
            // auto op1_record = symbol_table.get_variable_record(instNameWithAddr);
            // auto size = DL.getTypeSizeInBits(op1->getType()).getKnownMinValue() / 8;
            auto op_description= getSymbol(ReturnType::description,op1);
            auto op_addr = getSymbol(ReturnType::addr,op1);
            auto size = getSymbol(ReturnType::XXXType,op1).size.value();
            if (op_description=="local variable")
            {
                _RETURN_NRVO_Inst *MSRI = new _RETURN_NRVO_Inst(size, op_addr, return_addr, 0);
                inst_src = MSRI;
                if (inst_src)
                    emitter.emit(inst_src);
            }
            else if (op_description=="variable")
            {
                _RETURN_NRVO_Inst *MMMI = new _RETURN_NRVO_Inst(size, op_addr, return_addr, 1);
                llvm::outs()<<"return_addr is "<<return_addr<<"\n";

                inst_src = MMMI;
                if (inst_src)
                    emitter.emit(inst_src);
            }
        }
        else if (auto *CST = llvm::dyn_cast<llvm::Constant>(op1))
        {
            if (auto *CI = llvm::dyn_cast<llvm::ConstantInt>(CST))
            {
                auto value = CI->getSExtValue();
                _MOV_IMM_RX_Inst *MIRI = new _MOV_IMM_RX_Inst(1, 3, value);
                inst_src = MIRI;
                if (inst_src)
                    emitter.emit(inst_src);
                _RET_R0_Inst *RRI = new _RET_R0_Inst();
                inst_dst = RRI;
                if (inst_dst)
                    emitter.emit(inst_dst);
            }
            else if (auto *CFP = llvm::dyn_cast<llvm::ConstantFP>(CST))
            {
                llvm::APFloat Val = CFP->getValueAPF();
                // 转换为标准浮点类型
                if (Val.getSizeInBits(Val.getSemantics()) == 32)
                {
                    uint32_t value = 0xff;
                    llvm::APInt intBits = Val.bitcastToAPInt();
                    std::memcpy(reinterpret_cast<uint8_t *>(&value), reinterpret_cast<const uint8_t *>(intBits.getRawData()), 4);
                    _MOV_IMM_RX_Inst *MIRI = new _MOV_IMM_RX_Inst(1, 2, value);
                    inst_src = MIRI;
                    if (inst_src)
                        emitter.emit(inst_src);
                    _RET_R0_Inst *RRI = new _RET_R0_Inst();
                    inst_dst = RRI;
                    if (inst_dst)
                        emitter.emit(inst_dst);
                }
                else
                {
                    uint64_t value = 0xff;
                    llvm::APInt intBits = Val.bitcastToAPInt();
                    std::memcpy(reinterpret_cast<uint8_t *>(&value), reinterpret_cast<const uint8_t *>(intBits.getRawData()), 8);
                    _MOV_IMM_RX_Inst *MIRI = new _MOV_IMM_RX_Inst(1, 3, value);
                    inst_src = MIRI;
                    if (inst_src)
                        emitter.emit(inst_src);
                    _RET_R0_Inst *RRI = new _RET_R0_Inst();
                    inst_dst = RRI;
                    if (inst_dst)
                        emitter.emit(inst_dst);
                }
            }
            else
            {
                llvm::outs() << "unsupported constant type\n";
            }
        }
        else
        {
            llvm::outs() << "unsupported return type\n";
        }
    }
}
void FunCTX::createVar()
{
    for (auto bb : BBS)
    {
        std::string label;
        llvm::raw_string_ostream OS(label);
        bb->printAsOperand(OS);
        
        OS.flush();
        //去掉"label "前缀
        label = label.substr(6);
        BBaddr[label] = emitter.size();
        llvm::outs() << "BB name is " << label << "\n";
        llvm::outs() << "BB addr is " << BBaddr[label] << "\n";
        for(auto begin = bb->begin(), end = bb->end(); begin!=end;)
        {
            if(RETURN){
                break;
            }
            auto inst = &*begin++;
            
            handleStore(inst);
            handleLoad(inst);
            handleCall(inst);
            handleBinaryOperator(inst);
            handleICmp(inst);
            handleFCmp(inst);
            handleBranch(inst);
            handleRET(inst);
            handleInvoke(bb,inst);
        }
        if(RETURN){
            break;
        }
    }
    VMINST *MRR = new _MOV_RBP_RSP_Inst();

    emitter.emit(MRR);
    Stack.set_rsp(Stack.get_rbp());
    if (this->Fname == ExCollector.getPName())
    {
        llvm::outs() << "CODE SIZE:" << emitter.buffer.size() << "\n";
        VMINST *HI = new _HALT_Inst();
        emitter.emit(HI);
        RETURN=true;
        return;
    }
    VMINST *PPI = new _POP_PC_Inst();
    emitter.emit(PPI);
    VMINST *PRBPI = new _POP_RBP_Inst();
    emitter.emit(PRBPI);
    llvm::outs() << "CODE SIZE:" << emitter.buffer.size() << "\n";
    dump();
}
bool FunCTX::isUserFunction(std::string fname){
    if(ExCollector.isUserFunction(fname)){
        return true;
    }else{
        return false;
    }
}
void FunCTX::InitFCTXorder(){
    llvm::outs() << "Init FCTX order: " << this->Fname << "\n";
    if(this->Fname == ExCollector.getPName())
    {
        for(auto &bb:BBS){
            for(auto &inst : *bb){
                if(auto* CI = llvm::dyn_cast<llvm::CallInst>(&inst)){
                    auto fname = CI->getCalledFunction()->getName().str();
                    if(!isExternalFunction(fname)&&ExCollector.isUserFunction(fname)){
                        FCTXOrder->add_fctx_order(fname);
                    }
                }        
            }
        }
    }
}

bool FunCTX::isExternalFunction(std::string mangled_name){
    return ExCollector.is_f_external(mangled_name);
}

void FCTXOrder::build()
{
    for (auto &fname : fctxs_order)
    {
        llvm::outs() << "Build FCTX: " << fname << "\n";
        if(ExCollector.is_f_external(fname)){
            llvm::outs() << "External Function: " << fname << " skip \n";
            continue;
        }
        auto fctx = fctxs[fname];
        fctx->allocate();
        fctx->emitAlloca();
        fctx->createVar(); 
        fctx->insBBlabel();
        if(RETURN){
            break;
        }
    }
    class Linker linker(fctxs);
    linker.link_entry();
    linker.link_fctxs();
    linker.dump();
}
bool FunCTX::insertCXXCall(CXXFunCallInfo* info, std::string instNameWithAddr){
    for(auto it: this->CXXFunCalls){
        if(it->get_mangled_name() == info->get_mangled_name()&&it->get_sig() == info->get_sig()){
            llvm::outs() << "CXXCallInfo already exists: " << info->get_key_scope() << "\n";
            if(info->get_key_scope().empty()){
                llvm::outs() << "CXXCallInfo key_scope is empty" << "\n";
                abort();
            }
            for(auto set: info->get_instNameWithAddr()){
                it->set_instNameWithAddr(set);
            }
            it->set_instNameWithAddr(instNameWithAddr);
            return false;
            
        }
    }
    this->CXXFunCalls.insert(info);
    if(info->get_key_scope().empty()){
        llvm::outs() << "CXXCallInfo key_scope is empty2" << "\n";
        abort();
    }
    info->set_instNameWithAddr(instNameWithAddr);
    return true;

}
CXXFunCallInfo* FunCTX::get_existed_CXXCallInfo(CXXFunCallInfo* info){
    for(auto it: this->CXXFunCalls){
        if(it->get_mangled_name() == info->get_mangled_name()&&it->get_sig() == info->get_sig()){
            return it;
        }
    }
    return nullptr;
}
void FunCTX::handleExternalFCall(llvm::CallInst* inst){
    
    auto fname = inst->getCalledFunction()->getName().str();
    std::string signature;
    auto *FT = inst->getFunctionType();
    
    auto *ReturnType = FT->getReturnType();
    
    // 获取参数数量
    unsigned NumParams = FT->getNumParams();
    signature = fname + "(";
    // 遍历参数类型
    for (unsigned i = 0; i < NumParams; ++i) {
        auto *ParamType = FT->getParamType(i);
        if (i > 0) {
            signature += ", ";
        }
        std::string tname;
        llvm::raw_string_ostream OS(tname);
        ParamType->print(OS);
        OS.flush();
        signature += tname;
        
    }
    signature += ")";
    llvm::outs()<<"fname: "<<fname<<"\n";
    llvm::outs()<<"Function_prototypes: "<<signature<<"\n";

    CXXFunCallInfo* info = new CXXFunCallInfo(fname,signature);
    info->set_key_scope(ExCollector.get_key(fname));
    if(!isExternalFunction(fname)){
        return;
    }
    if(ExCollector.isCXXMemberFunction(fname)){
        handleCXXMemberCall(inst);
        return;
    }
    auto actual_args = inst->args();
    int count = 0;
    for(auto &arg : actual_args){
        auto arg_value = llvm::dyn_cast<llvm::Value>(arg);
        std::string instNameWithAddr;
        llvm::raw_string_ostream OS(instNameWithAddr);
        arg_value->printAsOperand(OS, false);
        OS.flush();
        llvm::outs() << "External Call arg: " << instNameWithAddr << "\n";
        std::string name;
        name = arg->getName().str();
        if(name.empty()){
            name = instNameWithAddr;
            llvm::outs() << "External Call arg has no name: " << name << "\n";
        }else{
            llvm::outs() << "External Call arg name: " << name << "\n";
        }
        if(ExCollector.is_v_external(name)||ExCollector.is_f_external(name)){
            llvm::outs() << "External Call arg is external: " << name << "\n";
            info->add_param(count,name);
        }else{
            // auto record = ExCollector.symbol_table.get_variable_record(name);
            // if(record == nullptr){
                llvm::errs() << "External Call arg is not found in Global Symbol Table: " << name << "\n";
                bool continue_ = false;
                for(auto it: this->CXXFunCalls){
                    auto set = it->get_instNameWithAddr();
                    auto map = it->get_order_inst_map();
                    if(set.find(instNameWithAddr)!=set.end()){
                        if(map.find(instNameWithAddr)!=map.end()){
                            info->add_other_oi_map(it->get_key_scope(),map[instNameWithAddr],count);
                        }else{
                            info->add_otherCXXCall_result(count,it->get_key_scope());
                        }
                        count++;     
                        continue_ = true;      
                        break;
                    }
                // }
 
                if(continue_){
                    continue;
                }else{
                    // record = this->symbol_table.get_variable_record(instNameWithAddr);
                    info->add_order_inst_map(instNameWithAddr,count);
                }
            }
            // auto size = record->var->type()->size();
            auto size = getSymbol(ReturnType::XXXType,name).size.value();
            auto description = getSymbol(ReturnType::description,name);
            auto addr = getSymbol(ReturnType::addr,name);
            llvm::outs() << "External Call arg is not external: " << name << " size: " << size << "\n";
            if(description == "local variable"){

                VMINST* src = new _MOV_IMM_RX_Inst(1,3,addr);
                VMINST* dst = new _PUSH_R1_Inst();
                VMINST* bool_ = new _MOV_IMM_RX_Inst(1,0,1);
                VMINST* P = new _PUSH_RX_WITH_SIZE_Inst(1,0);
                VMINST* inst2 = new _MOV_IMM_RX_Inst(1,0,size);
                VMINST* inst3 = new _PUSH_R1_Inst();
                emitter.emit(inst2);
                emitter.emit(inst3);
                emitter.emit(bool_);
                emitter.emit(P);
                emitter.emit(src);
                emitter.emit(dst);
                delete bool_;
                delete P;
                delete src;
                delete dst;
                delete inst2;
                delete inst3;
            }else if(description == "variable"||description.starts_with("arg")||description.starts_with("global variable")){
                // auto petch = size / 8;
                // auto res = size % 8;

                // for(auto i=0;i<petch;i++){
                //     VMINST* inst = new _MOV_MEM_RX_Inst(record->base_addr+res+(petch-i-1)*8,1,3);
                //     emitter.emit(inst);
                //     delete inst;
                //     VMINST* inst2 = new _PUSH_R1_Inst();
                //     emitter.emit(inst2);
                //     delete inst2;
                // }
                // while(res>0){
                //     if(res>=4){
                //         VMINST* inst = new _MOV_MEM_RX_Inst(record->base_addr+res-4,1,2);
                //         emitter.emit(inst);
                //         delete inst;
                //         VMINST* inst2 = new _PUSH_RX_WITH_SIZE_Inst(1,2);
                //         emitter.emit(inst2);
                //         delete inst2;
                //         res -= 4;
                //     }
                //     if(res>=2){
                //         VMINST* inst = new _MOV_MEM_RX_Inst(record->base_addr+res-2,1,1);
                //         emitter.emit(inst);
                //         delete inst;
                //         VMINST* inst2 = new _PUSH_RX_WITH_SIZE_Inst(1,1);
                //         emitter.emit(inst2);
                //         delete inst2;
                //         res -= 2;
                //     }
                //     if(res>=1){
                //         VMINST* inst = new _MOV_MEM_RX_Inst(record->base_addr+res-1,1,0);
                //         emitter.emit(inst);
                //         delete inst;
                //         VMINST* inst2 = new _PUSH_RX_WITH_SIZE_Inst(1,0);
                //         emitter.emit(inst2);
                //         delete inst2;
                //         res -= 1;
                //     }
                // }
                VMINST* src = new _MOV_IMM_RX_Inst(1,3,addr);
                VMINST* dst = new _PUSH_R1_Inst();
                VMINST* bool_ = new _MOV_IMM_RX_Inst(1,0,0);
                VMINST* P = new _PUSH_RX_WITH_SIZE_Inst(1,0);
                VMINST* inst2 = new _MOV_IMM_RX_Inst(1,0,size);
                VMINST* inst3 = new _PUSH_R1_Inst();
                emitter.emit(inst2);
                emitter.emit(inst3);
                emitter.emit(bool_);
                emitter.emit(P);
                emitter.emit(src);
                emitter.emit(dst);
                delete bool_;
                delete P;
                delete src;
                delete dst;
                delete inst2;
                delete inst3;
            }
        }
        count++;
    }
    std::string instNameWithAddr;
    llvm::raw_string_ostream OS(instNameWithAddr);
    if(inst->getType()->isVoidTy()){
        instNameWithAddr = "void";
        info->setVoidReturnFunc(true);
        llvm::outs() << "External Call has no return value: " << instNameWithAddr << "\n";

    }else{
        inst->printAsOperand(OS, false);
        OS.flush();
    }

    if(info->isCXXConstructor()){
        for(auto &arg:actual_args){
            auto arg_value = llvm::dyn_cast<llvm::Value>(arg);
            std::string classkey;
            llvm::raw_string_ostream OS(classkey);
            arg_value->printAsOperand(OS, false);
            OS.flush();
            instNameWithAddr = classkey;
        }

    }
    info->set_instNameWithAddr(instNameWithAddr);
    llvm::outs() << "info->set_instNameWithAddr: " << instNameWithAddr << "\n";
    if(insertCXXCall(info,instNameWithAddr)){
        if(info->get_key_scope().empty()){
            llvm::outs() << "CXXCallInfo key_scope is empty" << "\n";
            abort();
        }
        info->build();
    }else{
        info = get_existed_CXXCallInfo(info);
    }
    auto key = info->get_key_scope();
    uint8_t param_nums = info->get_param_nums();
    uint64_t addr =0;
    llvm::outs() << "External Call: " << key << "\n";

    std::memcpy(&addr,key.data(),key.size());
    VMINST* push_rbp = new _PUSH_RBP_Inst();
    emitter.emit(push_rbp);
    VMINST* msbi = new _MOV_RSP_RBP_Inst();
    emitter.emit(msbi);
    delete msbi;
    delete push_rbp;

    VMINST* ins = new _CALL_EXTERNAL_Inst(addr,param_nums);
    emitter.emit(ins);
    delete ins;
    VMINST* mbsi = new _MOV_RBP_RSP_Inst();
    emitter.emit(mbsi);
    VMINST* pop_rbp = new _POP_RBP_Inst();
    emitter.emit(pop_rbp);
    delete mbsi;
    delete pop_rbp;

}

void FunCTX::handleCXXMemberCall(llvm::CallInst* inst){
    auto fname = inst->getCalledFunction()->getName().str();
    std::string signature;
    auto *FT = inst->getFunctionType();
    
    auto *ReturnType = FT->getReturnType();
    
    // 获取参数数量
    unsigned NumParams = FT->getNumParams();
    signature = fname + "(";
    // 遍历参数类型
    for (unsigned i = 0; i < NumParams; ++i) {
        auto *ParamType = FT->getParamType(i);
        if (i > 0) {
            signature += ", ";
        }
        std::string tname;
        llvm::raw_string_ostream OS(tname);
        ParamType->print(OS);
        OS.flush();
        signature += tname;
        
    }
    signature += ")";
    llvm::outs()<<"fname: "<<fname<<"\n";
    llvm::outs()<<"Function_prototypes: "<<signature<<"\n";

    CXXFunCallInfo* info = new CXXFunCallInfo(fname,signature);

    if(ExCollector.isCXXConstructor(fname)){
        llvm::outs() << "CXX Constructor: " << fname << "\n";
        info->setCXXConstructor(true);
        info->set_key_scope(ExCollector.get_key(fname));
    }
    std::string this_instname;
    auto actual_args = inst->args();
    int count = 0;
    bool skip = false;
    std::string class_key_scope;
    int _count = 0;
    for(auto &arg : actual_args){
        _count++;
        auto arg_value = llvm::dyn_cast<llvm::Value>(arg);
        std::string instNameWithAddr;
        llvm::raw_string_ostream OS(instNameWithAddr);
        auto arg_name = arg->getName().str();
        arg_value->printAsOperand(OS, false);
        
        OS.flush();
        if(!skip){
            if(!info->isCXXConstructor()){
                if(!arg_name.empty()){
                    class_key_scope = ExCollector.get_key(arg_name);
                    llvm::outs() <<"fname : "<<fname<< "  ExternalCXXmember Call arg is class: " << arg_name << "\n";
                    llvm::outs() << "testCXXMenberBuidld BUG _count IS: " << _count << "\n";
                    info->set_key_scope(class_key_scope);
                    skip=true;
                }else{
                    for(auto it:CXXFunCalls){
                        auto set = it->get_instNameWithAddr();
                        for(auto map_it:set){
                            llvm::outs() << "it->get_instNameWithAddr(): " << map_it << "\n";
                        }
                        if(set.find(instNameWithAddr)!=set.end()){
                            skip = true;
                            class_key_scope = it->get_key_scope();
                            info->set_key_scope(class_key_scope);
                            llvm::outs() << "External Call arg is class: " << instNameWithAddr << "class_key_scope: " << class_key_scope << "\n";
                            break;
                        }
                    }
                    if(skip==false){
                        llvm::outs()<<"bug\n";
                        abort();
                    }
                    
                }
                continue;
            }else{
                class_key_scope = ExCollector.get_key(fname);
                llvm::outs() << "CXX Constructor Call arg is class: " << arg_name << "\n";
                info->set_instNameWithAddr(instNameWithAddr);
                this_instname= instNameWithAddr;
                info->set_key_scope(class_key_scope);
                skip=true;
            }
            if(skip==false){
                llvm::outs()<<"bug2\n";
                abort();
            }

        }
        llvm::outs() << "ExternalCXXmember Call arg: " << instNameWithAddr << "\n";
        std::string name;
        name = arg->getName().str();
        if(name.empty()){
            name = instNameWithAddr;
            llvm::outs() << "ExternalCXXmember Call arg has no name: " << name << "\n";
        }else{
            llvm::outs() << "ExternalCXXmember Call arg name: " << name << "\n";
        }

        if(ExCollector.is_v_external(name)||ExCollector.is_f_external(name)){
            llvm::outs() << "ExternalCXXmember Call arg is external: " << name << "\n";
            info->add_param(count,name);
        }else{
            // auto record = ExCollector.symbol_table.get_variable_record(name);
            // if(record == nullptr){
                
                llvm::outs() << "ExternalCXXmember Call arg record is not found in Global Symbol Table: " << name << " skip\n";
                llvm::errs() << "ExternalCXXmember Call arg is not found in Local Symbol Table: " << instNameWithAddr << "\n";
                bool continue_ = false;
                for(auto it: this->CXXFunCalls){
                    auto set = it->get_instNameWithAddr();
                    auto map = it->get_order_inst_map();
                    if(set.find(instNameWithAddr)!=set.end()){
                        if(map.find(instNameWithAddr)!=map.end()){
                            info->add_other_oi_map(it->get_key_scope(),map[instNameWithAddr],count);
                        }else{
                            info->add_otherCXXCall_result(count,it->get_key_scope());
                        }
                        count++;
                        continue_ = true;
                        break;
                    }

                // }

                if(continue_){

                    continue;
                }else{
                    // record = this->symbol_table.get_variable_record(name);
                    info->add_order_inst_map(instNameWithAddr,count);
                }
                
            }
            // auto size = record->var->type()->size();
            auto size = getSymbol(ReturnType::XXXType,name).size.value();
            auto description = getSymbol(ReturnType::description,name);
            auto addr = getSymbol(ReturnType::addr,name);
            llvm::outs() << "External Call arg is not external: " << name << " size: " << size << "\n";
            if(description == "local variable"){
                VMINST* src = new _MOV_IMM_RX_Inst(1,3,addr);
                VMINST* dst = new _PUSH_R1_Inst();
                VMINST* bool_ = new _MOV_IMM_RX_Inst(1,0,1);
                VMINST* P = new _PUSH_RX_WITH_SIZE_Inst(1,0);
                VMINST* inst2 = new _MOV_IMM_RX_Inst(1,0,size);
                VMINST* inst3 = new _PUSH_R1_Inst();
                emitter.emit(inst2);
                emitter.emit(inst3);
                emitter.emit(bool_);
                emitter.emit(P);
                emitter.emit(src);
                emitter.emit(dst);
                delete bool_;
                delete P;
                delete src;
                delete dst;
                delete inst2;
                delete inst3;
            }else if(description == "variable"||description.starts_with("arg")||description.starts_with("global variable")){
                // auto petch = size / 8;
                // auto res = size % 8;       
                // for(auto i=0;i<petch;i++){
                //     VMINST* inst = new _MOV_MEM_RX_Inst(record->base_addr+res+(petch-i-1)*8,1,3);
                //     emitter.emit(inst);
                //     delete inst;
                //     VMINST* inst2 = new _PUSH_R1_Inst();
                //     emitter.emit(inst2);
                //     delete inst2;
                // }
                // while(res>0){
                //     if(res>=4){
                //         VMINST* inst = new _MOV_MEM_RX_Inst(record->base_addr+res-4,1,2);
                //         emitter.emit(inst);
                //         delete inst;
                //         VMINST* inst2 = new _PUSH_RX_WITH_SIZE_Inst(1,2);
                //         emitter.emit(inst2);
                //         delete inst2;
                //         res -= 4;
                //     }
                //     if(res>=2){
                //         VMINST* inst = new _MOV_MEM_RX_Inst(record->base_addr+res-2,1,1);
                //         emitter.emit(inst);
                //         delete inst;
                //         VMINST* inst2 = new _PUSH_RX_WITH_SIZE_Inst(1,1);
                //         emitter.emit(inst2);
                //         delete inst2;
                //         res -= 2;
                //     }
                //     if(res>=1){
                //         VMINST* inst = new _MOV_MEM_RX_Inst(record->base_addr+res-1,1,0);
                //         emitter.emit(inst);
                //         delete inst;
                //         VMINST* inst2 = new _PUSH_RX_WITH_SIZE_Inst(1,0);
                //         emitter.emit(inst2);
                //         delete inst2;
                //         res -= 1;
                //     }
                // }
                VMINST* src = new _MOV_IMM_RX_Inst(1,3,addr);
                VMINST* dst = new _PUSH_R1_Inst();
                VMINST* bool_ = new _MOV_IMM_RX_Inst(1,0,0);
                VMINST* P = new _PUSH_RX_WITH_SIZE_Inst(1,0);
                VMINST* inst2 = new _MOV_IMM_RX_Inst(1,0,size);
                VMINST* inst3 = new _PUSH_R1_Inst();
                emitter.emit(inst2);
                emitter.emit(inst3);
                emitter.emit(bool_);
                emitter.emit(P);
                emitter.emit(src);
                emitter.emit(dst);
                delete bool_;
                delete P;
                delete src;
                delete dst;
                delete inst2;
                delete inst3;
            }
        }
        count++;
    }

    std::string instNameWithAddr;
    llvm::raw_string_ostream OS(instNameWithAddr);
    if(inst->getType()->isVoidTy()){
        instNameWithAddr = "void";
        info->setVoidReturnFunc(true);
    }else{
        inst->printAsOperand(OS, false);
        OS.flush();
    }
    if(!info->isCXXConstructor()){
        info->set_instNameWithAddr(instNameWithAddr);
    }
    
    if(insertCXXCall(info,this_instname)){
        if(class_key_scope.empty()){
            llvm::errs() << "CXXCallInfo class_key_scope is empty: " << info->get_key_scope() << "\n";
            inst->print(llvm::errs());

        }
        info->buildCXXMemberFunction(class_key_scope);
    }else{
        info = get_existed_CXXCallInfo(info);
    }
    auto key = info->get_key_scope();
    uint8_t param_nums = info->get_param_nums();
    if(info->isCXXConstructor()){
        param_nums+=1;
    }
    uint64_t addr =0;
    std::memcpy(&addr,key.data(),key.size());
    VMINST* push_rbp = new _PUSH_RBP_Inst();
    emitter.emit(push_rbp);
    VMINST* msbi = new _MOV_RSP_RBP_Inst();
    emitter.emit(msbi);
    delete msbi;
    delete push_rbp;
    VMINST* ins = new _CALL_EXTERNAL_Inst(addr,param_nums);
    emitter.emit(ins);
    delete ins;
    VMINST* mbsi = new _MOV_RBP_RSP_Inst();
    emitter.emit(mbsi);
    VMINST* pop_rbp = new _POP_RBP_Inst();
    emitter.emit(pop_rbp);
    delete mbsi;
    delete pop_rbp;
}


void CXXFunCallInfo::buildCXXMemberFunction(std::string class_key_scope)
{
    bool isVoid = false;
    llvm::outs() << "Build CXXMemberFunction: " << this->mangled_name << " class_key_scope: " << class_key_scope << "\n";
    if(this->isCXXConstructor_m){
        buildCXXConstructor(class_key_scope);
        return;
    }
    std::ifstream json(jsonFile);
    if (!json.is_open())
    {
        llvm::errs() << "Failed to open file: " << jsonFile << "\n";
        return;
    }
    nlohmann::json j;
    json >> j;
    json.close();
    for (auto &param : params)
    {
        for (auto &var : j["Var"])
        {
            if (var["mangledName"] == param.second)
            {
                gen_params.insert({param.first, var["key"].dump().substr(1, var["key"].dump().size() - 2)});
            }
        }
    }
    for (auto &param : params)
    {
        for (auto &var : j["Func"])
        {
            if (var["mangledName"] == param.second)
            {
                gen_params.insert({param.first, var["key"].dump().substr(1, var["key"].dump().size() - 2)});
            }
        }
    }
    for(auto &param:otherCXXCall_result)
    {
        if(param.second.starts_with("t")||param.second.starts_with("t")){
            gen_params.insert({param.first,"*"+param.second+"_::res"});
            continue;
        }
        gen_params.insert({param.first,param.second+"_::res"});
    }
    for(auto &param:other_order_inst_map){
        gen_params.insert({param.first,param.second.first+"_::p"+std::to_string(param.second.second)});
    }
    int count = 0;
    int count_ = 0;
    for (auto &func : j["Func"])
    {
        if (func["mangledName"] == mangled_name)
        {
            key_scope = func["key"].dump().substr(1, func["key"].dump().size() - 2);
            if (func["returnType"].dump().ends_with("&\""))
            {
                ret_is_ref=true;
                result  = func["returnType"].dump().substr(1, func["returnType"].dump().size() - 3)+" *";
                return_ = "const auto " + func["key"].dump().substr(1, func["key"].dump().size() - 2) + "_instance";
            }
            else
            {
                result = func["returnType"].dump().substr(1, func["returnType"].dump().size() - 2) + " *";
                return_ = "auto " + func["key"].dump().substr(1, func["key"].dump().size() - 2) + "_instance";
            }
            if(func["returnType"]=="void")
            {
                isVoid = true;
            }
            for (auto &param : func["params"])
            {
                param_nums++;
                if (this->order.find(count) != this->order.end())
                {
                    if (param.dump().ends_with("&\""))
                    {
                        // gen_params[count].insert(0, "*");
                    }
                    count++;
                    continue;
                }
                if (param.dump().ends_with("&\""))
                {
                    gen_params_stub.push_back(param.dump().substr(1, param.dump().size() - 3) + " p" + std::to_string(count_));
                    if(gen_params_stub.back().starts_with("const "))
                    {
                        gen_params_stub.back().erase(0, 6);
                    }
                    
                }
                else
                {
                    gen_params_stub.push_back(param.dump().substr(1, param.dump().size() - 2) + " p" + std::to_string(count_));
                    if(gen_params_stub.back().starts_with("const "))
                    {
                        gen_params_stub.back().erase(0, 6);
                    }
                }
                count_++;
            }
            break;
        }
    }
    llvm::outs() << "param_nums: " << param_nums << "\n";
    for (auto p : gen_params)
    {
        llvm::outs() << p.first << " " << p.second << "\n";
    }
    std::fstream stub(stubFile, std::ios::app);
    if (!stub.is_open())
    {
        llvm::errs() << "Failed to open file: " << stubFile << "\n";
        return;
    }
    stub << "\n";
    stub << "namespace " << key_scope + "_ {\n";
    for(int i=0;i<gen_params_stub.size();i++){
        if(gen_params_stub[i].ends_with(std::string(" * p")+std::to_string(i))){
            stub << "\t" << gen_params_stub[i] << " = new " << gen_params_stub[i].substr(0, gen_params_stub[i].find(" ")) << "();\n";
        }else{
            stub << "\t" << gen_params_stub[i] << ";\n";
        }
    }
    if(!isVoid){
        stub << "\t" << result << " res;\n";
    }
    
    stub << "\t" << return_ << " = " << "[";
    // for (int i = 0; i < gen_params_stub.size(); i++)
    // {
    //     stub << "p" << i;
    //     if (i != gen_params_stub.size() - 1)
    //     {
    //         stub << ", ";
    //     }
    // }
    stub << "](";
    stub << "){\n";
    stub << "\t\t" << (isVoid?"":std::string("res = ")+(ret_is_ref?"&":""))<<"(" << ((class_key_scope.starts_with("f")||class_key_scope.starts_with("t"))?(class_key_scope+"_::res->*"):class_key_scope+".*") << key_scope<<")(";
    count_ = 0;
    for (int i = 0; i < param_nums; i++)
    {
        if (gen_params.find(i) != gen_params.end())
        {
            stub << gen_params[i];
        }
        else
        {
            stub << "p" << count_;
            count_++;
        }
        if (i != param_nums - 1)
        {
            stub << ", ";
        }
    }
    stub << ");\n";
    stub << "\t};\n";
    stub << "}\n";
    stub.close();
    GenStubMap::add_cxx_fun_call_info(this);
}
void CXXFunCallInfo::buildCXXConstructor(std::string class_key_scope){
    std::ifstream json(jsonFile);
    if (!json.is_open())
    {
        llvm::errs() << "Failed to open file: " << jsonFile << "\n";
        return;
    }
    nlohmann::json j;
    json >> j;
    json.close();
    for (auto &param : params)
    {
        for (auto &var : j["Var"])
        {
            if (var["mangledName"] == param.second)
            {
                gen_params.insert({param.first, var["key"].dump().substr(1, var["key"].dump().size() - 2)});
            }
        }
    }
    for (auto &param : params)
    {
        for (auto &var : j["Func"])
        {
            if (var["mangledName"] == param.second)
            {
                gen_params.insert({param.first, var["key"].dump().substr(1, var["key"].dump().size() - 2)});
            }
        }
    }
    for(auto &param:otherCXXCall_result)
    {
        if(param.second.starts_with("t")||param.second.starts_with("t")){
            gen_params.insert({param.first,"*"+param.second+"_::res"});
            continue;
        }
        gen_params.insert({param.first,param.second+"_::res"});
    }
    for(auto &param:other_order_inst_map){
        gen_params.insert({param.first,param.second.first+"_::p"+std::to_string(param.second.second)});
    }
    int count = 0;
    int count_ = 0;
    for (auto &func : j["Func"])
    {
        if (func["mangledName"] == mangled_name)
        {
            key_scope = func["key"].dump().substr(1, func["key"].dump().size() - 2);
            if (func["returnType"].dump().ends_with("&\""))
            {
                ret_is_ref=true;
                result  = func["returnType"].dump().substr(1, func["returnType"].dump().size() - 3)+" *";
                return_ = "const auto " + func["key"].dump().substr(1, func["key"].dump().size() - 2) + "_instance";
            }
            else
            {
                result = func["returnType"].dump().substr(1, func["returnType"].dump().size() - 2) + " *";
                return_ = "auto " + func["key"].dump().substr(1, func["key"].dump().size() - 2) + "_instance";
            }
            for (auto &param : func["params"])
            {
                param_nums++;
                if (this->order.find(count) != this->order.end())
                {
                    if (param.dump().ends_with("&\""))
                    {
                        if(gen_params[count].find("::res")!= std::string::npos&&!gen_params[count].starts_with("*t")){
                            gen_params[count].insert(0, "*");
                        }
                    }
                    count++;
                    continue;
                }
                if (param.dump().ends_with("&\""))
                {
                    gen_params_stub.push_back(param.dump().substr(1, param.dump().size() - 3) + " p" + std::to_string(count_));
                    if(gen_params_stub.back().starts_with("const "))
                    {
                        gen_params_stub.back().erase(0, 6);
                    }
                    
                }
                else
                {
                    gen_params_stub.push_back(param.dump().substr(1, param.dump().size() - 2) + " p" + std::to_string(count_));
                    if(gen_params_stub.back().starts_with("const "))
                    {
                        gen_params_stub.back().erase(0, 6);
                    }
                }
                count_++;
            }
            break;
        }
    }
    llvm::outs() << "param_nums: " << param_nums << "\n";
    for (auto p : gen_params)
    {
        llvm::outs() << p.first << " " << p.second << "\n";
    }
    std::fstream stub(stubFile, std::ios::app);
    if (!stub.is_open())
    {
        llvm::errs() << "Failed to open file: " << stubFile << "\n";
        return;
    }
    stub << "\n";
    stub << "namespace " << key_scope + "_ {\n";
    for(int i=0;i<gen_params_stub.size();i++){
        if(gen_params_stub[i].ends_with(std::string(" * p")+std::to_string(i))){
            stub << "\t" << gen_params_stub[i] << " = new " << gen_params_stub[i].substr(0, gen_params_stub[i].find(" ")) << "();\n";
        }else{
            stub << "\t" << gen_params_stub[i] << ";\n";
        }
    }

    stub << "\t" << key_scope << "* res;\n";
    stub << "\tuint8_t addr[sizeof("<<key_scope<<")];\n";
    stub << "\t" << return_ << " = " << "[";
    // for (int i = 0; i < gen_params_stub.size(); i++)
    // {
    //     stub << "p" << i;
    //     if (i != gen_params_stub.size() - 1)
    //     {
    //         stub << ", ";    
    //     }
    // }

    stub << "](";
    stub<<"){\n";
    stub << "\t\t" << "res = new(addr) "<<key_scope<< "(";
    count_ = 0;
    for (int i = 0; i < param_nums; i++)
    {
        if (gen_params.find(i) != gen_params.end())
        {
            stub << gen_params[i];
        }
        else
        {
            stub << "p" << count_;
            count_++;
        }
        if (i != param_nums - 1)
        {
            stub << ", ";
        }
    }
    stub << ");\n";
    stub << "\t};\n";
    stub << "}\n";
    stub.close();
    GenStubMap::add_cxx_fun_call_info(this);

}


void CXXFunCallInfo::build()
{
    bool isVoid = false;
    std::ifstream json(jsonFile);
    if (!json.is_open())
    {
        llvm::errs() << "Failed to open file: " << jsonFile << "\n";
        return;
    }
    nlohmann::json j;
    json >> j;
    json.close();
    for (auto &param : params)
    {
        for (auto &var : j["Var"])
        {
            if (var["mangledName"] == param.second)
            {
                gen_params.insert({param.first, var["key"].dump().substr(1, var["key"].dump().size() - 2)});
            }
        }
    }
    for (auto &param : params)
    {
        for (auto &var : j["Func"])
        {
            if (var["mangledName"] == param.second)
            {
                gen_params.insert({param.first, var["key"].dump().substr(1, var["key"].dump().size() - 2)});
            }
        }
    }
    for(auto &param:otherCXXCall_result)
    {
        if(param.second.starts_with("t")||param.second.starts_with("t")){
            gen_params.insert({param.first,"*"+param.second+"_::res"});
            continue;
        }
        gen_params.insert({param.first,param.second+"_::res"});
    }
    int count = 0;
    int count_ = 0;
    for (auto &func : j["Func"])
    {
        if (func["mangledName"] == mangled_name)
        {
            key_scope = func["key"].dump().substr(1, func["key"].dump().size() - 2);
            if (func["returnType"].dump().ends_with("&\""))
            {
                ret_is_ref=true;
                result  = func["returnType"].dump().substr(1, func["returnType"].dump().size() - 3)+" *";
                return_ = "const auto " + func["key"].dump().substr(1, func["key"].dump().size() - 2) + "_instance";
            }
            else
            {
                result = func["returnType"].dump().substr(1, func["returnType"].dump().size() - 2) + " *";
                return_ = "auto " + func["key"].dump().substr(1, func["key"].dump().size() - 2) + "_instance";
            }
            if(func["returnType"]== "void"){
                isVoid = true;
            }
            for (auto &param : func["params"])
            {
                param_nums++;
                if (this->order.find(count) != this->order.end())
                {
                    if (param.dump().ends_with("&\""))
                    {
                        if(gen_params[count].find("::res")!= std::string::npos&&!gen_params[count].starts_with("*t")){
                            gen_params[count].insert(0, "*");
                        }
                    }
                    count++;
                    continue;
                }
                if (param.dump().ends_with("&\""))
                {
                    gen_params_stub.push_back(param.dump().substr(1, param.dump().size() - 3) + " p" + std::to_string(count_));
                    if(gen_params_stub.back().starts_with("const "))
                    {
                        gen_params_stub.back().erase(0, 6);
                    }
                    
                }
                else
                {
                    gen_params_stub.push_back(param.dump().substr(1, param.dump().size() - 2) + " p" + std::to_string(count_));
                    if(gen_params_stub.back().starts_with("const "))
                    {
                        gen_params_stub.back().erase(0, 6);
                    }
                }
                count_++;
            }
            break;
        }
    }
    llvm::outs() << "param_nums: " << param_nums << "\n";
    for (auto p : gen_params)
    {
        llvm::outs() << p.first << " " << p.second << "\n";
    }
    std::fstream stub(stubFile, std::ios::app);
    if (!stub.is_open())
    {
        llvm::errs() << "Failed to open file: " << stubFile << "\n";
        return;
    }
    stub << "\n";
    stub << "namespace " << key_scope + "_ {\n";
    for(int i=0;i<gen_params_stub.size();i++){
        if(gen_params_stub[i].ends_with(std::string(" * p")+std::to_string(i))){
            stub << "\t" << gen_params_stub[i] << " = new " << gen_params_stub[i].substr(0, gen_params_stub[i].find(" ")) << "();\n";
        }else{
            stub << "\t" << gen_params_stub[i] << ";\n";
        }
    }
    if(!isVoid){
        stub << "\t" << result << "res;\n";
    }
    
    stub << "\t" << return_ << " = " << "[";
    // for (int i = 0; i < gen_params_stub.size(); i++)
    // {
    //     stub << "p" << i;
    //     if (i != gen_params_stub.size() - 1)
    //     {
    //         stub << ", ";
    //     }
    // }
    stub << "](";
    stub << "){\n";
    stub << "\t\t" << (isVoid?"":std::string("res = ")+(ret_is_ref?"&":""))<<key_scope << "(";
    count_ = 0;
    for (int i = 0; i < param_nums; i++)
    {
        if (gen_params.find(i) != gen_params.end())
        {
            stub << gen_params[i];
        }
        else
        {
            stub << "p" << count_;
            count_++;
        }
        if (i != param_nums - 1)
        {
            stub << ", ";
        }
    }
    stub << ");\n";
    stub << "\t};\n";
    stub << "}\n";

    // stub << "std::unordered_map<std::string, void*> var_map = {\n";
    // for(int i=0;i<gen_params_stub.size();i++){
    //     if(gen_params_stub[i].ends_with(std::string(" * p")+std::to_string(i))){
    //         stub << "\t{\"" << key_scope + "_::p" + std::to_string(i) + "\", " << key_scope << "_::p" + std::to_string(i) << "},\n";
    //     }
    //     else{
    //         stub << "\t{\"" << key_scope + "_::p" + std::to_string(i) + "\", " << "&"<< key_scope << "_::p" + std::to_string(i) << "->get()},\n";
    //     }
    // }
    // stub << "};\n";
    // stub << "std::unordered_map<std::string, std::function<void()>> func_map = {\n";
    // stub << "\t{\"" << key_scope << "\", " << key_scope << "_::" << key_scope << "_instance}\n";
    // stub << "};\n";
    stub.close();
    GenStubMap::add_cxx_fun_call_info(this);
}