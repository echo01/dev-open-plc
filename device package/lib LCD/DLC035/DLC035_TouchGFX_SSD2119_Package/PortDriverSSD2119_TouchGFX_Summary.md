# สรุปการ Port Driver SSD2119 ไปใช้กับ TouchGFX

## Pin Mapping Update (v0.2.4)

Package v0.2.4 และ project หลักเปลี่ยนมาใช้ PG15 = LCD_DC/RS, PB7 = LCD_RST, PB6 = LCD_CS, PB3 = SPI1_SCK, PB4 = SPI1_MISO และ PB5 = SPI1_MOSI. Driver ใช้ macro จาก Core/Inc/main.h จึงไม่ hard-code GPIO ภายใน ssd2119.c.

## เป้าหมาย

Port driver LCD `SSD2119` สำหรับจอ `DLC0350QEM06DT-1` ขนาด `320x240 RGB565` ให้ใช้งานกับ TouchGFX Designer/CubeMX project บน `NUCLEO-H563ZI` โดยไม่มี external RAM และไม่มี FMC display bus. จอถูกขับผ่าน `SPI1` เขียนเข้า GRAM ภายใน SSD2119.

Project reference ที่แก้แล้ว:

```text
C:\TouchGFXProjects\DLC035
```

## Hardware Pin

| Function | STM32H563ZI Pin | CubeMX Signal | หมายเหตุ |
|---|---:|---|---|
| LCD SCK | PB3 | SPI1_SCK | SPI clock |
| LCD MOSI | PB5 | SPI1_MOSI | ใช้ส่ง command/data |
| LCD MISO | PB4 | SPI1_MISO | optional, project เปิด Full Duplex แต่จอใช้งานหลักเป็น write-only |
| LCD RST | PB7 | GPIO_Output, label `LCD_RST` | reset SSD2119 |
| LCD DC/RS | PG15 | GPIO_Output, label `LCD_DC` | 0=command, 1=data |
| LCD CS | PB6 | GPIO_Output, label `LCD_CS` | software chip select |

ไม่ได้ใช้:

- FMC / parallel LCD bus
- External SDRAM/SRAM
- LCD TE interrupt pin
- LTDC
- DMA2D/ChromART ในรอบ bring-up นี้

## Module ที่ใช้งาน

CubeMX / middleware:

- `SPI1`
- `GPIO`
- `GPDMA1` จาก template เดิม สามารถคงไว้ แต่ SSD2119 path ปัจจุบันยังไม่ใช้ DMA
- `ICACHE`
- `CRC`
- `X-CUBE-FREERTOS 1.5.0`, CMSIS RTOS V2
- `X-CUBE-TOUCHGFX 4.26.1`

TouchGFX setting:

- Interface: `Custom`
- Resolution: `320 x 240`
- Color depth: `16 bit`, RGB565
- Framebuffer Strategy: `Partial Buffer - GRAM display`
- Number of Blocks: `3`
- Block Size: `2048 bytes`
- RTOS: `CMSIS_RTOS_V2`

Clock/SPI setting จาก `.ioc`:

```text
SYSCLK = 180 MHz
HCLK   = 180 MHz
SPI1 clock source = CLKP
SPI1 clock = 32 MHz
SPI1 prescaler = 8
SPI1 calculated baudrate = 4.0 Mbits/s
SPI1 data size = 8 bit
SPI1 mode = Master, 2 lines
```

## Driver ที่เพิ่มเข้า Project

เพิ่มไฟล์:

```text
Core/Inc/ssd2119.h
Core/Src/ssd2119.c
Core/Inc/Adafruit_GFX.h
```

API สำคัญที่ใช้กับ TouchGFX:

```c
HAL_StatusTypeDef SSD2119_Init(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef SSD2119_SetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
HAL_StatusTypeDef SSD2119_WritePixels(const uint16_t *pixels, uint32_t count);
HAL_StatusTypeDef SSD2119_WriteBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *pixels);
uint8_t SSD2119_IsTransferBusy(void);
void SSD2119_TxCpltCallback(SPI_HandleTypeDef *hspi);
```

