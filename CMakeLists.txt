cmake_minimum_required(VERSION 3.16)
project(ConsoleCommandManager VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(filemanager example.cpp)

# 设置包含目录
target_include_directories(filemanager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 编译选项
if(MSVC)
    target_compile_options(filemanager PRIVATE /W4)
else()
    target_compile_options(filemanager PRIVATE -Wall -Wextra -Wpedantic)
endif()
