# Arduino SSD2119 Port

This folder contains the Arduino-side port of the SSD2119 LCD driver. It is separate from the STM32CubeIDE HAL reference project in the repository root.

## Contents

```text
Arduino/
  libraries/
    OpenPLC_SSD2119/
      library.properties
      src/
        OpenPLC_SSD2119.h
        OpenPLC_SSD2119.cpp
        OpenPLC_LCD.h
        OpenPLC_LCD.cpp
      examples/
        NUCLEO_H563ZI_SSD2119_Test/
  SSD2119_Arduino_Test/
    SSD2119_Arduino_Test.ino
```

## Dependencies

Install these in Arduino IDE / Arduino CLI:

```text
Adafruit GFX Library
STM32 Arduino core with NUCLEO-H563ZI / STM32H563ZI support
```

## Compile Idea

When `arduino-cli` and the STM32 board package are installed, compile with a command similar to:

```powershell
arduino-cli compile --fqbn STMicroelectronics:stm32:<your_h563_board_fqbn> --libraries Arduino/libraries Arduino/SSD2119_Arduino_Test
```

Replace `<your_h563_board_fqbn>` with the exact FQBN reported by:

```powershell
arduino-cli board listall H563
arduino-cli board listall Nucleo
```

## NUCLEO-H563ZI LCD Pinout

```text
PB5  = SPI MOSI
PA5  = SPI SCK
PG9  = SPI MISO, not used by the write-only LCD path
PF3  = LCD RST
PD15 = LCD DC
PD14 = LCD CS
```

## Notes

- `OpenPLC_SSD2119` inherits `Adafruit_GFX` and uses Arduino `SPI`.
- `OpenPLC_LCD_Init()` and `OpenPLC_LCD_Task()` are provided for OpenPLC Baremetal template integration.
- The STM32CubeIDE project remains the HAL reference implementation and was build-tested after this Arduino port was added.

## Fix Arduino IDE: `OpenPLC_SSD2119.h: No such file or directory`

Arduino IDE does not automatically search this repository's `Arduino/libraries` folder. Install the library before compiling:

### Option A: Add ZIP Library

Use Arduino IDE:

```text
Sketch > Include Library > Add .ZIP Library...
```

Select:

```text
Arduino/OpenPLC_SSD2119.zip
```

### Option B: Manual Install

Copy this folder:

```text
Arduino/libraries/OpenPLC_SSD2119
```

Into your Arduino sketchbook libraries folder, usually:

```text
Documents/Arduino/libraries/OpenPLC_SSD2119
```

Then restart Arduino IDE.

Also install `Adafruit GFX Library`, because the sketch includes:

```cpp
#include <Adafruit_GFX.h>
#include <Fonts/FreeSerif9pt7b.h>
```
