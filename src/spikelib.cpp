#include <sys/time.h>
#include "processor.h"
#include "devices.h"
#include "memif.h"
#include "mmu.h"
#include "sim.h"
#include "trap.h"
#include "config.h"
#include "spikelib.h"

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

void print_byte_array(uint8_t* byte_array, int size) {
    for (int i = 0; i < size; i++)
    {
        printf("%02X", byte_array[i]);
    }
    printf("\n");
}

// =====================================
//         TIME FUNCTION
// =====================================


static inline int64_t get_clock_realtime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000000LL + (tv.tv_usec * 1000);
}

// =====================================
//        ERROR CODE FORMATTING
// =====================================

EXPORT const char* sp_strerror(int code) {
    switch(code) {
        // _________ OK ________
        case SP_ERR_OK:
            return "OK (SP_ERR_OK)";
        // _____ Execution _____
        case SP_ERR_TIMEOUT:
            return "Execution timeout (SP_ERR_TIMEOUT)";
        case SP_ERR_MAX_COUNT:
            return "Max number of instructions reached (SP_ERR_MAX_COUNT)";
        // ________ OOM ________
        case SP_ERR_NO_MEM:
            return "No memory available or memory not present (SP_ERR_NO_MEM)";
        // ______ Unmapped _____
        case SP_ERR_READ_UNMAPPED:
            return "Invalid memory read (SP_ERR_READ_UNMAPPED)";
        case SP_ERR_WRITE_UNMAPPED:
            return "Invalid memory write (SP_ERR_WRITE_UNMAPPED)";
        case SP_ERR_FETCH_UNMAPPED:
            return "Invalid memory fetch (SP_ERR_FETCH_UNMAPPED)";
        // ______ Invalid ______
        case SP_ERR_REGID_INVALID:
            return "Invalid register id (SP_ERR_REGID_INVALID)";
        case SP_ERR_INSN_INVALID:
            return "Invalid instruction (SP_ERR_INSN_INVALID)";
        case SP_ERR_MAP_INVALID:
            return "Invalid memory mapping (SP_ERR_MAP)";
        // _____ Protection _____
        case SP_ERR_READ_PROT:
            return "Read from non-readable memory (SP_ERR_READ_PROT)";
        case SP_ERR_WRITE_PROT:
            return "Write to write-protected memory (SP_ERR_WRITE_PROT)";
        case SP_ERR_FETCH_PROT:
            return "Fetch from non-executable memory (SP_ERR_FETCH_PROT)";
        // _____ Alignment ______
        case SP_ERR_READ_MISALIGNED:
            return "Read from unaligned memory (SP_ERR_READ_MISALIGNED)";
        case SP_ERR_WRITE_MISALIGNED:
            return "Write to unaligned memory (SP_ERR_WRITE_MISALIGNED)";
        case SP_ERR_FETCH_MISALIGNED:
            return "Fetch from unaligned memory (SP_ERR_FETCH_MISALIGNED)";
        // ______ Unknown _______
        default:
            return "Unknown error code";
    }
}

// =====================================
//         PHARO API WRAPPERS
// =====================================


