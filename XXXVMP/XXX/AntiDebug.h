#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <atomic>
#include <functional>
#include <memory>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <algorithm>
#include <set>

// Linux系统特定头文件
#include <sys/ptrace.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>

class Logger {
    public:
        static bool isDebugEnabled() {
            static bool checked = false;
            static bool enabled = false;
            
            if (!checked) {
                checked = true;
                const char* env = std::getenv("ANTIDBG");
                enabled = (env != nullptr);
            }
            
            return enabled;
        }
        
        static void debug(const std::string& component, const std::string& message) {
            if (isDebugEnabled()) {
                std::cout << "[" << component << "] " << message << std::endl;
            }
        }
        
        static void info(const std::string& component, const std::string& message) {
            // 信息级别日志总是输出
            std::cout << "[" << component << "] " << message << std::endl;
        }
        
        static void error(const std::string& component, const std::string& message) {
            // 错误级别日志总是输出
            std::cerr << "[ERROR][" << component << "] " << message << std::endl;
        }
};

// 检测方法分组和策略
enum class DetectionReliability {
    HIGH,       // 高可靠性方法 (如ptrace检测)
    MEDIUM,     // 中等可靠性方法 (如父进程检测)
    LOW         // 低可靠性方法 (如时间检测)
};

// 检测结果类型
enum class DetectionResult {
    DETECTED,       // 确定检测到调试器
    NOT_DETECTED,   // 确定没有检测到调试器
    INCONCLUSIVE    // 结果不确定
};

// 日志记录类
class DebugLogger {
private:
    static std::mutex log_mutex;
    static std::ofstream log_file;
    static bool initialized;
    
public:
    static void init(const std::string& filename = "debug_detection.log") {
        try {
            std::lock_guard<std::mutex> lock(log_mutex);
            if (!initialized) {
                log_file.open(filename, std::ios::out | std::ios::app);
                initialized = true;
            }
        } catch (const std::exception& e) {
            std::cerr << "初始化日志失败: " << e.what() << std::endl;
        }
    }
    
    static void log(const std::string& method, DetectionResult result, int confidence, 
                   DetectionReliability reliability) {
        try {
            std::lock_guard<std::mutex> lock(log_mutex);
            
            // 获取当前时间
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            std::stringstream time_ss;
            time_ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
            std::string time_str = time_ss.str();
            
            std::string status;
            switch (result) {
                case DetectionResult::DETECTED:
                    status = "检测到调试器";
                    break;
                case DetectionResult::NOT_DETECTED:
                    status = "未检测到调试器";
                    break;
                case DetectionResult::INCONCLUSIVE:
                    status = "结果不确定";
                    break;
            }
            
            std::string reliability_str;
            switch (reliability) {
                case DetectionReliability::HIGH:
                    reliability_str = "高";
                    break;
                case DetectionReliability::MEDIUM:
                    reliability_str = "中";
                    break;
                case DetectionReliability::LOW:
                    reliability_str = "低";
                    break;
            }
            
            std::ostringstream msg_stream;
            msg_stream << time_str << " | " << method << " | " 
                      << status << " | 置信度: " << confidence << "% | 可靠性: " << reliability_str;
            std::string msg = msg_stream.str();
            
            // 记录到文件
            if (log_file.is_open()) {
                log_file << msg << std::endl;
                log_file.flush();
            }
            
            // // 输出到控制台
            // std::cout << "[DEBUG DETECTION] " << msg << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "记录日志失败: " << e.what() << std::endl;
        }
    }
    
    static void warning(int confidence, const std::vector<std::string>& methods) {
        try {
            std::lock_guard<std::mutex> lock(log_mutex);
            
            // 获取当前时间
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            std::stringstream time_ss;
            time_ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
            std::string time_str = time_ss.str();
            
            std::ostringstream warning_stream;
            warning_stream << "警告: 检测到调试器! 置信度: " << confidence << "%";
            
            std::string methods_str;
            if (!methods.empty()) {
                methods_str = " | 检测方法: ";
                for (size_t i = 0; i < methods.size(); ++i) {
                    if (i > 0) methods_str += ", ";
                    methods_str += methods[i];
                }
            }
            
            std::string warning_msg = warning_stream.str() + methods_str;
            
            // 记录到文件
            if (log_file.is_open()) {
                log_file << time_str << " | " << warning_msg << std::endl;
                log_file.flush();
            }
            
            // 输出到控制台
            std::cout << "\n[!] ======================================= [!]" << std::endl;
            std::cout << "[!]     " << warning_msg << std::endl;
            std::cout << "[!] ======================================= [!]\n" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "记录警告失败: " << e.what() << std::endl;
        }
    }
    
    static void close() {
        try {
            std::lock_guard<std::mutex> lock(log_mutex);
            if (log_file.is_open()) {
                log_file.close();
            }
            initialized = false;
        } catch (const std::exception& e) {
            std::cerr << "关闭日志失败: " << e.what() << std::endl;
        }
    }
};

std::mutex DebugLogger::log_mutex;
std::ofstream DebugLogger::log_file;
bool DebugLogger::initialized = false;

// 文件读取辅助类
class FileReader {
public:
    static std::string readFirstLine(const std::string& path) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                return "";
            }
            
            std::string line;
            std::getline(file, line);
            return line;
        } catch (const std::exception&) {
            return "";
        }
    }
    
    static std::vector<std::string> readAllLines(const std::string& path) {
        std::vector<std::string> lines;
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                return lines;
            }
            
            std::string line;
            while (std::getline(file, line)) {
                lines.push_back(line);
            }
        } catch (const std::exception&) {
            // 出错时返回已读取的行
        }
        return lines;
    }
    
    static bool contains(const std::string& text, const std::string& pattern) {
        return text.find(pattern) != std::string::npos;
    }
};

// 检测方法基类
class DetectionMethod {
public:
    virtual ~DetectionMethod() = default;
    virtual DetectionResult detect() = 0;
    virtual std::string getName() const = 0;
    virtual DetectionReliability getReliability() const = 0;
};

