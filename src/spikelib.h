#pragma once

#include <stdint.h>

#define EXPORT  __attribute__ ((visibility ("default")))

// =====================================
//          MEMORY LAYOUT
// =====================================

typedef struct {
    uint64_t base;
    uint64_t size;
    void* content;
} memory_region;

extern "C" {
    // void* doInitialize_sim(); // when using C wrapper
    EXPORT void* initialize_sim(memory_region* memories, int regions_number);
    EXPORT int read_register(void* sim, int regid, void* value);
    EXPORT int write_register(void* sim, int regid, void* value);
    EXPORT const char* sp_strerror(int code);
    EXPORT int write_memory(void* sim, uint64_t address, uint64_t size, void* value);
    EXPORT int read_memory(void* sim, uint64_t address, uint64_t size, void* value);
}

// =====================================
//      RISCV REGISTERS ENUMERATION
// =====================================

typedef enum {
    SPIKE_RISCV_REG_X0,
    SPIKE_RISCV_REG_X1,
    SPIKE_RISCV_REG_X2,
    SPIKE_RISCV_REG_X3, 
    SPIKE_RISCV_REG_X4, 
    SPIKE_RISCV_REG_X5,
    SPIKE_RISCV_REG_X6, 
    SPIKE_RISCV_REG_X7, 
    SPIKE_RISCV_REG_X8, 
    SPIKE_RISCV_REG_X9,
    SPIKE_RISCV_REG_X10, 
    SPIKE_RISCV_REG_X11, 
    SPIKE_RISCV_REG_X12, 
    SPIKE_RISCV_REG_X13, 
    SPIKE_RISCV_REG_X14, 
    SPIKE_RISCV_REG_X15, 
    SPIKE_RISCV_REG_X16, 
    SPIKE_RISCV_REG_X17, 
    SPIKE_RISCV_REG_X18, 
    SPIKE_RISCV_REG_X19,
    SPIKE_RISCV_REG_X20,
    SPIKE_RISCV_REG_X21, 
    SPIKE_RISCV_REG_X22,
    SPIKE_RISCV_REG_X23, 
    SPIKE_RISCV_REG_X24, 
    SPIKE_RISCV_REG_X25, 
    SPIKE_RISCV_REG_X26, 
    SPIKE_RISCV_REG_X27, 
    SPIKE_RISCV_REG_X28, 
    SPIKE_RISCV_REG_X29, 
    SPIKE_RISCV_REG_X30,
    SPIKE_RISCV_REG_X31,
    SPIKE_RISCV_REG_PC
} spike_riscv_reg;

// =====================================
//      SPIKE POSSIBLE ERROR CODES
// =====================================

typedef enum {
    SP_ERR_OK = 0,          // No error: everything was fine
    SP_ERR_NO_MEM,          // Out-Of-Memory error: uc_open(), uc_emulate()
    SP_ERR_READ_UNMAPPED,   // READ  on unmapped memory
    SP_ERR_WRITE_UNMAPPED,  // WRITE on unmapped memory
    SP_ERR_FETCH_UNMAPPED,  // FETCH on unmapped memory
    SP_ERR_INSN_INVALID,    // Invalid Instruction
    SP_ERR_MAP,             // Invalid memory mapping
    SP_ERR_READ_PROT,       // READ  PROTECTION violation
    SP_ERR_WRITE_PROT,      // WRITE PROTECTION violation
    SP_ERR_FETCH_PROT,      // FETCH PROTECTION violation
    SP_ERR_READ_UNALIGNED,  // Unaligned READ
    SP_ERR_WRITE_UNALIGNED, // Unaligned WRITE
    SP_ERR_FETCH_UNALIGNED, // Unaligned FETCH
} sp_err;