EXPORT void* initialize_sim_with_isa(memory_region* memories, int regions_number, const char* isa){
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

EXPORT void* initialize_sim(memory_region* memories, int regions_number) {
    // DEFAULT_ISA: (rv32 or rv64 with extensions, g = imafd)  DEFAULT = IMAFDC
    return initialize_sim_with_isa(memories, regions_number, DEFAULT_ISA);
}

EXPORT void release_sim(void* sim) {
    delete((sim_t*) sim);
}


/* Read a value from a register into a buffer
   Arguments: sim (void *) - Pointer to the simulation 
*/
EXPORT int read_register(void* sim, int regid, void* value) {
    sim_t* real_sim = (sim_t*) sim;
    switch(regid) {
        // PC
        case SPIKE_RISCV_REG_PC: {
            *((uint64_t*) value) = real_sim->get_core(0)->get_state()->pc;
            break;
        }
        // General Registers
        case SPIKE_RISCV_REG_X0:
        case SPIKE_RISCV_REG_X1:
        case SPIKE_RISCV_REG_X2:
        case SPIKE_RISCV_REG_X3:
        case SPIKE_RISCV_REG_X4:
        case SPIKE_RISCV_REG_X5:
        case SPIKE_RISCV_REG_X6:
        case SPIKE_RISCV_REG_X7:
        case SPIKE_RISCV_REG_X8:
        case SPIKE_RISCV_REG_X9:
        case SPIKE_RISCV_REG_X10:
        case SPIKE_RISCV_REG_X11:
        case SPIKE_RISCV_REG_X12:
        case SPIKE_RISCV_REG_X13:
        case SPIKE_RISCV_REG_X14:
        case SPIKE_RISCV_REG_X15:
        case SPIKE_RISCV_REG_X16:
        case SPIKE_RISCV_REG_X17:
        case SPIKE_RISCV_REG_X18:
        case SPIKE_RISCV_REG_X19:
        case SPIKE_RISCV_REG_X20:
        case SPIKE_RISCV_REG_X21:
        case SPIKE_RISCV_REG_X22:
        case SPIKE_RISCV_REG_X23:
        case SPIKE_RISCV_REG_X24:
        case SPIKE_RISCV_REG_X25:
        case SPIKE_RISCV_REG_X26:
        case SPIKE_RISCV_REG_X27:
        case SPIKE_RISCV_REG_X28:
        case SPIKE_RISCV_REG_X29:
        case SPIKE_RISCV_REG_X30:
        case SPIKE_RISCV_REG_X31: {
            *((uint64_t*) value) = real_sim->get_core(0)->get_state()->XPR[regid-SPIKE_RISCV_REG_X0];
            break;
        }
        // Floating Point Registers
        case SPIKE_RISCV_REG_F0:
        case SPIKE_RISCV_REG_F1:
        case SPIKE_RISCV_REG_F2:
        case SPIKE_RISCV_REG_F3:
        case SPIKE_RISCV_REG_F4:
        case SPIKE_RISCV_REG_F5:
        case SPIKE_RISCV_REG_F6:
        case SPIKE_RISCV_REG_F7:
        case SPIKE_RISCV_REG_F8:
        case SPIKE_RISCV_REG_F9:
        case SPIKE_RISCV_REG_F10:
        case SPIKE_RISCV_REG_F11:
        case SPIKE_RISCV_REG_F12:
        case SPIKE_RISCV_REG_F13:
        case SPIKE_RISCV_REG_F14:
        case SPIKE_RISCV_REG_F15:
        case SPIKE_RISCV_REG_F16:
        case SPIKE_RISCV_REG_F17:
        case SPIKE_RISCV_REG_F18:
        case SPIKE_RISCV_REG_F19:
        case SPIKE_RISCV_REG_F20:
        case SPIKE_RISCV_REG_F21:
        case SPIKE_RISCV_REG_F22:
        case SPIKE_RISCV_REG_F23:
        case SPIKE_RISCV_REG_F24:
        case SPIKE_RISCV_REG_F25:
        case SPIKE_RISCV_REG_F26:
        case SPIKE_RISCV_REG_F27:
        case SPIKE_RISCV_REG_F28:
        case SPIKE_RISCV_REG_F29:
        case SPIKE_RISCV_REG_F30:
        case SPIKE_RISCV_REG_F31: {
            *((float128_t*) value) = real_sim->get_core(0)->get_state()->FPR[regid-SPIKE_RISCV_REG_F0];
            break;
        }
        // Unknown Regid
        default: return SP_ERR_REGID_INVALID;
    }
    return SP_ERR_OK;
}

EXPORT int write_register(void* sim, int regid, void* value) {  
    sim_t* real_sim = (sim_t*) sim;
    switch(regid) {
        // PC
        case SPIKE_RISCV_REG_PC: {
            real_sim->get_core(0)->get_state()->pc = (*((uint64_t*)value));
            break;
        }
        // General Registers
        case SPIKE_RISCV_REG_X0:
        case SPIKE_RISCV_REG_X1:
        case SPIKE_RISCV_REG_X2:
        case SPIKE_RISCV_REG_X3:
        case SPIKE_RISCV_REG_X4:
        case SPIKE_RISCV_REG_X5:
        case SPIKE_RISCV_REG_X6:
        case SPIKE_RISCV_REG_X7:
        case SPIKE_RISCV_REG_X8:
        case SPIKE_RISCV_REG_X9:
        case SPIKE_RISCV_REG_X10:
        case SPIKE_RISCV_REG_X11:
        case SPIKE_RISCV_REG_X12:
        case SPIKE_RISCV_REG_X13:
        case SPIKE_RISCV_REG_X14:
        case SPIKE_RISCV_REG_X15:
        case SPIKE_RISCV_REG_X16:
        case SPIKE_RISCV_REG_X17:
        case SPIKE_RISCV_REG_X18:
        case SPIKE_RISCV_REG_X19:
        case SPIKE_RISCV_REG_X20:
        case SPIKE_RISCV_REG_X21:
        case SPIKE_RISCV_REG_X22:
        case SPIKE_RISCV_REG_X23:
        case SPIKE_RISCV_REG_X24:
        case SPIKE_RISCV_REG_X25:
        case SPIKE_RISCV_REG_X26:
        case SPIKE_RISCV_REG_X27:
        case SPIKE_RISCV_REG_X28:
        case SPIKE_RISCV_REG_X29:
        case SPIKE_RISCV_REG_X30:
        case SPIKE_RISCV_REG_X31: {
            real_sim->get_core(0)->get_state()->XPR.write(regid-SPIKE_RISCV_REG_X0, *((uint64_t*)value));
            break;
        }   
        // Floating Point Registers
        case SPIKE_RISCV_REG_F0:
        case SPIKE_RISCV_REG_F1:
        case SPIKE_RISCV_REG_F2:
        case SPIKE_RISCV_REG_F3:
        case SPIKE_RISCV_REG_F4:
        case SPIKE_RISCV_REG_F5:
        case SPIKE_RISCV_REG_F6:
        case SPIKE_RISCV_REG_F7:
        case SPIKE_RISCV_REG_F8:
        case SPIKE_RISCV_REG_F9:
        case SPIKE_RISCV_REG_F10:
        case SPIKE_RISCV_REG_F11:
        case SPIKE_RISCV_REG_F12:
        case SPIKE_RISCV_REG_F13:
        case SPIKE_RISCV_REG_F14:
        case SPIKE_RISCV_REG_F15:
        case SPIKE_RISCV_REG_F16:
        case SPIKE_RISCV_REG_F17:
        case SPIKE_RISCV_REG_F18:
        case SPIKE_RISCV_REG_F19:
        case SPIKE_RISCV_REG_F20:
        case SPIKE_RISCV_REG_F21:
        case SPIKE_RISCV_REG_F22:
        case SPIKE_RISCV_REG_F23:
        case SPIKE_RISCV_REG_F24:
        case SPIKE_RISCV_REG_F25:
        case SPIKE_RISCV_REG_F26:
        case SPIKE_RISCV_REG_F27:
        case SPIKE_RISCV_REG_F28:
        case SPIKE_RISCV_REG_F29:
        case SPIKE_RISCV_REG_F30:
        case SPIKE_RISCV_REG_F31: {
            real_sim->get_core(0)->get_state()->FPR.write(regid-SPIKE_RISCV_REG_F0, *((float128_t *)value));
            break;
        }   
        // Unknown regid
        default: return SP_ERR_REGID_INVALID;
    }
    return SP_ERR_OK;
}

EXPORT int read_memory(void* sim, uint64_t address, uint64_t size, void* value) {
    sim_t* real_sim = (sim_t*) sim;
    // Check alignment
    if ((int) address % 8 != 0) return SP_ERR_READ_MISALIGNED;
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
                ((uint8_t*) value)[i] = real_sim->get_core(0)->get_mmu()->load_uint8(address + i);
            }
    }   
    return SP_ERR_OK;
}