// 虚拟化环境检测
class VirtualizationDetection : public DetectionMethod {
    public:
        DetectionResult detect() override {
            Logger::debug("VirtualizationDetection", "开始执行虚拟化环境检测...");
            
            // 组合多种检测方法的结果
            bool detected = false;
            
            // 1. 检测常见虚拟机特有的CPUID特征
            if (checkCpuidVirtualization()) {
                Logger::debug("VirtualizationDetection", "通过CPUID检测到虚拟化环境");
                detected = true;
            }
            
            // 2. 检测常见虚拟机设备和文件
            if (!detected && checkVirtualDevices()) {
                Logger::debug("VirtualizationDetection", "通过设备文件检测到虚拟化环境");
                detected = true;
            }
            
            // 3. 检测DMI信息中的虚拟化痕迹
            if (!detected && checkDMIInfo()) {
                Logger::debug("VirtualizationDetection", "通过DMI信息检测到虚拟化环境");
                detected = true;
            }
            
            // 4. 检测内存中的虚拟化痕迹
            if (!detected && checkMemoryArtifacts()) {
                Logger::debug("VirtualizationDetection", "通过内存痕迹检测到虚拟化环境");
                detected = true;
            }
            
            DetectionResult result = detected ? DetectionResult::DETECTED : DetectionResult::NOT_DETECTED;
            Logger::debug("VirtualizationDetection", "检测完成，结果: " + 
                         std::string(result == DetectionResult::DETECTED ? "检测到虚拟化环境" : "未检测到虚拟化环境"));
            
            return result;
        }
        
        std::string getName() const override {
            return "虚拟化环境检测";
        }
        
        DetectionReliability getReliability() const override {
            return DetectionReliability::MEDIUM;
        }
        
    private:
        // 通过CPUID检测虚拟化环境
        bool checkCpuidVirtualization() {
            Logger::debug("VirtualizationDetection", "执行CPUID检测...");
            
            try {
                // CPUID指令获取处理器信息
                uint32_t eax, ebx, ecx, edx;
                
                // 获取Hypervisor信息
                __asm__ __volatile__(
                    "cpuid"
                    : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                    : "a"(0x1)
                );
                
                // 检查ECX寄存器的第31位，表示存在Hypervisor
                bool hypervisor_present = (ecx & (1 << 31)) != 0;
                
                if (hypervisor_present) {
                    Logger::debug("VirtualizationDetection", "CPUID表明存在Hypervisor");
                    
                    // 获取Hypervisor供应商ID
                    __asm__ __volatile__(
                        "cpuid"
                        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                        : "a"(0x40000000)
                    );
                    
                    // 构造厂商ID字符串
                    char vendor[13];
                    memcpy(vendor, &ebx, 4);
                    memcpy(vendor + 4, &ecx, 4);
                    memcpy(vendor + 8, &edx, 4);
                    vendor[12] = '\0';
                    
                    Logger::debug("VirtualizationDetection", "检测到Hypervisor厂商ID: " + std::string(vendor));
                    
                    // 检查常见虚拟机厂商ID
                    std::string vendorStr(vendor);
                    if (vendorStr == "VMwareVMware" || 
                        vendorStr == "XenVMMXenVMM" || 
                        vendorStr == "KVMKVMKVM" || 
                        vendorStr == "Microsoft Hv" || 
                        vendorStr == "VBoxVBoxVBox") {
                        Logger::debug("VirtualizationDetection", "识别到已知虚拟化厂商: " + vendorStr);
                        return true;
                    }
                } else {
                    Logger::debug("VirtualizationDetection", "CPUID未表明存在Hypervisor");
                }
                
                return false;
            } catch (const std::exception& e) {
                Logger::debug("VirtualizationDetection", "CPUID检测时发生异常: " + std::string(e.what()));
                return false;
            } catch (...) {
                Logger::debug("VirtualizationDetection", "CPUID检测时发生未知异常");
                return false;
            }
        }
        
        // 检测虚拟机特有的设备文件
        bool checkVirtualDevices() {
            Logger::debug("VirtualizationDetection", "检查虚拟机特有设备文件...");
            
            // 常见虚拟机设备文件路径
            std::vector<std::string> vm_devices = {
                "/dev/vmware",
                "/dev/vboxguest",
                "/dev/vboxuser",
                "/dev/hgcpu",
                "/proc/scsi/scsi",  // 检查SCSI设备名称
                "/sys/class/dmi/id/product_name",
                "/sys/class/dmi/id/sys_vendor"
            };
            
            for (const auto& device : vm_devices) {
                struct stat buffer;
                if (stat(device.c_str(), &buffer) == 0) {
                    Logger::debug("VirtualizationDetection", "发现虚拟机设备文件: " + device);
                    
                    // 对于某些文件，检查内容
                    if (device == "/proc/scsi/scsi" || 
                        device == "/sys/class/dmi/id/product_name" || 
                        device == "/sys/class/dmi/id/sys_vendor") {
                        
                        std::ifstream file(device);
                        if (file.is_open()) {
                            std::string content;
                            std::getline(file, content);
                            file.close();
                            
                            Logger::debug("VirtualizationDetection", "设备文件内容: " + content);
                            
                            // 检查内容中是否包含虚拟机特征
                            if (content.find("VMware") != std::string::npos ||
                                content.find("VirtualBox") != std::string::npos ||
                                content.find("QEMU") != std::string::npos ||
                                content.find("Virtual") != std::string::npos ||
                                content.find("Xen") != std::string::npos) {
                                
                                Logger::debug("VirtualizationDetection", "设备文件内容表明存在虚拟化环境");
                                return true;
                            }
                        }
                    } else {
                        // 仅设备存在就确认为虚拟机
                        return true;
                    }
                }
            }
            
            Logger::debug("VirtualizationDetection", "未发现虚拟机特有设备文件");
            return false;
        }
        
        // 检查DMI信息中的虚拟化痕迹
        bool checkDMIInfo() {
            Logger::debug("VirtualizationDetection", "检查DMI信息...");
            
            try {
                // 检查系统制造商
                std::ifstream vendor_file("/sys/class/dmi/id/sys_vendor");
                if (vendor_file.is_open()) {
                    std::string vendor;
                    std::getline(vendor_file, vendor);
                    vendor_file.close();
                    
                    Logger::debug("VirtualizationDetection", "系统制造商: " + vendor);
                    
                    if (vendor.find("VMware") != std::string::npos ||
                        vendor.find("innotek GmbH") != std::string::npos || // VirtualBox
                        vendor.find("QEMU") != std::string::npos ||
                        vendor.find("Microsoft Corporation") != std::string::npos || // Hyper-V
                        vendor.find("Xen") != std::string::npos) {
                        
                        Logger::debug("VirtualizationDetection", "DMI系统制造商表明存在虚拟化环境");
                        return true;
                    }
                }
                
                // 检查产品名称
                std::ifstream product_file("/sys/class/dmi/id/product_name");
                if (product_file.is_open()) {
                    std::string product;
                    std::getline(product_file, product);
                    product_file.close();
                    
                    Logger::debug("VirtualizationDetection", "产品名称: " + product);
                    
                    if (product.find("VMware") != std::string::npos ||
                        product.find("VirtualBox") != std::string::npos ||
                        product.find("Virtual Machine") != std::string::npos ||
                        product.find("HVM domU") != std::string::npos || // Xen
                        product.find("KVM") != std::string::npos) {
                        
                        Logger::debug("VirtualizationDetection", "DMI产品名称表明存在虚拟化环境");
                        return true;
                    }
                }
                
                Logger::debug("VirtualizationDetection", "DMI信息未表明存在虚拟化环境");
                return false;
            } catch (const std::exception& e) {
                Logger::debug("VirtualizationDetection", "检查DMI信息时发生异常: " + std::string(e.what()));
                return false;
            }
        }
        
