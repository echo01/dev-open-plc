# LCD SSD2119 + Font Integration for OpenPLC IDE Template

เอกสารนี้สรุปวิธีนำ driver `SSD2119`, Adafruit GFX font compatibility และ `OpenPLC_LCD_Task()` เข้าไปอยู่ใน Template ของ OpenPLC IDE เพื่อให้ทุกครั้งที่สร้าง project ใหม่หรือ compile/export ใหม่ ไฟล์ `Baremetal.ino` และ library ที่จำเป็นถูก generate ติดมาด้วยเสมอ

## 1. เป้าหมาย

ต้องการให้ OpenPLC IDE generate firmware ที่มี LCD support อัตโนมัติ โดยไม่ต้องแก้ไฟล์ใน `build/.../Baremetal.ino` หลัง compile ทุกครั้ง

สิ่งที่ต้องติดมากับ generated project:

```text
Baremetal.ino
openplc_lcd.h
openplc_lcd.c หรือ openplc_lcd.cpp
ssd2119.h
ssd2119.c
Adafruit_GFX.h
Fonts/FreeSerif9pt7b.h
Fonts/FreeSerif12pt7b.h
Fonts/FreeSerif18pt7b.h
Fonts/FreeSerif24pt7b.h
```

## 2. ห้ามแก้ไฟล์ Generated โดยตรง

ไฟล์นี้เป็น output จาก OpenPLC compiler/exporter:

```text
<project>/build/<board>/examples/Baremetal/Baremetal.ino
```

ถ้าแก้ไฟล์นี้โดยตรง เมื่อ compile ใหม่ OpenPLC IDE อาจ generate ทับและ code LCD จะหาย

ดังนั้นต้องแก้ที่ Template ต้นทางของ OpenPLC IDE หรือทำ post-generate patch script หลัง compile

## 3. จุดที่ต้องแก้ใน Baremetal.ino Template

จาก generated `Baremetal.ino` ปัจจุบัน lifecycle เป็นแบบนี้:

```cpp
void setup()
{
    runtime_bind_located_vars();
    runtime_discover_tasks();
    hardwareInit();
    setupCycleDelay(base_tick_ns);
}

void scheduler()
{
    runtime_plc_cycle();

    #ifdef USE_ARDUINO_SKETCH
        sketch_loop();
    #endif

    #ifdef MODBUS_ENABLED
        modbusTask();
    #endif
}

void loop()
{
    if ((micros() - last_run) >= scan_cycle)
    {
        scheduler();
        last_run += scan_cycle;
    }
}
```

ให้แก้ Template ดังนี้

### 3.1 เพิ่ม include

เพิ่มหลัง include OpenPLC runtime:

```cpp
#include "openplc_lcd.h"
```

ตัวอย่าง:

```cpp
#include "openplc.h"
#include "defines.h"
#include "arduino_runtime_glue.h"
#include "openplc_lcd.h"
```

### 3.2 เพิ่ม LCD init ใน setup()

ให้เรียกหลัง `hardwareInit()` เพราะ hardware layer ต้อง setup GPIO/SPI ก่อน

```cpp
// Initialize hardware (HAL -- unchanged)
hardwareInit();
OpenPLC_LCD_Init();
```

### 3.3 เพิ่ม LCD task ใน loop()

แนะนำใส่ใน `loop()` หลัง scheduler block ไม่ควรใส่ใน `runtime_plc_cycle()` และไม่ควรใส่ใน `updateInputBuffers()` หรือ `updateOutputBuffers()`

```cpp
void loop()
{
    if ((micros() - last_run) >= scan_cycle)
    {
        scheduler();
        last_run += scan_cycle;
    }

    OpenPLC_LCD_Task();

    #ifdef MODBUS_ENABLED
    if ((micros() - last_run) >= 10000)
    {
        modbusTask();
    }
    #endif

    #ifdef SIMULATOR_MODE
    __asm volatile("sleep");
    #endif
}
```

เหตุผล:

- `runtime_plc_cycle()` เป็น PLC scan path โดยตรง ต้องเร็วและ deterministic
- LCD SPI และ font rendering ช้ากว่า PLC scan มาก
- ให้ `OpenPLC_LCD_Task()` throttle/dirty-check เอง เช่น update ทุก 100-250 ms

## 4. โครงสร้าง Library ที่ควรเพิ่มใน Template

แนะนำเพิ่มเป็น library/module ภายใต้ generated Baremetal example หรือ OpenPLCUserLib source template

ตัวอย่างโครงสร้าง:

```text
examples/Baremetal/
  Baremetal.ino
  openplc_lcd.h
  openplc_lcd.c
  lcd/
    ssd2119.h
    ssd2119.c
    Adafruit_GFX.h
    Fonts/
      FreeSerif9pt7b.h
      FreeSerif12pt7b.h
      FreeSerif18pt7b.h
      FreeSerif24pt7b.h
```

หรือถ้า OpenPLC IDE มี global runtime/library template ให้ใส่ไว้ใน template ต้นทาง เช่น:

```text
runtime/arduino/examples/Baremetal/
runtime/arduino/libraries/OpenPLCUserLib/src/
runtime/arduino/libraries/OpenPLCUserLib/src/lcd/
```

ชื่อ path จริงขึ้นกับ source tree ของ OpenPLC IDE ที่ใช้ ให้ค้นหา template ด้วยคำว่า:

```text
Baremetal.ino
arduino_runtime_glue.h
OpenPLCUserLib
examples/Baremetal
```

## 5. openplc_lcd.h

ตัวอย่าง interface ที่ควรใส่ใน template:

```c
#ifndef OPENPLC_LCD_H
#define OPENPLC_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

void OpenPLC_LCD_Init(void);
void OpenPLC_LCD_Task(void);
void OpenPLC_LCD_RequestRefresh(void);

#ifdef __cplusplus
}
#endif

#endif /* OPENPLC_LCD_H */
```

## 6. openplc_lcd.c

ตัวอย่าง task แบบปลอดภัยต่อ scan cycle:

```c
#include "openplc_lcd.h"
#include "ssd2119.h"
#include "Fonts/FreeSerif12pt7b.h"
#include "Fonts/FreeSerif18pt7b.h"

extern SPI_HandleTypeDef hspi1;
extern IEC_BOOL *bool_output[MAX_DIGITAL_OUTPUT/8][8];

static uint8_t lcd_ready;
static uint8_t lcd_dirty = 1U;
static uint8_t last_qx00 = 0xFFU;

void OpenPLC_LCD_Init(void)
{
    if (SSD2119_Init(&hspi1) == HAL_OK)
    {
        lcd_ready = 1U;
        lcd_dirty = 1U;
        SSD2119_FillScreen(SSD2119_BLUE);
    }
}

void OpenPLC_LCD_RequestRefresh(void)
{
    lcd_dirty = 1U;
}

void OpenPLC_LCD_Task(void)
{
    static uint32_t last_ms = 0U;
    uint8_t qx00 = 0U;

    if (lcd_ready == 0U)
    {
        return;
    }

    if ((HAL_GetTick() - last_ms) < 100U)
    {
        return;
    }
    last_ms = HAL_GetTick();

    if (bool_output[0][0] != 0)
    {
        qx00 = (*bool_output[0][0] != 0) ? 1U : 0U;
    }

    if ((qx00 == last_qx00) && (lcd_dirty == 0U))
    {
        return;
    }

    last_qx00 = qx00;
    lcd_dirty = 0U;

    SSD2119_SetFont(&FreeSerif18pt7b);
    SSD2119_FillRect(0U, 0U, 320U, 80U, SSD2119_BLUE);
    SSD2119_Print(10U, 12U, "OpenPLC", SSD2119_WHITE, SSD2119_BLUE, 1U);

    SSD2119_SetFont(&FreeSerif12pt7b);
    SSD2119_FillRect(0U, 90U, 320U, 60U, qx00 ? SSD2119_GREEN : SSD2119_RED);
    SSD2119_Print(10U, 100U, qx00 ? "QX0.0: ON" : "QX0.0: OFF", SSD2119_WHITE,
                  qx00 ? SSD2119_GREEN : SSD2119_RED, 1U);
}
```

