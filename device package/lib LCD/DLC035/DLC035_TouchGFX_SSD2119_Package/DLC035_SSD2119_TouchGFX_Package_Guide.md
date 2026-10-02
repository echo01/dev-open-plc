# Cleanup Update (2026-10-02)

Historical archives were removed with user approval. scripts/package.ps1
now uses the retained 0.2.7 TPA and package-0.2.7.json directly. No 0.2.5
archive is required. Use -OutputDirectory for isolated verification;
published release files must not be overwritten. Retained historical
release notes document past procedures, not current archive dependencies.
See ../docs/GIT_PREPARATION.md for the tested source-control file policy.

# Current Release 0.2.7

See [RELEASE-0.2.7.md](RELEASE-0.2.7.md). Production packages now skip startup color diagnostics (LCD_COLOR_TEST_HOLD_MS=0U); development source remains unchanged. Historical releases below may enable the test.

# Current Release: 0.2.6

See [RELEASE-0.2.6.md](RELEASE-0.2.6.md). Version 0.2.5 omitted asset converters; do not use it for new projects. Use scripts/package.ps1 and scoped exclusions; never exclude every build directory. Historical content follows.

# Current Release 0.2.5

For the current fixes, reproducible packaging steps, and installation, see [RELEASE-0.2.5.md](RELEASE-0.2.5.md). Use the 0.2.5 TPA and updated installer. Older commands and checksums below are historical. MasterKeepIOState/AFCNTR ENABLE and SSD2119 0x72EF are user hardware-confirmed; preserve both.

---

# DLC035 SSD2119 TouchGFX Package Guide

## Pin Mapping ของ Package v0.2.4

| Signal | STM32H563ZI pin |
|---|---|
| LCD_DC / RS | PG15 |
| LCD_RST | PB7 |
| LCD_CS | PB6 |
| SPI1_SCK | PB3 |
| SPI1_MISO | PB4 |
| SPI1_MOSI | PB5 |

RS คือ Data/Command ส่วน RST คือ Reset. ก่อนสร้าง package ต้องตรวจทั้ง .ioc, Core/Inc/main.h, Core/Src/main.c และ Core/Src/stm32h5xx_hal_msp.c ให้ตรงกับตารางนี้.

เอกสารนี้สรุปวิธีสร้างและติดตั้ง TouchGFX Board Setup / Application Template package สำหรับโปรเจค
`NUCLEO-H563ZI + DLC035 SSD2119 SPI` บน TouchGFX Designer 4.26.1

## เป้าหมายของ package

Package นี้ทำให้ TouchGFX Designer แสดง template ของบอร์ดเราในหน้า:

```text
Create -> By Partners
```

จากนั้นสามารถสร้างโปรเจคใหม่จาก template ได้โดยไม่ต้อง download จาก internet

## ไฟล์ที่ใช้แจกจ่าย

ไฟล์ที่ต้องส่งให้ผู้ใช้งานมีเพียงไฟล์เดียว:

```text
DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
```

ไฟล์ `.tpa` เป็น zip package ของ TouchGFX ซึ่งภายในมีไฟล์สำคัญ:

```text
DLC035_SSD2119_NUCLEO_H563ZI.zip
package.json
```

รายละเอียด:

- `DLC035_SSD2119_NUCLEO_H563ZI.zip` คือ project template payload
- `package.json` คือ metadata ที่ TouchGFX Designer ใช้แสดงชื่อ board, version, resolution, color depth และตรวจสอบ checksum

ไม่ต้องแตกไฟล์ `.tpa` หรือ copy zip ด้านในเองตอนติดตั้งใช้งาน

## โครงสร้าง package ที่ถูกต้อง

โครงสร้างภายใน `.tpa` ต้องเป็นแบบนี้:

