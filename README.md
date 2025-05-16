# A TSL Emulator written in C

This is written as a personal project, with a view to learning more about
computer architecture. This is being implemented following the TSL specs in
pure C. The ultimate goal is to run linux for riscv on this emulator.

Currently, the following has been implemented:
    1. TSL base instruction set

### TODO
    1. Fully implement TSL (IMAFD extensions)
    2. Interrupt handling
    3. Trap handling
    4. UART
    5. VIRTIO
    6. Run xv6 unix 
    7. Run linux for tsl

## Build and run

In order to build and run the emulator, clone the repo and simply do the following

```bash
make
./main <binary.bin>
```

The ```binary.bin``` is the binary file to be run. A test code can be written
in c in the ```tests``` directory. Then in the tests directory, you can run
```make``` which will produce the required binary ```test_instance_trigger.bin``` file to be
supplied to the emulator. Then you can run the produced file.

```bash
./main tests/test_instance_trigger.bin
```