หมายเหตุ: ถ้า build ฝั่ง OpenPLC เป็น C++ ให้ใช้ `openplc_lcd.cpp` ก็ได้ แต่ควรเก็บ API เป็น `extern "C"` เพื่อให้เรียกจาก `Baremetal.ino` ง่าย

## 7. Dependency ที่ต้องมี

ถ้าใช้ HAL driver เดิม ต้องมีสิ่งเหล่านี้ใน firmware target:

```c
#include "stm32h5xx_hal.h"
#include "spi.h"
#include "gpio.h"
```

และต้องมี symbol:

```c
SPI_HandleTypeDef hspi1;
LCD_RST_GPIO_Port / LCD_RST_Pin
LCD_CS_GPIO_Port  / LCD_CS_Pin
LCD_DC_GPIO_Port  / LCD_DC_Pin
```

ถ้า OpenPLC target เป็น Arduino board เช่น Blackpill F411 Arduino core จะไม่มี `MX_SPI1_Init()`, `hspi1`, และ GPIO macro แบบ CubeMX โดยอัตโนมัติ กรณีนั้นมี 2 ทางเลือก:

1. Port `ssd2119.c` ไปใช้ Arduino API: `SPI.transfer()`, `digitalWrite()`, `pinMode()`
2. ใช้ STM32CubeIDE/HAL project เป็น firmware หลัก แล้วนำ OpenPLC generated runtime เข้ามา compile ใน Cube project

สำหรับ NUCLEO-H563ZI + LCD SSD2119 แนะนำทางเลือกที่ 2 เพราะ driver ปัจจุบันเป็น HAL-based อยู่แล้ว

## 8. การเพิ่ม Source เข้า Build System

ต้องให้ OpenPLC template/copy step นำไฟล์เหล่านี้เข้า generated build เสมอ:

```text
openplc_lcd.c
ssd2119.c
```

และ include path ต้องมองเห็น:

```text
.
lcd/
lcd/Fonts/
```

ถ้าเป็น Arduino CLI library layout แนะนำวางใน library `src/` เพื่อให้ถูก compile อัตโนมัติ:

```text
OpenPLCUserLib/src/openplc_lcd.cpp
OpenPLCUserLib/src/lcd/ssd2119.c
OpenPLCUserLib/src/lcd/ssd2119.h
OpenPLCUserLib/src/lcd/Adafruit_GFX.h
OpenPLCUserLib/src/lcd/Fonts/*.h
```

แล้ว include ใน `Baremetal.ino`:

```cpp
#include <openplc_lcd.h>
```

หรือถ้าวางข้าง `Baremetal.ino`:

```cpp
#include "openplc_lcd.h"
```

## 9. จุดที่ไม่ควรใส่ LCD Task

ไม่ควรใส่ใน:

```cpp
runtime_plc_cycle();
updateInputBuffers();
updateOutputBuffers();
all_programs[i]->run();
```

เพราะเป็น path หลักของ PLC scan cycle จะทำให้ scan jitter สูงและอาจทำให้ PLC response ช้า

## 10. Recommended Patch Summary for Baremetal.ino Template

Patch ที่ควรมีใน template:

```diff
 #include "openplc.h"
 #include "defines.h"
 #include "arduino_runtime_glue.h"
+#include "openplc_lcd.h"

 void setup()
 {
     runtime_bind_located_vars();
     runtime_discover_tasks();
     hardwareInit();
+    OpenPLC_LCD_Init();

     setupCycleDelay(base_tick_ns);
 }

 void loop()
 {
     if ((micros() - last_run) >= scan_cycle)
     {
         scheduler();
         last_run += scan_cycle;
     }

+    OpenPLC_LCD_Task();
+
     #ifdef MODBUS_ENABLED
     if ((micros() - last_run) >= 10000)
     {
         modbusTask();
     }
     #endif
 }
```

## 11. Font Selection

ไม่จำเป็นต้อง include font ทั้ง 4 ขนาดทุก project ถ้า flash เริ่มแน่น

แนะนำ:

```text
FreeSerif12pt7b.h  ใช้กับข้อความทั่วไป
FreeSerif18pt7b.h  ใช้กับ status RUN/STOP/FAULT
```

ถ้า include ทั้ง 4 ขนาดใน H563 project ปัจจุบัน build size ประมาณ:

```text
text = 61708 bytes
data = 132 bytes
bss  = 2244 bytes
```

## 12. Development Checklist

1. หา template ต้นทางของ `Baremetal.ino` ใน OpenPLC IDE source tree
2. เพิ่ม include `openplc_lcd.h`
3. เพิ่ม `OpenPLC_LCD_Init()` หลัง `hardwareInit()`
4. เพิ่ม `OpenPLC_LCD_Task()` ใน `loop()` หลัง scheduler block
5. เพิ่มไฟล์ library LCD เข้า template copy list หรือ Arduino library `src/`
6. ตรวจ include path ให้หา `ssd2119.h`, `Adafruit_GFX.h`, `Fonts/*.h` เจอ
7. ตรวจว่า target board มี SPI/GPIO init ตรงกับ SSD2119
8. ให้ `OpenPLC_LCD_Task()` throttle และ redraw เฉพาะ dirty region
9. Compile project ใหม่ แล้วตรวจ generated `Baremetal.ino` ว่ามี LCD hook ติดมาจริง
10. Flash board และตรวจ scan time ถ้า LCD ทำให้ jitter ให้ลด refresh rate หรือแยก draw เป็น state machine

## 13. Recommendation for This Project

สำหรับ NUCLEO-H563ZI + SSD2119 ที่ใช้ HAL driver ปัจจุบัน แนวทางที่เหมาะที่สุดคือ:

```text
OpenPLC IDE generate PLC runtime/code
STM32CubeIDE project เป็น firmware หลัก
LCD driver อยู่ใน Cube/HAL layer
OpenPLC runtime ถูกเรียกใน main loop
OpenPLC_LCD_Task() อยู่หลัง PLC cycle และ throttle เอง
```

ถ้าต้องให้ OpenPLC IDE compile Arduino Baremetal เอง ต้อง port SSD2119 driver จาก HAL SPI ไปเป็น Arduino SPI ก่อน หรือเพิ่ม board template สำหรับ NUCLEO-H563ZI ที่ใช้ HAL/Cube build flow

