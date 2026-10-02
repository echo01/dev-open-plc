# Port Driver SSD2119 to TouchGFX

## Pin Mapping Update (v0.2.4)

ตั้งแต่ package v0.2.4 ใช้ขา PG15 = LCD_DC/RS, PB7 = LCD_RST, PB6 = LCD_CS, PB3 = SPI1_SCK, PB4 = SPI1_MISO และ PB5 = SPI1_MOSI แทนชุดขาเดิม PD15/PD14/PF3/PA5/PG9. คำว่า RS ในเอกสารหมายถึง Data/Command และไม่ใช่ Reset; Reset ใช้ชื่อ RST.

เอกสารนี้สรุปแนวทางนำ driver `SSD2119` ของ project `DriveLCDSSD2119` ไปใช้กับ TouchGFX บนบอร์ด `NUCLEO-H563ZI / STM32H563ZITx` กับ LCD `DLC0350QEM06DT-1` ขนาด 320x240 ผ่าน SPI โดยไม่มี external RAM framebuffer

## 1. Hardware Target

```text
MCU board : NUCLEO-H563ZI / STM32H563ZITx
LCD       : DLC0350QEM06DT-1, 320x240 TFT
LCD IC    : SSD2119
Interface : SPI command/data + GPIO control
Pixel     : RGB565, 16-bit color
External RAM framebuffer : Not used
```

PIN ที่ใช้ใน project ปัจจุบัน:

```text
PB3  -> SPI1_SCK
PB5  -> SPI1_MOSI -> LCD SDI/SDA/DIN
PB4  -> SPI1_MISO -> Optional, ต่อเฉพาะกรณี LCD มี SDO/SO/DOUT
PB7  -> LCD_RST
PG15 -> LCD_DC / RS
PB6  -> LCD_CS, software chip select
```

จอ SSD2119 มี internal GRAM อยู่ในตัว LCD controller ดังนั้น MCU ไม่จำเป็นต้องมี framebuffer เต็มจอใน RAM ภายนอก แต่ TouchGFX ยังต้องมี framebuffer/partial framebuffer ภายใน SRAM สำหรับ render UI ก่อนส่งออกจอ

## 2. Driver ปัจจุบันที่นำไปใช้ต่อได้

ไฟล์หลักใน CubeIDE project:

```text
Core/Inc/ssd2119.h
Core/Src/ssd2119.c
```

API ที่มีอยู่แล้ว:

```c
HAL_StatusTypeDef SSD2119_Init(SPI_HandleTypeDef *hspi);
void SSD2119_Reset(void);
void SSD2119_FillScreen(uint16_t color);
void SSD2119_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void SSD2119_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
```

ส่วนที่สำคัญต่อ TouchGFX คือ logic ต่อไปนี้ใน `ssd2119.c`:

```text
SSD2119_SetWindow()       -> กำหนด GRAM window ของ SSD2119
SSD2119_WriteColorStream()-> ส่ง pixel สีเดียวจำนวนมากผ่าน SPI
SSD2119_WriteBytes()      -> low-level SPI transmit
SSD2119_WriteRegister()   -> เขียน register ของ SSD2119
```

ส่วน font Adafruit/GFX ที่เคย port มาไม่จำเป็นสำหรับ TouchGFX เพราะ TouchGFX จะ render font, widget, image, text ลง framebuffer เอง

## 3. แนวทาง TouchGFX ที่เหมาะกับ SSD2119 SPI

สำหรับ LCD แบบ SPI และมี GRAM ภายใน controller ควรใช้แนวทาง:

```text
TouchGFX Custom Display Interface
+ Partial Framebuffer
+ SSD2119 SPI flush driver
```

เหตุผล:

- STM32H563ZI ไม่มี external SDRAM สำหรับ framebuffer เต็มจอ
- Full framebuffer RGB565 ขนาด 320x240 ต้องใช้ RAM ประมาณ 150 KB
- Partial framebuffer ใช้ RAM น้อยกว่า และเหมาะกับ SPI LCD
- SSD2119 รับข้อมูลเป็น block/window ผ่าน GRAM ได้โดยตรง

เอกสาร TouchGFX ที่เกี่ยวข้อง:

```text
TouchGFX Custom Display Interface
TouchGFX Partial Framebuffer / Lowering Memory Usage
TouchGFX Framebuffer Strategies
```

