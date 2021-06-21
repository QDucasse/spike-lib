#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "spikelib.h"

// =====================================
//                TESTS
// =====================================

// Some instructions 
// 0xb3, 0x02, 0x73, 0x00, // add  x5 x6  x7
// 0x33, 0x03, 0x65, 0x00, // add  x6 x10 x6
// 0x05, 0x03,             // addi x6 x6 1 
// 0xe5, 0xbf              // j 0x1000

uint64_t test_register;
#define ASSERT_EQUALS(actual, expected) assert(actual == expected, __func__, __FILE__, __LINE__)
#define ASSERT_EQUALS_BYTE_ARRAY(actual, expected, size) assert(memcmp(actual, expected, size) == 0, __func__, __FILE__, __LINE__)
#define ASSERT_EQUALS_REGISTER(sim, register_id, expected) { \
    read_register(sim, register_id, &test_register); \
    ASSERT_EQUALS(test_register, expected); }

void assert(bool result, const char* function_name, const char* file_name, int line_nb) {
    if (!result) {
        printf("FAILED: %s:%d in %s\n", file_name, line_nb, function_name);
    } else {
        printf("PASSED: %s:%d in %s\n", file_name, line_nb, function_name);
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
    // Teardown
    release_sim(sim);
} 

void test_mem_read_write_2_bytes() {
    // uint16_t
    void* sim = setup_simulation();
    uint16_t mem_write_byte_16 = 0x1111;
    uint16_t mem_load_byte_16  = 0x0000;
    write_memory(sim, 0x1030, 2, (void*) &mem_write_byte_16);
    read_memory(sim, 0x1030, 2, (void*) &mem_load_byte_16);
    ASSERT_EQUALS(mem_load_byte_16, mem_write_byte_16);
    // Teardown
    release_sim(sim);
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
    // Teardown
    release_sim(sim);
}

void test_mem_read_write_10_bytes() {
    // Byte Array with 10 bytes
    void* sim = setup_simulation();    
    uint8_t mem_write_10_bytes[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t mem_load_10_bytes[10]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    write_memory(sim, 0x1200, 10, (void*) mem_write_10_bytes);
    read_memory(sim, 0x1200, 10, (void*) mem_load_10_bytes);
    // print_byte_array(mem_write_10_bytes, 10);
    ASSERT_EQUALS_BYTE_ARRAY(mem_load_10_bytes, mem_write_10_bytes, 10);
    // Teardown
    release_sim(sim);
}

void test_mem_read_write_18_bytes() {
    // Byte Array with 10 bytes
    void* sim = setup_simulation();    
    uint8_t mem_write_10_bytes[18] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    uint8_t mem_load_10_bytes[18]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    write_memory(sim, 0x1200, 10, (void*) mem_write_10_bytes);
    read_memory(sim, 0x1200, 10, (void*) mem_load_10_bytes);
    // print_byte_array(mem_write_10_bytes, 10);
    ASSERT_EQUALS_BYTE_ARRAY(mem_load_10_bytes, mem_write_10_bytes, 10);
    // Teardown
    release_sim(sim);
}

void test_exec_add_instruction() {
    // COMPLETE add x5, x6, x7
    void* sim = setup_simulation();
    uint32_t instr_add = 0x007302B3;
    // Write the two values to add
    uint64_t x5_value = 0x00000000;
    uint64_t x6_value = 0x11110000;
    uint64_t x7_value = 0x00001111;
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    write_register(sim, SPIKE_RISCV_REG_X6, &x6_value);
    write_register(sim, SPIKE_RISCV_REG_X7, &x7_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, 4, &instr_add);
    write_memory(sim, 0x1004, 4, &instr_add);
    // Execute the instructions
    int res = spike_start(sim, 0x1000, 0x1004, 0, 0);
    // Verify return values
    ASSERT_EQUALS_REGISTER(sim, SPIKE_RISCV_REG_X5, 0x11111111);
    ASSERT_EQUALS_REGISTER(sim, SPIKE_RISCV_REG_PC, 0x1004);
    ASSERT_EQUALS(res, SP_ERR_OK);
    // Teardown
    release_sim(sim);
}

void test_exec_jump_instruction_timeout() {
    void* sim = setup_simulation();
    uint8_t instructions[] {
        0xb3, 0x02, 0x73, 0x00, // add x5 x6 x7
        0xb3, 0x02, 0x73, 0x00, // add x5 x6 x7
        0xe5, 0xbf              // j 0x1000
    };
    uint64_t x5_value = 0x00000000;
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, 10, instructions);
    // Execute the instructions
    int res = spike_start(sim, 0x1000, 0x1020, 1000, 0);
    // Verify return values
    ASSERT_EQUALS(res, SP_ERR_TIMEOUT);
    // Teardown
    release_sim(sim);
}

void test_exec_jump_instruction_instruction_number() {
    void* sim = setup_simulation();
    uint8_t instructions[] {
        0xb3, 0x02, 0x73, 0x00, // add  x5 x6 x7
        0x05, 0x03,             // addi x6 x6 1  
        0xed, 0xbf              // j    0x1000
    };
    uint64_t x5_value = 0x00000000;
    uint64_t x6_value = 0x00000001;
    uint64_t x7_value = 0x00000001;
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    write_register(sim, SPIKE_RISCV_REG_X6, &x6_value);
    write_register(sim, SPIKE_RISCV_REG_X7, &x7_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, sizeof(instructions), instructions);
    // Execute the instructions
    int res = spike_start(sim, 0x1000, 0x1020, 0, 6);
    // Verify return values
    ASSERT_EQUALS_REGISTER(sim, SPIKE_RISCV_REG_X5, 3);
    ASSERT_EQUALS(res, SP_ERR_OK);
    // Teardown
    release_sim(sim);
}

void test_stop_reading_misalligned_address() { // mcause 4
    void* sim = setup_simulation();
    uint8_t instructions[] {
        0xb7, 0xd3, 0x04, 0x00, // lui   x7 0x4d
        0x9b, 0x83, 0xd3, 0xcc, // addiw x7, x7, -819
        0xb6, 0x03,             // slli  x7, x7, 0xd 
        0x93, 0x83, 0x93, 0x99, // addi  x7, x7, -1639
        0x83, 0xb2, 0x03, 0x00  // ld    x5, 0(x7)
    };
    // Fill registers
    uint64_t x5_value = 0x00000000;
    uint64_t x6_value = 0x00000001;
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, 18, instructions);
    // Verify return values
    int res = spike_start(sim, 0x1000, 0x1200, 0, 0);
    ASSERT_EQUALS(res, SP_ERR_READ_UNMAPPED);
    // Teardown
    release_sim(sim);
}

