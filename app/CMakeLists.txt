cmake_minimum_required(VERSION 3.4.1)

##官方标准配置
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions -Wall")

add_library(native-write
        SHARED
        src/main/cpp/native_write.c)

add_library(native-hook
        SHARED
        src/main/cpp/native_hook.c)

add_library(hook-simple
        SHARED
        src/main/cpp/hook_simple.c)

target_link_libraries(native-write android log)

target_link_libraries(native-hook android log)

target_link_libraries(hook-simple android log)