## 14. แนวทางสรุปสำหรับ OpenPLC Editor 4.2.8 ใน Project นี้

จากการตรวจโครงสร้าง `openplc-editor-development` นี้ จุดที่เกี่ยวข้องกับ Arduino/Baremetal firmware คือ:

```text
resources/sources/Baremetal/Baremetal.ino
resources/sources/arduino/openplc.h
resources/sources/arduino/arduino_runtime_glue.cpp
src/backend/shared/compile/steps/compose-firmware-bundle.ts
src/backend/shared/hardware/board-info-resolver.ts
```

เส้นทาง compile โดยย่อ:

```text
PLC project
+-- xml2st / strucpp generate C++ runtime files
+-- compose-firmware-bundle.ts รวม firmware skeleton
+-- Baremetal.ino + src/generated.* + src/defines.h + HAL
+-- arduino-cli compile
```

ดังนั้นมี 2 วิธีหลักในการเพิ่ม LCD SSD2119 เข้า OpenPLC โดยไม่แก้ไฟล์ generated หลัง compile ทุกครั้ง:

```text
วิธี A: แก้ Template Baremetal.ino โดยตรง
วิธี B: ใส่ LCD library ไว้ใน VPP package ของ board
```

แนะนำให้ใช้วิธี B ในระยะยาว เพราะจะผูก LCD support เฉพาะ board/package ที่ต้องการ ไม่กระทบ board อื่นทั้งหมด

## 15. วิธี A: เพิ่ม OpenPLC_LCD_Task() ใน Template Baremetal

วิธีนี้เหมาะเมื่อเราต้องการให้ OpenPLC firmware ทุกตัวที่ compile จาก editor นี้มี LCD hook ติดไปด้วยเสมอ

ไฟล์ต้นทางที่ต้องแก้:

```text
resources/sources/Baremetal/Baremetal.ino
```

### 15.1 เพิ่ม include

เพิ่มหลัง include runtime หลัก:

```cpp
#include "openplc.h"
#include "defines.h"
#include "arduino_runtime_glue.h"
#include "openplc_lcd.h"
```

### 15.2 เพิ่ม init ใน setup()

ตำแหน่งที่เหมาะคือหลัง `hardwareInit()` เพราะ HAL ของ board ควร setup GPIO/SPI ก่อน

```cpp
void setup()
{
    runtime_bind_located_vars();
    runtime_discover_tasks();

    hardwareInit();
    OpenPLC_LCD_Init();

    setupCycleDelay(base_tick_ns);
}
```

### 15.3 เพิ่ม task ใน loop()

ตำแหน่งที่เหมาะคือใน `loop()` หลัง block scheduler และอยู่นอก `runtime_plc_cycle()`

```cpp
void loop()
{
    if ((micros() - last_run) >= scan_cycle)
    {
        scheduler();
        last_run += scan_cycle;
    }

    OpenPLC_LCD_Task();

    #ifdef MODBUS_ENABLED
    if ((micros() - last_run) >= 10000)
    {
        modbusTask();
    }
    #endif

    #ifdef SIMULATOR_MODE
    __asm volatile("sleep");
    #endif
}
```

เหตุผลที่ไม่ใส่ใน `scheduler()`:

- `scheduler()` เรียก `runtime_plc_cycle()` ซึ่งเป็นเส้นทาง scan cycle ของ Ladder
- ถ้า LCD วาดจอช้า จะทำให้ scan time และ response ของ Ladder แกว่ง
- การวางใน `loop()` นอก scheduler ทำให้ LCD task ถูกเรียกในเวลาว่างระหว่าง scan ได้ง่ายกว่า

### 15.4 ไฟล์ LCD ที่ต้องเพิ่มใน template

ถ้าเลือกวางข้าง `Baremetal.ino`:

```text
resources/sources/Baremetal/openplc_lcd.h
resources/sources/Baremetal/openplc_lcd.cpp
resources/sources/Baremetal/lcd/ssd2119.h
resources/sources/Baremetal/lcd/ssd2119.cpp
resources/sources/Baremetal/lcd/Adafruit_GFX.h
resources/sources/Baremetal/lcd/Fonts/FreeSerif12pt7b.h
resources/sources/Baremetal/lcd/Fonts/FreeSerif18pt7b.h
```

ข้อควรระวัง: `compose-firmware-bundle.ts` จะ copy firmware skeleton ทั้งชุดไปยัง build output ดังนั้นไฟล์ที่อยู่ใต้ `resources/sources/Baremetal/` จะติดไปกับ generated firmware ได้ แต่ต้องตรวจว่า arduino-cli compile ไฟล์ `.cpp` ในตำแหน่งนั้นจริงหรือไม่ ถ้าไม่ compile ให้ย้ายไปเป็น Arduino library layout ตามวิธี B

## 16. วิธี B: เพิ่ม LCD Library ใน VPP Package

วิธีนี้เหมาะกับ board package ของเรา เช่น `com.openplc.stm32-wp` หรือ package ใหม่สำหรับ STM32F411CE + LCD

ข้อดี:

- ไม่กระทบ board อื่น
- package พก LCD driver ไปเอง
- install/remove package แล้วความสามารถ LCD ติดไปกับ board profile
- เหมาะกับการทดสอบบน OpenPLC app จริงผ่าน Package Manager

### 16.1 โครงสร้าง package ที่แนะนำ

ตัวอย่าง:

```text
device package/com.openplc.stm32-f411-lcd
+-- manifest.json
+-- signature.json
+-- assets
|   +-- logo.png
|   +-- boards
|       +-- stm32f411-lcd.png
+-- hal
|   +-- arduino
|       +-- stm32_f411ce_lcd.cpp
|       +-- libraries
|           +-- OpenPLC_LCD
|               +-- library.properties
|               +-- src
|                   +-- openplc_lcd.h
|                   +-- openplc_lcd.cpp
|                   +-- lcd
|                       +-- ssd2119.h
|                       +-- ssd2119.cpp
|                       +-- Adafruit_GFX.h
|                       +-- Fonts
|                           +-- FreeSerif12pt7b.h
|                           +-- FreeSerif18pt7b.h
+-- screens
    +-- modbus.json
```

### 16.2 เพิ่ม `hal.libraries` ใน manifest

ใน project นี้ resolver รองรับ field `device.hal.libraries` แล้ว โดยจะ resolve เป็น `localLibrariesDir`

ตัวอย่าง manifest เฉพาะส่วนสำคัญ:

```json
{
  "devices": [
    {
      "id": "stm32f411ce-lcd",
      "name": "STM32F411CE LCD",
      "target": {
        "type": "arduino-cli",
        "core": "STMicroelectronics:stm32",
        "platform": "STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE",
        "boardManagerUrl": "https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json"
      },
      "hal": {
        "type": "arduino-hal",
        "source": "hal/arduino/stm32_f411ce_lcd.cpp",
        "libraries": "hal/arduino/libraries",
        "compilerFlags": {
          "c_flags": ["-MMD", "-c", "-Wno-incompatible-pointer-types"],
          "cxx_flags": ["-fexceptions"]
        },
        "define": [
          "OPENPLC_LCD_ENABLED",
          "LCD_SSD2119",
          "LCD_SPI_CS PA4",
          "LCD_SPI_DC PA3",
          "LCD_SPI_RST PA2"
        ]
      }
    }
  ]
}
```

