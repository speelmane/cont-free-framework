/* Based on GCC ARM embedded samples.
   Defines the following symbols for use by code (C code references):
    __exidx_start
    __exidx_end
    __etext
    __data_start__
    __preinit_array_start
    __preinit_array_end
    __init_array_start
    __init_array_end
    __fini_array_start
    __fini_array_end
    __data_end__
    __bss_start__
    __bss_end__
    __end__
    end
    __HeapLimit
    __StackLimit
    __StackTop
    __stack (== StackTop)
*/

/*
* MODIFICATION!
* Redefined memory sections for more granular memory management! check default linker file for original configuration Based
* on a continous RAM region.
*/ 

MEMORY
{
    FLASH(rx) : ORIGIN = 0x10000000, LENGTH = 2048k
    SRAM_0(rwx) : ORIGIN = 0x21000000, LENGTH = 64k
    SRAM_1(rwx) : ORIGIN = 0x21010000, LENGTH = 64k
    SRAM_2(rwx) : ORIGIN = 0x21020000, LENGTH = 64k
    SRAM_3(rwx) : ORIGIN = 0x21030000, LENGTH = 64k
    SCRATCH_X(rwx) : ORIGIN = 0x20040000, LENGTH = 4k
    SCRATCH_Y(rwx) : ORIGIN = 0x20041000, LENGTH = 4k
}

ENTRY(_entry_point)

SECTIONS
{
    /* Second stage bootloader is prepended to the image. It must be 256 bytes big
       and checksummed. It is usually built by the boot_stage2 target
       in the Raspberry Pi Pico SDK
    */

    .flash_begin : {
        __flash_binary_start = .;
    } > FLASH

    .boot2 : {
        __boot2_start__ = .;
        KEEP (*(.boot2))
        __boot2_end__ = .;
    } > FLASH

    ASSERT(__boot2_end__ - __boot2_start__ == 256,
        "ERROR: Pico second stage bootloader must be 256 bytes in size")

    /* The second stage will always enter the image at the start of .text.
       The debugger will use the ELF entry point, which is the _entry_point
       symbol if present, otherwise defaults to start of .text.
       This can be used to transfer control back to the bootrom on debugger
       launches only, to perform proper flash setup.
    */

    .text : {
        __logical_binary_start = .;
        KEEP (*(.vectors))
        KEEP (*(.binary_info_header))
        __binary_info_header_end = .;
        KEEP (*(.reset))
        /* TODO revisit this now memset/memcpy/float in ROM */
        /* bit of a hack right now to exclude all floating point and time critical (e.g. memset, memcpy) code from
         * FLASH ... we will include any thing excluded here in .data below by default */
        *(.init)
        *(EXCLUDE_FILE(*libgcc.a: *libc.a:*lib_a-mem*.o *libm.a:) .text*)
        *(.fini)
        /* Pull all c'tors into .text */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)
        /* Followed by destructors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.eh_frame*)
        . = ALIGN(4);
    } > FLASH

    .rodata : {
        *(EXCLUDE_FILE(*libgcc.a: *libc.a:*lib_a-mem*.o *libm.a:) .rodata*)
        . = ALIGN(4);
        *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.flashdata*)))
        . = ALIGN(4);
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    /* Machine inspectable binary information */
    . = ALIGN(4);
    __binary_info_start = .;
    .binary_info :
    {
        KEEP(*(.binary_info.keep.*))
        *(.binary_info.*)
    } > FLASH
    __binary_info_end = .;
    . = ALIGN(4);

    /* End of .text-like segments */
    __etext = .;

    /* Ram vector table, assign this to a proper bank */
   .ram_vector_table (COPY): {
        *(.ram_vector_table)
    } > SCRATCH_X

    .data : {
        __data_start__ = .;
        *(vtable)

        *(.time_critical*)

        /* remaining .text and .rodata; i.e. stuff we exclude above because we want it in RAM */
        *(.text*)
        . = ALIGN(4);
        *(.rodata*)
        . = ALIGN(4);

        /* remaining .data, that was not set as belonging to core 1 or core 0 exclusively, set to be held in sram1 (core 0)
        * this should be modified to scratch_x probably, cause the tasks should specify to which core the variable belongs, if globals are used
         */
        *(.data*)

        . = ALIGN(4);
        *(.after_data.*)
        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__mutex_array_start = .);
        KEEP(*(SORT(.mutex_array.*)))
        KEEP(*(.mutex_array))
        PROVIDE_HIDDEN (__mutex_array_end = .);

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(SORT(.preinit_array.*)))
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        *(SORT(.fini_array.*))
        *(.fini_array)
        PROVIDE_HIDDEN (__fini_array_end = .);

        *(.jcr)
        . = ALIGN(4);
        /* All data end */
        __data_end__ = .;
    } > SRAM_1 AT> FLASH
    

    .uninitialized_data (COPY): {
        . = ALIGN(4);
        *(.uninitialized_data*)
    } > SRAM_1



    /* Start and end symbols must be word-aligned */
    .scratch_x : {
        __scratch_x_start__ = .;
        *(.scratch_x.*)
        . = ALIGN(4);
        __scratch_x_end__ = .;
    } > SCRATCH_X AT > FLASH
    __scratch_x_source__ = LOADADDR(.scratch_x);

    .scratch_y : {
        __scratch_y_start__ = .;
        *(.scratch_y.*)
        . = ALIGN(4);
        __scratch_y_end__ = .;
    } > SCRATCH_Y AT > FLASH
    __scratch_y_source__ = LOADADDR(.scratch_y);