        // 检查内存中的虚拟化痕迹
        bool checkMemoryArtifacts() {
            Logger::debug("VirtualizationDetection", "检查内存中的虚拟化痕迹...");
            
            try {
                // 尝试在进程映射中查找虚拟机相关模块
                std::ifstream maps_file("/proc/self/maps");
                if (maps_file.is_open()) {
                    std::string line;
                    while (std::getline(maps_file, line)) {
                        if (line.find("vmware") != std::string::npos ||
                            line.find("vbox") != std::string::npos ||
                            line.find("virtualbox") != std::string::npos ||
                            line.find("qemu") != std::string::npos) {
                            
                            Logger::debug("VirtualizationDetection", "在内存映射中发现虚拟化痕迹: " + line);
                            maps_file.close();
                            return true;
                        }
                    }
                    maps_file.close();
                }
                
                // 检查环境变量中的虚拟化痕迹
                const char* vm_env_vars[] = {"VBOX_", "VMWARE_", "QEMU_", "XEN_"};
                for (const auto& prefix : vm_env_vars) {
                    for (char** env = environ; *env != nullptr; env++) {
                        std::string env_var(*env);
                        if (env_var.find(prefix) == 0) {
                            Logger::debug("VirtualizationDetection", "在环境变量中发现虚拟化痕迹: " + env_var);
                            return true;
                        }
                    }
                }
                
                Logger::debug("VirtualizationDetection", "内存中未发现虚拟化痕迹");
                return false;
            } catch (const std::exception& e) {
                Logger::debug("VirtualizationDetection", "检查内存痕迹时发生异常: " + std::string(e.what()));
                return false;
            }
        }
};

    
// TracerPid检测
class TracerPidDetection : public DetectionMethod {
    public:
        DetectionResult detect() override {
            Logger::debug("TracerPidDetection", "开始执行检测...");
            
            try {
                std::string proc_path = "/proc/self/status";
                Logger::debug("TracerPidDetection", "尝试打开文件: " + proc_path);
                
                // 设置文件流超时
                std::ifstream proc_file(proc_path);
                if (!proc_file.is_open()) {
                    Logger::debug("TracerPidDetection", "无法打开文件: " + std::string(strerror(errno)));
                    return DetectionResult::INCONCLUSIVE;
                }
                
                Logger::debug("TracerPidDetection", "文件成功打开，开始读取内容");
                
                // 添加超时机制
                alarm(2); // 设置2秒超时
                
                std::string line;
                bool found_tracer_pid = false;
                while (std::getline(proc_file, line)) {
                    Logger::debug("TracerPidDetection", "读取行: " + line);
                    
                    if (line.compare(0, 10, "TracerPid:") == 0) {
                        found_tracer_pid = true;
                        Logger::debug("TracerPidDetection", "找到TracerPid行: " + line);
                        
                        std::istringstream iss(line.substr(10));
                        int pid;
                        iss >> pid;
                        
                        Logger::debug("TracerPidDetection", "解析到TracerPid值: " + std::to_string(pid));
                        
                        // 取消超时
                        alarm(0);
                        
                        DetectionResult result = pid != 0 ? DetectionResult::DETECTED : DetectionResult::NOT_DETECTED;
                        Logger::debug("TracerPidDetection", "检测结果: " + 
                                     std::string(result == DetectionResult::DETECTED ? "检测到调试器" : "未检测到调试器"));
                        return result;
                    }
                }
                
                // 取消超时
                alarm(0);
                
                if (!found_tracer_pid) {
                    Logger::debug("TracerPidDetection", "未找到TracerPid行");
                }
                
                proc_file.close();
                Logger::debug("TracerPidDetection", "文件已关闭");
                
                return DetectionResult::INCONCLUSIVE;
            } catch (const std::exception& e) {
                // 取消超时
                alarm(0);
                Logger::debug("TracerPidDetection", "捕获到异常: " + std::string(e.what()));
                return DetectionResult::INCONCLUSIVE;
            }
        }
        
        std::string getName() const override {
            return "TracerPid检测";
        }
        
        DetectionReliability getReliability() const override {
            return DetectionReliability::HIGH;
        }
        
        // 构造函数中设置信号处理
        TracerPidDetection() {
            signal(SIGALRM, alarmHandler);
        }
        
    private:
        // 静态信号处理函数
        static void alarmHandler(int sig) {
            Logger::debug("TracerPidDetection", "读取超时！");
            throw std::runtime_error("读取/proc/self/status超时");
        }
};
    
    

// PTRACE检测 - 安全版本（使用Logger类记录日志）
class PtraceDetection : public DetectionMethod {
    public:
        DetectionResult detect() override {
            Logger::debug("PtraceDetection", "开始执行检测...");
            
            try {
                // 使用fork创建子进程进行检测，避免影响主进程
                Logger::debug("PtraceDetection", "正在创建子进程...");
                pid_t child = fork();
                
                if (child == -1) {
                    // fork失败
                    Logger::debug("PtraceDetection", "fork()失败: " + std::string(strerror(errno)));
                    return DetectionResult::INCONCLUSIVE;
                }
                
                if (child == 0) {
                    // 子进程
                    Logger::debug("PtraceDetection-子进程", "执行ptrace(PTRACE_TRACEME)...");
                    int result = ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
                    
                    if (result == -1) {
                        Logger::debug("PtraceDetection-子进程", "ptrace失败: " + std::string(strerror(errno)));
                    } else {
                        Logger::debug("PtraceDetection-子进程", "ptrace成功");
                    }
                    
                    // 根据ptrace结果立即退出
                    _exit(result == -1 ? 1 : 0);
                } else {
                    // 父进程
                    Logger::debug("PtraceDetection", "子进程创建成功，PID: " + std::to_string(child));
                    Logger::debug("PtraceDetection", "等待子进程结束...");
                    
                    int status;
                    // 等待子进程结束
                    pid_t wait_result = waitpid(child, &status, 0);
                    
                    if (wait_result == -1) {
                        Logger::debug("PtraceDetection", "waitpid()失败: " + std::string(strerror(errno)));
                        return DetectionResult::INCONCLUSIVE;
                    }
                    
                    Logger::debug("PtraceDetection", "子进程已结束，状态码: " + std::to_string(status));
                    
                    if (WIFEXITED(status)) {
                        int exit_code = WEXITSTATUS(status);
                        Logger::debug("PtraceDetection", "子进程正常退出，退出码: " + std::to_string(exit_code));
                        
                        // 检查子进程退出状态
                        DetectionResult result = exit_code == 0 ? 
                                                DetectionResult::NOT_DETECTED : 
                                                DetectionResult::DETECTED;
                                                
                        Logger::debug("PtraceDetection", "检测结果: " + 
                                     std::string(result == DetectionResult::DETECTED ? "检测到调试器" : "未检测到调试器"));
                                      
                        return result;
                    } else if (WIFSIGNALED(status)) {
                        Logger::debug("PtraceDetection", "子进程被信号终止: " + std::to_string(WTERMSIG(status)));
                    } else {
                        Logger::debug("PtraceDetection", "子进程异常终止，未知状态");
                    }
                    
                    Logger::debug("PtraceDetection", "返回不确定结果");
                    return DetectionResult::INCONCLUSIVE;
                }
            } catch (const std::exception& e) {
                Logger::debug("PtraceDetection", "捕获到异常: " + std::string(e.what()));
                return DetectionResult::INCONCLUSIVE;
            } catch (...) {
                Logger::debug("PtraceDetection", "捕获到未知异常");
                return DetectionResult::INCONCLUSIVE;
            }
        }
        