สี base ที่ driver ควรคงไว้:

```c
SSD2119_BLACK
SSD2119_WHITE
SSD2119_RED
SSD2119_GREEN
SSD2119_BLUE
SSD2119_YELLOW
SSD2119_ORANGE
SSD2119_CYAN
SSD2119_MAGENTA
```

## TouchGFX Integration ที่แก้แล้ว

ไฟล์หลัก:

```text
TouchGFX/target/TouchGFXHAL.cpp
Core/Src/app_freertos.c
TouchGFX/target/KeySampler.cpp
STM32CubeIDE/.project
```

ใน `TouchGFXHAL.cpp`:

- เพิ่ม `#include "ssd2119.h"`
- เพิ่ม `extern "C" SPI_HandleTypeDef hspi1;`
- `TouchGFXHAL::initialize()` เรียก `initLCD()` ก่อน initialize TouchGFX generated HAL
- `initLCD()` เรียก `SSD2119_Init(&hspi1)`
- `touchgfxDisplayDriverTransmitBlock()` ส่ง partial framebuffer block ไปจอผ่าน `SSD2119_WriteBitmap(...)`
- `touchgfxDisplayDriverTransmitActive()` คืนค่า `SSD2119_IsTransferBusy()`
- `enableLCDControllerInterrupt()` ว่างไว้ เพราะยังไม่ต่อ TE/VSYNC
- หลังส่ง block แบบ blocking แล้วเรียก `touchgfx::startNewTransfer()`

ใน `app_freertos.c`:

- เพิ่ม `extern void touchgfxSignalVSync(void);`
- ใน `StartDefaultTask` เรียก `touchgfxSignalVSync(); osDelay(16);`
- เหตุผล: TouchGFX รอ VSYNC ผ่าน OS queue แต่ hardware TE path ถูกปิด จึงต้องมี software tick ให้ pipeline เดิน

ใน `KeySampler.cpp`:

- เปลี่ยน `BUTTON_USER_GPIO_Port` -> `BUTTON_GPIO_Port`
- เปลี่ยน `BUTTON_USER_Pin` -> `BUTTON_Pin`
- เหตุผล: `main.h` ของ CubeMX generate เป็น `BUTTON_*`

ใน `STM32CubeIDE/.project`:

- เพิ่ม linked source `Core/Src/ssd2119.c`
- ลบ linked source ของ FMC/NAND/NOR/SDRAM/SRAM/LL_FMC ที่ template เดิมทิ้งค้างไว้ เพราะ project นี้ไม่ใช้ FMC

## การตรวจสอบที่ทำแล้ว

Build command:

```powershell
& 'C:\ST\STM32CubeIDE_1.18.1\STM32CubeIDE\stm32cubeidec.exe' --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data 'C:\TouchGFXProjects\DLC035\.cubeide_ws' -import 'C:\TouchGFXProjects\DLC035\STM32CubeIDE' -build STM32H563ZI_NUCLEO_AZ2/Debug
```

ผลล่าสุด:

```text
Build Finished. 0 errors, 0 warnings.
text: 149998
data: 764
bss : 70752
```

## งานที่ต้องทำต่อ

1. Flash ลงบอร์ด NUCLEO-H563ZI และตรวจภาพจริงบนจอ DLC035.
2. ตรวจ orientation ว่า 180 องศาถูกต้องกับการติดตั้งจริงหรือไม่.
3. ตรวจสี `RGB565` ด้วย pattern: black, white, red, green, blue, orange, cyan, magenta.
4. ตรวจ partial framebuffer update: วาด rectangle หลายตำแหน่งและตรวจว่าไม่มีแถบเพี้ยน.
5. เพิ่ม SPI DMA เพื่อเพิ่ม frame rate:
   - เปลี่ยน `SSD2119_WriteBitmap()` เป็น async DMA
   - `touchgfxDisplayDriverTransmitActive()` ต้องคืน busy ระหว่าง DMA
   - เรียก `touchgfx::startNewTransfer()` ใน `HAL_SPI_TxCpltCallback`
