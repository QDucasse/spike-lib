#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mmu.h"
#include "sim.h"
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

#define RED   "\033[31m"
#define GREEN "\033[32m"  
#define RESET "\033[0m"
#define ASSERT_EQUALS(actual, expected) assert(actual == expected, __func__, __FILE__, __LINE__)
#define ASSERT_EQUALS_BYTE_ARRAY(actual, expected, size) assert(memcmp(actual, expected, size) == 0, __func__, __FILE__, __LINE__)
#define ASSERT_EQUALS_REGISTER(sim, register_id, expected) { \
    read_register(sim, register_id, &test_register); \
    ASSERT_EQUALS(test_register, expected); }

void assert(bool result, const char* function_name, const char* file_name, int line_nb) {
    if (!result) {
        printf(RED "FAILED: %s:%d in %s\n" RESET, file_name, line_nb, function_name);
    } else {
        printf(GREEN "PASSED: %s:%d in %s\n" RESET, file_name, line_nb, function_name);
    }
}

void* setup_simulation() {
    void* content = calloc(1, 4096);
    memory_region region[] = { {.base = 0x1000, .size = 4096, .content = content} };
    return initialize_sim(region, 1);
}

void* setup_simulation_with_isa(char* isa) {
    void* content = calloc(1, 4096);
    memory_region region[] = { {.base = 0x1000, .size = 4096, .content = content} };
    return initialize_sim_with_isa(region, 1, isa);
}


// =====================================
//           MEMORY WRITING
// =====================================

void test_mem_write_1_byte() {
    void* sim = setup_simulation();
    uint8_t mem_write_buffer = 0x11;
    uint8_t mem_load_buffer  = 0x00;
    // Write to memory
    write_memory(sim, 0x1000, 1, (void*) &mem_write_buffer);
    // Read and compare
    sim_t* real_sim = (sim_t*) sim;
    mem_load_buffer = real_sim->get_core(0)->get_mmu()->load_uint8(0x1000);
    ASSERT_EQUALS(mem_load_buffer, mem_write_buffer);
    // Teardown
    release_sim(sim);
} 

void test_mem_write_2_bytes() {
    void* sim = setup_simulation();
    uint16_t mem_write_buffer = 0x1111;
    uint16_t mem_load_buffer  = 0x0000;
    // Write to memory
    write_memory(sim, 0x1000, 2, (void*) &mem_write_buffer);
    // Read and compare
    sim_t* real_sim = (sim_t*) sim;
    mem_load_buffer = real_sim->get_core(0)->get_mmu()->load_uint16(0x1000);
    ASSERT_EQUALS(mem_load_buffer, mem_write_buffer);
    // Teardown
    release_sim(sim);
} 

void test_mem_write_4_bytes() {
    void* sim = setup_simulation();
    uint32_t mem_write_buffer = 0x11111111;
    uint32_t mem_load_buffer  = 0x00000000;
    // Write to memory
    write_memory(sim, 0x1000, 4, (void*) &mem_write_buffer);
    // Read and compare
    sim_t* real_sim = (sim_t*) sim;
    mem_load_buffer = real_sim->get_core(0)->get_mmu()->load_uint32(0x1000);
    ASSERT_EQUALS(mem_load_buffer, mem_write_buffer);
    // Teardown
    release_sim(sim);
} 

void test_mem_write_8_bytes() {
    void* sim = setup_simulation();
    uint64_t mem_write_buffer = 0x1111111111111111;
    uint64_t mem_load_buffer  = 0x0000000000000000;
    // Write to memory
    write_memory(sim, 0x1000, 8, (void*) &mem_write_buffer);
    // Read and compare
    sim_t* real_sim = (sim_t*) sim;
    mem_load_buffer = real_sim->get_core(0)->get_mmu()->load_uint64(0x1000);
    ASSERT_EQUALS(mem_load_buffer, mem_write_buffer);
    // Teardown
    release_sim(sim);
} 

void test_mem_write_10_bytes() {
    void* sim = setup_simulation();
    uint8_t mem_write_buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t mem_load_buffer[10]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // Write to memory
    write_memory(sim, 0x1000, 10, (void*) &mem_write_buffer);
    // Read and compare
    sim_t* real_sim = (sim_t*) sim;
    for (int i = 0; i < 10; i++) {
        ((uint8_t*) mem_load_buffer)[i] = real_sim->get_core(0)->get_mmu()->load_uint8(0x1000 + i);
    }
    ASSERT_EQUALS_BYTE_ARRAY(mem_load_buffer, mem_write_buffer, 10);
    // Teardown
    release_sim(sim);
} 

