#include "riscv-isa-sim/riscv/processor.h"
#include "riscv-isa-sim/riscv/devices.h"
#include "riscv-isa-sim/fesvr/memif.h"
#include "riscv-isa-sim/riscv/sim.h"
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
    hartids.push_back(0);  
    hartids.push_back(1);
    return hartids;
}

int main() {

    std::vector<std::pair<reg_t, abstract_device_t*>> plugin_devices;
    const std::vector<std::string> args;

    debug_module_config_t dm_config = {
        .progbufsize = 2,
        .max_bus_master_bits = 0,
        .require_authentication = false,
        .abstract_rti = 0,
        .support_hasel = true,
        .support_abstract_csr_access = true,
        .support_haltgroups = true,
        .support_impebreak = true
    };

    // processor_t* proc = new processor_t("rv32g", "m", "", NULL, 17, false, stderr);
    
    sim_t* sim = new sim_t(
            DEFAULT_ISA,    // RISC-V ISA string          (rv32 or rv64 with extensions, g = imafd)  DEFAULT = IMAFDC
            DEFAULT_PRIV,   // RISC-V privilege modes     (m, mu, msu)                               DEFAULT = msu
            DEFAULT_VARCH,  // RISC-V Vector uArch string (vlen, elen and slen)                      DEFAULT = vlen:128,elen:64,slen:128
            1,              // Number of processors
            true,           // Halted 
            false,          // Core local interrupt
            0,              // Load kernel initrd_start into memory
            0,              // Load kernel initrd_end   into memory
            NULL,           // Kernel bootstrap arguments
            -1,             // Override ELF entry point
            initialize_mems(0x1000, 4096), // Initialize memories with given base and size
            plugin_devices, // Memory Map Input devices
            args,
            initialize_hartids(),
            dm_config,
            nullptr,
            true,
            nullptr
        );

    return 0;
}