cmake_minimum_required(VERSION 3.13.4)
project(llvm_example_kale)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_COMPILER clang++)
set(LLVM_LINK_COMPONENTS support)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3 -fno-limit-debug-info")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-3 ")

find_package(LLVM REQUIRED CONFIG)

set(LLVM_LINK_COMPONENTS support)
find_package(Clang REQUIRED)

execute_process(COMMAND llvm-config --libs OUTPUT_VARIABLE LIBS)
execute_process(COMMAND llvm-config --system-libs OUTPUT_VARIABLE SYS_LIBS)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LDF)
message(STATUS "Found LLVM" ${LIBS})

string(STRIP ${LIBS} LIBS)
string(STRIP ${SYS_LIBS} SYS_LIBS)
string(STRIP ${LDF} LDF)

link_libraries(${LIBS} ${SYS_LIBS} ${LDF})

execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE CMAKE_CXX_FLAGS)
string(STRIP ${CMAKE_CXX_FLAGS} CMAKE_CXX_FLAGS)
add_executable(llvm_example_kale main.cpp)

target_link_libraries(llvm_example_kale
        PRIVATE
        clangFrontend
        clangTooling
        clangBasic
        clangASTMatchers
        clangAST
        clangBasic
        clangCodeGen
        clangLex
        clangParse
        clangSerialization)