void test_mem_write_misaligned() {
    void* sim = setup_simulation();
    uint8_t mem_write_buffer = 0x11;
    uint8_t mem_load_buffer  = 0x00;
    // Try to write to memory
    int res = write_memory(sim, 0x1001, 1, (void*) &mem_write_buffer);
    ASSERT_EQUALS(res, SP_ERR_WRITE_MISALIGNED);
    // Teardown
    release_sim(sim);
}


// =====================================
//          MEMORY READING
// =====================================

void test_mem_read_1_byte() {
    void* sim = setup_simulation();
    uint8_t mem_write_buffer = 0x11;
    uint8_t mem_load_buffer  = 0x00;
    // Write to memory
    sim_t* real_sim = (sim_t*) sim;
    real_sim->get_core(0)->get_mmu()->store_uint8(0x1000, mem_write_buffer);
    // Read and compare
    read_memory(sim, 0x1000, 1, (void*) &mem_load_buffer);
    ASSERT_EQUALS(mem_load_buffer, mem_write_buffer);
    // Teardown
    release_sim(sim);
} 

void test_mem_read_2_bytes() {
    void* sim = setup_simulation();
    uint16_t mem_write_buffer = 0x1111;
    uint16_t mem_load_buffer  = 0x0000;
    // Write to memory
    sim_t* real_sim = (sim_t*) sim;
    real_sim->get_core(0)->get_mmu()->store_uint16(0x1000, mem_write_buffer);
    // Read and compare
    read_memory(sim, 0x1000, 2, (void*) &mem_load_buffer);
    ASSERT_EQUALS(mem_load_buffer, mem_write_buffer);
    // Teardown
    release_sim(sim);
} 

void test_mem_read_4_bytes() {
    void* sim = setup_simulation();
    uint32_t mem_write_buffer = 0x11111111;
    uint32_t mem_load_buffer  = 0x00000000;
    // Write to memory
    sim_t* real_sim = (sim_t*) sim;
    real_sim->get_core(0)->get_mmu()->store_uint32(0x1000, mem_write_buffer);
    // Read and compare
    read_memory(sim, 0x1000, 4, (void*) &mem_load_buffer);
    ASSERT_EQUALS(mem_load_buffer, mem_write_buffer);
    // Teardown
    release_sim(sim);
} 

void test_mem_read_8_bytes() {
    void* sim = setup_simulation();
    uint64_t mem_write_buffer = 0x1111111111111111;
    uint64_t mem_load_buffer  = 0x0000000000000000;
    // Write to memory
    sim_t* real_sim = (sim_t*) sim;
    real_sim->get_core(0)->get_mmu()->store_uint64(0x1000, mem_write_buffer);
    // Read and compare
    read_memory(sim, 0x1000, 8, (void*) &mem_load_buffer);
    ASSERT_EQUALS(mem_load_buffer, mem_write_buffer);
    // Teardown
    release_sim(sim);
} 

void test_mem_read_10_bytes() {
    // Byte Array with 10 bytes
    void* sim = setup_simulation();    
    uint8_t mem_write_buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint8_t mem_load_buffer[10]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // Write to memory
    sim_t* real_sim = (sim_t*) sim;
    for (int i = 0; i < 10; i++){
         real_sim->get_core(0)->get_mmu()->store_uint8(0x1000 + i, ((uint8_t*) mem_write_buffer)[i]);
    }
    // Read and compare
    read_memory(sim, 0x1000, 10, (void*) &mem_load_buffer);
    ASSERT_EQUALS_BYTE_ARRAY(mem_load_buffer, mem_write_buffer, 10);
    // Teardown
    release_sim(sim);
}

void test_mem_read_misaligned() {
    void* sim = setup_simulation();
    uint8_t mem_write_buffer = 0x11;
    uint8_t mem_load_buffer  = 0x00;
    // Write to memory
    sim_t* real_sim = (sim_t*) sim;
    real_sim->get_core(0)->get_mmu()->store_uint8(0x1000, mem_write_buffer);
    // Read and compare
    int res = read_memory(sim, 0x1001, 1, (void*) &mem_load_buffer);
    ASSERT_EQUALS(res, SP_ERR_READ_MISALIGNED);
    // Teardown
    release_sim(sim);
} 

void test_mem_read_write_18_bytes() {
    // Byte Array with 10 bytes
    void* sim = setup_simulation();    
    uint8_t mem_write_buffer[18] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    uint8_t mem_load_buffer[18]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    write_memory(sim, 0x1000, 10, (void*) mem_write_buffer);
    read_memory(sim, 0x1000, 10, (void*) mem_load_buffer);
    ASSERT_EQUALS_BYTE_ARRAY(mem_load_buffer, mem_write_buffer, 10);
    // Teardown
    release_sim(sim);
}

// =====================================
//             EXECUTION
// =====================================

