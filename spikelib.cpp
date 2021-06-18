// #include "riscv-tools/riscv-isa-sim/riscv/execute.cc"
#include "riscv-tools/riscv-isa-sim/riscv/processor.h"
#include "riscv-tools/riscv-isa-sim/riscv/devices.h"
#include "riscv-tools/riscv-isa-sim/fesvr/memif.h"
#include "riscv-tools/riscv-isa-sim/riscv/mmu.h"
#include "riscv-tools/riscv-isa-sim/riscv/sim.h"
#include "riscv-tools/riscv-isa-sim/riscv/trap.h"
#include "config.h"

#define EXPORT  __attribute__ ((visibility ("default")))

typedef struct {
    uint64_t base;
    uint64_t size;
    void* content;
} memory_region;

extern "C" {
    // void* doInitialize_sim(); // when using C wrapper
    void* initialize_sim(memory_region* memories, int regions_number);
    int read_register(void* sim, int regid, void* value);
    int write_register(void* sim, int regid, void* value);
    const char* sp_strerror(int code);
    int write_memory(void* sim, uint64_t address, uint64_t size, void* value);
    int read_memory(void* sim, uint64_t address, uint64_t size, void* value);
}

// =====================================
//   SIMULATION INITIALIZATION HELPERS
// =====================================

std::vector<std::pair<reg_t, mem_t*>> initialize_mems(memory_region* memories, int regions_number) {
    std::vector<std::pair<reg_t, mem_t*>> res;
    // page-align base and size
    // TODO: in Pharo!
    // auto base0 = base, size0 = size;
    // size += base0 % PGSIZE;
    // base -= base0 % PGSIZE;
    // if (size % PGSIZE != 0)
    //     size += PGSIZE - size % PGSIZE;
    for (int i = 0; i < regions_number; i++) {
        res.push_back(
            std::make_pair(reg_t(memories[i].base), new mem_t(memories[i].size, (char*) memories[i].content))
        );
    }
    
    return res;
}

std::vector<int> initialize_hartids() {
    std::vector<int> hartids;
  //  hartids.push_back(0);  
    return hartids;
}

// =====================================
//         DEBUG/PRING HELPERS
// =====================================

