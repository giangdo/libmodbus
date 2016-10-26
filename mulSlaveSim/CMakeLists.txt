cmake_minimum_required (VERSION 2.6)

project (mulSlaveSim)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (mulSlaveSim_src
							modbus.c
							modbus.h
							modbus-data.c
							modbus-rtu.c
							modbus-tcp.c

							mulSlaveSim.c)

set (mulSlaveSim_lib pthread)

add_executable(mulSlaveSim ${mulSlaveSim_src})
target_link_libraries(mulSlaveSim ${mulSlaveSim_lib})
add_dependencies(mulSlaveSim force_version_update)