/* ADD
====== */

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
    // Execute the instructions
    int res = spike_start(sim, 0x1000, 0x1004, 0, 0);
    // printf("%s\n",sp_strerror(res));
    // Verify return values
    ASSERT_EQUALS_REGISTER(sim, SPIKE_RISCV_REG_X5, 0x11111111);
    ASSERT_EQUALS_REGISTER(sim, SPIKE_RISCV_REG_PC, 0x1004);
    ASSERT_EQUALS(res, SP_ERR_OK);
    // Teardown
    release_sim(sim);
}

void test_exec_until_last_does_not_execute_destination() {
    // COMPLETE add x5, x6, x7
    void* sim = setup_simulation();
    uint32_t instr_add     = 0x007302B3;
    uint32_t instr_garbage = 0x00000293; 
    // Write the two values to add
    uint64_t x5_value = 0x00000000;
    uint64_t x6_value = 0x11110000;
    uint64_t x7_value = 0x00001111;
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    write_register(sim, SPIKE_RISCV_REG_X6, &x6_value);
    write_register(sim, SPIKE_RISCV_REG_X7, &x7_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, 4, &instr_add);
    write_memory(sim, 0x1004, 4, &instr_garbage);
    // Execute the instructions
    int res = spike_start(sim, 0x1000, 0x1004, 0, 0);
    // printf("%s\n",sp_strerror(res));
    // Verify return values
    ASSERT_EQUALS_REGISTER(sim, SPIKE_RISCV_REG_X5, 0x11111111);
    ASSERT_EQUALS_REGISTER(sim, SPIKE_RISCV_REG_PC, 0x1004);
    ASSERT_EQUALS(res, SP_ERR_OK);
    // Teardown
    release_sim(sim);
}


/* TIMEOUT
========== */

void test_exec_jump_instruction_timeout() {
    void* sim = setup_simulation();
    uint8_t instructions[] {
        0xb3, 0x02, 0x73, 0x00, // add x5 x6 x7
        0xb3, 0x02, 0x73, 0x00, // add x5 x6 x7
        // 0xe5, 0xbf              // j 0x1000
        0x6f, 0x00, 0x00, 0x00  // j 0x1000
    };
    uint64_t x5_value = 0x00000000;
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, sizeof(instructions), instructions);
    // Execute the instructions
    int res = spike_start(sim, 0x1000, 0x1020, 1000, 0);
    // Verify return values
    ASSERT_EQUALS(res, SP_ERR_TIMEOUT);
    // Teardown
    release_sim(sim);
}

/* Instruction Number
===================== */

void test_exec_jump_instruction_instruction_number() {
    void* sim = setup_simulation();
    uint8_t instructions[] {
        0xb3, 0x02, 0x73, 0x00, // add  x5 x6 x7
        0x05, 0x03,             // addi x6 x6 1  
        0xed, 0xbf              // j    0x1000
        // 0x13, 0x03, 0x13, 0x03, // addi x6 x6 1  
        // 0x6f, 0x00, 0x00, 0x00  // j    0x1000
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
    int res = spike_start(sim, 0x1000, 0x1200, 0, 6);
    // printf("%s\n",sp_strerror(res));
    // Verify return values
    ASSERT_EQUALS_REGISTER(sim, SPIKE_RISCV_REG_X5, 3);
    ASSERT_EQUALS_REGISTER(sim, SPIKE_RISCV_REG_X6, 3);
    ASSERT_EQUALS_REGISTER(sim, SPIKE_RISCV_REG_X7, 1);
    ASSERT_EQUALS(res, SP_ERR_MAX_COUNT);
    // Teardown
    release_sim(sim);
}

// =====================================
//        INVALID MEMORY ACCESSES
// =====================================

/* MISALIGNED READ
================== */

void test_instruction_misaligned_address_load() { // mcause 4
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
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, sizeof(instructions), instructions);
    // Verify return values
    int res = spike_start(sim, 0x1000, 0x1200, 0, 0);
    // printf("%s\n",sp_strerror(res));
    ASSERT_EQUALS(res, SP_ERR_READ_MISALIGNED);
    // Teardown
    release_sim(sim);
}

/* UNMAPPED READ
================ */

void test_instruction_unmapped_address_load() {   // mcause 5
    void* sim = setup_simulation();
    uint8_t instructions[] {
        0xb7, 0xd3, 0x04, 0x00, // lui   x7  0x4d
        0x9b, 0x83, 0xd3, 0xcc, // addiw x7, x7, -819
        0xb6, 0x03,             // slli  x7, x7, 0xd 
        0x93, 0x83, 0x03, 0x90, // addi  x7, x7, -1792
        0x83, 0xb2, 0x03, 0x00  // ld    x5, 0(x7)
    };
    // Fill registers
    uint64_t x5_value = 0x00000000;
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, sizeof(instructions), instructions);
    // Verify return values
    int res = spike_start(sim, 0x1000, 0x1200, 0, 0);
    // printf("%s\n",sp_strerror(res));
    ASSERT_EQUALS(res, SP_ERR_READ_UNMAPPED);
    // Teardown
    release_sim(sim);
}

