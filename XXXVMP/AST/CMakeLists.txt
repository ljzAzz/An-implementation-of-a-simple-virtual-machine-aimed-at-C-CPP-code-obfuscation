cmake_minimum_required(VERSION 3.20)
project(clang_extern_collector)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM 18.1.8 REQUIRED CONFIG)
find_package(Clang 18.1.8 REQUIRED CONFIG)

include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

add_library(ExternCollectorPlugin SHARED
    ExternalFunctionCollector.cpp
)

target_compile_options(ExternCollectorPlugin PRIVATE
    -frtti
    -fPIC
    -Wno-unused-parameter
)

target_link_libraries(ExternCollectorPlugin PRIVATE
    clangAST
    clangBasic
    clangFrontend
    clangSerialization
    clangTooling
    clangFrontend
    clangDriver
)