        std::string getName() const override {
            return "PTRACE检测";
        }
        
        DetectionReliability getReliability() const override {
            return DetectionReliability::HIGH;
        }
};
    
    
    
    

// 父进程检测
class ParentProcessDetection : public DetectionMethod {
    public:
        DetectionResult detect() override {
            Logger::debug("ParentProcessDetection", "开始执行父进程检测...");
            
            try {
                pid_t ppid = getppid();
                Logger::debug("ParentProcessDetection", "获取到父进程ID: " + std::to_string(ppid));
                
                std::string proc_path = "/proc/" + std::to_string(ppid) + "/comm";
                Logger::debug("ParentProcessDetection", "尝试读取父进程信息文件: " + proc_path);
                
                std::string parent_name = FileReader::readFirstLine(proc_path);
                if (parent_name.empty()) {
                    Logger::debug("ParentProcessDetection", "无法读取父进程名称，文件为空或不存在");
                    return DetectionResult::INCONCLUSIVE;
                }
                
                // 移除可能的换行符
                if (!parent_name.empty() && parent_name.back() == '\n') {
                    parent_name.pop_back();
                }
                
                Logger::debug("ParentProcessDetection", "父进程名称: " + parent_name);
                
                // 检查常见调试器名称
                std::vector<std::string> debuggers = {
                    "gdb", "lldb", "strace", "ltrace", "valgrind", "frida"
                };
                
                Logger::debug("ParentProcessDetection", "检查父进程是否为已知调试器: " + 
                              std::accumulate(std::next(debuggers.begin()), debuggers.end(), 
                                             debuggers[0], 
                                             [](std::string a, std::string b) { return a + ", " + b; }));
                
                for (const auto& debugger : debuggers) {
                    if (parent_name == debugger) {
                        Logger::debug("ParentProcessDetection", "检测到父进程为已知调试器: " + debugger);
                        return DetectionResult::DETECTED;
                    }
                }
                
                Logger::debug("ParentProcessDetection", "父进程不是已知调试器");
                return DetectionResult::NOT_DETECTED;
            } catch (const std::exception& e) {
                Logger::debug("ParentProcessDetection", "检测过程发生异常: " + std::string(e.what()));
                return DetectionResult::INCONCLUSIVE;
            }
        }
        
        std::string getName() const override {
            return "父进程检测";
        }
        
        DetectionReliability getReliability() const override {
            return DetectionReliability::MEDIUM;
        }
};
    

// 环境变量检测
class EnvironmentVariableDetection : public DetectionMethod {
    public:
        DetectionResult detect() override {
            Logger::debug("EnvironmentVariableDetection", "开始执行环境变量检测...");
            
            try {
                std::vector<std::string> suspicious_vars = {
                    "LD_PRELOAD", "LD_DEBUG", 
                    "DYLD_INSERT_LIBRARIES", "DYLD_FORCE_FLAT_NAMESPACE"
                };
                
                Logger::debug("EnvironmentVariableDetection", "检查可疑环境变量: " + 
                              std::accumulate(std::next(suspicious_vars.begin()), suspicious_vars.end(), 
                                             suspicious_vars[0], 
                                             [](std::string a, std::string b) { return a + ", " + b; }));
                
                for (const auto& var : suspicious_vars) {
                    const char* value = std::getenv(var.c_str());
                    if (value != nullptr) {
                        Logger::debug("EnvironmentVariableDetection", "检测到可疑环境变量: " + var + " = " + value);
                        return DetectionResult::DETECTED;
                    } else {
                        Logger::debug("EnvironmentVariableDetection", "未检测到环境变量: " + var);
                    }
                }
                
                Logger::debug("EnvironmentVariableDetection", "未检测到任何可疑环境变量");
                return DetectionResult::NOT_DETECTED;
            } catch (const std::exception& e) {
                Logger::debug("EnvironmentVariableDetection", "检测过程发生异常: " + std::string(e.what()));
                return DetectionResult::INCONCLUSIVE;
            }
        }
        
        std::string getName() const override {
            return "环境变量检测";
        }
        
        DetectionReliability getReliability() const override {
            return DetectionReliability::MEDIUM;
        }
};
    

// 内存映射检测
class MemoryMappingDetection : public DetectionMethod {
    public:
        DetectionResult detect() override {
            Logger::debug("MemoryMappingDetection", "开始执行内存映射检测...");
            
            try {
                std::string maps_path = "/proc/self/maps";
                Logger::debug("MemoryMappingDetection", "尝试读取文件: " + maps_path);
                
                auto lines = FileReader::readAllLines(maps_path);
                Logger::debug("MemoryMappingDetection", "成功读取内存映射文件，共 " + std::to_string(lines.size()) + " 行");
                
                std::vector<std::string> patterns = {
                    "gdb", "valgrind", "frida", "inject", "debug"
                };
                
                Logger::debug("MemoryMappingDetection", "开始检查可疑模式: " + 
                              std::accumulate(std::next(patterns.begin()), patterns.end(), patterns[0],
                                             [](std::string a, std::string b) { return a + ", " + b; }));
                
                for (const auto& line : lines) {
                    for (const auto& pattern : patterns) {
                        if (FileReader::contains(line, pattern)) {
                            Logger::debug("MemoryMappingDetection", "检测到可疑模式 '" + pattern + "' 在行: " + line);
                            return DetectionResult::DETECTED;
                        }
                    }
                }
                
                Logger::debug("MemoryMappingDetection", "未在内存映射中检测到可疑模式");
                return DetectionResult::NOT_DETECTED;
            } catch (const std::exception& e) {
                Logger::debug("MemoryMappingDetection", "检测过程发生异常: " + std::string(e.what()));
                return DetectionResult::INCONCLUSIVE;
            }
        }
        
