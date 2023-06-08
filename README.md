# cont-free-framework

This project contains a framework, specifically designed to run 3-phase tasks on RP2040.

## Requirements

In order to use the framework, some general setup requirements must be filled:
- [Pico SDK](https://github.com/raspberrypi/pico-sdk) must be cloned and set up locally.
- Since no isolated libraries are created within this framework, the SDK is extensively used. However, some files, listed in **sdk-modifications** folder, need modification.  The files that must be modified directly are as follows:
  - PICO-SDK-PATH/src/rp2_common/pico_standard_link/crt0.S must be fully replaced with the source file sdk-modifications/crt0.S.
  - PICO-SDK-PATH/src/rp2_common/pico_multicore/multicore.c must be fully replaced with the source file sdk-modifications/multicore.c.
  - PICO-SDK-PATH/src/rp2_common/pico_multicore/include/pico/multicore.h must be fully replaced with the source file sdk-modifications/multicore.h.
- The toolchain and other necessities needed for code development must be installed as specified in the [RP2040 Getting started guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).

## 3-phase task creation
An executable 3-phase task 

Due to compiler and system limitations, there are several restrictions for tasks that must be followed:
- No global or static variables can be used, unless the access core is known beforehand. If the global variable is used, the corresponding function attribute (´__core_0_data´ or ´__core_1_data´) must be used for the variable.
- Due to compiler limitations, multiple operations such as
  - float and double operations;
  - division and modulo operations;
  - 64-bit multiplication
  
  are not permitted.
- Recursive execution functions cannot be used.


The framework is deeply tied with the linker scirpt and custom linker script sections for correct data/code placement in memroy.
Four usable sections are already defined (task1, task2, task3, task4), and each can be used for a UNIQUE task only.
If the total number of tasks to be run is <= 4, the task1, task2, task3 and task4 examples can be taken as templates accordingly,
since the tasks use already defined linker script sections.
If the number of tasks to be run is > 4, additional custom linker sections must be defined.
The process for defining new linker script sections is as follows:    
1. Open "linker_script.ld" file
2. Find line 261, which starts to define custom linker script sections
3. In a similar manner as the sections on line 261 and onward, add a section for your new task, replacing all "XXX" with a new number, e.g.,
   ```
   .taskXXX_runtime_copy : {
        __taskXXX_runtime_copy_start__ = .;
        KEEP(*(.taskXXX_runtime_copy.*))
        . = ALIGN(4);
        __taskXXX_runtime_copy_end__ = .;
    } > FLASH
    __taskXXX_runtime_copy_source__ = LOADADDR(.taskXXX_runtime_copy);
    ```

    becomes
    ```
    .task5_runtime_copy : {
        __task5_runtime_copy_start__ = .;
        KEEP(*(.task5_runtime_copy.*))
        . = ALIGN(4);
        __task5_runtime_copy_end__ = .;
    } > FLASH
    __task5_runtime_copy_source__ = LOADADDR(.task5_runtime_copy);
    ```

Use the template for tasks creation. Runtime section - defined in the linker script, must be unique per task, line ...


## Scheduling

This setup uses static, which means that the schedule is set up beforehand under schedules.c. Modify schedule.h to set start times.

## Build directives
Build the framework with CMake. then run make in the build folder. If the created tasks are valid, the .dis file in the build dir, containing task sections contains bl or jmp only to locations within the task space.