ถ้าต้องใช้ library จาก Arduino Library Manager เพิ่มเติม ให้ใช้ field `extraArduinoLibraries` ภายใต้ `hal`:

```json
"hal": {
  "extraArduinoLibraries": [
    "Adafruit GFX Library"
  ]
}
```

แต่ถ้ามีไฟล์ driver/font ของตัวเอง แนะนำให้วางเป็น local library ใน `hal/arduino/libraries` เพื่อลดปัญหา version และ internet ตอน compile

### 16.3 ตัวอย่าง `library.properties`

```ini
name=OpenPLC_LCD
version=0.1.0
author=OpenPLC WP
maintainer=OpenPLC WP
sentence=SSD2119 LCD support for OpenPLC Arduino targets
paragraph=Non-blocking LCD task and SSD2119 drawing helpers for OpenPLC.
category=Display
architectures=stm32
includes=openplc_lcd.h
```

### 16.4 การเรียก library จาก Baremetal.ino

ถ้าใช้วิธี B อย่างเดียว ต้องมี hook ให้ `Baremetal.ino` เรียก `OpenPLC_LCD_Init()` และ `OpenPLC_LCD_Task()` ได้ด้วย

มี 2 ทางเลือก:

```text
B1: แก้ Baremetal template ให้มี hook แบบ optional
B2: ไม่แก้ Baremetal แต่ใช้ C++ Function Block เป็นตัวเรียก LCD task
```

แนะนำ B1 เพราะสะอาดและทำให้ LCD task อยู่นอก scan cycle ได้จริง

### 16.5 Optional hook ใน Baremetal.ino

เพิ่ม include แบบมี guard:

```cpp
#ifdef OPENPLC_LCD_ENABLED
#include <openplc_lcd.h>
#endif
```

ใน `setup()`:

```cpp
hardwareInit();

#ifdef OPENPLC_LCD_ENABLED
OpenPLC_LCD_Init();
#endif
```

ใน `loop()`:

```cpp
#ifdef OPENPLC_LCD_ENABLED
OpenPLC_LCD_Task();
#endif
```

ข้อดีคือ board ที่ไม่ได้ define `OPENPLC_LCD_ENABLED` จะไม่ได้ include หรือเรียก LCD เลย

## 17. ตัวอย่าง openplc_lcd.h สำหรับ Arduino CLI

```cpp
#ifndef OPENPLC_LCD_H
#define OPENPLC_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

void OpenPLC_LCD_Init(void);
void OpenPLC_LCD_Task(void);
void OpenPLC_LCD_RequestRefresh(void);

#ifdef __cplusplus
}
#endif

#endif
```

## 18. ตัวอย่าง openplc_lcd.cpp แบบไม่ block Ladder

ตัวอย่างนี้ตั้งใจให้เป็น skeleton เท่านั้น ต้องแก้ pin และ SSD2119 API ให้ตรงกับ driver จริง

```cpp
#include <Arduino.h>
#include <SPI.h>
#include "openplc_lcd.h"
#include "lcd/ssd2119.h"

static bool lcdReady = false;
static bool lcdDirty = true;
static uint32_t lastRefreshMs = 0;

void OpenPLC_LCD_Init(void)
{
    SPI.begin();

    if (SSD2119_Init()) {
        lcdReady = true;
        lcdDirty = true;
        SSD2119_FillScreen(SSD2119_BLACK);
    }
}

void OpenPLC_LCD_RequestRefresh(void)
{
    lcdDirty = true;
}

void OpenPLC_LCD_Task(void)
{
    if (!lcdReady) return;

    const uint32_t now = millis();
    if ((now - lastRefreshMs) < 100U) return;
    lastRefreshMs = now;

    if (!lcdDirty) return;
    lcdDirty = false;

    SSD2119_FillRect(0, 0, 320, 40, SSD2119_BLUE);
    SSD2119_DrawText(8, 8, "OpenPLC", SSD2119_WHITE, SSD2119_BLUE);
}
```

กฎสำคัญ:

- ห้ามใช้ `delay()` ใน `OpenPLC_LCD_Task()`
- ห้ามวาดเต็มจอทุกครั้ง
- ให้ refresh เฉพาะตอน dirty หรือครบ interval
- ถ้ามีงานวาดเยอะ ให้ทำเป็น state machine แบ่งวาดทีละส่วน
- ถ้าต้องส่ง pixel จำนวนมาก ให้จำกัดเวลา เช่นไม่เกิน 500-1000 us ต่อรอบ

## 19. ถ้า SSD2119 Driver เดิมเป็น STM32 HAL/CubeMX

ถ้า driver เดิมใช้ API แบบนี้:

```c
extern SPI_HandleTypeDef hspi1;
HAL_SPI_Transmit(&hspi1, ...);
HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, ...);
HAL_GetTick();
```

จะใช้กับ OpenPLC Arduino CLI target ไม่ได้ทันที เพราะ STM32duino ไม่ได้ generate symbol เหล่านี้ให้เหมือน CubeMX

มี 2 ทางเลือก:

### 19.1 Port เป็น Arduino API

แปลง driver ให้ใช้:

```cpp
SPI.begin();
SPI.transfer();
digitalWrite();
pinMode();
millis();
```

เหมาะกับ STM32F411CE / STM32G071RB ที่ compile ผ่าน OpenPLC Editor Arduino CLI

### 19.2 ใช้ CubeIDE เป็น firmware หลัก

เหมาะกับ NUCLEO-H563ZI หรือ project ที่มี HAL driver พร้อมอยู่แล้ว

โครงสร้าง:

```text
STM32CubeIDE project
+-- main.c/main.cpp
+-- SSD2119 HAL driver
+-- OpenPLC generated runtime/source
+-- call runtime_plc_cycle()
+-- call OpenPLC_LCD_Task() after PLC cycle
```

วิธีนี้รักษา HAL driver เดิมได้มากที่สุด แต่ต้องทำ build integration เองนอก OpenPLC Arduino CLI

## 20. แนวทางที่แนะนำจริง

สำหรับงานของเรา แนะนำเป็นลำดับนี้:

```text
1. เพิ่ม optional LCD hook ใน resources/sources/Baremetal/Baremetal.ino
2. สร้าง VPP package เฉพาะ board ที่มี LCD
3. ใส่ LCD driver เป็น local Arduino library ใน hal/arduino/libraries
4. define OPENPLC_LCD_ENABLED เฉพาะ board นั้นใน manifest
5. port SSD2119 driver ให้เป็น Arduino SPI ถ้าใช้ STM32F411CE/STM32G071RB ผ่าน Arduino CLI
6. ให้ OpenPLC_LCD_Task() เป็น non-blocking, dirty update, throttle 100-250 ms
7. compile แล้วดู scan time / Sketch uses / Global variables use
```

สรุป:

```text
ทำได้
แต่ต้องแยก LCD task ออกจาก PLC scan cycle
และถ้าใช้ OpenPLC Arduino CLI ต้อง port HAL/Cube SSD2119 driver เป็น Arduino SPI
```
## 21. แนวทาง Full Framebuffer สำหรับวาด LCD ทั้งหน้า

