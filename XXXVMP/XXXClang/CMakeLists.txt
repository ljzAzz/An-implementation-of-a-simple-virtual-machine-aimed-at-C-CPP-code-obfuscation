cmake_minimum_required(VERSION 3.20)
project(Xlang LANGUAGES CXX)

# 配置LLVM/Clang依赖路径（可通过命令行-D参数覆盖）
set(LLVM_DIR "/usr/lib/llvm-18/cmake" CACHE PATH "LLVM 18的CMake配置路径")
set(Clang_DIR "${LLVM_DIR}/clang" CACHE PATH "Clang的CMake配置路径")

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找LLVM/Clang 18
find_package(LLVM 18.1.8 REQUIRED CONFIG)
find_package(Clang 18.1.8 REQUIRED CONFIG)

# 启用高级调试符号
add_compile_options(-g -fno-omit-frame-pointer)

# 查找LLVM和Clang组件
find_package(LLVM 18.1.8 REQUIRED CONFIG
    COMPONENTS
    Support
    Demangle
    Option
    FrontendOpenMP
)

find_package(Clang 18.1.8 REQUIRED CONFIG
    COMPONENTS
    Driver
    Frontend
    Basic
    Parse
    Serialization
)

# 配置头文件包含路径
include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
    SYSTEM  # 避免编译器警告
)

# 定义可执行目标
add_executable(Xlang
    XXXClang.cpp  # 替换为实际源文件
    /home/ljz/XXXVMP/Vmp/XXXTypeGen.cpp
    /home/ljz/XXXVMP/Vmp/Pass.cpp
    /home/ljz/XXXVMP/Vmp/Linker.cpp
    /home/ljz/XXXVMP/Vmp/XXXVMPCTX.cpp
)

# 链接LLVM/Clang组件库
target_link_libraries(Xlang
    PRIVATE
    clangDriver
    clangFrontend
    clangBasic
    clangSerialization
    clangParse
    LLVMSupport
    LLVMOption

)

# 处理Linux平台链接顺序问题
if(UNIX AND NOT APPLE)
    target_link_options(Xlang
        PRIVATE
        "-Wl,--no-as-needed"  # 强制链接所有指定库
        "-Wl,--start-group"   # 处理循环依赖
        "-Wl,--end-group"
    )
endif()

# 配置安装规则（可选）
install(TARGETS Xlang
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 用户提示信息
if(NOT EXISTS "${LLVM_DIR}")
    message(WARNING "LLVM 18未在默认路径找到，请通过-DLLVM_DIR=指定路径")
endif()
