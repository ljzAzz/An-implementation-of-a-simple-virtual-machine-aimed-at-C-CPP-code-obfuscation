cmake_minimum_required(VERSION 3.10)
project(FunctionCallRewriter)

# 查找 LLVM 和 Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# 设置 LLVM 和 Clang 的包含路径
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 设置插件目标
add_library(FunctionCallRewriter MODULE
  ReWrite.cpp
)

# 设置 C++ 标准
target_compile_features(FunctionCallRewriter PRIVATE cxx_std_23)

# 链接 LLVM 和 Clang 库
target_link_libraries(FunctionCallRewriter
  PRIVATE
  clangAST
  clangBasic
  clangFrontend
  clangRewrite
  clangTooling
)