## 4. สิ่งที่ต้องแก้ใน SSD2119 Driver

ต้องเพิ่ม API สำหรับส่ง block pixel จาก TouchGFX แทนการวาดเองทีละ object

แนะนำเพิ่มใน `ssd2119.h`:

```c
void SSD2119_SetWindowPublic(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void SSD2119_WritePixels(const uint16_t *pixels, uint32_t count);
void SSD2119_WriteBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *pixels);
uint8_t SSD2119_IsTransferBusy(void);
void SSD2119_TxCpltCallback(void);
```

หรือเปลี่ยน `SSD2119_SetWindow()` จาก `static` เป็น public function ชื่อ `SSD2119_SetWindow()` ก็ได้ แต่ควรระวังไม่ให้ชนกับ private helper เดิม

### Byte Order สำคัญมาก

TouchGFX framebuffer แบบ RGB565 บน STM32 อยู่ใน memory แบบ little-endian:

```text
uint16_t color = 0x001F  -> RAM byte order = 1F 00
```

แต่ SSD2119 ต้องการ SPI byte order แบบ high byte ก่อน:

```text
0x001F -> ส่ง SPI เป็น 00 1F
```

ดังนั้นถ้าส่ง buffer จาก TouchGFX ผ่าน `HAL_SPI_Transmit()` ตรง ๆ สีจะเพี้ยน ต้องเลือกวิธีใดวิธีหนึ่ง:

1. ทำ byte swap ลง temporary line/block buffer ก่อนส่ง SPI
2. ใช้ SPI data size 16-bit สำหรับ pixel stream แล้วตรวจสอบ endian/bit order บน logic analyzer
3. ใช้ DMA พร้อม buffer ที่ถูก swap แล้ว

แนวทางปลอดภัยช่วง bring-up คือทำ byte swap แบบ software ก่อน แล้วค่อย optimize เป็น DMA ภายหลัง

## 5. TouchGFX HAL Integration

เมื่อสร้าง TouchGFX project แล้วต้องเพิ่ม hook สำหรับ flush framebuffer block

ตัวอย่าง concept:

```cpp
extern "C" uint8_t isTransmittingData()
{
    return SSD2119_IsTransferBusy();
}

extern "C" void transmitFrameBufferBlock(uint8_t* pixels,
                                           uint16_t x,
                                           uint16_t y,
                                           uint16_t w,
                                           uint16_t h)
{
    SSD2119_WriteBitmap(x, y, w, h, (const uint16_t*)pixels);
}
```

ถ้าใช้ SPI DMA:

```text
transmitFrameBufferBlock()
  -> set busy flag
  -> SSD2119_SetWindow(x, y, w, h)
  -> start HAL_SPI_Transmit_DMA()
  -> return immediately

HAL_SPI_TxCpltCallback()
  -> clear busy flag
  -> LCD_CS high
  -> touchgfx::startNewTransfer()
```

ถ้าใช้ blocking SPI ช่วงแรก:

```text
transmitFrameBufferBlock()
  -> SSD2119_WriteBitmap() แบบ blocking
  -> touchgfx::startNewTransfer()
```

แบบ blocking ง่ายต่อการ bring-up แต่ UI จะช้ากว่าและ CPU ถูก hold ระหว่างส่ง SPI

## 6. CubeMX / CubeIDE Configuration

ตั้งค่า project สำหรับ TouchGFX ดังนี้:

```text
TouchGFX Display Interface : Custom
Resolution                 : 320 x 240
Pixel format               : RGB565
Framebuffer strategy       : Partial framebuffer
RTOS                       : Bare metal หรือ FreeRTOS ตาม template
LTDC                       : Disabled
FMC/SDRAM framebuffer      : Disabled
SPI1                       : Enabled
DMA for SPI1 TX            : Recommended
```

SPI1 ปัจจุบันใน project:

```text
Mode                 : Master
Direction            : 2 lines
Data size            : 8 bit
Clock polarity       : Low
Clock phase          : 1 edge
NSS                  : Software
Baud rate prescaler  : 16
SPI clock            : 32 MHz / 16 = 2 MHz
```

สำหรับ TouchGFX ควรเพิ่มความเร็ว SPI หลังจากจอทำงานนิ่งแล้ว เช่น:

```text
เริ่มทดสอบ : 2 MHz
ปรับขึ้น   : 8 MHz, 12 MHz, 16 MHz, 24 MHz ตาม waveform และความเสถียร
```

ต้องตั้ง GPIO speed ของ `PB3/PB4/PB5` เป็น high หรือ very high เมื่อเพิ่ม SPI speed

## 7. Memory Strategy

จอ 320x240 RGB565:

```text
Full framebuffer = 320 * 240 * 2 = 153,600 bytes
```

STM32H563 มี SRAM ภายในมากพอสำหรับ full framebuffer หนึ่งชุดในบางกรณี แต่ถ้ามี TouchGFX heap, stack, OpenPLC logic, communication stack หรือ assets เพิ่มขึ้น ควรใช้ partial framebuffer เพื่อลด RAM usage

ตัวอย่าง partial framebuffer:

```text
Block 320 x 16 pixels = 10,240 bytes
Block 320 x 24 pixels = 15,360 bytes
Block 320 x 32 pixels = 20,480 bytes
```

แนะนำเริ่มที่ 2 หรือ 3 blocks เพื่อให้ TouchGFX render block ถัดไปได้ระหว่าง block ก่อนหน้ากำลังส่งออกจอ

## 8. Performance Consideration

ข้อมูล full screen RGB565:

```text
320 * 240 * 2 = 153,600 bytes = 1,228,800 bits
```

เวลาโดยประมาณเฉพาะส่ง SPI:

```text
SPI 2 MHz  -> ประมาณ 614 ms / full frame
SPI 8 MHz  -> ประมาณ 154 ms / full frame
SPI 16 MHz -> ประมาณ 77 ms / full frame
SPI 24 MHz -> ประมาณ 51 ms / full frame
```

ดังนั้น UI แบบ TouchGFX บน SPI SSD2119 เหมาะกับ HMI ที่เปลี่ยนเฉพาะบางพื้นที่ เช่น text, gauge, icon, status ไม่เหมาะกับ animation เต็มจอความเร็วสูง

## 9. Orientation / 180 Degree Mapping

Project ปัจจุบันแก้จอกลับหัวด้วย logical coordinate mapping ใน `SSD2119_SetWindow()`:

```text
logical x,y -> physical x,y แบบ 180 degree
```

เมื่อนำไปใช้ TouchGFX มี 2 ทางเลือก:

1. คง mapping 180 degree ไว้ใน SSD2119 driver
2. ให้ TouchGFX ออกแบบหน้าจอตาม orientation จริง แล้วถอด mapping ออก

แนะนำช่วงแรกให้คง mapping ใน driver เพราะ hardware ที่ทดสอบอยู่แสดงว่าจอถูก mount กลับ 180 องศา

## 10. ขั้นตอน Port แนะนำ

1. Clone หรือ copy CubeIDE SSD2119 project ปัจจุบันเป็น project ใหม่สำหรับ TouchGFX
2. เปิด `.ioc` ด้วย STM32CubeMX/CubeIDE
3. Enable TouchGFX middleware
4. ตั้ง Display Interface เป็น Custom, RGB565, 320x240
5. ตั้ง framebuffer เป็น Partial framebuffer
6. Enable SPI1 TX DMA
7. เพิ่ม/ย้าย driver `ssd2119.c/.h` เข้า project ใหม่
8. เพิ่ม public API สำหรับ `WriteBitmap()` และ transfer busy state
9. Implement TouchGFX custom flush function ให้เรียก SSD2119 driver
10. ทดสอบ blocking SPI ก่อนให้ภาพขึ้นถูกต้อง
11. ตรวจสี RGB565 เช่น BLACK, WHITE, RED, GREEN, BLUE
12. ตรวจ orientation ด้วย text หรือ arrow test
13. เปลี่ยนเป็น SPI DMA
14. เพิ่ม SPI clock ทีละระดับและวัดความเสถียร
15. สร้าง TouchGFX screen test เช่น label, button, gauge, box fill

## 11. การทำให้ขึ้นใน TouchGFX Designer By Partners

เมื่อ project TouchGFX + SSD2119 ใช้งานได้จริงแล้ว สามารถ pack เป็น TouchGFX Board Setup / Application Template package ได้