```text
DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
|-- DLC035_SSD2119_NUCLEO_H563ZI.zip
`-- package.json
```

ภายใน payload zip ต้องมี project template ที่เปิดใช้งานได้จริง และต้องมีไฟล์ `.touchgfx` อยู่ใน path ที่ตรงกับค่า `PathToDotTouchGFX`

สำหรับ package นี้ใช้:

```json
"PathToDotTouchGFX": "TouchGFX"
```

ดังนั้นภายใน payload zip ต้องมีไฟล์ลักษณะนี้:

```text
TouchGFX/DLC035.touchgfx
```

## Metadata สำคัญใน package.json

ค่าที่ต้องระวังมากที่สุดคือ `MD5sum` และ `Size`

ตัวอย่าง metadata สำหรับ version 0.2.4:

```json
{
  "Meta": {
    "Url": "http://packages.touchgfx.com/V2/DLC035_SSD2119_NUCLEO_H563ZI/DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.zip",
    "MD5sum": "84CD6A87E1A69F97173EBB49021299F3",
    "Size": 33579259,
    "TouchGFXVersion": {
      "Major": 4,
      "Minor": 26,
      "Build": 1
    },
    "PathToDotTouchGFX": "TouchGFX",
    "EmbeddedOs": "FreeRTOS"
  },
  "Data": {
    "Version": {
      "Major": 0,
      "Minor": 2,
      "Build": 3
    },
    "Name": "DLC035_SSD2119_NUCLEO_H563ZI",
    "HumanFriendlyName": "NUCLEO-H563ZI + DLC035 SSD2119 SPI",
    "BoardName": "NUCLEO-H563ZI",
    "Type": "TGAT",
    "Vendor": "Custom",
    "Category": "By Partners",
    "AvailableResolutions": [
      {
        "Height": 240,
        "Width": 320
      }
    ],
    "TargetBpp": [
      16
    ]
  }
}
```

ข้อสำคัญ:

- `MD5sum` ต้องเป็น MD5 ของไฟล์ payload zip ด้านใน คือ `DLC035_SSD2119_NUCLEO_H563ZI.zip`
- `Size` ต้องเป็นขนาด byte ของไฟล์ payload zip ด้านใน
- ห้ามใช้ MD5 หรือ Size ของไฟล์ `.tpa` ทั้งก้อน
- ถ้า `MD5sum` หรือ `Size` ไม่ตรง TouchGFX Designer จะมอง local package เป็น invalid แล้วพยายาม download จาก `Url`

## สาเหตุของปัญหา Download failed

อาการที่พบ:

```text
Download of NUCLEO-H563ZI + DLC035 SSD2119 SPI failed
```

ใน log พบข้อความ:

```text
Wrong MD5Sum: DLC035_SSD2119_NUCLEO_H563ZI-0.2.2
Invalid local package: DLC035_SSD2119_NUCLEO_H563ZI.tpa
Downloading DLC035_SSD2119_NUCLEO_H563ZI.tpa
404 Not Found
```

สาเหตุคือ Designer เจอไฟล์ `.tpa` ในเครื่องแล้ว แต่ validate ไม่ผ่าน เพราะค่า `MD5sum` ใน `package.json` ไม่ตรงกับ payload zip ด้านใน หลังจากนั้น Designer จึง fallback ไป download จาก internet และ fail เพราะ package ของเราไม่ได้อยู่บน server ของ TouchGFX

วิธีแก้คือสร้าง `.tpa` ใหม่โดยให้ `MD5sum` และ `Size` ตรงกับ payload zip ด้านใน

## ขั้นตอนสร้าง package

### 1. เตรียม project template payload

สร้าง zip payload จาก project ที่ใช้งานได้จริง:

```text
DLC035_SSD2119_NUCLEO_H563ZI.zip
```

ภายใน zip ต้องมี source project ครบ รวมถึง:

```text
TouchGFX/DLC035.touchgfx
STM32CubeIDE/
Core/
Drivers/
Middlewares/
```

แนะนำให้ไม่ใส่ build output, cache, temporary files หรือไฟล์ขนาดใหญ่ที่ไม่จำเป็น

### 2. คำนวณ MD5 และ Size ของ payload zip

ใช้ PowerShell:

```powershell
Get-FileHash -Algorithm MD5 -Path .\DLC035_SSD2119_NUCLEO_H563ZI.zip
Get-Item .\DLC035_SSD2119_NUCLEO_H563ZI.zip | Select-Object Length
```

ค่าที่ได้จาก package 0.2.4:

```text
MD5  = 84CD6A87E1A69F97173EBB49021299F3
Size = 33579259
```

นำค่านี้ไปใส่ใน `package.json`

### 3. สร้าง package.json

ตั้งค่า version และ metadata ให้ถูกต้อง:

```text
Data.Version = 0.2.4
Data.Name = DLC035_SSD2119_NUCLEO_H563ZI
Data.Type = TGAT
Data.Category = By Partners
Meta.TouchGFXVersion = 4.26.1
Meta.PathToDotTouchGFX = TouchGFX
Meta.EmbeddedOs = FreeRTOS
```

ตรวจว่าค่า `MD5sum` และ `Size` ตรงกับ payload zip

### 4. Pack เป็น .tpa

นำไฟล์ 2 ไฟล์นี้ไป zip รวมกัน:

```text
DLC035_SSD2119_NUCLEO_H563ZI.zip
package.json
```

แล้วเปลี่ยนนามสกุลจาก `.zip` เป็น `.tpa`

ชื่อไฟล์ผลลัพธ์:

```text
DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
```

ตัวอย่าง PowerShell:

```powershell
$temp = "$env:TEMP\dlc035_tpa"
New-Item -ItemType Directory -Path $temp -Force
Copy-Item .\DLC035_SSD2119_NUCLEO_H563ZI.zip $temp
Copy-Item .\package.json $temp
Compress-Archive -Path "$temp\*" -DestinationPath .\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.zip -Force
Rename-Item .\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.zip .\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
```

## วิธีติดตั้ง package

### 1. ปิด TouchGFX Designer

ต้องปิด Designer ก่อน เพื่อให้เปิดใหม่แล้ว rescan package

### 2. Copy ไฟล์ .tpa ไปที่ packages folder

สำหรับเครื่องนี้:

```text
E:\TouchGFX\4.26.1\app\packages
```

นำไฟล์นี้ไปวาง:

```text
E:\TouchGFX\4.26.1\app\packages\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
```

ถ้าติดตั้ง TouchGFX ไว้ path อื่น ให้ใช้:

```text
<TouchGFX install path>\4.26.1\app\packages
```

### 3. เปิด TouchGFX Designer ใหม่

ไปที่:

```text
Create -> By Partners
```

เลือก:

```text
NUCLEO-H563ZI + DLC035 SSD2119 SPI
```

เลือก version:

```text
v0.2.4
```

แล้วกด `Create`

## Optional: ติดตั้งลง Downloads cache

โดยปกติวาง `.tpa` ใน `app\packages` ก็เพียงพอ

ถ้าต้องการให้ Designer เห็นลักษณะคล้าย package ที่ downloaded และแสดง Available Offline สามารถ copy เพิ่มไปที่:

```text
C:\Users\User\AppData\Roaming\TouchGFX-4.26.1\Downloads\DLC035_SSD2119_NUCLEO_H563ZI\0.2.4
```

ตัวอย่าง:

```text
C:\Users\User\AppData\Roaming\TouchGFX-4.26.1\Downloads\DLC035_SSD2119_NUCLEO_H563ZI\0.2.4\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
```

ขั้นตอนนี้ไม่ควรจำเป็นสำหรับการสร้าง project ถ้า local package validate ผ่านแล้ว

## วิธีตรวจสอบ package

ตรวจว่า `.tpa` มีไฟล์ด้านในถูกต้อง:

```powershell
Add-Type -AssemblyName System.IO.Compression.FileSystem
$tpa = "E:\TouchGFX\4.26.1\app\packages\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa"
$zip = [IO.Compression.ZipFile]::OpenRead($tpa)
$zip.Entries | Select-Object FullName, Length
$zip.Dispose()
```

ควรเห็น:

```text
DLC035_SSD2119_NUCLEO_H563ZI.zip
package.json
```

ตรวจ checksum ของ payload ภายใน `.tpa`:

```powershell
Add-Type -AssemblyName System.IO.Compression.FileSystem
$tpa = "E:\TouchGFX\4.26.1\app\packages\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa"
$zip = [IO.Compression.ZipFile]::OpenRead($tpa)
$entry = $zip.GetEntry("DLC035_SSD2119_NUCLEO_H563ZI.zip")
$md5 = [System.Security.Cryptography.MD5]::Create()
$stream = $entry.Open()
$hashBytes = $md5.ComputeHash($stream)
$hash = ([BitConverter]::ToString($hashBytes)).Replace("-", "")
$stream.Dispose()
$md5.Dispose()
$zip.Dispose()
$hash
```

ค่าที่ได้ต้องตรงกับ `Meta.MD5sum` ใน `package.json`

## Log ที่ใช้ตรวจปัญหา

TouchGFX Designer log อยู่ที่:

```text
C:\Users\User\AppData\Roaming\TouchGFX-4.26.1\TouchGFXDesigner.log
```

ถ้าเจอ:

```text
Wrong MD5Sum
Invalid local package
Downloading ... .tpa
404 Not Found
```

ให้ตรวจ `MD5sum` และ `Size` ใน `package.json` อีกครั้ง โดยต้องเทียบกับ payload zip ด้านใน `.tpa`

## สรุปไฟล์สุดท้าย

ไฟล์ package ที่ใช้งานได้:

```text
DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
```

ตำแหน่งติดตั้งหลัก:

```text
E:\TouchGFX\4.26.1\app\packages
```

เมื่อเปิด TouchGFX Designer 4.26.1 ใหม่ จะสามารถสร้าง project จาก:

```text
Create -> By Partners -> NUCLEO-H563ZI + DLC035 SSD2119 SPI -> v0.2.4
```

## Reproduce Checklist สำหรับ Codex หรือผู้พัฒนา

เมื่อจะสร้าง package ใหม่ ให้ทำตาม checklist นี้ตามลำดับ ห้ามข้ามขั้น MD5/Size

```text
[ ] 1. เปิด project ต้นทาง C:\TouchGFXProjects\DLC035
[ ] 2. ตรวจว่า TouchGFX Designer Generate Code แล้ว project ยัง build ผ่าน
[ ] 3. ตรวจว่า LCD hardware แสดง UI จาก TouchGFX ได้จริง
[ ] 4. ลบ/ไม่รวม build output, cache, temporary files จาก payload zip
[ ] 5. สร้าง payload zip ชื่อ DLC035_SSD2119_NUCLEO_H563ZI.zip
[ ] 6. ตรวจว่าภายใน payload zip มี TouchGFX/DLC035.touchgfx
[ ] 7. คำนวณ MD5 ของ payload zip
[ ] 8. คำนวณ Size byte ของ payload zip
[ ] 9. เขียน package.json โดยใส่ MD5/Size ของ payload zip
[ ] 10. zip package.json + payload zip เป็นไฟล์เดียว
[ ] 11. rename zip package เป็น .tpa
[ ] 12. copy .tpa ไปที่ E:\TouchGFX\4.26.1\app\packages
[ ] 13. ปิดและเปิด TouchGFX Designer ใหม่
[ ] 14. Create project จาก By Partners
[ ] 15. เปิด project ที่สร้างใหม่และ Generate Code
[ ] 16. Build ใน STM32CubeIDE
[ ] 17. Burn/Debug บน board และตรวจ LCD
```

## Known Good Values สำหรับ version 0.2.4

ใช้ชุดข้อมูลนี้เป็น baseline ถ้าต้องตรวจว่ามีอะไรเปลี่ยนไปหรือไม่

```text
TPA file          : DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
Payload zip       : DLC035_SSD2119_NUCLEO_H563ZI.zip
Payload MD5       : 84CD6A87E1A69F97173EBB49021299F3
Payload Size      : 33579259
TouchGFX version  : 4.26.1
Install path      : E:\TouchGFX\4.26.1\app\packages
Designer log      : C:\Users\User\AppData\Roaming\TouchGFX-4.26.1\TouchGFXDesigner.log
Template category : By Partners
Template name     : NUCLEO-H563ZI + DLC035 SSD2119 SPI
Package Data.Name : DLC035_SSD2119_NUCLEO_H563ZI
Package Type      : TGAT
Resolution        : 320 x 240
TargetBpp         : 16
Embedded OS       : FreeRTOS
```

## คำสั่งสร้าง package แบบทำซ้ำได้

ตัวอย่างนี้ถือว่าอยู่ใน folder:

```text
C:\TouchGFXProjects\DLC035\DLC035_TouchGFX_SSD2119_Package
```

และมีไฟล์พร้อมแล้ว:

```text
DLC035_SSD2119_NUCLEO_H563ZI.zip
package.json
```

สร้าง `.tpa`:

```powershell
$ErrorActionPreference = "Stop"
$pkgDir = "C:\TouchGFXProjects\DLC035\DLC035_TouchGFX_SSD2119_Package"
$payload = Join-Path $pkgDir "DLC035_SSD2119_NUCLEO_H563ZI.zip"
$tpa = Join-Path $pkgDir "DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa"