หัวข้อนี้เป็นแผนการออกแบบก่อนเริ่มแก้ source code ยังไม่ใช่ implementation ที่เปิดใช้งานแล้ว

### 21.1 เป้าหมาย

ต้องการให้ Function Block หลายตัวสามารถประกอบหน้าจอในหน่วยความจำก่อน แล้วจึงสั่งแสดงผลเมื่อข้อมูลของทั้งหน้าพร้อม โดยมีเป้าหมายดังนี้:

- ไม่เรียก `fillScreen()` บน LCD ก่อนวาดข้อความแต่ละบรรทัด
- ไม่ให้ผู้ใช้เห็นช่วงจอดำระหว่าง `Clear Screen` และ `Draw Text`
- ไม่ส่ง SPI จาก C/C++ Function Block โดยตรง
- ไม่ให้ FB ตัวหนึ่งแก้ buffer ที่ `OpenPLC_LCD_Task()` กำลังส่ง
- จำกัดเวลาที่งาน LCD รบกวน PLC scan
- รองรับตำแหน่ง ข้อความ ขนาด font สีตัวอักษร และสีพื้นหลัง
- รองรับการวาดเฉพาะพื้นที่ที่เปลี่ยนในระยะถัดไป

### 21.2 ความแตกต่างระหว่าง Command Buffer และ Full Framebuffer

Command Buffer เก็บรายการคำสั่ง:

```text
Clear black
Draw text "LINE 1" at (10, 50)
Draw text "LINE 2" at (10, 105)
```

Full Framebuffer เก็บสีของทุก pixel:

```text
frameBuffer[y * 320 + x] = RGB565 color
```

Command Buffer ใช้ RAM น้อย แต่ตอน render ยังต้องประมวลผล font และวาดทีละคำสั่ง ส่วน Full Framebuffer สร้างภาพสำเร็จใน RAM ก่อนส่ง pixel ไปยัง SSD2119 จึงไม่จำเป็นต้องล้างจอจริงก่อนเริ่มวาดข้อความ

### 21.3 ขนาด RAM

จอ SSD2119 ที่ใช้มีความละเอียด `320 x 240` และใช้สี RGB565 ขนาด 16 bits หรือ 2 bytes ต่อ pixel:

```text
320 x 240 x 2 = 153,600 bytes
```

| รูปแบบ | RAM สำหรับ pixel |
|---|---:|
| Single framebuffer | 153,600 bytes |
| Double framebuffer | 307,200 bytes |
| Dirty map/metadata | เพิ่มเล็กน้อยตามจำนวน rectangle หรือ tile |

STM32H563ZI มี RAM เพียงพอในเชิงขนาดสำหรับแนวทางนี้ แต่ต้องตรวจค่าจริงหลัง link เพราะ OpenPLC runtime, Modbus, variables, stack, heap และ Arduino core ใช้ RAM ร่วมกัน ห้ามตัดสินจาก RAM รวมของ MCU เพียงค่าเดียว

ขั้นตอนตรวจสอบทุกครั้ง:

```text
1. Compile firmware
2. อ่านรายงาน Global variables use
3. ตรวจ linker map ถ้ามี
4. เหลือพื้นที่สำหรับ stack และ runtime อย่างปลอดภัย
5. ทดสอบ PLC + Modbus + LCD พร้อมกันเป็นเวลานาน
```

คำแนะนำเริ่มต้นคือใช้ Single framebuffer ก่อน เพราะใช้ RAM น้อยกว่า Double framebuffer ครึ่งหนึ่ง

### 21.4 สถาปัตยกรรมที่แนะนำ

```text
PLC scan
  |
  +-- LCD_FRAME_BEGIN
  +-- LCD_FILL_RECT / LCD_TEXT / LCD_VALUE
  +-- LCD_FRAME_PRESENT
             |
             v
       Pending frame
             |
             v
      OpenPLC_LCD_Task()
             |
             +-- Full flush, chunked flush หรือ DMA
             v
         SSD2119 GRAM
```

FB ทุกตัวเขียนลง RAM เท่านั้น ไม่เขียน SPI โดยตรง

สถานะหลัก:

```text
IDLE       ไม่มี frame รอส่ง
BUILDING   Function Blocks กำลังประกอบภาพใน framebuffer
READY      Present แล้ว รอ LCD task
FLUSHING   LCD task กำลังส่ง pixel ไป SSD2119
```

### 21.5 รูปแบบ Buffer

แบบ Single framebuffer:

```cpp
static uint16_t lcdFrameBuffer[SSD2119_WIDTH * SSD2119_HEIGHT];
```

ควรประกาศแบบ global/static เพื่อไม่วางข้อมูลขนาด 153,600 bytes บน stack และไม่ควร `malloc()` ทุกครั้งที่เปลี่ยนหน้า

โครงสร้างสถานะตัวอย่าง:

```cpp
struct OpenPLC_LCD_FrameState {
  bool building;
  bool ready;
  bool flushing;
  uint16_t dirtyX1;
  uint16_t dirtyY1;
  uint16_t dirtyX2;
  uint16_t dirtyY2;
};
```

ถ้าต้องการหลาย dirty rectangles:

```cpp
struct OpenPLC_LCD_DirtyRect {
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
};

static OpenPLC_LCD_DirtyRect dirtyRects[16];
static uint8_t dirtyRectCount;
```

### 21.6 การวาด Font ลง Framebuffer

driver ปัจจุบันใช้ Adafruit GFX font เช่น:

```text
FreeSerif12pt7b
FreeSerif18pt7b
FreeSerif24pt7b
```

การทำ Full Framebuffer ต้องเปลี่ยนปลายทางการวาดจาก SSD2119 โดยตรงเป็น RAM ก่อน มีสองแนวทาง:

1. ใช้ `GFXcanvas16` ถ้า Adafruit GFX รุ่นที่ติดตั้งรองรับ
2. สร้าง canvas class ที่สืบทอดจาก `Adafruit_GFX` และ implement `drawPixel()` ให้เขียน `lcdFrameBuffer`

แนวทางที่ควบคุม memory ได้ชัดเจนคือใช้ static buffer และ canvas wrapper:

```cpp
void drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if (x < 0 || y < 0 || x >= SSD2119_WIDTH || y >= SSD2119_HEIGHT) {
    return;
  }
  lcdFrameBuffer[(uint32_t)y * SSD2119_WIDTH + x] = color;
}
```

จากนั้น `setFont()`, `setCursor()`, `setTextColor()` และ `print()` จะวาดลง framebuffer แทนการส่ง SPI

### 21.7 C API ที่เสนอ

```cpp
uint8_t OpenPLC_LCD_FrameBegin(
  uint16_t backgroundColor,
  uint8_t clearFrame
);

uint8_t OpenPLC_LCD_FrameFillRect(
  uint16_t x,
  uint16_t y,
  uint16_t width,
  uint16_t height,
  uint16_t color
);

uint8_t OpenPLC_LCD_FrameText(
  uint16_t x,
  uint16_t y,
  const char *text,
  uint8_t fontSize,
  uint16_t fontColor,
  uint16_t backgroundColor,
  uint8_t clearTextBackground
);

uint8_t OpenPLC_LCD_FramePresent(void);
uint8_t OpenPLC_LCD_FrameBusy(void);
uint8_t OpenPLC_LCD_FrameReady(void);
```

