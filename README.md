## spike-lib: Spike as a shared library

Spike-Lib is an attempt to generate an API from [Spike](https://github.com/riscv/riscv-isa-sim) as a shared library. The main objective is to present this API to the [Pharo](https://github.com/pharo-project/pharo) environment  to port its VM to RISC-V. This simulator is needed to make the VM comply to test harnesses already deployed for other ISAs using [Unicorn](https://github.com/unicorn-engine/unicorn).

### Installation

**Git cloning and initialization:**

```bash
$ git clone https://github.com/QDucasse/spike-lib
$ cd spike-lib
$ git submodule update --init --recursive
```

**Building with CMake:**

```bash
$ cd ..
$ mkdir build
$ cmake -S spike-lib -B build
$ cmake --build build --target all
```

**Executable and Tests:**

```bash
$ ./build/spikelib-ex     # for the executable (experiments in spikelib.cpp >> main)
$ ./build/spikelib-tests  # for the tests      (unit tests  in tests.cpp >> all tests called in main)
```

---

### FFI Functions

The library defines several functions that can later be called from another language FFI API. First, `spike-lib` uses a fork of Spike that redefines the memory API so that a simulator can be initialized with given memory regions. This is defined in the `memory_region` structure:

```c
typedef struct {
    uint64_t base;
    uint64_t size;
    void* content;
} memory_region;
```

All functions that are *exposed* need to be marked as `EXPORT` before their definition and added in the `extern "C" { ... }` list. Moreover, only basic types can be used (`int`,`void`, `char`, ...) and this is why pointers to any specific structure are defined as `void*` and need to be casted at the beginning of each function in the C++ side.

**Simulation Initialization:** 

- **`void* initialize_sim_with_isa(memory_region* memories, int region_numbers)`**  initializes a simulator with given memory regions and the extensions for RISC-V. By default the ISA is encoded as `DEFAULT_ISA` in Spike and corresponds to extensions `IMAFDC`. The default behavior is embedded in the **`void * initialize_sim(memory_region* memories, int region_numbers, const char* isa)`**. 
- **`void release_sim(void* sim)`** frees the memory from the simulator. Important note that the memories should be freed by the user separately (if initialized in the host language for example).

**Register Access:**

- **`int read_register(void* sim, int regid, void* value)` ** reads the contents of a given register (X0-X31, PC or F0-F31) and writes the value to the given buffer. 
- **`int write_register(void* sim, int regid, void* value)` ** writes the contents of value to the given register.

**Memory Access:**

- **`int read_memory(void* sim, uint64_t address, uint64_t size, void* value)`** reads memory starting at the address and for a given size and stores the result in the buffer.
- **`int write_memory(void* sim, uint64_t address, uint64_t size, void* value)`** writes the buffer to the given address in memory.

**Simulation Runtime:**

- **`int spike_start(void* sim, uint64_t begin_address, uint64_t end_address, uint64_t timeout, size_t max_instruction_number)`** is the main simulation function. It starts the simulation by looping on the underlying `step` function of the debugger and stops when one of the conditions is reached:
  - `end_address` is reached, this means that the PC is equal to this address (but the instruction stored at this address will not be executed!).
  - `timeout` is reached, this is measured through the `gettimeofday` function (setting 0 means this condition will not be taken in consideration).
  - `max_instruction_number` is reached, counting the instructions executed (setting 0 means this condition will not be taken in consideration).
  - any memory error will stop execution and return the corresponding code (e.g. invalid instruction, misaligned access, ...).

**Error Codes:**

- **`const char* sp_strerror(int code)`** transforms the error code (`int` from an `enum`) to a string with the reason.