void print_registers(processor_t* proc) {
    // Print the content of the 32 general registers
    for (int i = 0; i < 32; i++) {
        printf("XPR%d register value: 0d%04lu 0x%04lx\n", i, proc->get_state()->XPR[i], proc->get_state()->XPR[i]);
    }
}
void print_pc(processor_t* proc) {
    reg_t pc_reg = proc->get_state()->pc;
    printf("PC register value: 0x%lx\n", pc_reg);
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

EXPORT const char* sp_strerror(int code) {
    switch(code) {
        default:
            return "Unknown error code";
        case SP_ERR_OK:
            return "OK (SP_ERR_OK)";
        case SP_ERR_NO_MEM:
            return "No memory available or memory not present (SP_ERR_NO_MEM)";
        case SP_ERR_READ_UNMAPPED:
            return "Invalid memory read (SP_ERR_READ_UNMAPPED)";
        case SP_ERR_WRITE_UNMAPPED:
            return "Invalid memory write (SP_ERR_WRITE_UNMAPPED)";
        case SP_ERR_FETCH_UNMAPPED:
            return "Invalid memory fetch (SP_ERR_FETCH_UNMAPPED)";
        case SP_ERR_INSN_INVALID:
            return "Invalid instruction (SP_ERR_INSN_INVALID)";
        case SP_ERR_MAP:
            return "Invalid memory mapping (SP_ERR_MAP)";
        case SP_ERR_READ_PROT:
            return "Read from non-readable memory (SP_ERR_READ_PROT)";
        case SP_ERR_WRITE_PROT:
            return "Write to write-protected memory (SP_ERR_WRITE_PROT)";
        case SP_ERR_FETCH_PROT:
            return "Fetch from non-executable memory (SP_ERR_FETCH_PROT)";
        case SP_ERR_READ_UNALIGNED:
            return "Read from unaligned memory (SP_ERR_READ_UNALIGNED)";
        case SP_ERR_WRITE_UNALIGNED:
            return "Write to unaligned memory (SP_ERR_WRITE_UNALIGNED)";
        case SP_ERR_FETCH_UNALIGNED:
            return "Fetch from unaligned memory (SP_ERR_FETCH_UNALIGNED)";
    }
}


// =====================================
//         PHARO API WRAPPERS
// =====================================

EXPORT void* initialize_sim(memory_region* memories, int regions_number){
    const char* isa            = DEFAULT_ISA;    // RISC-V ISA string          (rv32 or rv64 with extensions, g = imafd)  DEFAULT = IMAFDC
    size_t nprocs              = size_t(1);      // Number of processors                         
    bool halted                = false;          // Start halted, allowing a debugger to connect    
    reg_t start_pc             = reg_t(0x1000);  // Start PC
    std::vector<std::pair<reg_t, mem_t*>> mems;  // Memories
    mems = initialize_mems(memories, regions_number); // -
    std::vector<std::string> htif_args;          // Arguments for htif
    std::string str("toto");                     // -
    htif_args.push_back(str);                    // -
    std::vector<int> hartids;                    // Hartids
    hartids = initialize_hartids();              // -
    unsigned progsize = 2;                       // Progsize for the debug module
    unsigned max_bus_master_bits = 0;            // Debug bus master supports up to
    bool require_authentication = false;         // Debug module requires debugger to authenticate
    unsigned abstract_rti = 0;                   // Number of Run-Test/Idle cycles

    void* sim;

    try{
        sim = new sim_t(
        isa, 
        nprocs, 
        halted, 
        start_pc, 
        mems, 
        htif_args, 
        std::move(hartids),
        progsize, 
        max_bus_master_bits, 
        require_authentication,
        abstract_rti
    );

    } catch(...){
        return NULL;
    }

    return static_cast<void*>(sim);
}

EXPORT int read_register(void* sim, int regid, void* value) {
    // Casting the void* to sim_t for the simulation
    sim_t* real_sim = (sim_t*) sim;
    // Getting the value of the register regid
    // Casting the address of the resulting value back to void*
    switch(regid) {
        case SPIKE_RISCV_REG_PC: {
            *((uint32_t*) value) = real_sim->get_core(0)->get_state()->pc;
            break;
        }
        default: {
            *((uint32_t*) value) = real_sim->get_core(0)->get_state()->XPR[regid];
        }
    }
    return SP_ERR_OK;
}

EXPORT int write_register(void* sim, int regid, void* value) {  // Returns an sp_err but simplified to int for export
    // Casting the void* to sim_t for the simulation
    sim_t* real_sim = (sim_t*) sim;
    // Casting the content of value to uint32_t
    // Writing the result to register regid
    switch(regid) {
        case SPIKE_RISCV_REG_PC: {
            real_sim->get_core(0)->get_state()->pc = (*((uint32_t*)value));
        }
        default: {
            real_sim->get_core(0)->get_state()->XPR.write(regid, *((uint32_t*)value));
        }   
    }
    return SP_ERR_OK;
}

EXPORT int read_memory(void* sim, uint64_t address, uint64_t size, void* value) {
    sim_t* real_sim = (sim_t*) sim;
    // Switch on the size to call the proper function
    switch(size) {
        case 1:
            *((uint8_t*) value) = real_sim->get_core(0)->get_mmu()->load_uint8(address);
            break;
        case 2:
            *((uint16_t*) value) = real_sim->get_core(0)->get_mmu()->load_uint16(address);
            break;
        case 4:
            *((uint32_t*) value) = real_sim->get_core(0)->get_mmu()->load_uint32(address);
            break;
        case 8:
            *((uint64_t*) value) = real_sim->get_core(0)->get_mmu()->load_uint64(address);
            break;
        default:
            // If the size is not standard, load the bytes one by one
            for (int i=0; i<size; i++) {
                *((uint8_t*) value + i*8) = real_sim->get_core(0)->get_mmu()->load_uint8(address + i*8);
            }
    }   
    return SP_ERR_OK;
}

EXPORT int write_memory(void* sim, uint64_t address, uint64_t size, void* value) {
    sim_t* real_sim = (sim_t*) sim;
    // Switch on the size to call the proper function
    switch(size) {
        case 1:
            real_sim->get_core(0)->get_mmu()->store_uint8(address, *((uint8_t*) value));
            break;
        case 2:
            real_sim->get_core(0)->get_mmu()->store_uint16(address, *((uint16_t*) value));
            break;
        case 4:
            real_sim->get_core(0)->get_mmu()->store_uint32(address, *((uint32_t*) value));
            break;
        case 8:
            real_sim->get_core(0)->get_mmu()->store_uint64(address, *((uint64_t*) value));
            break;
        default:
            // If the size is not standard, store the bytes one by one
            for (int i=0; i<size; i++) {
                real_sim->get_core(0)->get_mmu()->store_uint8(address + i*8, *((uint8_t*) value + i*8));
            }
    }   
    return SP_ERR_OK;
}


// =====================================
//       MAIN FOR EXPERIMENTATIONS
// =====================================

int main() {

    void* content = calloc(1, 4096);
    memory_region region[] = { {.base = 0x1000, .size = 4096, .content = content} };

    // Default values
    const char* isa            = DEFAULT_ISA;    // RISC-V ISA string          (rv32 or rv64 with extensions, g = imafd)  DEFAULT = IMAFDC
    size_t nprocs              = size_t(1);      // Number of processors                         
    bool halted                = false;          // Start halted, allowing a debugger to connect    
    reg_t start_pc             = reg_t(0x1000);  // Start PC
    std::vector<std::pair<reg_t, mem_t*>> mems;  // Memories
    mems = initialize_mems(region, 1);    // -
    std::vector<std::string> htif_args;          // Arguments for htif
    std::string str("toto");                     // -
    htif_args.push_back(str);                    // -
    std::vector<int> hartids;                    // Hartids
    hartids = initialize_hartids();              // -
    unsigned progsize = 2;                       // Progsize for the debug module
    unsigned max_bus_master_bits = 0;            // Debug bus master supports up to
    bool require_authentication = false;         // Debug module requires debugger to authenticate
    unsigned abstract_rti = 0;                   // Number of Run-Test/Idle cycles

    // Additional values with master branch
    // const char* privilege_mode = DEFAULT_PRIV;   // RISC-V privilege modes     (m, mu, msu)                               DEFAULT = msu
    // const char* varch          = DEFAULT_VARCH;  // RISC-V Vector uArch string (vlen, elen and slen)                      DEFAULT = vlen:128,elen:64,slen:128
    // bool real_time_clint       = false;          // Increment clint time at a real-time rate
    // reg_t initrd_start         = 0;              // Load kernel initrd into memory
    // reg_t initrd_end           = 0;              // -
    // const char* bootargs       = NULL;           // Kernel boot arguments        
    // std::vector<std::pair<reg_t, abstract_device_t*>> plugin_devices; // Memory Map Input devices   
    // debug_module_config_t dm_config = {
    //     .progbufsize                 = 2,
    //     .max_bus_master_bits         = 0,
    //     .require_authentication      = false,
    //     .abstract_rti                = 0,
    //     .support_hasel               = true,
    //     .support_abstract_csr_access = true,
    //     .support_haltgroups          = true,
    //     .support_impebreak           = true
    // };
    // const char *log_path = nullptr;
    // bool dtb_enabled     = true;
    // const char *dtb_file = nullptr;

    sim_t* sim = new sim_t(
        isa, 
        nprocs, 
        halted, 
        start_pc, 
        mems, 
        htif_args, 
        std::move(hartids),
        progsize, 
        max_bus_master_bits, 
        require_authentication,
        abstract_rti
    );

    // Number of processors
    printf("Number of processors: %d\n", sim->nprocs());

    // Extract the processor
    processor_t *proc = sim->get_core(size_t(0));

    // Print pc and registers
    print_pc(proc);
    print_registers(proc);

    // Store one byte
    const uint8_t stored_byte = 0x11;
    proc->get_mmu()->store_uint8(0x1000, stored_byte);

    // Load one byte
    uint8_t loaded_byte = proc->get_mmu()->load_uint8(0x1000);
    printf("Loaded bytes: %d\n", loaded_byte);

    // Load values in x6 and x7
    proc->get_state()->XPR.write(size_t(6), 17);
    proc->get_state()->XPR.write(size_t(7), 42);

    // Some instruction codes
    // ___________________________________
    // COMPRESSED add a5,a5,a4 -> 97ba
    // uint16_t instr_cadd = 0x97ba;
    // COMPRESSED mov a0 a5
    // uint16_t instr_mov = 0x3e85;
    // COMPLETE add x5, x6, x7
    // uint32_t instr_add = 0x007302B3;
    // ___________________________________

    // Load instruction and decode some information
    proc->get_mmu()->flush_icache();
    insn_fetch_t fetched_instr = proc->get_mmu()->load_insn(0x1000);
    printf("INSTR: %lx\n", fetched_instr.insn.bits());
    printf("RD:    %lu\n", fetched_instr.insn.rd());
    printf("RS1:   %lu\n", fetched_instr.insn.rs1());
    // Step to run ADD
    proc->step(size_t(1));
    print_registers(proc);

    // Write the load instruction corresponding at address 1004 (new pc)
    proc->get_state()->XPR.write(size_t(6), 16);
    uint32_t instr_load = 6 << 15 | 0b011 << 12 | 7 << 7 | 0b0000011;
    proc->get_mmu()->store_uint32(0x1004, instr_load);

    // Load instruction and decode some information
    proc->get_mmu()->flush_icache();
    fetched_instr = proc->get_mmu()->load_insn(0x1004);
    printf("INSTR: 0x%lx\n", fetched_instr.insn.bits());
    printf("RD:    %lu\n", fetched_instr.insn.rd());
    printf("RS1:   %lu\n", fetched_instr.insn.rs1());

    // Step to run LOAD
    proc->step(size_t(1));

    // Print PC and registers
    print_pc(proc);
    print_registers(proc);



    // Testing the memory read/write functions
    // uint8_t
    uint8_t mem_write_byte_8 = 0x11;
    uint8_t mem_load_byte_8  = 0x00;
    write_memory(sim, 0x1000, 1, (void*) &mem_write_byte_8);
    read_memory(sim, 0x1000, 1, (void*) &mem_load_byte_8);
    printf("Mem read/write 1 byte  test: 0x%x\n", mem_load_byte_8);

    // uint16_t
    uint16_t mem_write_byte_16 = 0x1111;
    uint16_t mem_load_byte_16  = 0x0000;
    write_memory(sim, 0x1030, 2, (void*) &mem_write_byte_16);
    read_memory(sim, 0x1030, 2, (void*) &mem_load_byte_16);
    printf("Mem read/write 2 bytes test: 0x%x\n", mem_load_byte_16);

    // uint32_t
    uint32_t mem_write_byte_32 = 0x11111111;
    uint32_t mem_load_byte_32  = 0x00000000;
    write_memory(sim, 0x1060, 4, (void*) &mem_write_byte_32);
    read_memory(sim, 0x1060, 4, (void*) &mem_load_byte_32);
    printf("Mem read/write 4 bytes test: 0x%x\n", mem_load_byte_32);

    // uint64_t
    uint64_t mem_write_byte_64 = 0x1111111111111111;
    uint64_t mem_load_byte_64  = 0x0000000000000000;
    write_memory(sim, 0x1120, 8, (void*) &mem_write_byte_64);
    read_memory(sim, 0x1120, 8, (void*) &mem_load_byte_64);
    printf("Mem read/write 8 bytes test: 0x%lx\n", mem_load_byte_64);

    return 0;
}