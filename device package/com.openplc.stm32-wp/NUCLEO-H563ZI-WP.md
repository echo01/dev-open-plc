# NUCLEO-H563ZI WP Board Package Notes

เอกสารนี้สรุปข้อมูล board model `NUCLEO-H563ZI WP` ที่เพิ่มเข้า package `com.openplc.stm32-wp` เพื่อใช้ทดสอบกับ OpenPLC Editor และ STM32duino Arduino core.

## 1. Board Identity

| รายการ | ค่า |
| --- | --- |
| Package | `com.openplc.stm32-wp` |
| Board ID | `nucleo-h563zi-wp` |
| Board Name | `NUCLEO-H563ZI WP` |
| MCU | `STM32H563ZI` |
| Arduino Core | `STMicroelectronics:stm32` |
| Arduino FQBN | `STMicroelectronics:stm32:Nucleo_144:pnum=NUCLEO_H563ZI,upload_method=swdMethod` |
| Flash | `2 MB` |
| RAM | `640 KB` |
| CPU | ARM Cortex-M33, up to 250 MHz |

STM32duino รองรับ NUCLEO-H563ZI ในกลุ่ม Nucleo-144 ดังนั้น board นี้สามารถใช้กับ OpenPLC ผ่าน Arduino CLI ได้ หากติดตั้ง STM32duino core แล้ว

```text
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
```

## 2. OpenPLC Online Debug

OpenPLC Online Debug สำหรับ board นี้กำหนดให้ใช้ `USART3` ผ่าน ST-LINK VCP โดยใน STM32duino จะเป็น default `Serial` instance

| Function | MCU Pin | Arduino |
| --- | --- | --- |
| USART3_TX | `PD8` | `Serial TX` |
| USART3_RX | `PD9` | `Serial RX` |

หมายเหตุ: ถ้ามองจากฝั่ง ST-LINK/PC ชื่อ TX/RX อาจกลับทิศกับฝั่ง MCU ให้ยึดการกำหนดใน firmware เป็นฝั่ง MCU คือ `PD8 = TX`, `PD9 = RX`

## 3. USART2 / RS485

`USART2` ถูกกันไว้สำหรับ RS485 field/device communication แยกจาก Online Debug

| Function | Pin | Arduino |
| --- | --- | --- |
| USART2_TX | `PD5` | `Serial2 TX` |
| USART2_RX | `PD6` | `Serial2 RX` |
| RS485 DE/RE | `PD4` | GPIO control |

ใน manifest เพิ่ม define สำหรับเปิดใช้งาน `Serial2`:

```c
ENABLE_HWSERIAL2
PIN_SERIAL2_RX PD6
PIN_SERIAL2_TX PD5
```

ข้อจำกัดปัจจุบัน: OpenPLC firmware มาตรฐานมี Modbus RTU channel หลักตัวเดียวสำหรับ debug/Modbus server หากต้องการให้ debug อยู่ USART3 และให้ USART2 ทำ Modbus user/device พร้อมกัน ต้องเพิ่ม firmware logic แยกในขั้นตอนถัดไป

## 4. LCD TFT SSD2119

LCD SSD2119 ใช้ `SPI1` แยกจาก AD7792

| Function | Pin |
| --- | --- |
| SPI1_SCK | `PA5` |
| SPI1_MOSI | `PB5` |
| SPI1_MISO | `PG9` |
| LCD_CS | `PD14` |
| LCD_DC | `PD15` |
| LCD_RST | `PF3` |

SSD2119 ใช้ส่งข้อมูลไปจอเป็นหลัก ดังนั้น `PG9` ถูกระบุไว้เป็น MISO ของ SPI1 แต่ driver จออาจไม่ได้ใช้งานจริง

### LCD OpenPLC Integration

board นี้เปิด LCD ผ่าน optional hook ใน `resources/sources/Baremetal/Baremetal.ino` โดยใช้ define `OPENPLC_LCD_ENABLED` เฉพาะ `NUCLEO-H563ZI WP` เท่านั้น board อื่นที่ไม่มี define นี้จะไม่ include หรือเรียก LCD code

LCD library ถูกวางใน package ที่:

```text
hal/arduino/libraries/OpenPLC_SSD2119
```

Arduino CLI จะได้รับ path นี้ผ่าน `hal.libraries` และติดตั้ง dependency `Adafruit GFX Library` ผ่าน `hal.extraArduinoLibraries`

## 5. Analog Input

กำหนด ADC เป็น single-ended channel เพื่อให้เข้ากับ OpenPLC HAL ปัจจุบันที่ใช้ `analogRead()`

| OpenPLC | Pin | STM32duino ADC Mapping |
| --- | --- | --- |
| AIN0 | `PF11` | ADC1_INP2 |
| AIN1 | `PF12` | ADC1_INP6 |
| AIN2 | `PF13` | ADC2_INP2 |
| AIN3 | `PF14` | ADC2_INP6 |
| AIN4 | `PC2` | ADC1_INP12 |
| AIN5 | `PC3` | ADC1_INP13 |

หากต้องการ differential ADC จริงในอนาคต ไม่ควรใช้ `analogRead()` ธรรมดา ต้องเพิ่ม STM32 HAL/LL ADC driver เฉพาะ

## 6. I2C / FRAM

ใช้ `I2C1` สำหรับ FRAM

| Function | Pin |
| --- | --- |
| I2C1_SCL | `PB8` |
| I2C1_SDA | `PB9` |

## 7. GPIO Mapping

### Digital Inputs

