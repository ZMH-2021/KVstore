cmake_minimum_required(VERSION 3.10)
project(KVStore C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-relax")

# 使用现代 CMake 风格
find_library(DL_LIB dl)

# 可执行文件1：主程序
add_executable(kvstore
    src/main.c
    src/kvstore.c
    src/network/reactor/reactor.c 
    src/network/reactor/multi_reactor.c
    src/store/array.c
    src/store/rbtree.c
    src/store/hash.c
)

target_include_directories(kvstore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network/reactor
    ${CMAKE_CURRENT_SOURCE_DIR}/src/store
)

target_link_libraries(kvstore PRIVATE ${DL_LIB})

# 可执行文件2：测试程序
add_executable(testcase src/testcase.c)
target_include_directories(testcase PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(testcase PRIVATE ${DL_LIB})

# 可执行文件3：测试程序
add_executable(testcase2 src/testcase2.c)
target_include_directories(testcase2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(testcase2 PRIVATE ${DL_LIB})