ความหมาย:

- `FrameBegin()` เริ่มประกอบภาพ แต่ยังไม่ส่ง SPI
- `FrameFillRect()` วาดพื้นที่สีลง RAM
- `FrameText()` วาด font ลง RAM
- `FramePresent()` ปิด frame และส่งมอบให้ LCD task
- `FrameBusy()` ใช้ตรวจว่ากำลัง flush
- `FrameReady()` ใช้ตรวจว่ามี frame รอส่งหรือไม่

### 21.8 Function Blocks ที่เสนอ

```text
LCD_FRAME_BEGIN
  EN
  BG_COLOR
  CLEAR
  DONE
  ERROR

LCD_FRAME_TEXT
  EN
  X
  Y
  TEXT
  FONT_SIZE
  FONT_COLOR
  BG_COLOR
  CLEAR_BG
  DONE
  ERROR

LCD_FRAME_PRESENT
  EN
  DONE
  BUSY
  ERROR
```

ตัวอย่างการใช้งาน:

```text
1-second pulse
  |
  +-- LCD_FRAME_BEGIN(BG_COLOR = BLACK, CLEAR = TRUE)
  +-- LCD_FRAME_TEXT(X=10, Y=50,  TEXT=LINE1, FONT=24, COLOR=ORANGE)
  +-- LCD_FRAME_TEXT(X=10, Y=105, TEXT=LINE2, FONT=24, COLOR=ORANGE)
  +-- LCD_FRAME_TEXT(X=10, Y=160, TEXT=LINE3, FONT=24, COLOR=ORANGE)
  +-- LCD_FRAME_TEXT(X=10, Y=215, TEXT=LINE4, FONT=24, COLOR=ORANGE)
  +-- LCD_FRAME_PRESENT()
```

ลำดับ FB ต้องชัดเจน หาก OpenPLC ไม่รับประกันลำดับประมวลผลของ FB หลาย rung ควรสร้าง FB ระดับสูงหนึ่งตัว เช่น `LCD_PAGE4_FRAME` เพื่อเรียก Begin, Text และ Present ภายใน FB เดียว

### 21.9 การส่ง Framebuffer ไป SSD2119

driver ต้องเพิ่ม API สำหรับส่ง RGB565 block:

```cpp
void OpenPLC_SSD2119::drawRGBBitmap(
  int16_t x,
  int16_t y,
  const uint16_t *pixels,
  int16_t width,
  int16_t height
);
```

หรือ API เฉพาะสำหรับ full frame:

```cpp
void OpenPLC_SSD2119::writeFrameBuffer(
  const uint16_t *pixels,
  uint16_t width,
  uint16_t height
);
```

ขั้นตอนระดับ driver:

```text
1. ตั้ง address window เป็นพื้นที่ที่จะส่ง
2. เลือก SSD2119 GRAM data register
3. ดึง CS ลงครั้งเดียว
4. ส่ง RGB565 ต่อเนื่อง
5. ยก CS เมื่อจบ block
```

ห้ามเรียก `digitalWrite(CS)` และตั้ง address ใหม่ทุก pixel เพราะจะช้ามากและเพิ่มเวลาที่รบกวน PLC scan

### 21.10 ผลต่อ PLC Scan Time

Full framebuffer ขนาด 153,600 bytes ต้องใช้เวลาส่งผ่าน SPI เวลาทฤษฎีโดยไม่รวม overhead:

```text
20 MHz SPI: ประมาณ 61.4 ms
40 MHz SPI: ประมาณ 30.7 ms
```

ถ้า `OpenPLC_LCD_Task()` ส่งทั้ง frame แบบ blocking เวลานี้จะทำให้รอบ PLC ถัดไปล่าช้า จึงไม่ควรใช้ full-frame blocking flush ใน production โดยไม่วัด scan time

ทางเลือก:

1. Chunked flush: ส่งครั้งละ 2-8 แถว แล้วคืน control ให้ main loop
2. SPI DMA: เริ่ม DMA แล้วให้ PLC runtime ทำงานต่อ เหมาะที่สุดถ้า STM32duino core และ driver รองรับอย่างเสถียร
3. Dirty rectangle flush: ส่งเฉพาะพื้นที่ที่เปลี่ยน เหมาะกับ HMI ที่ข้อความเปลี่ยนเพียงบางตำแหน่ง

แนวทางเริ่มต้นที่แนะนำ:

```text
Single framebuffer
+ dirty rectangles
+ chunked flush ที่มี time budget
```

หลังจากทำงานถูกต้องแล้วจึงพิจารณา SPI DMA

### 21.11 Time Budget ของ LCD Task

กำหนดเวลาสูงสุดที่ LCD ใช้ได้ต่อการเรียก:

```cpp
#define OPENPLC_LCD_TASK_BUDGET_US 1000UL
```

ตัวอย่าง logic:

```cpp
void OpenPLC_LCD_Task(void)
{
  uint32_t startedAt = micros();

  while (frameFlushing) {
    flushNextChunk();

    if ((micros() - startedAt) >= OPENPLC_LCD_TASK_BUDGET_US) {
      return;
    }
  }
}
```

ต้องตรวจว่าการส่ง SPI หนึ่ง chunk ไม่เกิน time budget มากเกินไป และเลือกจำนวนแถวต่อ chunk ตาม scan period จริง

### 21.12 การป้องกัน Buffer Conflict

ห้ามให้ FB แก้ framebuffer ขณะ LCD task กำลังอ่าน buffer เดียวกัน

Single framebuffer มีตัวเลือก:

- ไม่อนุญาต `FrameBegin()` ระหว่าง `FLUSHING`
- ให้ FB เห็น `BUSY = TRUE` และลองใหม่ภายหลัง

Double framebuffer มีตัวเลือก:

- PLC วาดลง Back Buffer
- LCD ส่ง Front Buffer
- เมื่อ `Present()` จึงสลับ pointer

Double framebuffer ใช้งานสะดวกกว่า แต่ใช้ RAM 307,200 bytes และ SSD2119 ยังไม่มี hardware page flip ดังนั้นช่วยป้องกัน memory conflict แต่ไม่ได้ทำให้ panel สลับภาพทั้งหน้าในจังหวะเดียว

สำหรับระยะแรกให้ใช้ Single framebuffer พร้อม state lock ก่อน

### 21.13 Flicker และ Tearing

Full framebuffer แก้ปัญหาการกระพริบแบบจอดำได้ เพราะไม่ต้อง:

```text
fillScreen(BLACK) บน LCD
รอ
วาด LINE1
รอ
วาด LINE2
```

แต่ระหว่างส่ง frame ใหม่ SSD2119 ยังแสดง GRAM ไปพร้อมกับการรับ pixel จึงอาจเห็นภาพเก่าและใหม่ผสมกันชั่วคราว เรียกว่า tearing

วิธีลด tearing:

- ส่ง block ต่อเนื่องและใช้ SPI clock ที่ทดสอบแล้วว่าเสถียร
- ใช้ dirty rectangle เพื่อให้พื้นที่และเวลาส่งสั้นลง
- ส่ง frame ในทิศทางเดียวอย่างสม่ำเสมอ
- ใช้ TE/VSync signal หากโมดูล LCD เปิดขานี้ออกมาและ driver รองรับ
- ใช้ SPI DMA เพื่อลดช่องว่างระหว่าง byte/word

ถ้าโมดูลไม่มี TE signal จะไม่สามารถรับประกัน tear-free 100% ได้ด้วย software framebuffer เพียงอย่างเดียว

### 21.14 นโยบายเมื่อ Frame ใหม่มาระหว่างกำลังส่ง

ต้องเลือกพฤติกรรมหนึ่งแบบ:

1. Reject: `FramePresent()` คืน 0 และ FB แสดง `BUSY`
2. Latest wins: เก็บเฉพาะ frame ล่าสุดและทิ้ง frame เก่าที่ยังไม่เริ่มส่ง
3. Queue: เก็บหลาย frame ซึ่งใช้ RAM มากและอาจทำให้ภาพล่าช้าจากค่าจริง

สำหรับหน้าจอ PLC HMI แนะนำ `Reject` ใน Single framebuffer ระยะแรก เพราะตรวจสอบพฤติกรรมง่ายและไม่ทำให้ข้อมูลใน frame ที่กำลังส่งเปลี่ยนกลางทาง

### 21.15 ลำดับการพัฒนา

```text
Phase 1
- เพิ่ม static RGB565 framebuffer
- เพิ่ม canvas drawPixel
- เพิ่ม FrameBegin / FrameText / FramePresent
- ใช้ blocking full flush เพื่อยืนยันความถูกต้องของสีและตำแหน่ง

Phase 2
- วัด PLC scan time ก่อนและระหว่าง full flush
- เพิ่ม dirty rectangle
- ไม่ส่ง frame ถ้าข้อมูลไม่เปลี่ยน

Phase 3
- เปลี่ยนเป็น chunked flush พร้อม time budget
- ทดสอบ Online Debug และ Modbus ระหว่าง LCD update

Phase 4
- พิจารณา SPI DMA
- พิจารณา TE synchronization ถ้า hardware รองรับ
```

Phase 1 ใช้เพื่อพิสูจน์การทำงานเท่านั้น ถ้า blocking flush ทำให้ scan time เกินข้อกำหนด ห้ามใช้เป็น production configuration

### 21.16 Acceptance Tests

ต้องทดสอบอย่างน้อย:

```text
1. เปิดหน้าจอแล้วไม่มีช่วง Clear สีดำก่อนข้อความ
2. ข้อความ 4 บรรทัดปรากฏด้วยตำแหน่งและสีถูกต้อง
3. เปลี่ยนข้อความบรรทัดเดียวแล้วบรรทัดอื่นไม่กระพริบ
4. STRING ที่สั้นลงไม่เหลือตัวอักษรเก่า
5. Present ขณะ Busy ไม่ทำให้ buffer เสีย
6. PLC input/output และ timer ยังทำงานตาม scan period
7. OpenPLC Online Debug ผ่าน USART3/ST-LINK VCP ยังทำงาน
8. Modbus RTU บน USART2 ยังทำงานระหว่าง LCD update
9. ไม่มี RAM overflow หรือ reset หลังทำงานต่อเนื่อง
10. วัด worst-case scan time เมื่อเปลี่ยนทั้งหน้า
```

### 21.17 ข้อสรุปก่อนเริ่ม Implementation

แนวทาง Full Framebuffer สามารถใช้กับ NUCLEO-H563ZI และ SSD2119 ได้ แต่ควรเริ่มจาก:

```text
Single static RGB565 framebuffer ขนาด 153,600 bytes
+ Adafruit GFX-compatible canvas
+ FrameBegin / FrameText / FramePresent
+ state lock
+ dirty rectangles
+ chunked SPI flush
```

ไม่ควรเริ่มด้วย Double framebuffer หรือ full-screen blocking flush เป็นรูปแบบสุดท้าย เพราะใช้ RAM มากและอาจกระทบ PLC scan time การ implementation ต้องเปิดใช้งานเฉพาะ board/package ที่กำหนด `OPENPLC_LCD_ENABLED` เพื่อไม่ให้ board อื่นจอง RAM สำหรับ LCD โดยไม่จำเป็น
## 22. สถานะการพัฒนา Full Framebuffer

เริ่ม implementation ตามแนวทางหัวข้อ 21 แล้วใน `OpenPLC_SSD2119` version `0.2.0`

### 22.1 สิ่งที่พัฒนาแล้ว

```text
- Single static RGB565 framebuffer ขนาด 320 x 240 x 2 = 153,600 bytes
- เปิดใช้งานเฉพาะ NUCLEO-H563ZI WP
- Adafruit_GFX-compatible RAM canvas สำหรับ FreeSerif 12/18/24
- FrameBegin / FrameFillRect / FrameText / FramePresent
- FrameAbort / FrameBusy / FrameReady / FrameEnabled
- Dirty rectangle แบบ bounding rectangle
- Chunked flush สูงสุด 2 แถวหรือ 1,000 us ต่อ OpenPLC_LCD_Task() call
- RGB565 row streaming ที่รองรับ panel rotation 180
- Legacy Text/Page4 API ยังคงใช้งานได้
```

### 22.2 Define ของ NUCLEO-H563ZI WP

```text
OPENPLC_LCD_FRAMEBUFFER_ENABLED 1
OPENPLC_LCD_TASK_BUDGET_US 1000UL
OPENPLC_LCD_ROWS_PER_TASK 2U
```

Board อื่นที่ไม่มี `OPENPLC_LCD_FRAMEBUFFER_ENABLED=1` จะไม่จอง framebuffer 153,600 bytes และ Frame API จะคืนค่า `0`

### 22.3 ผล Compile Verification

```text
NUCLEO-H563ZI, framebuffer enabled
Sketch: 67,912 / 2,097,152 bytes (3%)
Global RAM: 156,912 / 655,360 bytes (23%)
Remaining RAM: 498,448 bytes

STM32F411CE, framebuffer disabled
Sketch: 59,136 / 524,288 bytes (11%)
Global RAM: 6,768 / 131,072 bytes (5%)
Remaining RAM: 124,304 bytes
```

ผลนี้ยืนยันว่า H563ZI สามารถจอง Single RGB565 framebuffer ได้ และ F411CE ไม่ได้รับผลจาก buffer เมื่อ feature ถูกปิด

### 22.4 ลำดับ API

```cpp
if (OpenPLC_LCD_FrameBegin(SSD2119_BLACK, 1U)) {
  OpenPLC_LCD_FrameText(10, 50,  "LINE 1", 24, SSD2119_ORANGE, SSD2119_BLACK, 0U);
  OpenPLC_LCD_FrameText(10, 105, "LINE 2", 24, SSD2119_ORANGE, SSD2119_BLACK, 0U);
  OpenPLC_LCD_FrameText(10, 160, "LINE 3", 24, SSD2119_ORANGE, SSD2119_BLACK, 0U);
  OpenPLC_LCD_FrameText(10, 215, "LINE 4", 24, SSD2119_ORANGE, SSD2119_BLACK, 0U);
  OpenPLC_LCD_FramePresent();
}
```