        std::string getName() const override {
            return "内存映射检测";
        }
        
        DetectionReliability getReliability() const override {
            return DetectionReliability::MEDIUM;
        }
};
    

// 时间检测
class TimingDetection : public DetectionMethod {
    public:
        DetectionResult detect() override {
            Logger::debug("TimingDetection", "开始执行时间检测...");
            
            try {
                // 获取高精度时间
                Logger::debug("TimingDetection", "开始计时");
                auto start = std::chrono::high_resolution_clock::now();
                
                // 执行一系列简单操作
                volatile int sum = 0;
                for (int i = 0; i < 10000; i++) {
                    sum += i;
                }
                
                auto end = std::chrono::high_resolution_clock::now();
                
                // 计算执行时间（纳秒）
                auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                Logger::debug("TimingDetection", "执行循环操作耗时: " + std::to_string(elapsed) + " 纳秒");
                
                // 在调试器下，这个操作会明显变慢
                if (elapsed > 1000000) { // 1ms阈值
                    Logger::debug("TimingDetection", "执行时间超过1ms阈值，很可能存在调试器");
                    return DetectionResult::DETECTED;
                } else if (elapsed > 500000) { // 0.5ms可疑阈值
                    Logger::debug("TimingDetection", "执行时间超过0.5ms阈值，可能存在调试器但不确定");
                    return DetectionResult::INCONCLUSIVE;
                } else {
                    Logger::debug("TimingDetection", "执行时间正常，未检测到调试器");
                    return DetectionResult::NOT_DETECTED;
                }
            } catch (const std::exception& e) {
                Logger::debug("TimingDetection", "检测过程发生异常: " + std::string(e.what()));
                return DetectionResult::INCONCLUSIVE;
            }
        }
        
        std::string getName() const override {
            return "执行时间检测";
        }
        
        DetectionReliability getReliability() const override {
            return DetectionReliability::LOW;
        }
};
    

// 进程等待通道检测
class WchanDetection : public DetectionMethod {
    public:
        DetectionResult detect() override {
            Logger::debug("WchanDetection", "开始执行进程等待通道检测...");
            
            try {
                std::string wchan_path = "/proc/self/wchan";
                Logger::debug("WchanDetection", "尝试读取文件: " + wchan_path);
                
                std::string wchan = FileReader::readFirstLine(wchan_path);
                
                if (wchan.empty()) {
                    Logger::debug("WchanDetection", "读取到的wchan内容为空");
                    return DetectionResult::INCONCLUSIVE;
                }
                
                Logger::debug("WchanDetection", "读取到的wchan内容: " + wchan);
                
                // 调试时可能会显示特定的等待通道
                if (FileReader::contains(wchan, "ptrace")) {
                    Logger::debug("WchanDetection", "检测到wchan包含'ptrace'，可能存在调试器");
                    return DetectionResult::DETECTED;
                } 
                
                if (FileReader::contains(wchan, "debug")) {
                    Logger::debug("WchanDetection", "检测到wchan包含'debug'，可能存在调试器");
                    return DetectionResult::DETECTED;
                }
                
                Logger::debug("WchanDetection", "wchan内容正常，未检测到调试器痕迹");
                return DetectionResult::NOT_DETECTED;
            } catch (const std::exception& e) {
                Logger::debug("WchanDetection", "检测过程发生异常: " + std::string(e.what()));
                return DetectionResult::INCONCLUSIVE;
            }
        }
        
        std::string getName() const override {
            return "进程等待通道检测";
        }
        
        DetectionReliability getReliability() const override {
            return DetectionReliability::MEDIUM;
        }
};
    

// 进程状态检测
class ProcessStateDetection : public DetectionMethod {
    public:
        DetectionResult detect() override {
            Logger::debug("ProcessStateDetection", "开始执行进程状态检测...");
            
            try {
                std::string stat_path = "/proc/self/stat";
                Logger::debug("ProcessStateDetection", "尝试读取文件: " + stat_path);
                
                std::string stat_line = FileReader::readFirstLine(stat_path);
                
                if (stat_line.empty()) {
                    Logger::debug("ProcessStateDetection", "读取到的文件内容为空");
                    return DetectionResult::INCONCLUSIVE;
                }
                
                Logger::debug("ProcessStateDetection", "读取到的进程状态行: " + stat_line.substr(0, 50) + "...");
                
                // 解析状态，格式为: pid (comm) state ...
                size_t first_paren = stat_line.find('(');
                size_t last_paren = stat_line.find(')', first_paren);
                
                if (first_paren == std::string::npos || last_paren == std::string::npos) {
                    Logger::debug("ProcessStateDetection", "无法解析进程状态行格式");
                    return DetectionResult::INCONCLUSIVE;
                }
                
                // 状态字符在第二个括号后面
                if (last_paren + 2 < stat_line.size()) {
                    char state = stat_line[last_paren + 2];
                    Logger::debug("ProcessStateDetection", "进程状态字符: " + std::string(1, state));
                    
                    // 'T'状态通常表示被跟踪或停止
                    if (state == 'T') {
                        Logger::debug("ProcessStateDetection", "检测到进程处于被跟踪或停止状态(T)，可能存在调试器");
                        return DetectionResult::DETECTED;
                    }
                    
                    Logger::debug("ProcessStateDetection", "进程状态正常，未检测到调试器");
                    return DetectionResult::NOT_DETECTED;
                }
                
                Logger::debug("ProcessStateDetection", "进程状态字符解析失败");
                return DetectionResult::INCONCLUSIVE;
            } catch (const std::exception& e) {
                Logger::debug("ProcessStateDetection", "检测过程发生异常: " + std::string(e.what()));
                return DetectionResult::INCONCLUSIVE;
            }
        }
        
        std::string getName() const override {
            return "进程状态检测";
        }
        
        DetectionReliability getReliability() const override {
            return DetectionReliability::MEDIUM;
        }
};
    

// 调试检测器
class DebugDetector {
private:
    // 检测方法组
    struct DetectionGroup {
        std::string name;
        std::vector<std::unique_ptr<DetectionMethod>> methods;
    };
    
    // 检测结果
    struct DetectionOutcome {
        std::string method;
        DetectionResult result;
        DetectionReliability reliability;
    };
    
