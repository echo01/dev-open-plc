# Build Your Own OpenPLC Device Package

This guide summarizes how to create an OpenPLC Editor VPP device package based on the existing STM32 community package.

Example target in this guide:

```text
STM32 NUCLEO-G071RB
MCU: STM32G071RBT6
Arduino core: STMicroelectronics:stm32
FQBN: STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G071RB
```

## 1. Existing Package Structure

Example package:

```text
com.openplc.stm32-community
+-- manifest.json
+-- signature.json
+-- assets
|   +-- logo.png
|   +-- boards
|       +-- blackpill-stm32f411.jpg
|       +-- bluepill-stm32f103.png
|       +-- generic.png
|       +-- nucleo-stm32f446.png
+-- hal
|   +-- arduino
|       +-- stm32_f103cb.cpp
|       +-- stm32_f411ce.cpp
|       +-- stm32_f446zet_nucleo.cpp
+-- screens
    +-- modbus.json
```

คำอธิบายภาษาไทย:

- manifest.json คือไฟล์หลักของ package ใช้ประกาศชื่อ package, รายการ device, target board, HAL, pin เริ่มต้น และหน้าตั้งค่าเพิ่มเติม
- signature.json คือไฟล์ลายเซ็น ใช้ตรวจสอบว่า package ถูกแก้ไขหลังจาก sign หรือไม่
- assets/ เก็บรูป logo และรูป board ที่แสดงใน UI ของ OpenPLC Editor
- hal/arduino/ เก็บไฟล์ C++ HAL ที่เชื่อม OpenPLC runtime เข้ากับ GPIO, ADC และ PWM ของบอร์ด
- screens/ เก็บไฟล์ JSON สำหรับสร้างหน้าตั้งค่าเพิ่มเติม เช่นหน้า Modbus

หมายเหตุเรื่องตัวอักษร: ไฟล์นี้ควรบันทึกเป็น UTF-8 และตัวอย่างผังโฟลเดอร์ใช้ตัวอักษร ASCII เช่น +-- และ | เพื่อหลีกเลี่ยงปัญหาภาษาไทยหรือสัญลักษณ์ tree กลายเป็นตัวอ่านไม่ได้บน Windows terminal บางชุด
A VPP package is installed from a `.vpp` file. The `.vpp` file is a zip archive whose root contains:

```text
manifest.json
signature.json
assets/...
hal/...
screens/...
```

Do not zip a parent folder that contains those files. The files must be at the archive root.

## 2. Important Files

### `manifest.json`

This is the main package description. It defines:

- package metadata
- device list
- Arduino CLI target
- board image
- board specs
- HAL source file
- default pin mapping
- optional configuration screens
- optional debugger connection rules

### `hal/arduino/*.cpp`

This file connects OpenPLC runtime buffers to Arduino GPIO/ADC/PWM functions.

The build pipeline generates these macros in `defines.h`:

```cpp
PINMASK_DIN
PINMASK_AIN
PINMASK_DOUT
PINMASK_AOUT
NUM_DISCRETE_INPUT
NUM_ANALOG_INPUT
NUM_DISCRETE_OUTPUT
NUM_ANALOG_OUTPUT
```

The HAL file reads those macros and uses them to call:

```cpp
pinMode()
digitalRead()
digitalWrite()
analogRead()
analogWrite()
```

### `screens/modbus.json`

This defines the Modbus configuration UI shown in the Device editor.

The STM32 community package already has a reusable Modbus screen. You can copy it unchanged for a first test.

### `signature.json`

OpenPLC Editor verifies package signatures during install.

In the current editor code:

```ts
const REQUIRE_SIGNATURE = true
```

That means an unsigned package or modified signed package will fail to install.

For local development only, you can temporarily disable signature verification in:

```text
src/backend/editor/package-manager/package-manager-module.ts
```

Change:

```ts
const REQUIRE_SIGNATURE = true
```

to:

```ts
const REQUIRE_SIGNATURE = false
```

Then run the app in development mode:

```powershell
npm.cmd run dev
```

Do not keep this disabled for production.

## 3. Create a New Package Folder

Create a new folder:

```text
device package/com.yourname.stm32g071rb
```

Recommended structure:

```text
com.yourname.stm32g071rb
+-- manifest.json
+-- signature.json
+-- assets
|   +-- logo.png
|   +-- boards
|       +-- nucleo-stm32g071rb.png
+-- hal
|   +-- arduino
|       +-- stm32_g071rb_nucleo.cpp
+-- screens
    +-- modbus.json
```

