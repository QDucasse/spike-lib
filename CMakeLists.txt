cmake_minimum_required(VERSION 3.1)

project(SpikeLib VERSION 1.0
                 LANGUAGES C CXX)

add_executable(spikelib spikelib.cpp)

include(ExternalProject)
ExternalProject_Add(spike
   SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riscv-isa-sim
   CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/riscv-isa-sim/configure
   BUILD_COMMAND make
   INSTALL_COMMAND echo)

target_include_directories(spikelib PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/build)
target_include_directories(spikelib 
PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-isa-sim/riscv
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-isa-sim/softfloat
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-isa-sim/fesvr
)


   # add_library()
target_link_libraries(spikelib PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/device.o)