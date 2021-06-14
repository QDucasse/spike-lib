#include "riscv-isa-sim/riscv/processor.h"

int main() {
    processor_t* proc = new processor_t("rv32g", "m", "", NULL, 17, false, stderr);
    return 0;
}