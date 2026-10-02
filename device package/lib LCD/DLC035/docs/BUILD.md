# Build Guide

Supported Windows workflow for DLC035 SSD2119 TouchGFX.

## Prerequisites

- PowerShell 5.1 or 7.
- TouchGFX Designer 4.26.1.
- STM32CubeIDE 1.18.1 or compatible with `stm32cubeidec.exe`.

Detected defaults:

```text
C:\TouchGFX\4.26.1
E:\TouchGFX\4.26.1
C:\ST\STM32CubeIDE_1.18.1\STM32CubeIDE
```

Custom installations may use `TOUCHGFX_ROOT`, `STM32CUBEIDE_ROOT`, or parameters.

## Quick Start

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\doctor.ps1
.\scripts\build.ps1
```

Default: regenerate TouchGFX, explicitly run make assets, then clean Debug build.

## Doctor

```powershell
.\scripts\doctor.ps1 `
  -TouchGFXRoot "E:\TouchGFX\4.26.1" `
  -CubeIDERoot "C:\ST\STM32CubeIDE_1.18.1\STM32CubeIDE"
```

Checks project files, TouchGFX/tgfx/Ruby, CubeIDE CLI, pins/macros, package 0.2.7 MD5/size, tar, and robocopy. Required failures return code 1.

## Build Options

| Parameter | Purpose |
|---|---|
| `-Configuration Debug` | Default Debug build |
| `-Configuration Release` | Release build |
| `-TouchGFXRoot <path>` | Override TouchGFX discovery |
| `-CubeIDERoot <path>` | Override CubeIDE discovery |
| `-Workspace <path>` | Select Eclipse workspace |
| `-SkipGenerate` | Build existing generated files |
| `-SkipDoctor` | Skip checks for controlled automation |

Debug outputs are ELF/MAP/LIST under `STM32CubeIDE/Debug`.

## Verified Commands

```powershell
& "E:\TouchGFX\4.26.1\designer\tgfx.exe" `
  generate -v -p ".\TouchGFX\DLC035.touchgfx"

& "C:\ST\STM32CubeIDE_1.18.1\STM32CubeIDE\stm32cubeidec.exe" `
  --launcher.suppressErrors -nosplash `
  -application org.eclipse.cdt.managedbuilder.core.headlessbuild `
  -data ".\.cubeide_ws" -import ".\STM32CubeIDE" `
  -cleanBuild "STM32H563ZI_NUCLEO_AZ2/Debug"
```

Known good: `Build Finished. 0 errors, 0 warnings.`

## Troubleshooting

`DLC035.touchgfx` generates GUI, texts, images, fonts, and project links. `T___SINGLEUSE_xxxx was not declared` means stale output. Never edit IDs manually; run full generate/build.

If raw `gcc/Makefile` reports command syntax errors on Windows, use `scripts/build.ps1`. Pass `-TouchGFXRoot` or `-CubeIDERoot` when discovery fails.

Expected pins: PG15 DC/RS, PB7 RST, PB6 CS, PB3 SCK, PB4 MISO, PB5 MOSI.

## Flash

Open `STM32CubeIDE/`, select Debug, and Run/Debug. Verify backlight, GUI, RGB565 colors, orientation, and partial redraw. Record hardware results in `CURRENT_STATUS.md`.

## Package Asset Regression (0.2.6)

Use scripts/package.ps1. Do not exclude bare build directories: vendor imageconvert/build and fontconvert/build are runtime prerequisites for Generate Assets. Extract the final TPA and run make -B -f simulator/gcc/Makefile assets -j8 from its TouchGFX folder with TouchGFX MinGW/MSYS/Ruby on PATH. Then clean-build the extracted STM32CubeIDE project. CLI generation and firmware build alone are insufficient. See the package RELEASE-0.2.6.md for repair and verification details.
