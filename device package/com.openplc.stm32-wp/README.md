# com.openplc.stm32-wp

Custom OpenPLC VPP package for STM32G071RB WP.

## Device

```text
STM32G071RB WP
FQBN: STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_G071RB
Arduino core: STMicroelectronics:stm32
```

## Pin Plan

| Function | Pins |
| --- | --- |
| Analog inputs | PA0, PA1, PA4, PA6, PA7 |
| Digital inputs | PB8, PB2, PB3, PB4, PB5, PB9, PB10 |
| Digital outputs | PB12, PB13, PB14, PB15, PC13, PC14, PC15 |
| USART2 debug/ST-LINK | PA2, PA3 |
| USART1 Modbus RTU | PC4, PC5 |
| I2C1 | SDA PA10, SCL PA9 |
| SPI1 | MOSI PA12, MISO PA11, SCK PD8, CS PD9 |

## Modbus RTU / Debug

This package declares Modbus RTU as the debugger channel.

Use the `Modbus` device screen and enable `Modbus RTU`.

Recommended settings:

```text
Interface: Serial1
Baud Rate: 115200
Slave ID: 1
```

USART1 PC4/PC5 is intended for Modbus RTU communication with external devices.
USART2 PA2/PA3 is reserved for ST-LINK serial debug/logging.

## Local Install Note

`signature.json` is a placeholder.

The current OpenPLC Editor package manager has signature verification enabled:

```ts
const REQUIRE_SIGNATURE = true
```

For local development only, temporarily set it to:

```ts
const REQUIRE_SIGNATURE = false
```

Then run:

```powershell
npm.cmd run dev
```

After that, zip this package as `.vpp` and install it from Package Manager > Add from file.

## Build VPP

From this package folder:

```powershell
Compress-Archive -Path manifest.json,signature.json,assets,hal,screens,README.md -DestinationPath ..\com.openplc.stm32-wp.vpp -Force
```

The archive root must contain `manifest.json` directly.
