# Repository Cleanup (2026-10-02)

Approved old archives and temporary package/build folders were removed. The
published 0.2.7 TPA and install ZIP are unchanged. Packaging now uses 0.2.7 as
its baseline without depending on 0.2.5. All 885 payload file hashes matched
in an isolated repack test.

A clean export of Git-visible files passed doctor, TouchGFX generation,
asset conversion and CubeIDE clean Debug build: 0 errors, 0 warnings;
text/data/bss = 320280/764/79200. This tests development source, whose GUI
and diagnostic defaults differ from the packaged template. It does not
claim that a remote clone or hardware flash was tested.

Evidence: docs/verification/clean-checkout-2026-10-02.log and
docs/verification/cleanup-2026-10-02.csv. Original 0.2.6/0.2.7 logs are now
under docs/verification; historical work/verify paths below no longer exist.
See docs/GIT_PREPARATION.md. No repository was initialized at the project
root, and no commit or push was performed.

# Current Release 0.2.7 (2026-09-24)

User requested no startup color page in the template. Package TouchGFXHAL.cpp now defaults LCD_COLOR_TEST_HOLD_MS to 0U, compiling out color drawing and the 5-second delay. Development source and existing projects are unchanged. Driver fixes and asset converters are preserved.

Final TPA was extracted into verify-0.2.7/project; CLI generation, forced make assets, and clean CubeIDE Debug build passed (0 errors, 0 warnings; text/data/bss 319880/764/79200). Logs are in DLC035_TouchGFX_SSD2119_Package/verify-0.2.7. Hardware reset/flash was not tested by the agent. See the package RELEASE-0.2.7.md for installation and existing-project instructions.

---

# Current Release 0.2.6 (2026-09-24)

Root cause of Designer Generate Assets failure: 0.2.5 used --exclude=build and removed the vendor imageconvert/build and fontconvert/build directories. Earlier CLI code-generation/build checks reused assets and missed the failure. This is a confirmed packaging regression, not an LCD driver issue.

0.2.6 restores both converter directories, keeps all 0.2.5 hardware fixes, and uses scoped application-output exclusions in scripts/package.ps1. scripts/build.ps1 now runs make assets explicitly, and doctor checks the converters inside the nested TPA.

Verification: extracted the FINAL 0.2.6 TPA into verify-0.2.6/project; tgfx generate passed; make -B -f simulator/gcc/Makefile assets -j8 passed; CubeIDE clean Debug build passed with 0 errors and 0 warnings (text/data/bss 320108/764/79200). Logs: DLC035_TouchGFX_SSD2119_Package/verify-0.2.6/{generate,assets,build}.log. MyApplication_8 missing tool folders were repaired without modifying the GUI, and its previously failing make assets command passed. Designer GUI Create itself and hardware flashing were not automated in this verification.

See [0.2.6 instructions](../DLC035_TouchGFX_SSD2119_Package/RELEASE-0.2.6.md). 0.2.5 is historical and should not be used for new projects.

---

# Current Release: 0.2.5 (2026-09-24)

The user confirmed correct LCD output after enabling MasterKeepIOState/AFCNTR in SPI1_Init 2 and restoring SSD2119 output control to 0x72EF. This supersedes older experimental color-order recommendations below.

Package: DLC035_SSD2119_NUCLEO_H563ZI-0.2.5.tpa. Payload MD5: 6BB8E73245E858EA8DEB0DDB41FC4BA5; size: 29999830 bytes. TPA SHA256: 509F552603BD440C1A60E26E1AEF4473DD9EE48DA5E78EE8035E9454C9717F17.

Staged template generation and CubeIDE clean build passed: 0 errors, 0 warnings; text/data/bss 320104/764/79200. Installer first install and same-path reinstall passed in isolated APPDATA; all four copies match SHA256. The newly packaged template was not flashed by the agent and Designer GUI Create on another machine has not been retested.

See [release and installation guide](../DLC035_TouchGFX_SSD2119_Package/RELEASE-0.2.5.md). Installing does not update existing projects. The template preserves the 5-second startup color test and the previous template GUI. Current source prescaler is 4, not the older documented 8.

---

## Historical Notes (Superseded Where They Conflict)

# Current Status

Last verified: 2026-09-23

## Overall

NUCLEO-H563ZI drives DLC0350QEM06DT-1 through SSD2119 SPI and renders TouchGFX. Source was regenerated with TouchGFX 4.26.1 and clean-built with STM32CubeIDE 1.18.1.

```text
Configuration : Debug
Result        : PASS, 0 errors, 0 warnings
ELF           : STM32CubeIDE/Debug/STM32H563ZI_NUCLEO_AZ2.elf
text/data/bss : 319752 / 764 / 79200 bytes
```

This update was compile-tested but not flashed again. Build and hardware verification are separate.

## Hardware

| Signal | Pin |
|---|---|
| LCD DC/RS | PG15 |
| LCD RST | PB7 |
| LCD CS | PB6 |
| SPI1 SCK | PB3 |
| SPI1 MISO | PB4 |
| SPI1 MOSI | PB5 |

Do not reuse legacy PD15/PD14/PF3/PA5/PG9 mapping.

## TouchGFX/Driver

