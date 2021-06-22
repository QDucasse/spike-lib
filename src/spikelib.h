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
    EXPORT int spike_start(void* sim, uint64_t begin_address, uint64_t end_address, uint64_t timeout, size_t max_instruction_number);
    EXPORT void release_sim(void* sim);
}

// =====================================
//      RISCV REGISTERS ENUMERATION
// =====================================

typedef enum {
    // ___ GENERAL REGISTERS ____

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

    // _____________ PC ________________
    
    SPIKE_RISCV_REG_PC,

    // ___ FLOATING POINT REGISTERS ____
    
    SPIKE_RISCV_REG_F0,
    SPIKE_RISCV_REG_F1,
    SPIKE_RISCV_REG_F2,
    SPIKE_RISCV_REG_F3, 
    SPIKE_RISCV_REG_F4, 
    SPIKE_RISCV_REG_F5,
    SPIKE_RISCV_REG_F6, 
    SPIKE_RISCV_REG_F7, 
    SPIKE_RISCV_REG_F8, 
    SPIKE_RISCV_REG_F9,
    SPIKE_RISCV_REG_F10, 
    SPIKE_RISCV_REG_F11, 
    SPIKE_RISCV_REG_F12, 
    SPIKE_RISCV_REG_F13, 
    SPIKE_RISCV_REG_F14, 
    SPIKE_RISCV_REG_F15, 
    SPIKE_RISCV_REG_F16, 
    SPIKE_RISCV_REG_F17, 
    SPIKE_RISCV_REG_F18, 
    SPIKE_RISCV_REG_F19,
    SPIKE_RISCV_REG_F20,
    SPIKE_RISCV_REG_F21, 
    SPIKE_RISCV_REG_F22,
    SPIKE_RISCV_REG_F23, 
    SPIKE_RISCV_REG_F24, 
    SPIKE_RISCV_REG_F25, 
    SPIKE_RISCV_REG_F26, 
    SPIKE_RISCV_REG_F27, 
    SPIKE_RISCV_REG_F28, 
    SPIKE_RISCV_REG_F29, 
    SPIKE_RISCV_REG_F30,
    SPIKE_RISCV_REG_F31

} spike_riscv_reg;

// =====================================
//      SPIKE POSSIBLE ERROR CODES
// =====================================

typedef enum {
    SP_ERR_OK = 0,           // No error: everything was fine
    SP_ERR_TIMEOUT,          // Execution timeout
    SP_ERR_NO_MEM,           // Out-Of-Memory error
    SP_ERR_READ_UNMAPPED,    // READ  on unmapped memory
    SP_ERR_WRITE_UNMAPPED,   // WRITE on unmapped memory
    SP_ERR_FETCH_UNMAPPED,   // FETCH on unmapped memory
    SP_ERR_READ_PROT,        // READ  PROTECTION violation
    SP_ERR_WRITE_PROT,       // WRITE PROTECTION violation
    SP_ERR_FETCH_PROT,       // FETCH PROTECTION violation
    SP_ERR_READ_MISALIGNED,  // Misaligned READ
    SP_ERR_WRITE_MISALIGNED, // Misaligned WRITE
    SP_ERR_FETCH_MISALIGNED, // Misaligned FETCH
    SP_ERR_REGID_INVALID,    // Invalid Register id
    SP_ERR_INSN_INVALID,     // Invalid Instruction
    SP_ERR_MAP_INVALID,      // Invalid memory mapping
    SP_ERR_UNKNOWN           // Other error
} sp_err;