| OpenPLC | Pin |
| --- | --- |
| %IX0.0 | `PF0` |
| %IX0.1 | `PF1` |
| %IX0.2 | `PF2` |
| %IX0.3 | `PE2` |
| %IX0.4 | `PE3` |
| %IX0.5 | `PE4` |
| %IX0.6 | `PE5` |
| %IX0.7 | `PE6` |
| %IX1.0 | `PC13` |

### Test Switch Input

`PC13` ถูกเพิ่มเป็น digital input สำหรับต่อ switch เพื่อทดสอบโปรแกรม Ladder ได้สะดวก

| Function | Pin | OpenPLC |
| --- | --- | --- |
| Test Switch | `PC13` | `%IX1.0` |

### Digital Outputs

| OpenPLC | Pin |
| --- | --- |
| %QX0.0 | `PE8` |
| %QX0.1 | `PE9` |
| %QX0.2 | `PE10` |
| %QX0.3 | `PE11` |
| %QX0.4 | `PE12` |
| %QX0.5 | `PE13` |
| %QX0.6 | `PE14` |
| %QX0.7 | `PE15` |
| %QX1.0 | `PB10` |
| %QX1.1 | `PF4` |
| %QX1.2 | `PG4` |

### Test LED Outputs

`PF4` และ `PG4` เป็น LED output สำหรับเขียนโปรแกรมทดสอบบน NUCLEO-H563ZI ได้สะดวก

| Function | Pin | Note |
| --- | --- | --- |
| LED2 | `PF4` | On-board yellow LED |
| LED3 | `PG4` | On-board red LED |

## 8. AD7792 SPI Bus

เพิ่ม SPI อีก 1 ช่องสำหรับอ่าน AD7792 จำนวน 4 chip โดยใช้ `SPI2`

| Function | Pin |
| --- | --- |
| SPI2_SCK | `PA12` |
| SPI2_MOSI | `PB15` |
| SPI2_MISO | `PB14` |
| AD7792_CS0 | `PA11` |
| AD7792_CS1 | `PB12` |
| AD7792_CS2 | `PA3` |
| AD7792_CS3 | `PB4` |

เลือก SPI2 เพราะไม่ชนกับ LCD SPI1, USART2/USART3, I2C1, ADC และ GPIO output หลัก

## 9. Manifest Defines

Defines ที่เพิ่มใน board model:

```c
WP_STM32H563ZI
ENABLE_HWSERIAL2
PIN_SERIAL2_RX PD6
PIN_SERIAL2_TX PD5
WP_DEBUG_USART Serial
WP_RS485_USART Serial2
WP_RS485_TX PD5
WP_RS485_RX PD6
WP_RS485_DE PD4
WP_LCD_SPI_MOSI PB5
WP_LCD_SPI_MISO PG9
WP_LCD_SPI_SCK PA5
WP_LCD_CS PD14
WP_LCD_DC PD15
WP_LCD_RST PF3
WP_I2C_SCL PB8
WP_I2C_SDA PB9
WP_SWITCH_TEST PC13
WP_LED_YELLOW PF4
WP_LED_RED PG4
WP_AD7792_SPI_MOSI PB15
WP_AD7792_SPI_MISO PB14
WP_AD7792_SPI_SCK PA12
WP_AD7792_CS0 PA11
WP_AD7792_CS1 PB12
WP_AD7792_CS2 PA3
WP_AD7792_CS3 PB4
```

## 10. Files Added

```text
com.openplc.stm32-wp
|-- manifest.json
|-- NUCLEO-H563ZI-WP.md
|-- hal
|   `-- arduino
|       `-- stm32_h563zi_wp.cpp
`-- screens
    `-- modbus.json
```

## 11. Next Steps

1. ติดตั้งหรือ update STM32duino core ให้รองรับ `NUCLEO_H563ZI`
2. Install package `com.openplc.stm32-wp` เข้า OpenPLC Editor แบบ local test
3. เลือก board `NUCLEO-H563ZI WP`
4. ในหน้า Modbus ให้ enable Modbus RTU และเลือก interface เป็น `Serial` สำหรับ Online Debug ผ่าน ST-LINK VCP
5. ทดสอบ compile/upload firmware พื้นฐานก่อน
6. ทดสอบ compile/upload firmware พร้อม LCD hook และตรวจว่า console แสดงการใช้ library `OpenPLC_SSD2119`
7. ทดสอบจอ SSD2119 ว่าแสดงหน้า OpenPLC และ status `QX0.0` ตาม output แรก
8. หากต้องใช้ USART2/RS485 เป็น Modbus user/device แยกจาก debug ต้องเพิ่ม firmware logic เฉพาะสำหรับ `Serial2`

## Upload and Online Debug Routing

- Firmware upload/burn: uses ST-LINK SWD through STM32CubeProgrammer by setting Arduino FQBN option `upload_method=swdMethod`.
- OpenPLC Online Debug: still uses ST-LINK Virtual COM Port through USART3, with MCU pins `PD8 = USART3_TX` and `PD9 = USART3_RX`.
- USART2 remains reserved for RS485/device Modbus, using `PD5 = USART2_TX`, `PD6 = USART2_RX`, and `PD4 = RS485_DE_RE`.

Recommended OpenPLC Editor setup:

```text
Device: NUCLEO-H563ZI WP
Upload/Burn: ST-LINK SWD via STM32CubeProgrammer
Communication Port for Online Debug: STMicroelectronics STLink Virtual COM Port, for example COM9
Modbus RTU screen: enabled, Interface = Serial, Baud Rate = 115200, Slave ID = 1 for Online Debug over USART3/ST-LINK VCP
```

Important for existing projects: verify the Modbus RTU Interface field is Serial. Older projects may still keep Serial1 from the previous package default, which makes MD5 requests time out on COM9/ST-LINK VCP.
