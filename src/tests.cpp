#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "spikelib.h"

// =====================================
//                TESTS
// =====================================

#define ASSERT_EQUALS(actual, expected) assert(actual == expected, __func__, __FILE__, __LINE__)
#define ASSERT_EQUALS_BYTE_ARRAY(actual, expected, size) assert(memcmp(actual, expected, size) == 0, __func__, __FILE__, __LINE__)

void assert(bool result, const char* function_name, const char* file_name, int line_nb) {
    if (!result) {
        printf("FAILED: %s:%d in %s\n", file_name, line_nb, function_name);
    }
}

void* setup_simulation() {
    void* content = calloc(1, 4096);
    memory_region region[] = { {.base = 0x1000, .size = 4096, .content = content} };
    return initialize_sim(region, 1);
}

void test_mem_read_write_1_byte() {
    // Testing the memory read/write functions
     // uint8_t
    void* sim = setup_simulation();
    uint8_t mem_write_byte_8 = 0x11;
    uint8_t mem_load_byte_8  = 0x00;
    write_memory(sim, 0x1000, 1, (void*) &mem_write_byte_8);
    read_memory(sim, 0x1000, 1, (void*) &mem_load_byte_8);
    ASSERT_EQUALS(mem_load_byte_8, mem_write_byte_8);
} 

void test_mem_read_write_2_bytes() {
    // uint16_t
    void* sim = setup_simulation();
    uint16_t mem_write_byte_16 = 0x1111;
    uint16_t mem_load_byte_16  = 0x0000;
    write_memory(sim, 0x1030, 2, (void*) &mem_write_byte_16);
    read_memory(sim, 0x1030, 2, (void*) &mem_load_byte_16);
    ASSERT_EQUALS(mem_load_byte_16, mem_write_byte_16);
}

void test_mem_read_write_4_bytes() {
    // uint32_t
    void* sim = setup_simulation();
    uint32_t mem_write_byte_32 = 0x11111111;
    uint32_t mem_load_byte_32  = 0x00000000;
    write_memory(sim, 0x1060, 4, (void*) &mem_write_byte_32);
    read_memory(sim, 0x1060, 4, (void*) &mem_load_byte_32);
    ASSERT_EQUALS(mem_load_byte_32, mem_write_byte_32);
}

void test_mem_read_write_8_bytes() {
    // uint64_t
    void* sim = setup_simulation();
    uint64_t mem_write_byte_64 = 0x1111111111111111;
    uint64_t mem_load_byte_64  = 0x0000000000000000;
    write_memory(sim, 0x1120, 8, (void*) &mem_write_byte_64);
    read_memory(sim, 0x1120, 8, (void*) &mem_load_byte_64);
    ASSERT_EQUALS(mem_load_byte_64, mem_write_byte_64);
}

void test_mem_read_write_10_bytes() {

    // Byte Array with 10 bytes
    void* sim = setup_simulation();    
    uint8_t mem_write_10_bytes[10] = {17, 17, 17, 17, 17, 17, 17, 17, 17, 17};
    uint8_t mem_load_10_bytes[10]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    write_memory(sim, 0x1200, 10, (void*) mem_write_10_bytes);
    read_memory(sim, 0x1200, 10, (void*) mem_load_10_bytes);
    // print_byte_array(mem_write_10_bytes, 10);
    ASSERT_EQUALS_BYTE_ARRAY(mem_load_10_bytes, mem_write_10_bytes, 10);
}

int main() {
    test_mem_read_write_1_byte();
    test_mem_read_write_2_bytes();
    test_mem_read_write_4_bytes();
    test_mem_read_write_8_bytes();
    test_mem_read_write_10_bytes();

}