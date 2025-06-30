cmake_minimum_required(VERSION 3.20)
project(clangAttrPlugin)

# 设置C++标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找LLVM/Clang 18
find_package(LLVM 18.1.8 REQUIRED CONFIG)
find_package(Clang 18.1.8 REQUIRED CONFIG)

# 设置包含目录
include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

# 添加插件目标
add_library(AttrPlugin SHARED
    AttrPlugin.cpp
)

# 设置编译选项
target_compile_options(AttrPlugin PRIVATE
    -frtti
    -fPIC
    -Wno-unused-parameter
)

# 链接Clang库
target_link_libraries(AttrPlugin PRIVATE
    clangAST
    clangBasic
    clangFrontend
    clangSerialization
    clangTooling
    clangFrontend
    clangDriver
)
