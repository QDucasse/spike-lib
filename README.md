### Spike as a shared library

Spike-Lib is an attempt to generate an API from [Spike](https://github.com/riscv/riscv-isa-sim) as a shared library. The main objective is to present this API to the [Pharo](https://github.com/pharo-project/pharo) environment  to port its VM to RISC-V. This simulator is needed to make the VM comply to test harnesses already deployed for other ISAs using [Unicorn](https://github.com/unicorn-engine/unicorn).



#### Installation

```bash
$ git clone https://github.com/QDucasse/spike-lib
$ cd spike-lib
$ git submodule update --init --recursive
```



