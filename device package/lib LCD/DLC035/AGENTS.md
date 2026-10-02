# AGENTS.md

Read this before changing code, CubeMX, TouchGFX generated files, or the package.

## Read First

1. `AGENTS.md` - invariants and working rules.
2. `docs/CURRENT_STATUS.md` - latest verified state.
3. `docs/BUILD.md` - supported build workflow.
4. `PortDriverSSD2119toTouchGFX.md` - detailed port design/history.
5. `DLC035_TouchGFX_SSD2119_Package/DLC035_SSD2119_TouchGFX_Package_Guide.md` - packaging/install.

Before substantial work:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\doctor.ps1
```

## Project

- NUCLEO-H563ZI / STM32H563ZITx.
- DLC0350QEM06DT-1, SSD2119, 320x240 RGB565.
- SPI1 writes to SSD2119 internal GRAM.
- TouchGFX Designer 4.26.1, FreeRTOS/CMSIS-RTOS v2.
- Partial framebuffer in internal SRAM.
- Blocking SPI is the current reliable transfer method.
- Current package: `DLC035_SSD2119_NUCLEO_H563ZI-0.2.7.tpa`.

## Canonical Pins

| Signal | Pin | Configuration |
|---|---|---|
| LCD DC/RS | PG15 | GPIO output, `LCD_DC` |
| LCD reset | PB7 | GPIO output, `LCD_RST` |
| LCD chip select | PB6 | GPIO output, `LCD_CS` |
| SPI1 clock | PB3 | `SPI1_SCK`, AF5 |
| SPI1 MISO | PB4 | `SPI1_MISO`, AF5; optional for write-only use |
| SPI1 MOSI | PB5 | `SPI1_MOSI`, AF5 |

`RS` means Register Select/Data-Command and equals `DC`; it is not reset. Reset is `RST`.

Never restore the legacy 0.2.3 mapping: PD15 DC, PD14 CS, PF3 RST, PA5 SCK, PG9 MISO.

Verify pins in `STM32H563ZI_NUCLEO_AZ2.ioc`, `Core/Inc/main.h`, `Core/Src/main.c`, and `Core/Src/stm32h5xx_hal_msp.c`. The driver must use generated `LCD_*_Pin` and `LCD_*_GPIO_Port` macros, not hard-coded ports.

## Display Flow

```text
TouchGFX -> partial RGB565 framebuffer
         -> touchgfxDisplayDriverTransmitBlock()
         -> SSD2119_WriteBitmap()
         -> RGB565 byte swap -> SPI1 -> SSD2119 GRAM
```

- `TouchGFXHAL::initialize()` calls `SSD2119_Init(&hspi1)`.
- Busy state comes from `touchgfxDisplayDriverTransmitActive()`.
- Blocking completion calls `touchgfx::startNewTransfer()`.
- RGB565 is sent most-significant byte first.
- SPI1_Init 2 explicitly enables MasterKeepIOState/AFCNTR. Preserve this:
  HAL disables SPI between chunks while LCD CS stays low. The user confirmed the color fix with 0x72EF on 2026-09-24; see CURRENT_STATUS.md.
- Development source retains startup diagnostics. Production packages from 0.2.7 disable them with LCD_COLOR_TEST_HOLD_MS=0U; preserve that default.
- Physical 180-degree correction is in `SSD2119_SetWindow()`.
- Keep `layout_rotation=0`; do not rotate assets again.
- Do not reintroduce the test pattern into normal rendering.

## Generated Code

- Treat `TouchGFX/generated/**` and `TouchGFX/generated/gui_generated/**` as generated.
- Edit `TouchGFX/DLC035.touchgfx` or use Designer, then regenerate.
- Never repair `T___SINGLEUSE_*` IDs manually.
- After generation re-check pins, `TouchGFXHAL.cpp`, `KeySampler.cpp`, `app_freertos.c`, and `STM32CubeIDE/.project`.
- Put user code in `Core/Src`, `Core/Inc`, `TouchGFX/gui`, or CubeMX user sections.

## Build and Verify

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
```

This runs doctor, `tgfx.exe generate`, then a CubeIDE headless clean build. Use `-SkipGenerate` only deliberately.

For driver/pin/TouchGFX/package changes:

1. Run doctor and build; require zero errors.
2. Confirm all six pins.
3. Bump package version if payload changes.
4. Recalculate embedded payload MD5 and size.
5. Validate final TPA metadata against the embedded payload.
6. State when hardware flashing was not performed.

## Package Rules

- Type `TGAT`, category `By Partners`, `PathToDotTouchGFX = TouchGFX`.
- MD5/Size describe the payload ZIP, never the TPA.
- ZIP paths must use forward slashes; use `tar.exe -a -cf`.
- Exclude `.cubeide_ws`, `.vscode`, `TouchGFX/build`, CubeIDE Debug/Release, and package work directory.
- Preserve the published 0.2.7 baseline. Older archives were removed with user approval on 2026-10-02; retain historical notes and verification evidence.
- `scripts/package.ps1` uses the 0.2.7 TPA and metadata as its baseline. Use `-OutputDirectory` for isolated repack testing. It does not automatically import development GUI or driver changes.
- Install versioned/fallback TPA files and AppData cache with the installer.

Current 0.2.7 payload:

```text
MD5  = 374D8A263D98DEDB40ED5DDF33B11763
Size = 33583901 bytes
```

These values become stale whenever payload content changes.

## Safety

Preserve user changes, keep work scoped to this board/display, do not delete historical packages without a request, and distinguish compile verification from hardware verification.

## Knowledge Preservation

If you discover information that required significant
investigation, such as:

- build commands
- toolchain locations
- environment requirements
- dependency versions
- hardware configuration
- working compiler flags
- flashing procedure
- debugging procedure

do not leave this information only in the conversation.

Update the appropriate documentation or automation script.

Prefer:

scripts/*.ps1
docs/BUILD.md
docs/ARCHITECTURE.md
docs/CURRENT_STATUS.md
## Packaging Regression Guard

Never exclude directories by the bare name build. Vendor imageconvert/build and fontconvert/build contain required executables. Use scripts/package.ps1 and scoped application-output exclusions. Always extract the finished TPA and run make -B -f simulator/gcc/Makefile assets -j8 from TouchGFX before clean-building. tgfx.exe generate alone does not exercise Designer's asset stage.
