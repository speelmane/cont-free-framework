# cont-free-framework

This project contains the necessary setup to run 3-phase tasks on RP2040.

In order to use the framework, some general setup requirements must be filled:
- [Pico SDK](https://github.com/raspberrypi/pico-sdk) must be cloned and set up locally.
- Since no isolated libraries are created within this framework, the SDK is extensively used. However, some files, listed in **sdk-modifications** folder, need modification.  The files that must be modified directly are as follows:
  - PICO-SDK-PATH/src/rp2_common/pico_standard_link/crt0.S must be fully replaced with the source file sdk-modifications/crt0.S.
  - PICO-SDK-PATH/src/rp2_common/pico_multicore/multicore.c must be fully replaced with the source file sdk-modifications/multicore.c.
  - PICO-SDK-PATH/src/rp2_common/pico_multicore/include/pico/multicore.h must be fully replaced with the source file sdk-modifications/multicore.h.
