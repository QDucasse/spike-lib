cmake_minimum_required(VERSION 3.1)

project(SpikeLib VERSION 1.0
                 LANGUAGES C CXX)

# add_executable(spikelib spikelib.cpp)
add_library(spikelib SHARED spikelib.cpp)

set(default_build_type "Debug")
add_compile_options(-fPIC -fvisibility=hidden)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")




include(ExternalProject)

ExternalProject_Add(spike
   SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim
   CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/configure CPPFLAGS=-fPIC
   BUILD_COMMAND     make
   INSTALL_COMMAND   sh -c "ar ru spikelib.a ${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/*.o")

target_include_directories(spikelib PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build)
target_include_directories(spikelib
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/riscv
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/softfloat
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv-tools/riscv-isa-sim/fesvr
)

# file(GLOB SPIKELIBS ${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/*.o)
# list(REMOVE_ITEM SPIKELIBS  
#     "${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/spike-dasm.o" 
#     "${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/spike_dasm_option_parser.o" 

#     "${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/elf2hex.o" 
#     "${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/spike.o" 
#     "${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/spike-log-parser.o" 

#     "${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/termios-xspike.o" 
#     "${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/xspike.o" 
#     )

target_link_libraries(spikelib PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/spike-prefix/src/spike-build/spikelib.a)
target_link_libraries(spikelib PRIVATE dl pthread)
