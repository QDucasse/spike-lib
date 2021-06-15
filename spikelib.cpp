#include "riscv-tools/riscv-isa-sim/riscv/processor.h"
#include "riscv-tools/riscv-isa-sim/riscv/devices.h"
#include "riscv-tools/riscv-isa-sim/fesvr/memif.h"
#include "riscv-tools/riscv-isa-sim/riscv/mmu.h"
#include "riscv-tools/riscv-isa-sim/riscv/sim.h"
#include "config.h"


 std::vector<std::pair<reg_t, mem_t*>> initialize_mems(int base, int size) {
    std::vector<std::pair<reg_t, mem_t*>> res;
     // page-align base and size
    auto base0 = base, size0 = size;
    size += base0 % PGSIZE;
    base -= base0 % PGSIZE;
    if (size % PGSIZE != 0)
      size += PGSIZE - size % PGSIZE;
    res.push_back(std::make_pair(reg_t(base), new mem_t(size)));
    return res;
 }

std::vector<int> initialize_hartids() {
    std::vector<int> hartids;
  //  hartids.push_back(0);  
    return hartids;
}

int main() {

    // Default values
    const char* isa            = DEFAULT_ISA;    // RISC-V ISA string          (rv32 or rv64 with extensions, g = imafd)  DEFAULT = IMAFDC
    size_t nprocs              = size_t(1);      // Number of processors                         
    bool halted                = true;           // Start halted, allowing a debugger to connect    
    reg_t start_pc             = reg_t(0x1000);  // Start PC
    std::vector<std::pair<reg_t, mem_t*>> mems;  // Memories
    mems = initialize_mems(0x1000, 4096);        // -
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

    sim_t sim(
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
    printf("Number of processors: %d\n", sim.nprocs());

    // Extract the PC from the state of the processor
    processor_t *proc = sim.get_core(size_t(0));   
    reg_t reg = proc->get_state()->pc;
    printf("PC register value: %ld\n", reg);

    // Print the content of the 32 general registers
    for (int i = 0; i < 32; i++) {
        printf("XPR%d register value: %ld\n", i, proc->get_state()->XPR[i]);
    }

    // Store one byte
    const uint8_t stored_byte = 0x11;
    proc->get_mmu()->store_uint8(0x1000, stored_byte);

    // Load one byte
    uint8_t loaded_byte = proc->get_mmu()->load_uint8(0x1000);
    printf("Loaded bytes: %d\n", loaded_byte);

    // Load values in x0 and x1
    proc->get_state()->XPR.write(size_t(14), 17);
    proc->get_state()->XPR.write(size_t(15), 42);

    // add a0,a0,a1 -> 952e
    // uint16_t instr_add = 0x97ba;
    uint16_t instr_mov = 0x3e85;

    proc->get_mmu()->store_uint16(0x1000, instr_mov);
    proc->get_mmu()->flush_icache();

    auto i = proc->get_mmu()->load_insn(reg);
    printf("%ud\n", i);
    
    while(proc->get_state()->pc == 0x1000){
        printf("hello\n");
        proc->step(size_t(1));
    }

    // Print the content of the 32 general registers
    for (int i = 0; i < 32; i++) {
        printf("XPR%d register value: %ld\n", i, proc->get_state()->XPR[i]);
    }
    reg = proc->get_state()->pc;
    printf("PC register value: %ld\n", reg);

    // Store instructions
    // Flush instruction cache?

    return 0;
}