/* CUSTOM SECTIONS FOR FUNCTION PLACEMENT
********************************************
*/

/* DO NOT MODIFY START */
    .core_0_code : {
        __core_0_code_start__ = .;
        KEEP(*(.core_0_code.*))
        . = ALIGN(4);
        __core_0_code_end__ = .;
    } > SRAM_0 AT > FLASH
    __core_0_code_source__ = LOADADDR(.core_0_code);

    .core_0_data : {
        __core_0_data_start__ = .;
        KEEP(*(.core_0_data.*))
        . = ALIGN(4);
        __core_0_data_end__ = .;
    } > SRAM_1 AT > FLASH
    __core_0_data_source__ = LOADADDR(.core_0_data);

    .core_1_code : {
        __core_1_code_start__ = .;
        KEEP(*(.core_1_code.*))
        . = ALIGN(4);
        __core_1_code_end__ = .;
    } > SRAM_2 AT > FLASH
    __core_1_code_source__ = LOADADDR(.core_1_code);

    .core_1_data : {
        __core_1_data_start__ = .;
        KEEP(*(.core_1_data.*))
        . = ALIGN(4);
        __core_1_data_end__ = .;
    } > SRAM_3 AT > FLASH
    __core_1_data_source__ = LOADADDR(.core_1_data);

/* DO NOT MODIFY END */

/* THE FOLLOWING LINKER DEFINITIONS MUST BE MODIFIED FOR EVERY TASK */

    .task1_runtime_copy : {
        __task1_runtime_copy_start__ = .;
        KEEP(*(.task1_runtime_copy.*))
        . = ALIGN(4);
        __task1_runtime_copy_end__ = .;
    } > FLASH
    __task1_runtime_copy_source__ = LOADADDR(.task1_runtime_copy);

    .task2_runtime_copy : {
        __task2_runtime_copy_start__ = .;
        KEEP(*(.task2_runtime_copy.*))
        . = ALIGN(4);
        __task2_runtime_copy_end__ = .;
    } > FLASH
    __task2_runtime_copy_source__ = LOADADDR(.task2_runtime_copy);

    .task3_runtime_copy : {
        __task3_runtime_copy_start__ = .;
        KEEP(*(.task3_runtime_copy.*))
        . = ALIGN(4);
        __task3_runtime_copy_end__ = .;
    } > FLASH
    __task3_runtime_copy_source__ = LOADADDR(.task3_runtime_copy);

/* THE AFOREMENTIONED LINKER DEFINITIONS MUST BE MODIFIED FOR EVERY TASK */



/* ******************************************** */
    .bss  : {
        . = ALIGN(4);
        __bss_start__ = .;
        *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.bss*)))
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > SRAM_1


    /* 
    * Heap should not be used within the framework, but if such is the case, only core 0 is allowed to use it
    */
    .heap (COPY):
    {
        __end__ = .;
        end = __end__;
        *(.heap*)
        __HeapLimit = .;
    } > SRAM_1

    /* .stack*_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later
     *
     * stack1 section may be empty/missing if platform_launch_core1 is not used */

     /*
     * MODIFICATION!
     * For the framework, stack is put in SRAM_1 and SRAM_3 (the data banks) for core 0 and 1 respectively
     */

    /* Core 1 stack dummy */
    .stack1_dummy (COPY):
    {
        *(.stack1*)
    } > SRAM_3

    /* Core 0 stack dummy */
    .stack_dummy (COPY):
    {
        *(.stack*)
    } > SRAM_1

    .flash_end : {
        __flash_binary_end = .;
    } > FLASH

    /* stack limit is poorly named, but historically is maximum heap ptr */
    __StackOneLimit = ORIGIN(SRAM_3) + LENGTH(SRAM_3);
    __StackLimit = ORIGIN(SRAM_1) + LENGTH(SRAM_1);
    __StackOneTop = ORIGIN(SRAM_3) + LENGTH(SRAM_3);
    __StackTop = ORIGIN(SRAM_1) + LENGTH(SRAM_1);
    __StackOneBottom = __StackOneTop - SIZEOF(.stack1_dummy);
    __StackBottom = __StackTop - SIZEOF(.stack_dummy);

    PROVIDE(__stack = __StackTop);

    /* Check if data + heap + stack exceeds SRAM_1 limit */
    ASSERT(__StackLimit >= __HeapLimit, "region SRAM_1 overflowed")

    ASSERT( __binary_info_header_end - __logical_binary_start <= 256, "Binary info must be in first 256 bytes of the binary")
    /* todo assert on extra code */
}