/* MISALIGNED WRITE
=================== */

void test_instruction_misaligned_address_store() { // mcause 6
    void* sim = setup_simulation();
    uint8_t instructions[] {
        0xb7, 0xd3, 0x04, 0x00, // lui   x7 0x4d
        0x9b, 0x83, 0xd3, 0xcc, // addiw x7, x7, -819
        0xb6, 0x03,             // slli  x7, x7, 0xd 
        0x93, 0x83, 0x93, 0x99, // addi  x7, x7, -1639
        0x23, 0xb0, 0x53, 0x00  // sd    x5, 0(x7)
    };
    // Fill registers
    uint64_t x5_value = 0x00000000;
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, sizeof(instructions), instructions);
    // Verify return values
    int res = spike_start(sim, 0x1000, 0x1200, 0, 0);
    // printf("%s\n",sp_strerror(res));
    ASSERT_EQUALS(res, SP_ERR_WRITE_MISALIGNED);
    // Teardown
    release_sim(sim);
}

/* UNMAPPED WRITE
================= */

void test_instruction_unmapped_address_store() {   // mcause 7
    void* sim = setup_simulation();
    uint8_t instructions[] {
        0xb7, 0xd3, 0x04, 0x00, // lui   x7  0x4d
        0x9b, 0x83, 0xd3, 0xcc, // addiw x7, x7, -819
        0xb6, 0x03,             // slli  x7, x7, 0xd 
        0x93, 0x83, 0x03, 0x90, // addi  x7, x7, -1792
        0x23, 0xb0, 0x53, 0x00  // sd    x5, 0(x7)
    };
    // Fill registers
    uint64_t x5_value = 0x00000000;
    uint64_t x6_value = 0x00000001;
    write_register(sim, SPIKE_RISCV_REG_X5, &x5_value);
    // Write instructions to memory
    write_memory(sim, 0x1000, sizeof(instructions), instructions);
    // Verify return values
    int res = spike_start(sim, 0x1000, 0x1200, 0, 0);
    // printf("%s\n",sp_strerror(res));
    ASSERT_EQUALS(res, SP_ERR_WRITE_UNMAPPED);
    // Teardown
    release_sim(sim);
}

/* MISALIGNED INSTRUCTION
========================= */

void test_misaligned_instruction_address() {
    void* sim = setup_simulation_with_isa("IMAFD");
    uint8_t instructions[] {
        0x93, 0x03, 0x70, 0x09, // addi	x7, x0, 0x97
        0x67, 0x80, 0x03, 0x00  // jr   x7
    };
    // Write instructions to memory
    write_memory(sim, 0x1000, sizeof(instructions), instructions);
    // Verify return values
    int res = spike_start(sim, 0x1000, 0x1200, 0, 0);
    // printf("%s\n",sp_strerror(res));
    ASSERT_EQUALS(res, SP_ERR_FETCH_MISALIGNED);
    // Teardown
    release_sim(sim);
}


/* INVALID INSTRUCTION
====================== */

void test_invalid_instruction_fetch() {
    void* sim = setup_simulation();
    uint8_t instructions[] {
        0x99, 0x99, 0x99, 0x99 // Wrong instruction
    };
    // Write instructions to memory
    write_memory(sim, 0x1000, sizeof(instructions), instructions);
    // Verify return values
    int res = spike_start(sim, 0x1000, 0x1200, 0, 0);
    // printf("%s\n",sp_strerror(res));
    ASSERT_EQUALS(res, SP_ERR_INSN_INVALID);
    // Teardown
    release_sim(sim);
}



// =====================================
//        INVALID MEMORY ACCESSES
// =====================================

int main() {
    // Memory write tests
    test_mem_write_1_byte();
    test_mem_write_2_bytes();
    test_mem_write_4_bytes();
    test_mem_write_8_bytes();
    test_mem_write_10_bytes();
    test_mem_write_misaligned();

    // Memory read tests
    test_mem_read_1_byte();
    test_mem_read_2_bytes();
    test_mem_read_4_bytes();
    test_mem_read_8_bytes();
    test_mem_read_10_bytes();
    test_mem_read_misaligned();

    // Execution tests
    test_exec_add_instruction();
    test_exec_jump_instruction_timeout();
    test_exec_jump_instruction_instruction_number();
    test_exec_until_last_does_not_execute_destination();
    
    // Memory errors tests
    test_instruction_misaligned_address_load();
    test_instruction_unmapped_address_load();
    test_instruction_misaligned_address_store();
    test_instruction_unmapped_address_store();

    // Instruction tests
    test_invalid_instruction_fetch();
    test_misaligned_instruction_address();
}