#include "XXXVMPCore.h"
#include <fstream>
#include <cstring>
#include <bitset>
#include <iomanip>
#include <chrono>
#include <utility>
#include "VMPInterface.h"

void XXXVMPCore::Init(){
    if(!InterFace::getIsVoid()){
        this->ret = InterFace::getRet();
    }
    const auto m_native_args=InterFace::getNativeArgs();
    const auto offsets = InterFace::getOffset();
    for(size_t i=1;i<offsets.size();i++){

        stack.write(offsets[i],(uint8_t*)m_native_args+offsets[i],offsets[i]-offsets[i-1]);
        uint8_t* data = stack.read(offsets[i],offsets[i]-offsets[i-1]);
    }

}

#ifdef NOPROTECT
int main()
#else
int RUN()
#endif
{
    try {
        std::cout << "程序启动，开始调试检测..." << std::endl;
        
        // 开始先执行一次性快速检测（只使用高可靠性方法）
        int quick_confidence = perform_quick_debug_detection();
        std::cout << "快速检测置信度: " << quick_confidence << "%" << std::endl;
        
        if (quick_confidence < 100) {
            // 如果快速检测不确定，执行一次完整检测
            int full_confidence = perform_debug_detection();
            std::cout << "完整检测置信度: " << full_confidence << "%" << std::endl;
            
            if (full_confidence >= 85) {
                std::cout << "检测到调试器! 检测方法: " << std::endl;
                auto methods = get_detected_methods();
                for (const auto& method : methods) {
                    std::cout << "- " << method << std::endl;
                }
            }
        } else {
            std::cout << "高可靠性检测确认存在调试器!" << std::endl;
        }
        
        std::cout << "启动完整定期检测..." << std::endl;
        start_comprehensive_detection();
        
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        XXXVMPCore core;
        
        // std::string program = "/home/ljz/XXXVMP/XXXClang/build/XXXins_new.bin";
        core.load_initGlobal();
        // core.load_program(program);
        core.load_program();
        core.Init();
        core.run();
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        

        std::cout << "程序运行时间: " << time_span.count() << " 秒" << std::endl;
        core.dump();
        stop_detection();
        std::cout << "检测已停止" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << std::endl;
    }
    return 0;
}