    std::vector<DetectionGroup> detection_groups;
    std::vector<DetectionOutcome> results;
    std::mutex results_mutex;
    int confidence_threshold;
    std::mt19937 rng; // 随机数生成器
    std::atomic<bool> running;
    
    // 初始化检测方法组
    void initialize_detection_groups() {
        // 高可靠性组 - ptrace相关检测
        DetectionGroup high_reliability_group;
        high_reliability_group.name = "高可靠性检测组";
        high_reliability_group.methods.push_back(std::make_unique<TracerPidDetection>());
        high_reliability_group.methods.push_back(std::make_unique<PtraceDetection>());
        detection_groups.push_back(std::move(high_reliability_group));
        
        // 中等可靠性组 - 进程和环境检测
        DetectionGroup medium_reliability_group;
        medium_reliability_group.name = "中等可靠性检测组";
        medium_reliability_group.methods.push_back(std::make_unique<ParentProcessDetection>());
        medium_reliability_group.methods.push_back(std::make_unique<EnvironmentVariableDetection>());
        medium_reliability_group.methods.push_back(std::make_unique<MemoryMappingDetection>());
        medium_reliability_group.methods.push_back(std::make_unique<WchanDetection>());
        medium_reliability_group.methods.push_back(std::make_unique<ProcessStateDetection>());
        medium_reliability_group.methods.push_back(std::make_unique<VirtualizationDetection>());
        detection_groups.push_back(std::move(medium_reliability_group));
        
        // 低可靠性组 - 行为和时间检测
        DetectionGroup low_reliability_group;
        low_reliability_group.name = "低可靠性检测组";
        low_reliability_group.methods.push_back(std::make_unique<TimingDetection>());
        detection_groups.push_back(std::move(low_reliability_group));
    }
    
    // 从一个组中选择多个检测方法
    std::vector<DetectionMethod*> select_methods_from_group(const DetectionGroup& group, int count) {
        std::vector<DetectionMethod*> selected;
        if (group.methods.empty() || count <= 0) {
            return selected;
        }
        
        // 如果请求的数量超过可用方法，返回所有方法
        if (count >= static_cast<int>(group.methods.size())) {
            for (const auto& method : group.methods) {
                selected.push_back(method.get());
            }
            return selected;
        }
        
        // 随机选择不重复的方法
        std::vector<size_t> indices(group.methods.size());
        for (size_t i = 0; i < indices.size(); ++i) {
            indices[i] = i;
        }
        std::shuffle(indices.begin(), indices.end(), rng);
        
        for (int i = 0; i < count; ++i) {
            selected.push_back(group.methods[indices[i]].get());
        }
        
        return selected;
    }
    
    // 计算置信度 - 根据检测结果和可靠性
    int calculate_confidence() {
        // 移除互斥锁，因为调用者已经获取了锁
        
        Logger::debug("DetectionManager", "开始计算置信度...");
        
        if (results.empty()) {
            Logger::debug("DetectionManager", "结果列表为空，置信度为0");
            return 0;
        }
        
        // 首先检查高可靠性方法
        for (const auto& outcome : results) {
            if (outcome.reliability == DetectionReliability::HIGH && 
                outcome.result == DetectionResult::DETECTED) {
                Logger::debug("DetectionManager", "高可靠性方法检测到调试器，置信度100%");
                return 100;
            }
        }
        
        // 计算各可靠性级别的检测结果
        int high_detected = 0, high_total = 0;
        int medium_detected = 0, medium_total = 0;
        int low_detected = 0, low_total = 0;
        
        for (const auto& outcome : results) {
            if (outcome.result == DetectionResult::INCONCLUSIVE) {
                Logger::debug("DetectionManager", "忽略不确定的结果: " + outcome.method);
                continue;
            }
            
            bool is_detected = (outcome.result == DetectionResult::DETECTED);
            
            switch (outcome.reliability) {
                case DetectionReliability::HIGH:
                    high_total++;
                    if (is_detected) high_detected++;
                    Logger::debug("DetectionManager", "高可靠性方法 " + outcome.method + 
                                 (is_detected ? " 检测到调试器" : " 未检测到调试器"));
                    break;
                case DetectionReliability::MEDIUM:
                    medium_total++;
                    if (is_detected) medium_detected++;
                    Logger::debug("DetectionManager", "中可靠性方法 " + outcome.method + 
                                 (is_detected ? " 检测到调试器" : " 未检测到调试器"));
                    break;
                case DetectionReliability::LOW:
                    low_total++;
                    if (is_detected) low_detected++;
                    Logger::debug("DetectionManager", "低可靠性方法 " + outcome.method + 
                                 (is_detected ? " 检测到调试器" : " 未检测到调试器"));
                    break;
            }
        }
        
        // 计算加权置信度
        const int HIGH_WEIGHT = 70;
        const int MEDIUM_WEIGHT = 25;
        const int LOW_WEIGHT = 5;
        
        int weighted_sum = 0;
        int total_weight = 0;
        
        if (high_total > 0) {
            int high_confidence = high_detected * 100 / high_total;
            weighted_sum += high_confidence * HIGH_WEIGHT;
            total_weight += HIGH_WEIGHT;
            Logger::debug("DetectionManager", "高可靠性方法置信度: " + std::to_string(high_confidence) + "%");
        }
        
        if (medium_total > 0) {
            int medium_confidence = medium_detected * 100 / medium_total;
            weighted_sum += medium_confidence * MEDIUM_WEIGHT;
            total_weight += MEDIUM_WEIGHT;
            Logger::debug("DetectionManager", "中可靠性方法置信度: " + std::to_string(medium_confidence) + "%");
        }
        
        if (low_total > 0) {
            int low_confidence = low_detected * 100 / low_total;
            weighted_sum += low_confidence * LOW_WEIGHT;
            total_weight += LOW_WEIGHT;
            Logger::debug("DetectionManager", "低可靠性方法置信度: " + std::to_string(low_confidence) + "%");
        }
        
        if (total_weight == 0) {
            Logger::debug("DetectionManager", "没有有效的检测结果，置信度为0");
            return 0;
        }
        
        int final_confidence = weighted_sum / total_weight;
        Logger::debug("DetectionManager", "最终计算的置信度: " + std::to_string(final_confidence) + "%");
        
        return final_confidence;
    }
    
public:
    DebugDetector(int threshold = 85) : 
        confidence_threshold(threshold), 
        rng(std::random_device{}()),
        running(false) {
        try {
            DebugLogger::init();
            initialize_detection_groups();
        } catch (const std::exception& e) {
            std::cerr << "初始化调试器检测器失败: " << e.what() << std::endl;
        }
    }
    