void test_stop_reading_invalid_address() {   // mcause 5
    void* sim = setup_simulation();
    uint8_t instructions[] {
        0xb7, 0xd3, 0x04, 0x00, // lui   x7 0x4d
        0x9b, 0x83, 0xd3, 0xcc, // addiw x7, x7, -819
        0xb6, 0x03,             // slli  x7, x7, 0xd 
        0x93, 0x83, 0x03, 0x90, // addi  x7, x7, -1792
        0x83, 0xb2, 0x03, 0x00  // ld    x5, 0(x7)
    };
    // Fill registers
    uint64_t x5_value = 0x00000000;
    uint64_t x6_value = 0x00000001;
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, 18, instructions);
    // Verify return values
    int res = spike_start(sim, 0x1000, 0x1200, 0, 0);
    ASSERT_EQUALS(res, SP_ERR_READ_UNMAPPED);
    // Teardown
    release_sim(sim);
}

int main() {
    // Memory writing tests
    test_mem_read_write_1_byte();
    test_mem_read_write_2_bytes();
    test_mem_read_write_4_bytes();
    test_mem_read_write_8_bytes();
    test_mem_read_write_10_bytes();
    // Execution tests
    test_exec_add_instruction();
    test_exec_jump_instruction_timeout();
    test_exec_jump_instruction_instruction_number();
    // Memory errors tests
    test_stop_reading_invalid_address();
}