- TouchGFX 4.26.1; `TouchGFX/DLC035.touchgfx`.
- 320x240 RGB565, partial framebuffer, FreeRTOS/CMSIS-RTOS v2.
- Custom display interface, blocking SPI.
- `SSD2119_Init(&hspi1)` runs from `TouchGFXHAL::initialize()`.
- Blocks use `SSD2119_WriteBitmap()`; RGB565 bytes are swapped.
- 180-degree correction is in `SSD2119_SetWindow()`; `layout_rotation=0`.
- 2026-09-23: changed SSD2119 Driver Output Control from `0x72EF` to `0x7AEF` to enable the BGR source-order bit. This targets the observed red/blue channel swap (orange rendered blue and blue rendered red). Build verification passed; hardware color verification is still required before publishing a replacement package.
- GPIO uses macros from `Core/Inc/main.h`.

## Package

```text
File         : DLC035_TouchGFX_SSD2119_Package/DLC035_SSD2119_NUCLEO_H563ZI-0.2.4.tpa
Version      : 0.2.4
Payload MD5  : 84CD6A87E1A69F97173EBB49021299F3
Payload size : 33579259 bytes
TPA SHA256   : 121BE65CEDF015735BDD5A0DE5E5A1D3E3528C60097D20105C2A6C955916639A
```

TPA structure, embedded checksum/size, and payload pin mapping were validated.

## Recent Fixes

- Migrated pins to PB3/PB4/PB5/PB6/PB7/PG15.
- Released package 0.2.4.
- Regenerated GUI/text assets to fix stale `T___SINGLEUSE_*` IDs.
- Updated CubeIDE links and added repeatable doctor/build automation.

## Limitations

- SPI is blocking; DMA is pending.
- Full-screen animation is SPI-bandwidth limited.
- MISO is configured but rendering is mainly write-only.
- Package checksum/size changes after every payload rebuild.
- Build success does not replace an LCD hardware test.

## Next Check

```powershell
.\scripts\doctor.ps1
.\scripts\build.ps1
```

Then flash from STM32CubeIDE and verify content, colors, orientation, and redraws.

## Startup LCD Color Diagnostic

User reports the color problem persists after the BGR change. Root cause is
unconfirmed; the earlier RGB/BGR diagnosis is not a verified hardware fix.

TouchGFXHAL.cpp now draws a diagnostic page immediately after LCD initialization,
before TouchGFX initialization/rendering. It holds the completed page for 5000 ms,
clears the display, then proceeds to the normal Designer startup screen.

Both rows should show, left to right:
white, yellow, cyan, green, magenta, red, blue, black.
Each bar is 40 pixels wide. The upper 120 rows use SSD2119_FillRect;
the lower 120 rows use SSD2119_WriteBitmap with a 320-pixel RGB565 scanline.
Canonical literal RGB565 values are used, including green 0x07E0.

- Both rows wrong: investigate shared driver/controller/SPI path.
- Upper row correct but lower row wrong: inspect bitmap transfer/window handling.
- Both rows correct but normal UI wrong: inspect TouchGFX rendering/block transfer.
- This does not validate the controller's wire pixel format; both paths currently
  use the existing two-byte pixel transport.

Set LCD_COLOR_TEST_HOLD_MS in TouchGFX/target/TouchGFXHAL.cpp to change the delay;
0 disables the test. This is a hardware startup diagnostic, not a Designer page.
It runs once per initialization, without drawing over the running application.
Package 0.2.4 is unchanged and does not contain this diagnostic.
Hardware flashing and visual verification are pending.

## Color Diagnostic Follow-up (49518.jpg)

Hardware observation: the upper solid-fill bars are uniform but show
white/cyan/yellow/green/magenta/blue/red/black, confirming red/blue inversion
with the trial 0x7AEF setting. The lower bitmap bars have different colors
and horizontal artifacts before TouchGFX starts. The fault therefore also
occurs outside TouchGFX; it is not isolated to Designer assets.

Changes for the next hardware trial:
- Restore Driver Output Control to 0x72EF (BGR=0).
- Enable SPI_CFG2_AFCNTR and set hspi1.Init.MasterKeepIOState to ENABLE in
  the SPI1_Init 2 user section, after HAL_SPI_Init and before LCD traffic.
  This override survives CubeMX regeneration.
- Retain the same startup test, SPI speed, RGB565 transport and bitmap order.

Reason: the local HAL calls SPI_CloseTransfer, which disables SPI after every
HAL_SPI_Transmit. The LCD driver keeps CS low across multiple transfers.
Keep IO State was disabled, allowing output-level changes between transfers;
bitmap conversion leaves longer gaps than repeated solid fills.
This is a supported failure mechanism and a candidate cause, not a measured
clock-glitch diagnosis. Confirm on hardware; probe SCK at the LCD across chunk
boundaries if artifacts remain.

Expected: both rows match white/yellow/cyan/green/magenta/red/blue/black.
Normal UI should then retain its green/orange/blue colors.
Package 0.2.4 remains unchanged. Hardware verification is pending.

Reference:
https://community.st.com/stm32-mcus-products-25/stm32h5-spi-gets-disabled-by-hal-after-each-transfer-141237