คำอธิบายภาษาไทย:

- สร้าง folder package ใหม่หนึ่งตัวต่อหนึ่งกลุ่ม device ที่ต้องการติดตั้ง
- ชื่อ folder ควรตรงกับ package.id เช่น com.yourname.stm32g071rb
- เวลา zip เป็น .vpp ต้อง zip ไฟล์และ folder ที่อยู่ด้านใน package folder ไม่ใช่ zip folder แม่ทั้งก้อน
- ในไฟล์ .vpp ต้องเห็น manifest.json อยู่ที่ root ของ archive ทันที ไม่ใช่อยู่ซ้อนใน folder อีกชั้น
For the first local test, copy these from the STM32 community package:

```text
assets/logo.png
assets/boards/generic.png
screens/modbus.json
```

You can rename `generic.png` to:

```text
assets/boards/nucleo-stm32g071rb.png
```

## 4. Example `manifest.json`

Start with a small package that contains only one device.

```json
{
  "formatVersion": "1.0",
  "package": {
    "id": "com.yourname.stm32g071rb",
    "name": "STM32G071RB boards",
    "version": "0.1.0",
    "vendor": {
      "name": "Your Name",
      "url": "https://example.com",
      "logo": "assets/logo.png"
    },
    "description": "OpenPLC HAL support for STM32 NUCLEO-G071RB.",
    "license": "GPL-3.0",
    "minEditorVersion": "4.1.4",
    "inDevelopment": true
  },
  "devices": [
    {
      "id": "nucleo-stm32-g071rb",
      "name": "NUCLEO STM32-G071RB",
      "category": "Development Board",
      "preview": "assets/boards/nucleo-stm32g071rb.png",
      "target": {
        "type": "arduino-cli",
        "core": "STMicroelectronics:stm32",
        "platform": "STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G071RB",
        "boardManagerUrl": "https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json"
      },
      "specs": {
        "CPU": "STM32G071RBT6 ARM Cortex-M0+ at 64MHz",
        "RAM": "36 KB",
        "Flash": "128 KB",
        "Digital Pins": "TBD",
        "Analog Pins": "TBD",
        "PWM Pins": "TBD",
        "WiFi": "No",
        "Bluetooth": "No",
        "Ethernet": "No"
      },
      "hal": {
        "type": "arduino-hal",
        "source": "hal/arduino/stm32_g071rb_nucleo.cpp",
        "compilerFlags": {
          "c_flags": ["-MMD", "-c", "-Wno-incompatible-pointer-types"],
          "cxx_flags": ["-fexceptions"]
        }
      },
      "defaults": {
        "pins": {
          "defaultDin": ["PA0", "PA1"],
          "defaultDout": ["PA5"],
          "defaultAin": ["PA0"],
          "defaultAout": []
        }
      },
      "screens": {
        "Modbus": "screens/modbus.json"
      },
      "debug": {
        "channels": [
          {
            "label": "Modbus RTU",
            "channel": "rtu",
            "enabledWhen": {
              "$ref": "screens.modbus_rtu.enabled"
            },
            "params": {
              "port": {
                "$ref": "configuration.communicationPort",
                "required": "No serial port selected. Pick a port in the Board Settings screen."
              },
              "baudRate": {
                "$ref": "screens.modbus_rtu.rtu_baud_rate",
                "default": "115200",
                "as": "number"
              },
              "slaveId": {
                "$ref": "screens.modbus_rtu.rtu_slave_id",
                "default": 1,
                "as": "number"
              }
            }
          }
        ],
        "messages": {
          "noneEnabled": {
            "title": "Modbus Required",
            "body": "Modbus RTU must be enabled in the Modbus screen for the debugger to connect."
          }
        }
      }
    }
  ]
}
```

## 5. Example HAL File

Create:

```text
hal/arduino/stm32_g071rb_nucleo.cpp
```

Example:

```cpp
#include <stdlib.h>

extern "C" {
#include "openplc.h"
}

#include "Arduino.h"
#include "defines.h"

// OpenPLC HAL for STM32 NUCLEO-G071RB.
//
// Actual pins are provided by the OpenPLC pin mapping table and converted
// into PINMASK_* macros in defines.h.

uint8_t pinMask_DIN[] = {PINMASK_DIN};
uint8_t pinMask_AIN[] = {PINMASK_AIN};
uint8_t pinMask_DOUT[] = {PINMASK_DOUT};
uint8_t pinMask_AOUT[] = {PINMASK_AOUT};

void hardwareInit()
{
    analogReadResolution(12);
    analogWriteResolution(12);

    for (int i = 0; i < NUM_DISCRETE_INPUT; i++)
    {
        pinMode(pinMask_DIN[i], INPUT);
    }

    for (int i = 0; i < NUM_ANALOG_INPUT; i++)
    {
        pinMode(pinMask_AIN[i], INPUT);
    }

    for (int i = 0; i < NUM_DISCRETE_OUTPUT; i++)
    {
        pinMode(pinMask_DOUT[i], OUTPUT);
    }

    for (int i = 0; i < NUM_ANALOG_OUTPUT; i++)
    {
        pinMode(pinMask_AOUT[i], OUTPUT);
    }
}

void updateInputBuffers()
{
    for (int i = 0; i < NUM_DISCRETE_INPUT; i++)
    {
        if (bool_input[i / 8][i % 8] != NULL)
        {
            *bool_input[i / 8][i % 8] = digitalRead(pinMask_DIN[i]);
        }
    }

    for (int i = 0; i < NUM_ANALOG_INPUT; i++)
    {
        if (int_input[i] != NULL)
        {
            *int_input[i] = analogRead(pinMask_AIN[i]) * 16;
        }
    }
}

void updateOutputBuffers()
{
    for (int i = 0; i < NUM_DISCRETE_OUTPUT; i++)
    {
        if (bool_output[i / 8][i % 8] != NULL)
        {
            digitalWrite(pinMask_DOUT[i], *bool_output[i / 8][i % 8]);
        }
    }

    for (int i = 0; i < NUM_ANALOG_OUTPUT; i++)
    {
        if (int_output[i] != NULL)
        {
            analogWrite(pinMask_AOUT[i], *int_output[i] / 16);
        }
    }
}
```

If STM32G071RB compile fails because of `analogWriteResolution()`, remove that line and test again.

If PWM output range is wrong, test these output scale options:

```cpp
analogWrite(pinMask_AOUT[i], *int_output[i] / 16);
```

or:

```cpp
analogWrite(pinMask_AOUT[i], *int_output[i] / 256);
```

The existing STM32 package uses both patterns on different boards.

## 6. How Pin Mapping Works

The `defaults.pins` values are only initial values shown in the UI.

When the project builds, the active pin mapping becomes C macros:

```cpp
#define PINMASK_DIN PA0, PA1
#define PINMASK_AIN PA0
#define PINMASK_DOUT PA5
#define PINMASK_AOUT
#define NUM_DISCRETE_INPUT 2
#define NUM_ANALOG_INPUT 1
#define NUM_DISCRETE_OUTPUT 1
#define NUM_ANALOG_OUTPUT 0
```

Then the HAL reads those macros:

```cpp
uint8_t pinMask_DIN[] = {PINMASK_DIN};
```

So the HAL should usually stay generic and should not hardcode a fixed board pin list.

## 7. Build `.vpp` File

From inside your package folder:

```powershell
cd "D:\Product\OpenPLC\openplc-editor-development\device package\com.yourname.stm32g071rb"
Compress-Archive -Path manifest.json,signature.json,assets,hal,screens -DestinationPath ..\com.yourname.stm32g071rb.vpp -Force
```

If you disabled signature verification for local testing, `signature.json` can be a copied placeholder file, but the import may still read it if verification is enabled.

For proper signed packages, `signature.json` must contain:

- `formatVersion`
- `alg: ed25519`
- `keyId`
- `packageId`
- `version`
- `signedAt`
- `files` hash map
- `signature`

The editor verifies every file hash and rejects extra, missing, or changed files.

## 8. Install in OpenPLC Editor

1. Run the editor:

```powershell
npm.cmd run dev
```

2. Open Package Manager.

3. Click `+`.

4. Choose `Add from file...`.

5. Select:

```text
device package/com.yourname.stm32g071rb.vpp
```

6. After install, open Device Configuration.

7. Search for:

```text
NUCLEO STM32-G071RB
```

## 9. Common Problems

### Install fails: signature error

Cause:

```text
REQUIRE_SIGNATURE = true
```

Fix for local development:

```ts
const REQUIRE_SIGNATURE = false
```

Then restart the app with:

```powershell
npm.cmd run dev
```

### Board does not appear in dropdown

Check:

- package installed successfully
- `registry.json` includes your package
- `manifest.json` has at least one item in `devices`
- `device.name` is unique and readable
- package folder has `manifest.json`