`FrameBegin(..., 1)` ล้างเฉพาะ framebuffer ใน RAM จอจริงยังแสดง frame เดิมอยู่จนกว่า `FramePresent()` และ `OpenPLC_LCD_Task()` จะเริ่ม flush จึงไม่เกิดช่วงจอดำระหว่างคำสั่ง Text

### 22.5 Partial Update

ถ้าต้องการเปลี่ยนเฉพาะหนึ่งบรรทัด:

```cpp
if (OpenPLC_LCD_FrameBegin(SSD2119_BLACK, 0U)) {
  OpenPLC_LCD_FrameFillRect(0, 120, 320, 55, SSD2119_BLACK);
  OpenPLC_LCD_FrameText(10, 160, "NEW LINE 3", 24, SSD2119_ORANGE, SSD2119_BLACK, 0U);
  OpenPLC_LCD_FramePresent();
}
```

`clearFrame=0` ใช้ได้เมื่อ framebuffer synchronized กับ panel เท่านั้น หากก่อนหน้านี้ใช้ legacy direct drawing API ต้องเริ่ม frame ถัดไปด้วย `clearFrame=1`

### 22.6 Function Block Examples

ตัวอย่างอยู่ใน package library:

```text
extras/OpenPLC_Function_Blocks/LCD_FRAME_BEGIN_FB.cpp
extras/OpenPLC_Function_Blocks/LCD_FRAME_TEXT_FB.cpp
extras/OpenPLC_Function_Blocks/LCD_FRAME_PRESENT_FB.cpp
extras/OpenPLC_Function_Blocks/LCD_FRAME_PAGE4_FB.cpp
```

สำหรับหน้า 4 บรรทัด แนะนำ `LCD_FRAME_PAGE4_FB` เพราะ Begin, Text ทั้งสี่บรรทัด และ Present อยู่ใน FB เดียว จึงไม่เสี่ยงจากลำดับ execution ของหลาย rung

### 22.7 งานที่ยังต้องทดสอบบน Hardware

```text
- ตรวจทิศทาง row streaming เมื่อ panel rotation 180
- ตรวจสี RGB565 จาก framebuffer
- วัด actual PLC scan jitter ระหว่าง full-screen flush
- ตรวจ Online Debug ผ่าน USART3/ST-LINK VCP ระหว่าง flush
- ตรวจ Modbus RTU USART2 ระหว่าง flush
- ทดสอบข้อความสั้นลงและ partial dirty rectangle
- ปรับ ROWS_PER_TASK หรือ TASK_BUDGET_US จากผลวัดจริง
```

ยังไม่ได้เพิ่ม SPI DMA ใน phase นี้ ระบบใช้ bounded chunked SPI เพื่อให้ทดสอบและวัด scan time ได้ง่ายก่อน
### 22.8 แก้ไข FrameBegin คืนค่า 0 และจอดำ

พบจาก firmware linker map ว่า `OpenPLC_LCD_FrameBegin()` มีขนาดเพียง 4 bytes และไม่มี `lcdFrameBuffer` แสดงว่า library ถูก compile ด้วย `OPENPLC_LCD_FRAMEBUFFER_ENABLED=0`

สาเหตุคือ VPP `hal.define` ถูกสร้างใน generated `defines.h` ซึ่ง `Baremetal.ino` include ได้ แต่ Arduino library source ถูก compile เป็น translation unit แยกและไม่ได้ include generated `defines.h`

แก้ใน library version `0.2.1` ให้ตรวจ board macro ที่ STM32duino ส่งให้ทุก translation unit:

```cpp
#ifndef OPENPLC_LCD_FRAMEBUFFER_ENABLED
#if defined(ARDUINO_NUCLEO_H563ZI)
#define OPENPLC_LCD_FRAMEBUFFER_ENABLED 1
#else
#define OPENPLC_LCD_FRAMEBUFFER_ENABLED 0
#endif
#endif
```

ผลทดสอบโดยไม่ส่ง custom framebuffer define:

```text
NUCLEO-H563ZI: RAM 156,912 bytes, framebuffer enabled
STM32F411CE:   RAM   6,768 bytes, framebuffer disabled
```

หลังอัปเดต library ต้อง Build & Upload firmware ใหม่ ตัว firmware เดิมบน board ยังมี stub API ที่คืนค่า 0
## 23. การแยก Frame เป็น Begin, Text และ Present

เพิ่มตัวอย่าง FB แยกสามส่วนเพื่อให้หนึ่ง frame มีข้อความหลายตำแหน่ง หลาย font และหลายสี:

```text
LCD_FRAME_BEGIN_FB.cpp
LCD_FRAME_TEXT_FB.cpp
LCD_FRAME_PRESENT_FB.cpp
```

ตัวแปรสำคัญ:

```text
LCD_FRAME_BEGIN
- BG_COLOR UINT
- CLEAR BOOL
- LAST_EN Local BOOL

LCD_FRAME_TEXT
- X, Y UINT
- TEXT STRING
- FONT_SIZE USINT
- FONT_COLOR UINT
- BG_COLOR UINT
- CLEAR_BG BOOL
- LAST_EN Local BOOL

LCD_FRAME_PRESENT
- LAST_EN Local BOOL
- ACTIVE Local BOOL
- DONE, BUSY, ERROR Output BOOL
```

ใช้ pulse เดียวกันกับทุก FB และเรียง rung จาก Begin ก่อน ตามด้วย Text ทุก instance แล้วจบด้วย Present:

```text
Rung 1  UPDATE_PULSE -> FRAME_BEGIN
Rung 2  UPDATE_PULSE -> FRAME_TEXT_1
Rung 3  UPDATE_PULSE -> FRAME_TEXT_2
Rung 4  UPDATE_PULSE -> FRAME_TEXT_3
Rung 5  UPDATE_PULSE -> FRAME_PRESENT
```

ต้อง block pulse ใหม่ระหว่าง `FRAME_PRESENT.BUSY=TRUE` และ pulse ต้องกลับเป็น FALSE อย่างน้อยหนึ่ง PLC scan ก่อน frame ถัดไป

ห้ามใช้ C++ `static LAST_EN` ใน Text FB เพราะหลาย instance จะใช้ state ร่วมกัน ต้องประกาศ `LAST_EN` เป็น Local BOOL ใน variable table เพื่อให้แต่ละ instance มี edge state ของตัวเอง

ตัวอย่างค่าหลายรูปแบบ:

| Text | X | Y baseline | Font | Color RGB565 |
|---|---:|---:|---:|---:|
| `'OPENPLC'` | 10 | 45 | 24 | `16#FBA1` orange |
| `'RUNNING'` | 10 | 90 | 18 | `16#FFFF` white |
| `'PV=25.4'` | 170 | 140 | 12 | `16#07E0` green |
| `'ALARM'` | 10 | 210 | 24 | `16#F800` red |

เมื่อ `FRAME_BEGIN.CLEAR=TRUE` ให้ Text ใช้ `CLEAR_BG=FALSE` ได้ เพราะพื้นหลังทั้งหมดถูกเตรียมใน RAM แล้ว ปัจจุบัน font mapping รองรับ 12, 18 และ 24