$payloadItem = Get-Item -LiteralPath $payload
$payloadMd5 = (Get-FileHash -Algorithm MD5 -LiteralPath $payload).Hash.ToUpperInvariant()

Write-Host "Payload MD5  = $payloadMd5"
Write-Host "Payload Size = $($payloadItem.Length)"

# ต้องนำ $payloadMd5 และ $payloadItem.Length ไปใส่ใน package.json ก่อน pack

$tmpRoot = Join-Path $env:TEMP ("dlc035_tpa_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $tmpRoot | Out-Null
Copy-Item -LiteralPath $payload -Destination (Join-Path $tmpRoot "DLC035_SSD2119_NUCLEO_H563ZI.zip")
Copy-Item -LiteralPath (Join-Path $pkgDir "package.json") -Destination (Join-Path $tmpRoot "package.json")

$tmpZip = Join-Path $env:TEMP ("DLC035_SSD2119_NUCLEO_H563ZI-0.2.4_" + [guid]::NewGuid().ToString("N") + ".zip")
Compress-Archive -Path (Join-Path $tmpRoot "*") -DestinationPath $tmpZip -CompressionLevel Optimal
Move-Item -LiteralPath $tmpZip -Destination $tpa -Force
Remove-Item -LiteralPath $tmpRoot -Recurse -Force
```

ติดตั้ง:

```powershell
$installDir = "E:\TouchGFX\4.26.1\app\packages"
Copy-Item -LiteralPath "C:\TouchGFXProjects\DLC035\DLC035_TouchGFX_SSD2119_Package\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa" `
  -Destination (Join-Path $installDir "DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa") `
  -Force
```

## Decision Tree เมื่อติดตั้งแล้วใช้งานไม่ได้

### ไม่เห็น template ใน By Partners

ตรวจ:

```text
1. วาง .tpa ถูก path หรือไม่
2. ปิดและเปิด Designer ใหม่แล้วหรือไม่
3. package.json มี Data.Type = TGAT หรือไม่
4. package.json มี Data.Category = By Partners หรือไม่
5. TouchGFXVersion ตรงกับ Designer 4.26.1 หรือไม่
```

### เห็น template แต่กด Create แล้ว Download failed

ตรวจ log:

```text
C:\Users\User\AppData\Roaming\TouchGFX-4.26.1\TouchGFXDesigner.log
```

ถ้าเจอ:

```text
Wrong MD5Sum
Invalid local package
Downloading ... .tpa
404 Not Found
```

ให้แก้:

```text
1. แตก/เปิด .tpa
2. หา payload zip ด้านใน
3. คำนวณ MD5 ของ payload zip
4. คำนวณ Size byte ของ payload zip
5. แก้ package.json ให้ตรง
6. pack .tpa ใหม่
7. restart Designer
```

### Create ผ่าน แต่ project เปิดไม่ได้

ตรวจ:

```text
1. PathToDotTouchGFX ใน package.json ตรงกับ path จริงหรือไม่
2. ภายใน payload zip มี TouchGFX/DLC035.touchgfx หรือไม่
3. payload zip ไม่ได้ซ้อน folder เกิน เช่น DLC035/DLC035/TouchGFX
4. ไฟล์ project จาก CubeIDE ยังอยู่ครบหรือไม่
```

### Generate Code แล้ว build fail

ตรวจไฟล์ที่มักถูก generate ทับ:

```text
TouchGFX/target/TouchGFXHAL.cpp
Core/Src/app_freertos.c
TouchGFX/target/KeySampler.cpp
STM32CubeIDE/.project
Core/Inc/main.h
Core/Src/main.c
```

โดยเฉพาะ:

```text
BUTTON_GPIO_Port / BUTTON_Pin
SSD2119_Init(&hspi1)
SSD2119_WriteBitmap(...)
touchgfx::startNewTransfer()
layout_rotation = 0
```

## ความสัมพันธ์กับเอกสารอื่น

เอกสารนี้ครอบคลุมเฉพาะการสร้างและติดตั้ง package

สำหรับการ port driver และแก้ TouchGFX display path ให้อ่าน:

```text
..\PortDriverSSD2119toTouchGFX.md
```

สำหรับสรุปสถานะ implementation ล่าสุดให้อ่าน:

```text
PortDriverSSD2119_TouchGFX_Summary.md
```
## ติดตั้งบนเครื่องอื่นแล้วเห็น template แต่ Create แล้ว Download failed

อาการนี้เกิดได้แม้ไฟล์ `.tpa` จะถูก copy ไปที่ `app\packages` แล้ว เพราะ TouchGFX Designer แยกพฤติกรรมเป็น 2 ช่วง:

```text
1. หน้า Create / By Partners
   Designer อ่าน metadata จาก local package แล้วแสดง card ได้

2. ตอนกด Create
   Designer ต้องหา package payload ที่ validate ผ่าน หรือหา cache ใน AppData Downloads
```

ถ้าเครื่องปลายทางมีเฉพาะไฟล์:

```text
<TouchGFX>\4.26.1\app\packages\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
```

บางกรณี Designer จะยังพยายามเรียกชื่อ fallback หรือ cache เช่น:

```text
DLC035_SSD2119_NUCLEO_H563ZI.tpa
%APPDATA%\TouchGFX-4.26.1\Downloads\DLC035_SSD2119_NUCLEO_H563ZI\0.2.4\...
```

ถ้าไม่พบ หรือ validate ไม่ผ่าน Designer จะ fallback ไป download จาก `Meta.Url` ใน `package.json` แล้ว fail เพราะ package นี้เป็น local/custom package ไม่ได้อยู่บน server ของ TouchGFX

### วิธีติดตั้งแบบแนะนำสำหรับเครื่องอื่น

ให้ส่ง folder package ที่มีอย่างน้อย 2 ไฟล์นี้:

```text
DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
Install_DLC035_TouchGFX_Package.ps1
```

บนเครื่องปลายทางให้ปิด TouchGFX Designer ก่อน แล้วเปิด PowerShell ใน folder เดียวกับไฟล์ `.tpa` จากนั้นรัน:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\Install_DLC035_TouchGFX_Package.ps1 -TouchGFXRoot "C:\TouchGFX\4.26.1"
```

ถ้า TouchGFX ติดตั้งอยู่ที่ drive อื่น เช่น `E:` ให้รัน:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\Install_DLC035_TouchGFX_Package.ps1 -TouchGFXRoot "E:\TouchGFX\4.26.1"
```

สคริปต์จะติดตั้งไฟล์ทั้งหมดนี้ให้:

```text
<TouchGFXRoot>\app\packages\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
<TouchGFXRoot>\app\packages\DLC035_SSD2119_NUCLEO_H563ZI.tpa
%APPDATA%\TouchGFX-4.26.1\Downloads\DLC035_SSD2119_NUCLEO_H563ZI\0.2.4\DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
%APPDATA%\TouchGFX-4.26.1\Downloads\DLC035_SSD2119_NUCLEO_H563ZI\0.2.4\DLC035_SSD2119_NUCLEO_H563ZI.tpa
```

หลังจากนั้นให้เปิด TouchGFX Designer 4.26.1 ใหม่ แล้วไปที่:

```text
Create -> By Partners -> NUCLEO-H563ZI + DLC035 SSD2119 SPI -> v0.2.4 -> Create
```

### ถ้ายัง Download failed บนเครื่องอื่น

ให้เปิด log นี้บนเครื่องปลายทาง:

```text
%APPDATA%\TouchGFX-4.26.1\TouchGFXDesigner.log
```

ตรวจ keyword:

```text
DLC035_SSD2119_NUCLEO_H563ZI
Wrong MD5Sum
Invalid local package
Downloading
404 Not Found
```

ถ้ามี `Wrong MD5Sum` ให้ตรวจว่าไฟล์ `.tpa` ที่ copy ไปเป็นตัวล่าสุดจริง และไม่ได้ถูกแก้ไขระหว่างย้ายไฟล์

ถ้ามี `Downloading ... 404 Not Found` แต่ไม่มี `Wrong MD5Sum` ให้ติดตั้ง fallback/cache path ด้วยสคริปต์ด้านบน เพราะเครื่องนั้นยังไม่มี local cache ที่ Designer ใช้ตอน Create