    // 添加检测结果
    void add_result(const std::string& method, DetectionResult result, DetectionReliability reliability) {
        Logger::debug("DetectionManager", "开始添加检测结果: " + method + 
                      ", 结果: " + std::to_string(static_cast<int>(result)) + 
                      ", 可靠性: " + std::to_string(static_cast<int>(reliability)));
        
        try {
            Logger::debug("DetectionManager", "尝试获取结果互斥锁...");
            std::lock_guard<std::mutex> lock(results_mutex);
            Logger::debug("DetectionManager", "互斥锁获取成功");
            
            Logger::debug("DetectionManager", "添加结果到结果列表");
            results.push_back({method, result, reliability});
            
            // 记录日志
            Logger::debug("DetectionManager", "开始计算当前置信度...");
            int current_confidence = -1;
            try {
                current_confidence = calculate_confidence();
                Logger::debug("DetectionManager", "当前置信度计算结果: " + std::to_string(current_confidence));
            } catch (const std::exception& e) {
                Logger::debug("DetectionManager", "计算置信度时发生异常: " + std::string(e.what()));
                return;
            }
            
            Logger::debug("DetectionManager", "调用DebugLogger记录日志...");
            try {
                DebugLogger::log(method, result, current_confidence, reliability);
                Logger::debug("DetectionManager", "日志记录成功");
            } catch (const std::exception& e) {
                Logger::debug("DetectionManager", "记录日志时发生异常: " + std::string(e.what()));
            }
            
            // 如果置信度超过阈值，显示警告
            if (current_confidence >= confidence_threshold) {
                Logger::debug("DetectionManager", "置信度超过阈值，准备显示警告...");
                std::vector<std::string> detected_methods;
                
                for (const auto& r : results) {
                    if (r.result == DetectionResult::DETECTED) {
                        detected_methods.push_back(r.method);
                    }
                }
                
                Logger::debug("DetectionManager", "检测到的方法数量: " + std::to_string(detected_methods.size()));
                
                try {
                    DebugLogger::warning(current_confidence, detected_methods);
                    Logger::debug("DetectionManager", "警告显示成功");
                } catch (const std::exception& e) {
                    Logger::debug("DetectionManager", "显示警告时发生异常: " + std::string(e.what()));
                }
            } else {
                Logger::debug("DetectionManager", "置信度未超过阈值，不显示警告");
            }
            
            Logger::debug("DetectionManager", "结果添加完成");
        } catch (const std::exception& e) {
            Logger::debug("DetectionManager", "添加检测结果时发生异常: " + std::string(e.what()));
        } catch (...) {
            Logger::debug("DetectionManager", "添加检测结果时发生未知异常");
        }
    }
    
    
    
