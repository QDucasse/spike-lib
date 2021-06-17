// #include "riscv-tools/riscv-isa-sim/riscv/execute.cc"
#include "riscv-tools/riscv-isa-sim/riscv/processor.h"
#include "riscv-tools/riscv-isa-sim/riscv/devices.h"
#include "riscv-tools/riscv-isa-sim/fesvr/memif.h"
#include "riscv-tools/riscv-isa-sim/riscv/mmu.h"
#include "riscv-tools/riscv-isa-sim/riscv/sim.h"
#include "riscv-tools/riscv-isa-sim/riscv/trap.h"
#include "config.h"

#define EXPORT  __attribute__ ((visibility ("default")))

extern "C" {
    void* doInitialize_sim();
    // void* initialize_sim();
    // uint32_t read_register(void* sim, int nb);
    // void write_register(void* sim, int nb, uint32_t value);
}

std::vector<std::pair<reg_t, mem_t*>> initialize_mems(int base, int size, char* contents) {
    std::vector<std::pair<reg_t, mem_t*>> res;
        // page-align base and size
    auto base0 = base, size0 = size;
    size += base0 % PGSIZE;
    base -= base0 % PGSIZE;
    if (size % PGSIZE != 0)
        size += PGSIZE - size % PGSIZE;
    res.push_back(std::make_pair(reg_t(base), new mem_t(size,contents)));
    return res;
}

std::vector<int> initialize_hartids() {
    std::vector<int> hartids;
  //  hartids.push_back(0);  
    return hartids;
}

void print_registers(processor_t* proc) {
    // Print the content of the 32 general registers
    for (int i = 0; i < 32; i++) {
        printf("XPR%d register value: 0d%04lu 0x%04lx\n", i, proc->get_state()->XPR[i], proc->get_state()->XPR[i]);
    }
}

void* doInitialize_sim(){

        const char* isa            = DEFAULT_ISA;    // RISC-V ISA string          (rv32 or rv64 with extensions, g = imafd)  DEFAULT = IMAFDC
        size_t nprocs              = size_t(1);      // Number of processors                         
        bool halted                = false;          // Start halted, allowing a debugger to connect    
        reg_t start_pc             = reg_t(0x1000);  // Start PC
        std::vector<std::pair<reg_t, mem_t*>> mems;  // Memories
        char* contents = (char*)calloc(1, 4096);
        mems = initialize_mems(0x1000, 4096, contents); // -
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


// =================================
//       PHARO API WRAPPERS
// =================================

// EXPORT void* initialize_sim(){
//     return doInitialize_sim();
// }

// uint32_t read_register(void* sim, int nb) {
//    sim_t* real_sim = (sim_t*) sim;
//    return real_sim->get_core(0)->get_state()->XPR[nb];
// }

// void write_register(void* sim, int nb, uint32_t value) {
//    sim_t* real_sim = (sim_t*) sim;
//    real_sim->get_core(0)->get_state()->XPR.write(nb, value);
// }




int main() {
/*
    // Default values
    const char* isa            = DEFAULT_ISA;    // RISC-V ISA string          (rv32 or rv64 with extensions, g = imafd)  DEFAULT = IMAFDC
    size_t nprocs              = size_t(1);      // Number of processors                         
    bool halted                = false;          // Start halted, allowing a debugger to connect    
    reg_t start_pc             = reg_t(0x1000);  // Start PC
    std::vector<std::pair<reg_t, mem_t*>> mems;  // Memories
    char* contents = (char*)calloc(1, 4096);
    mems = initialize_mems(0x1000, 4096, contents); // -
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

    // Extract the PC from the state of the processor
    processor_t *proc = sim->get_core(size_t(0));
    reg_t pc_reg = proc->get_state()->pc;
    printf("PC register value: 0x%lx\n", pc_reg);

    // Print state
    print_registers(proc);

    // Store one byte
    const uint8_t stored_byte = 0x11;
    proc->get_mmu()->store_uint8(0x1000, stored_byte);

    // Load one byte
    uint8_t loaded_byte = proc->get_mmu()->load_uint8(0x1000);
    printf("Loaded bytes: %d\n", loaded_byte);

    // Load values in x0 and x1
    proc->get_state()->XPR.write(size_t(6), 17);
    proc->get_state()->XPR.write(size_t(7), 42);

    // COMPRESSED add a5,a5,a4 -> 97ba
    // uint16_t instr_cadd = 0x97ba;
    // COMPRESSED mov a0 a5
    // uint16_t instr_mov = 0x3e85;
    // COMPLETE add x5, x6, x7
    // ((uint32_t*)contents)[0] = 0x007302B3;
    // uint32_t instr_add = 0x007302B3;
    // proc->get_mmu()->store_uint32(0x1000, instr_add);
    proc->get_mmu()->flush_icache();

    insn_fetch_t fetched_instr = proc->get_mmu()->load_insn(0x1000);
    printf("INSTR: %lx\n", fetched_instr.insn.bits());
    printf("RD:    %lu\n", fetched_instr.insn.rd());
    printf("RS1:   %lu\n", fetched_instr.insn.rs1());


    proc->step(size_t(1));
    print_registers(proc);
    // proc->get_state()->pc = 0xFFF0;

    proc->get_state()->XPR.write(size_t(6), 16);
    uint32_t instr_load = 6 << 15 | 0b011 << 12 | 7 << 7 | 0b0000011;
    proc->get_mmu()->store_uint32(0x1004, instr_load);
    proc->get_mmu()->flush_icache();

    fetched_instr = proc->get_mmu()->load_insn(0x1004);
    printf("INSTR: %lx\n", fetched_instr.insn.bits());
    printf("RD:    %lu\n", fetched_instr.insn.rd());
    printf("RS1:   %lu\n", fetched_instr.insn.rs1());

    proc->step(size_t(1));

    print_registers(proc);

    // pc_reg = execute_insn(proc, pc_reg, fetched_instr);    

    // proc->step(1000);
    // while(proc->get_state()->pc == 0x1000){
    //     printf("hello\n");
    //     proc->step(size_t(1));
    // }


    pc_reg = proc->get_state()->pc;
    printf("PC register value: 0x%lx\n", pc_reg);

    // Store instructions
    // Flush instruction cache?
*/
    return 0;
}