EXPORT int write_memory(void* sim, uint64_t address, uint64_t size, void* value) {
    sim_t* real_sim = (sim_t*) sim;
    // Check alignment
    if ((int) address % 8 != 0) return SP_ERR_WRITE_MISALIGNED;
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
                real_sim->get_core(0)->get_mmu()->store_uint8(address + i, ((uint8_t*) value)[i]);
            }
    }   
    real_sim->get_core(0)->get_mmu()->flush_icache();
    return SP_ERR_OK;
}

EXPORT int get_memory_exception_cause(void* sim) {
    sim_t* real_sim = (sim_t*) sim;
    processor_t* core = real_sim->get_core(0);
    state_t* state = core->get_state();
    sp_err error = SP_ERR_OK;
    // Switch on the mcause register that holds any issue the memory access might have
    switch(state->mcause & 0b111) {
        // P. 102-105
        case(0): {
            if (state->pc == state->mtvec) {
                error = SP_ERR_FETCH_MISALIGNED;
            }
            break; 
        }
        case(1): error = SP_ERR_FETCH_UNMAPPED;   break; // mcause = 1 | Instruction access fault
        case(2): error = SP_ERR_INSN_INVALID;     break; // mcause = 2 | Illegal instruction
        case(4): error = SP_ERR_READ_MISALIGNED;  break; // mcause = 4 | Load address misaligned
        case(5): error = SP_ERR_READ_UNMAPPED;    break; // mcause = 5 | Load access fault
        case(6): error = SP_ERR_WRITE_MISALIGNED; break; // mcause = 6 | Store address misaligned
        case(7): error = SP_ERR_WRITE_UNMAPPED;   break; // mcause = 7 | Store access fault
        default: break;
    }
    return error;

}