Installed packages are stored at:

```text
C:\Users\User\AppData\Roaming\open-plc-editor\packages
```

Registry:

```text
C:\Users\User\AppData\Roaming\open-plc-editor\packages\registry.json
```

### Build fails: board not found

Check `target.platform`.

For NUCLEO-G071RB:

```text
STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G071RB
```

Also check the STM32 board manager URL:

```text
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
```

### Build fails in HAL

Check:

- pin names are valid for STM32duino, such as `PA0`, `PA1`, `PB5`
- `analogWriteResolution()` is supported by the selected core
- `analogReadResolution()` is supported by the selected core
- selected analog output pins are PWM-capable

## 10. Minimum Checklist

Before testing install:

- [ ] Folder has `manifest.json`
- [ ] Folder has `hal/arduino/stm32_g071rb_nucleo.cpp`
- [ ] Folder has `screens/modbus.json`
- [ ] Folder has board image referenced by `preview`
- [ ] `package.id` is unique
- [ ] `device.id` is unique
- [ ] `device.name` is what you want to see in the dropdown
- [ ] `target.platform` is correct
- [ ] signature verification is handled
- [ ] package is zipped with files at archive root
## SIGNATURE Details

OpenPLC device packages use a signed-package model. The editor does not just check that `signature.json` exists; it verifies that every file in the package matches the signed file hash list.

Verification code:

```text
src/backend/shared/utils/vpp/verify-package-signature.ts
```

Trusted public keys:

```text
src/backend/shared/utils/vpp/trusted-keys.ts
```

The official STM32 community package uses:

```json
"keyId": "openplc-2026"
```

That key id maps to a PUBLIC KEY only. The private key is not in this repository. The source comment says the private key lives only in the OpenPLC package signing pipeline / CI secret.

Important consequences:

- You can verify official packages with the public key.
- You cannot create a new valid official signature with the public key.
- You cannot copy `signature.json` from another package and reuse it.
- If you edit any file in a signed package, the signature becomes invalid.

A valid `signature.json` signs this payload:

```json
{
  "formatVersion": "1.0",
  "alg": "ed25519",
  "keyId": "openplc-2026",
  "packageId": "com.openplc.stm32-community",
  "version": "1.0.0",
  "signedAt": "2026-06-05T13:36:25.324Z",
  "files": {
    "manifest.json": "sha256...",
    "hal/arduino/example.cpp": "sha256..."
  },
  "signature": "base64-ed25519-signature"
}
```

The `files` object must contain every regular file in the package except top-level `signature.json`.

The editor rejects the package when any of these happens:

- `signature.json` is missing
- `signature.json` is malformed
- `keyId` is not listed in `TRUSTED_PACKAGE_KEYS`
- signature verification fails
- a listed file hash does not match
- a listed file is missing
- an extra unsigned file is present

### Local development option

For local testing, the simplest path is to disable verification temporarily:

```ts
const REQUIRE_SIGNATURE = false
```

File:

```text
src/backend/editor/package-manager/package-manager-module.ts
```

Then run:

```powershell
npm.cmd run dev
```

After testing, change it back:

```ts
const REQUIRE_SIGNATURE = true
```

Do not keep verification disabled for production builds.

### Using your own signing key

For a real custom signed package, create your own Ed25519 key pair:

- keep the private key outside the repo
- add the public key to `TRUSTED_PACKAGE_KEYS`
- set your own `keyId`, for example `wp-local-2026`
- generate `signature.json` with your private key

Example trusted key entry:

```ts
export const TRUSTED_PACKAGE_KEYS: Record<string, string> = {
  'openplc-2026': `-----BEGIN PUBLIC KEY-----
...
-----END PUBLIC KEY-----
`,
  'wp-local-2026': `-----BEGIN PUBLIC KEY-----
YOUR_PUBLIC_KEY_HERE
-----END PUBLIC KEY-----
`,
}
```

The signing script must match OpenPLC verification exactly:

1. Walk all regular files recursively.
2. Use POSIX-style relative paths with `/`.
3. Exclude top-level `signature.json`.
4. Hash each file with SHA-256 lower-case hex.
5. Build the signature payload.
6. Canonicalize JSON by sorting object keys recursively.
7. Sign the canonical payload with Ed25519.
8. Store the signature as base64 in `signature.json`.

For official distribution, the package must be signed by the OpenPLC package signing pipeline or by a key trusted by the editor build you distribute.