6. ถ้าต่อ TE pin ในอนาคต:
   - เพิ่ม GPIO EXTI pin สำหรับ TE
   - ย้ายจาก synthetic VSYNC ไปใช้ `touchgfxSignalVSync()` จาก interrupt
   - เปิด path `LCD_SignalTearingEffectEvent()` ตาม TouchGFX HAL pattern
7. ทำ Board Setup package จริงสำหรับ TouchGFX Designer:
   - ใช้ package นี้เป็น source
   - สร้าง thumbnail/front image
   - สร้าง `.tpa` ที่มี `package.json` และ zip template
   - ทดสอบ import/เปิดจาก Designer

## ข้อควรระวังเมื่อ Generate Code ซ้ำ

CubeMX/TouchGFX อาจเขียนไฟล์ generated ใหม่ และอาจทำให้รายการ source ใน `.project` กลับไปอ้าง FMC/NAND/NOR/SDRAM/SRAM ได้อีก. หลัง generate ทุกครั้งให้ตรวจ:

- `TouchGFX/target/TouchGFXHAL.cpp`
- `Core/Src/app_freertos.c`
- `STM32CubeIDE/.project`
- `Core/Inc/main.h` ว่า label pin LCD ยังอยู่
- `Core/Src/main.c` ว่า SPI1 ยังเป็น prescaler 8 และ data size 8-bit


## สถานะล่าสุดหลังทดสอบกับ TouchGFX Designer

สถานะนี้ใหม่กว่าส่วน "งานที่ต้องทำต่อ" ด้านบน และใช้เป็นสถานะปัจจุบันของ project

```text
LCD + TouchGFX       : ทำงานได้บน hardware จริง
Designer Generate    : Generate code แล้ว UI จาก TouchGFX แสดงบน LCD ได้
Board setup package  : สร้าง package ได้แล้ว
Designer Create      : สร้าง project จาก Create -> By Partners ได้แล้ว
Package version      : DLC035_SSD2119_NUCLEO_H563ZI v0.2.4
```

ไฟล์ package ที่ใช้งานได้:

```text
DLC035_TouchGFX_SSD2119_Package\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
```

ตำแหน่งติดตั้งที่ทดสอบแล้ว:

```text
E:\TouchGFX\4.26.1\app\packages
```

## Known Good Package Metadata

ใช้ค่าชุดนี้เมื่อสร้าง `.tpa` ซ้ำ

```text
TouchGFX Designer version : 4.26.1
Package Data.Name        : DLC035_SSD2119_NUCLEO_H563ZI
Package Data.Version     : 0.2.4
Package Type             : TGAT
Package Category         : By Partners
Payload zip              : DLC035_SSD2119_NUCLEO_H563ZI.zip
Payload MD5              : 84CD6A87E1A69F97173EBB49021299F3
Payload Size             : 33579259
PathToDotTouchGFX        : TouchGFX
EmbeddedOs               : FreeRTOS
Resolution               : 320 x 240
TargetBpp                : 16
```

ข้อสำคัญ:

```text
Meta.MD5sum และ Meta.Size ต้องเป็นของ payload zip ด้านใน .tpa
ห้ามใช้ MD5/Size ของไฟล์ .tpa ทั้งก้อน
```

ถ้าค่านี้ผิด Designer จะขึ้น `Download failed` และใน log จะพบ:

```text
Wrong MD5Sum
Invalid local package
Downloading ... .tpa
404 Not Found
```

## เอกสารอ้างอิงใน project

อ่านเอกสารตามลำดับนี้เมื่อจะทำซ้ำ:

```text
1. ..\PortDriverSSD2119toTouchGFX.md
   - guide หลักของการ port SSD2119 driver เข้า TouchGFX

2. PortDriverSSD2119_TouchGFX_Summary.md
   - สรุปสถานะ implementation และ known good ล่าสุด

3. DLC035_SSD2119_TouchGFX_Package_Guide.md
   - วิธีสร้างและติดตั้ง TouchGFX package .tpa
```