เป้าหมาย:

```text
ให้ TouchGFX Designer 4.26.1 แสดง board/template ของเราใน Create -> By Partners
```

แนวทาง:

1. เตรียม project ให้ build ผ่าน และ generate code จาก Designer ได้
2. ใส่ driver SSD2119 และ TouchGFX flush code ไว้ใน template
3. ใส่ metadata ของ board/template
4. pack เป็น `.tpa` ด้วย tool ของ TouchGFX Designer
5. copy `.tpa` ไปยัง package folder ของ TouchGFX Designer
6. restart TouchGFX Designer

ตัวอย่าง command โดยประมาณ:

```text
C:\TouchGFX\4.26.1\designer\tgfx.exe pack -d <ProjectFolder>
C:\TouchGFX\4.26.1\designer\tgfx.exe pack -rc -d <ProjectFolder>
```

ตำแหน่ง package local โดยทั่วไป:

```text
C:\TouchGFX\4.26.1\app\packages
```

หมายเหตุ:

```text
เพิ่มเป็น local/custom package บนเครื่องหรือในทีมได้
ถ้าต้องการให้เป็น official online partner package ต้องผ่านช่องทาง partner/ST
```

## 12. สิ่งที่ต้องตรวจสอบบน Hardware

Checklist bring-up:

```text
[ ] SSD2119_Init() ผ่านและจอเปิดติด
[ ] fill BLACK/WHITE/RED/GREEN/BLUE ถูกต้อง
[ ] byte order RGB565 ถูกต้อง
[ ] orientation 180 degree ถูกต้อง
[ ] partial framebuffer block ไม่เหลื่อมตำแหน่ง
[ ] SPI DMA complete callback ทำงาน
[ ] touchgfx::startNewTransfer() ถูกเรียกหลังส่งเสร็จ
[ ] UI ไม่มี tearing มากเกินไป
[ ] SPI speed สูงสุดที่เสถียรถูกบันทึกไว้
```

## 13. สรุป

Driver `SSD2119` ใน project นี้สามารถ modify เพื่อใช้กับ TouchGFX ได้ โดยเปลี่ยนให้เป็น custom display flush driver สำหรับส่ง partial framebuffer block เข้า SSD2119 GRAM ผ่าน SPI

งานหลักที่ต้องทำคือ:

```text
1. เพิ่ม API WriteBitmap/WritePixels สำหรับรับ framebuffer จาก TouchGFX
2. แก้ byte order RGB565 ก่อนส่ง SPI
3. เพิ่ม SPI DMA และ transfer busy callback
4. ตั้ง TouchGFX เป็น Custom Display + Partial Framebuffer
5. ทดสอบสี, orientation, window, DMA, performance
6. เมื่อใช้งานได้แล้ว pack เป็น TouchGFX .tpa เพื่อเพิ่มใน By Partners แบบ local package
```

## 14. Known Good Configuration ล่าสุด

ส่วนนี้เป็นค่าที่ทดสอบแล้วกับ hardware จริง และควรใช้เป็น baseline เมื่อให้ Codex หรือคนอื่นทำซ้ำ

```text
Project root        : C:\TouchGFXProjects\DLC035
MCU board           : NUCLEO-H563ZI / STM32H563ZITx
LCD module          : DLC0350QEM06DT-1
LCD controller      : SSD2119
Display resolution  : 320 x 240
Color depth         : RGB565 / 16 bit
Display interface   : SPI1 command/data + GPIO control
Framebuffer mode    : TouchGFX Partial Buffer - GRAM display
External RAM        : Not used
LTDC/FMC/DMA2D       : Not used for current display path
RTOS                : FreeRTOS / CMSIS RTOS V2
TouchGFX Designer   : 4.26.1
STM32CubeIDE        : 1.18.1
Package version     : DLC035_SSD2119_NUCLEO_H563ZI v0.2.4
```

Known good LCD pin mapping:

```text
PB3  -> SPI1_SCK
PB5  -> SPI1_MOSI -> LCD SDI/SDA/DIN
PB4  -> SPI1_MISO -> optional/readback only
PB7  -> LCD_RST
PG15 -> LCD_DC / RS
PB6  -> LCD_CS, software chip select
```

Known good TouchGFX settings:

