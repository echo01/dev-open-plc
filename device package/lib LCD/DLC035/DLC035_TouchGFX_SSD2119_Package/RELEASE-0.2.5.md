# DLC035 Package 0.2.5

Release date: 2026-09-24. TouchGFX Designer: 4.26.1.

## Changes

- Keep SPI1 SCK/MOSI state between blocking HAL transfers: enable MasterKeepIOState and set SPI_CFG2_AFCNTR in the SPI1_Init 2 user section AFTER HAL_SPI_Init. Preserve both lines when regenerating CubeMX.
- SSD2119_OUTPUT_CTRL_DLC035 is 0x72EF. The user confirmed correct LCD colors with this combination; do not restore the experimental 0x7AEF.
- Preserve current source SPI1 prescaler 4, 8-bit, one-line writes. IOC reports 8.0 Mbits/s; this is configuration, not a scope measurement.
- Pins unchanged: PG15 DC/RS, PB7 RST, PB6 CS, PB3 SCK, PB5 MOSI. PB4 MISO is optional and unused for one-line writes. RS is NOT reset.
- Keep the startup color test for 5000 ms before the normal GUI. In TouchGFX/target/TouchGFXHAL.cpp set LCD_COLOR_TEST_HOLD_MS to 0 to disable it.
- Retain the 0.2.4 template GUI, not the development project's HMI screen.

## Install On This Or Another PC

1. Install TouchGFX Designer 4.26.1 and close Designer.
2. Put DLC035_SSD2119_NUCLEO_H563ZI-0.2.5.tpa and Install_DLC035_TouchGFX_Package.ps1 in the SAME folder. No source, work directory, or separate package.json is required.
3. Open PowerShell in that folder. For the E: installation run:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\Install_DLC035_TouchGFX_Package.ps1 -TouchGFXRoot "E:\TouchGFX\4.26.1"
```

For a C: installation change the root to C:\TouchGFX\4.26.1. An absolute package location can be supplied with -PackageFile. Use the same Windows account that will run Designer; its APPDATA cache matters. Folder write permissions may be needed. Do not run as a different account and populate the wrong cache.

4. The installer verifies embedded payload MD5/size, obtains the version from package.json, and installs BOTH versioned and unversioned TPA names to:
   - <TouchGFXRoot>\app\packages
   - %APPDATA%\TouchGFX-4.26.1\Downloads\DLC035_SSD2119_NUCLEO_H563ZI\0.2.5
5. Restart Designer, Create -> By Partners -> NUCLEO-H563ZI + DLC035 SSD2119 SPI, select 0.2.5, and create into a NEW directory. Generate, build, and flash the new project's ELF.

Copying only the TPA into app/packages can show the board without making its payload available offline. This custom package is not published at the metadata's remote URL; use the installer to populate the local cache. If download is attempted, verify the selected version, account, and all four installed files. The script safely skips copying a file onto itself when run from app/packages.

Existing projects are NOT updated by installing a package. They need the corresponding driver/HAL changes, regeneration, rebuild, and reflash. Do not overwrite their GUI with the template GUI.

## Reproduce The Release

1. Extract the outer 0.2.4 TPA (ZIP format), then its DLC035_SSD2119_NUCLEO_H563ZI.zip payload into an isolated staging project. Do not change historic release files.
2. Overlay these verified source files, keeping relative paths:

```text
Core/Src/main.c
Core/Src/ssd2119.c
Core/Src/stm32h5xx_hal_msp.c
Core/Inc/main.h
Core/Inc/ssd2119.h
STM32H563ZI_NUCLEO_AZ2.ioc
TouchGFX/target/TouchGFXHAL.cpp
```

3. Generate using an ABSOLUTE .touchgfx path and stage root as working directory:

```powershell
& 'E:\TouchGFX\4.26.1\designer\tgfx.exe' generate -v -p '<STAGE>\TouchGFX\DLC035.touchgfx'
```

4. Clean-build using an isolated Eclipse workspace. Use native absolute Windows paths for -data and -import; forward-slash C:/ import paths were interpreted as URI schemes in this environment.

```powershell
& 'C:\ST\STM32CubeIDE_1.18.1\STM32CubeIDE\stm32cubeidec.exe' --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data '<WORKSPACE>' -import '<STAGE>\STM32CubeIDE' -cleanBuild 'STM32H563ZI_NUCLEO_AZ2/Debug'
```

Replace bracketed paths; require zero errors/warnings and a new ELF. Do not package Debug/Release, simulator build folders, logs, Eclipse workspace, or this release work directory.
5. Create the inner payload ZIP with forward-slash entry paths and project folders at its root. Set package.json Data.Version to 0.2.5, Meta.CreatedAt, Meta.Url version, Meta.Size to the INNER ZIP byte length, and Meta.MD5sum to its MD5. Keep Data.Name and payload filename stable.
6. The outer TPA is a ZIP with exactly two root entries: package.json and DLC035_SSD2119_NUCLEO_H563ZI.zip. Use ZIP compression, then the .tpa extension. Retain package-0.2.5.json as a release metadata snapshot.
7. Reopen the finished TPA, verify inner size/MD5 and source-file hashes, and verify there are no build outputs or parent-relative paths. Test installer twice using a disposable TouchGFXRoot and APPDATA: first install and install with source already in app/packages. Verify all four copies match the release SHA256.
8. On an actual target machine, verify Designer Create, Generate, build, flash, startup colors, and GUI before distributing further revisions.

## Verification Scope

The staged 0.2.5 template passed Designer generation and CubeIDE Debug clean build on 2026-09-24: 0 errors, 0 warnings; text/data/bss = 320104 / 764 / 79200 bytes. Logs are in work-0.2.5. The source LCD fix is hardware-confirmed by the user. This new archive has not been flashed by the agent; Designer GUI Create on another PC remains a user verification step.
