cmake_minimum_required (VERSION 2.6)

project (ctrlSlave)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

set (ctrlSlave_src ctrlSlave.c)

add_executable(ctrlSlave ${ctrlSlave_src})
