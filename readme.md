# TinyEMU on ESP32 (and other systems, embedded or otherwise)

This project is based on Fabrice Bellard's [TinyEmu](https://bellard.org/tinyemu/) RISCV emulator ported to Windows and ESP32. 

It uses virtual memory based on files to provide the emulator with desired RAM.

* RISC-V system emulator supporting the RV128IMAFDQC base ISA (user level ISA version 2.2, priviledged architecture version 1.10) including:
    * 32/64/128 bit integer registers
    * 32/64/128 bit floating point instructions (using the SoftFP Library)
    * Compressed instructions
    * Dynamic XLEN change
* VirtIO console, network, block device, input and ~~9P filesystem~~ (inactive at this time)

# ESP32

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/f3a3xeTRj_A/0.jpg)](https://www.youtube.com/watch?v=f3a3xeTRj_A)


* it takes about 5 seconds to start the kernel
* 1:35 more to get to the init
* 3:20 more to run init and get to bash
* 24 more seconds to finish ```ls -l``` in the root

Currently there's no support for networking but it should be pretty easy to implement, though I'm not sure how functional its going to be given the above stellar execution time.

Compile and Upload to ESP32
```
pio run -e esp32 -t upload
```

## Basic Configuration

The arguments for running temu on ESP32 are hardcoded to ```root-riscv32.cfg``` on sdcard under ```emu``` subfolder, you should put your bbl, kernel and rootfs in the same folder and configure it:
```
{
    version: 1,
    machine: "riscv32",
    memory_size: 128,
    bios: "bbl32.bin",
    kernel: "kernel32.bin",
    cmdline: "console=hvc0 debug ignore_loglevel earlycon=sbi root=/dev/vda rw",
    drive0: { file: "rootfs32.bin" }
}
```

# How to build your own linux
Please see [buildroot-tinyemu](https://github.com/drorgl/buildroot-tinyemu)

# Changes in layered_cache branch

New caching scheme where there are 4 layers
- file system for storing the page file
- himem for storing cache pages before they go to page file - slow read and write since it needs the mmu to change the pages mapped.
- psram for storing smaller pages and act as a cache for the faster internal memory
- internal ram for quicker cache

Unfortunately no improvements, but this branch is kept for educational purposes

# References
* [TinyEMU](https://bellard.org/tinyemu/)
* [TinyEMU Technical Documentation](https://bellard.org/tinyemu/readme.txt)

