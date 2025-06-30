cmake_minimum_required(VERSION 3.20)
project(VmpPass)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -frtti")
# 查找LLVM包
find_package(LLVM 18.1.8 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# 设置LLVM相关选项
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# 设置C++标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 添加库目标
add_library(VmpPass SHARED
    XXXTypeGen.cpp
    Pass.cpp
    # generate.cpp
    # register.cpp
    # VmpContext.cpp
    Linker.cpp
    XXXVMPCTX.cpp
    
    
    
)

# 设置目标属性
set_target_properties(VmpPass PROPERTIES
    COMPILE_FLAGS "-frtti"
    PREFIX ""
)

# 链接LLVM库
target_link_libraries(VmpPass
    PRIVATE
    LLVM
)