```text
TouchGFX interface  : Custom
Resolution          : 320 x 240
Color depth         : 16 bit
Framebuffer         : Partial Buffer
Number of blocks    : 3
Block size          : 2048 bytes
layout_rotation     : 0
```

Known good SSD2119 behavior:

```text
SSD2119_Init(&hspi1) is called from TouchGFXHAL::initialize()
SSD2119_WriteBitmap(...) is called from touchgfxDisplayDriverTransmitBlock()
touchgfxDisplayDriverTransmitActive() returns SSD2119_IsTransferBusy()
Blocking SPI path calls touchgfx::startNewTransfer() after each block transfer
RGB565 byte order is swapped before SPI byte stream transmit
180 degree display mounting is handled in SSD2119_SetWindow()
```

## 15. Debug History และอาการที่เคยพบ

ให้ใช้ตารางนี้เป็นลำดับตรวจปัญหา เมื่อต้อง port ซ้ำหรือหลังจาก Generate Code ใหม่

| อาการ | สาเหตุที่พบ | วิธีแก้ที่ใช้ได้ |
|---|---|---|
| LCD ดำ แต่ไฟ backlight ติด | TouchGFX pipeline ไม่ได้ flush ไป SSD2119 หรือ LCD init ยังไม่ถูกเรียก | ให้ `TouchGFXHAL::initialize()` เรียก `SSD2119_Init(&hspi1)` และตรวจ `touchgfxDisplayDriverTransmitBlock()` |
| Test pattern สีขึ้นถูกต้อง แต่ UI จาก TouchGFX ไม่ขึ้น | ยังมี test pattern/debug drawing ทับ flow ของ TouchGFX หรือ framebuffer ไม่ถูกส่ง | ลบ test pattern ชั่วคราว และให้ partial framebuffer ส่งผ่าน `SSD2119_WriteBitmap()` เท่านั้น |
| UI จาก Designer สี/ภาพเพี้ยน | byte order RGB565 หรือ asset orientation ไม่ตรง | swap byte ก่อนส่ง SPI และตั้ง `layout_rotation` เป็น `0` |
| ภาพขึ้นแต่กลับหัว | physical LCD mount กลับ 180 องศา | ทำ logical-to-physical mapping ใน `SSD2119_SetWindow()` |
| Generate Code แล้ว build fail เรื่อง button | CubeMX สร้าง macro เป็น `BUTTON_*` แต่ code เดิมอ้าง `BUTTON_USER_*` | แก้ `TouchGFX/target/KeySampler.cpp` ให้ใช้ `BUTTON_GPIO_Port` และ `BUTTON_Pin` |
| Generate Code แล้วมี source FMC/NAND/NOR ค้าง | template เดิมมี linked resources ไม่ตรง hardware | ตรวจและแก้ `STM32CubeIDE/.project` หลัง generate |
| Package แสดงใน By Partners แต่ Create แล้ว Download failed | `package.json` ใน `.tpa` มี `MD5sum`/`Size` ไม่ตรงกับ payload zip ด้านใน | คำนวณ MD5/Size จาก `DLC035_SSD2119_NUCLEO_H563ZI.zip` ไม่ใช่จาก `.tpa` |

## 16. เอกสารที่เกี่ยวข้อง

เอกสารนี้เป็น guide หลักสำหรับการ port LCD driver เข้า TouchGFX

เอกสารสรุปสถานะ implementation:

```text
DLC035_TouchGFX_SSD2119_Package\PortDriverSSD2119_TouchGFX_Summary.md
```

เอกสารสร้างและติดตั้ง TouchGFX Board Setup / Application Template package:

```text
DLC035_TouchGFX_SSD2119_Package\DLC035_SSD2119_TouchGFX_Package_Guide.md
```

เมื่อข้อมูลขัดแย้งกัน ให้ยึดลำดับความล่าสุดนี้:

```text
1. Hardware/project ที่ build และ burn ผ่านจริง
2. DLC035_SSD2119_TouchGFX_Package_Guide.md สำหรับเรื่อง package
3. PortDriverSSD2119_TouchGFX_Summary.md สำหรับสถานะ implementation ล่าสุด
4. PortDriverSSD2119toTouchGFX.md สำหรับหลักการและขั้นตอน port แบบละเอียด
```