EXPORT int spike_start(void* sim, uint64_t begin_address, uint64_t end_address, uint64_t timeout_us, size_t max_instruction_number) {
    sim_t* real_sim = (sim_t*) sim;
    processor_t* core = real_sim->get_core(0);
    state_t* state = core->get_state();

    // Write the begin address to the PC
    write_register(sim, SPIKE_RISCV_REG_PC, &begin_address);
    // Initialize the timer
    int64_t current_time_us = get_clock_realtime();
    // Check for the end address to be ok, if not STOP execution directly
    bool has_timed_out     = false;
    bool has_reached_count = false;
    bool has_reached_end   = false;
    bool has_mem_exception = false;
    int instruction_count = 0;
    while(!has_reached_end && !has_timed_out && !has_reached_count && !has_mem_exception) {
        core->step(1);
        // Check time out, instruction count and final pc
        has_timed_out     = !(((uint64_t)(get_clock_realtime() - current_time_us) < timeout_us) || timeout_us == 0);
        has_reached_count = (++instruction_count == max_instruction_number) && (max_instruction_number != 0);
        has_reached_end   = (state->pc == end_address);
        has_mem_exception = (get_memory_exception_cause(sim) != SP_ERR_OK);
    }
    
    if (has_reached_end)   return SP_ERR_OK;
    if (has_reached_count) return SP_ERR_MAX_COUNT;
    if (has_timed_out)     return SP_ERR_TIMEOUT;
    if (has_mem_exception) return get_memory_exception_cause(sim);

    return SP_ERR_UNKNOWN;
}


// =====================================
//       MAIN FOR EXPERIMENTATIONS
// =====================================

int main() {
    // Simulation initialization
    printf("%s\n", DEFAULT_ISA);
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

    // Extract the processor
    processor_t *proc = sim->get_core(size_t(0));
    // Print pc and registers
    // print_pc(proc);
    // print_registers(proc);
    // Store one byte
    // const uint8_t stored_byte = 0x11;
    // proc->get_mmu()->store_uint8(0x1000, stored_byte);
    // Load one byte
    // uint8_t loaded_byte = proc->get_mmu()->load_uint8(0x1000);
    // printf("Loaded bytes: %d\n", loaded_byte);
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
    // // ___________________________________

    // // Load instruction and decode some information
    // proc->get_mmu()->store_uint32(0x1000, instr_add);
    // proc->get_mmu()->flush_icache();
    // insn_fetch_t fetched_instr = proc->get_mmu()->load_insn(0x1000);
    
    // printf("INSTR: 0x%lx\n", fetched_instr.insn.bits());
    // printf("RD:    %lu\n", fetched_instr.insn.rd());
    // printf("RS1:   %lu\n", fetched_instr.insn.rs1());
    // // Step to run ADD
    // proc->step(size_t(1));
    // print_registers(proc);


    uint32_t instr_lui = 0x0004d3b7;
    // ___________________________________

    // Load instruction and decode some information
    proc->get_mmu()->store_uint32(0x1000, instr_lui);
    proc->get_mmu()->flush_icache();
    insn_fetch_t fetched_instr = proc->get_mmu()->load_insn(0x1000);
    
    printf("INSTR: 0x%lx\n", fetched_instr.insn.bits());
    printf("RD:    %lu\n", fetched_instr.insn.rd());
    printf("RS1:   %lu\n", fetched_instr.insn.rs1());
    // Step to run 
    proc->step(size_t(1));
    print_registers(proc);


    uint32_t instr_addiw = 0xccd3839b; 

    // ___________________________________

    // Load instruction and decode some information
    proc->get_mmu()->store_uint32(0x1004, instr_addiw);
    proc->get_mmu()->flush_icache();
    fetched_instr = proc->get_mmu()->load_insn(0x1004);
    
    printf("INSTR: 0x%lx\n", fetched_instr.insn.bits());
    printf("RD:    %lu\n", fetched_instr.insn.rd());
    printf("RS1:   %lu\n", fetched_instr.insn.rs1());
    // Step to run 
    proc->step(size_t(1));
    print_registers(proc);

    // Write the load instruction corresponding at address 1004 (new pc)
    // proc->get_state()->XPR.write(size_t(6), 16);
    // uint32_t instr_load = 6 << 15 | 0b011 << 12 | 7 << 7 | 0b0000011;
    // proc->get_mmu()->store_uint32(0x1004, instr_load);

    // // Load instruction and decode some information
    // proc->get_mmu()->flush_icache();
    // fetched_instr = proc->get_mmu()->load_insn(0x1004);
    // printf("INSTR: 0x%lx\n", fetched_instr.insn.bits());
    // printf("RD:    %lu\n", fetched_instr.insn.rd());
    // printf("RS1:   %lu\n", fetched_instr.insn.rs1());

    // // Step to run LOAD
    // proc->step(size_t(1));

    // // Print PC and registers
    // print_pc(proc);
    // print_registers(proc);

    return 0;
}