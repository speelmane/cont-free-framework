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
A modifiable 3-phase task template is found under src/tasktemplate.c, having additional commentary on task setup.

Due to compiler and system limitations, there are several restrictions for tasks that must be followed:
- No global or static variables can be used, unless the access core is known beforehand. If the global variable is used, the corresponding function attribute (´__core_0_data´ or ´__core_1_data´) must be used for the variable.
- Due to compiler limitations, multiple operations such as
  - float and double operations;
  - division and modulo operations;
  - 64-bit multiplication
  
  are not permitted.
- Recursive execution functions cannot be used.


The framework is deeply tied with the linker script and custom linker script sections for correct data/code placement in memory.
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

## Scheduling

The setup uses static scheduling, which means that the schedule is set up beforehand under src/schedules.c. The schedule is set from tasks to be run in a single hyperperiod.
Two structures can be found:
- `task_array` (e.g. `task_array[TABLE_SIZE_CORE_0])(subschedule_t) = {task1, task4, task2, task1, task4, task4, task2, task1}`)
- `scheduled_wait_time_array`(e.g. `uint64_t scheduled_wait_time_array[TABLE_SIZE_CORE_0] = {550, 200, 50, 100, 200, 300, 500, 100};`)

The `task_array` notes down the sequence of tasks to be run within one hyperperiod. The first value of `scheduled_wait_time_array` (e.g. 550) refers to the relative waiting time between `task_array`'s last task release and first task release. The rest of the numbers (starting from the second value (e.g. 200)) refer to the relative waiting time between the tasks in the task sequence. Thus, `scheduled_wait_time_array[n] = timeof(task_array[n]) - timeof(task_array[n-1])`, where n != 0. The size of each array must be fixed and provided as a definition in include/schedule.h. Additionally, include/schedule.h definitions `START_OFFSET_CORE_0` and `START_OFFSET_CORE_1` must be modified to set the release time of each core.
If dynamic scheduling must be used, the algorithms can be performed within each core's  `main` function in main.c.

## Build directives
Once the task is created, its file reference must be added to CMakeLists.txt for compilation, If any of the tasks/files is not used, they should be removed from the compilation list.
Build the framework with CMake and run `make` in the build folder. If the created tasks are valid, the .dis file in the build directory, containing disassembled task definitions, will allow bl or jmp only to locations within the task space. If the corresponding task's jumps and branches are to `aeabi` functions or any other locations outside the task's code space, the task execution will not be successful.
