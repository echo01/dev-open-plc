# STM32H563ZI_NUCLEO_AZ2 TBS

The default IDE is set to STM32CubeIDE, to change IDE open the STM32H563ZI_NUCLEO_AZ2.ioc with STM32CubeMX and select from the supported IDEs (EWARM, MDK-ARM and STM32CubeIDE). Supports flashing of the STM32H563ZI-NUCLEO board directly from TouchGFX Designer using GCC and STM32CubeProgrammer. Flashing the board requires STM32CubeProgrammer which can be downloaded from the ST webpage.

This TBS is configured for 240 x 320 pixels 16 bpp screen resolution.

Performance testing can be done using the GPIO pins:

- VSYNC_FREQ  - PG12 (CN10 - pin 2)
- RENDER_TIME - PE14 (CN10 - pin 8)
- FRAME_RATE  - PG14 (CN10 - pin 12)
- MCU_ACTIVE  - PE12 (CN10 - pin 26)

The USER button (B1 on STM32H563ZI-NUCLEO board, which is connected to PC13 on the MCU) is implemented in the TBS and is mapped to the button key 48 (ASCII '0').
The JOY_STICK (B1 on display board) is mapped to the following button key values in the TBS:
    CENTER: 53 (ASCII '5')
    LEFT:   52 (ASCII '4')
    RIGHT:  54 (ASCII '6')
    DOWN:   50 (ASCII '2')
    UP:     56 (ASCII '8')
When running the Simulator in TouchGFX Designer, these buttons can be simulated by pressing the corresponding key on the PC keyboard keypad.