    // 执行高可靠性组的检测
    void run_high_reliability_detection() {
        try {
            if (detection_groups.empty() || !running) return;
            
            // 从高可靠性组中选择所有方法
            const auto& high_group = detection_groups[0]; // 第一组是高可靠性组
            auto methods = select_methods_from_group(high_group, high_group.methods.size());
            for (auto method : methods) {
                if (!running) break;
                
                auto result = method->detect();
                add_result(method->getName(), result, method->getReliability());
                
                // 如果高可靠性方法检测到调试器，立即返回
                if (result == DetectionResult::DETECTED) {
                    break;
                }
                
                // 短暂延迟
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        } catch (const std::exception& e) {
            std::cerr << "执行高可靠性检测时发生异常: " << e.what() << std::endl;
        }
    }
    
    // 执行中等可靠性组的检测
    void run_medium_reliability_detection(int count = 2) {
        try {
            if (detection_groups.size() < 2 || !running) return;
            
            // 从中等可靠性组中选择指定数量的方法
            const auto& medium_group = detection_groups[1]; // 第二组是中等可靠性组
            auto methods = select_methods_from_group(medium_group, count);
            
            for (auto method : methods) {
                if (!running) break;
                
                auto result = method->detect();
                add_result(method->getName(), result, method->getReliability());
                
                // 短暂延迟
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        } catch (const std::exception& e) {
            std::cerr << "执行中等可靠性检测时发生异常: " << e.what() << std::endl;
        }
    }
    
    // 执行低可靠性组的检测
    void run_low_reliability_detection() {
        try {
            if (detection_groups.size() < 3 || !running) return;
            
            // 从低可靠性组中选择所有方法
            const auto& low_group = detection_groups[2]; // 第三组是低可靠性组
            auto methods = select_methods_from_group(low_group, low_group.methods.size());
            
            for (auto method : methods) {
                if (!running) break;
                
                auto result = method->detect();
                add_result(method->getName(), result, method->getReliability());
                
                // 短暂延迟
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        } catch (const std::exception& e) {
            std::cerr << "执行低可靠性检测时发生异常: " << e.what() << std::endl;
        }
    }
    
    // 执行完整的检测策略
    void run_comprehensive_detection() {
        try {
            // 首先运行高可靠性检测
            run_high_reliability_detection();
            
            // 检查是否已经确定有调试器
            int confidence = calculate_confidence();
            if (confidence >= 100) {
                return; // 已经100%确定有调试器，不需要继续检测
            }
            
            // 运行中等可靠性检测
            run_medium_reliability_detection(2);
            
            // 再次检查置信度
            confidence = calculate_confidence();
            if (confidence >= confidence_threshold) {
                return; // 已经超过阈值，不需要继续检测
            }
            
            // 最后运行低可靠性检测
            run_low_reliability_detection();
        } catch (const std::exception& e) {
            std::cerr << "执行综合检测时发生异常: " << e.what() << std::endl;
        }
    }
    
    // 获取当前置信度
    int get_confidence() {
        return calculate_confidence();
    }
    
    // 获取检测到调试器的方法列表
    std::vector<std::string> get_detection_methods() {
        std::lock_guard<std::mutex> lock(results_mutex);
        std::vector<std::string> detected_methods;
        
        for (const auto& result : results) {
            if (result.result == DetectionResult::DETECTED) {
                detected_methods.push_back(result.method);
            }
        }
        
        return detected_methods;
    }
    
    // 清除结果
    void clear_results() {
        try {
            std::lock_guard<std::mutex> lock(results_mutex);
            results.clear();
        } catch (const std::exception& e) {
            std::cerr << "清除结果时发生异常: " << e.what() << std::endl;
        }
    }
    
    // 启动检测
    void start() {
        running = true;
    }
    
    // 停止检测
    void stop() {
        running = false;
    }
    
    // 是否正在运行
    bool is_running() const {
        return running;
    }
    
    // 析构函数
    ~DebugDetector() {
        try {
            stop();
            DebugLogger::close();
        } catch (const std::exception& e) {
            std::cerr << "销毁检测器时发生异常: " << e.what() << std::endl;
        }
    }
};

// 全局检测器管理类
class DetectorManager {
private:
    static std::unique_ptr<DebugDetector> detector;
    static std::thread detection_thread;
    static std::mutex detector_mutex;
    
public:
    static DebugDetector* get_instance() {
        try {
            std::lock_guard<std::mutex> lock(detector_mutex);
            if (!detector) {
                detector = std::make_unique<DebugDetector>(85);
            }
            return detector.get();
        } catch (const std::exception& e) {
            std::cerr << "获取检测器实例时发生异常: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    // 执行一次完整检测
    static int perform_comprehensive_detection() {
        try {
            auto detector_ptr = get_instance();
            if (!detector_ptr) return 0;
            
            detector_ptr->start();
            detector_ptr->clear_results();
            detector_ptr->run_comprehensive_detection();
            int confidence = detector_ptr->get_confidence();
            detector_ptr->stop();
            return confidence;
        } catch (const std::exception& e) {
            std::cerr << "执行完整检测时发生错误: " << e.what() << std::endl;
            return 0;
        }
    }
    
    // 只执行高可靠性检测
    static int perform_high_reliability_detection() {
        try {
            auto detector_ptr = get_instance();
            if (!detector_ptr) return 0;
            
            detector_ptr->start();
            detector_ptr->clear_results();
            detector_ptr->run_high_reliability_detection();
            int confidence = detector_ptr->get_confidence();
            detector_ptr->stop();
            return confidence;
        } catch (const std::exception& e) {
            std::cerr << "执行高可靠性检测时发生错误: " << e.what() << std::endl;
            return 0;
        }
    }
    
    // 定期检测函数
    static void periodic_detection_function(bool comprehensive) {
        try {
            auto detector_ptr = get_instance();
            if (!detector_ptr) return;
            
            detector_ptr->start();
            
            while (detector_ptr->is_running()) {
                detector_ptr->clear_results();
                
                if (comprehensive) {
                    detector_ptr->run_comprehensive_detection();
                } else {
                    // 先执行高可靠性检测
                    detector_ptr->run_high_reliability_detection();
                    
                    // 如果没有100%确定，再执行部分中等可靠性检测
                    if (detector_ptr->get_confidence() < 100) {
                        detector_ptr->run_medium_reliability_detection(1);
                    }
                }
                
                // 检测间隔 - 高可靠性检测更频繁
                std::this_thread::sleep_for(
                    comprehensive ? 
                    std::chrono::milliseconds(5000) :  // 完整检测每5秒
                    std::chrono::milliseconds(2000)    // 简化检测每2秒
                );
            }
        } catch (const std::exception& e) {
            std::cerr << "定期检测线程发生异常: " << e.what() << std::endl;
        }
    }
    
    // 启动定期完整检测
    static void start_comprehensive_periodic_detection() {
        try {
            std::lock_guard<std::mutex> lock(detector_mutex);
            
            // 如果已经有线程在运行，先停止它
            if (detector && detector->is_running()) {
                detector->stop();
                if (detection_thread.joinable()) {
                    detection_thread.join();
                }
            }
            
            // 创建检测器（如果还没有）
            if (!detector) {
                detector = std::make_unique<DebugDetector>(85);
            }
            
            // 启动新的检测线程 - 完整检测
            detection_thread = std::thread(periodic_detection_function, true);
            detection_thread.detach(); // 分离线程，让它在后台运行
        } catch (const std::exception& e) {
            std::cerr << "启动定期完整检测时发生异常: " << e.what() << std::endl;
        }
    }
    
    // 启动定期简化检测（主要是高可靠性方法）
    static void start_lightweight_periodic_detection() {
        try {
            std::lock_guard<std::mutex> lock(detector_mutex);
            
            // 如果已经有线程在运行，先停止它
            if (detector && detector->is_running()) {
                detector->stop();
                if (detection_thread.joinable()) {
                    detection_thread.join();
                }
            }
            
            // 创建检测器（如果还没有）
            if (!detector) {
                detector = std::make_unique<DebugDetector>(85);
            }
            
            // 启动新的检测线程 - 简化检测
            detection_thread = std::thread(periodic_detection_function, false);
            detection_thread.detach(); // 分离线程，让它在后台运行
        } catch (const std::exception& e) {
            std::cerr << "启动定期简化检测时发生异常: " << e.what() << std::endl;
        }
    }
    
    // 停止定期检测
    static void stop_periodic_detection() {
        try {
            std::lock_guard<std::mutex> lock(detector_mutex);
            if (detector) {
                detector->stop();
            }
        } catch (const std::exception& e) {
            std::cerr << "停止定期检测时发生异常: " << e.what() << std::endl;
        }
    }
    
    // 获取检测到的方法列表
    static std::vector<std::string> get_detected_methods() {
        try {
            auto detector_ptr = get_instance();
            if (!detector_ptr) return {};
            
            return detector_ptr->get_detection_methods();
        } catch (const std::exception& e) {
            std::cerr << "获取检测方法时发生异常: " << e.what() << std::endl;
            return {};
        }
    }
    
    // 清理资源
    static void cleanup() {
        try {
            std::lock_guard<std::mutex> lock(detector_mutex);
            if (detector) {
                detector->stop();
                if (detection_thread.joinable()) {
                    detection_thread.join();
                }
                detector.reset();
            }
        } catch (const std::exception& e) {
            std::cerr << "清理检测器资源时发生异常: " << e.what() << std::endl;
        }
    }
};

// 静态成员初始化
std::unique_ptr<DebugDetector> DetectorManager::detector = nullptr;
std::thread DetectorManager::detection_thread;
std::mutex DetectorManager::detector_mutex;

// 程序退出时清理资源
class DetectorCleaner {
public:
    ~DetectorCleaner() {
        DetectorManager::cleanup();
    }
};

// 全局清理器对象
static DetectorCleaner g_cleaner;

// 公开API
// 执行一次完整检测，返回置信度
int perform_debug_detection() {
    return DetectorManager::perform_comprehensive_detection();
}

// 执行一次高可靠性检测，返回置信度
int perform_quick_debug_detection() {
    return DetectorManager::perform_high_reliability_detection();
}

// 启动定期完整检测（每5秒执行一次完整检测）
void start_comprehensive_detection() {
    DetectorManager::start_comprehensive_periodic_detection();
}

// 启动定期简化检测（每2秒执行一次高可靠性检测）
void start_lightweight_detection() {
    DetectorManager::start_lightweight_periodic_detection();
}

// 停止定期检测
void stop_detection() {
    DetectorManager::stop_periodic_detection();
}

// 获取检测到的方法列表
std::vector<std::string> get_detected_methods() {
    return DetectorManager::get_detected_methods();
}
