cmake_minimum_required(VERSION 3.1)

project(SpikeLib VERSION 1.0
                 LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE "Debug")
add_compile_options(-fPIC -fvisibility=hidden)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Library 


add_library(spikelib SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/spikelib.cpp)
include(ExternalProject)
ExternalProject_Add(spike
   SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim
   CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/configure CPPFLAGS=-fPIC
   BUILD_COMMAND     make
   INSTALL_COMMAND   sh -c "ar ru spikelib.a ${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/*.o")

add_dependencies(spikelib spike)

target_include_directories(spikelib
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/riscv
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/softfloat
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/fesvr
    ${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build
)

target_link_libraries(spikelib PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/spikelib.a)
target_link_libraries(spikelib PRIVATE dl pthread)
target_include_directories(spikelib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Executable

add_executable(spikelib-ex ${CMAKE_CURRENT_SOURCE_DIR}/src/spikelib.cpp)
target_include_directories(spikelib-ex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(spikelib-ex
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/riscv
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/softfloat
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/fesvr
    ${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build
)
target_link_libraries(spikelib-ex PRIVATE spikelib)

# Executable for tests

add_executable(spikelib-tests ${CMAKE_CURRENT_SOURCE_DIR}/src/tests.cpp)
target_include_directories(spikelib-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(spikelib-tests PRIVATE spikelib)

