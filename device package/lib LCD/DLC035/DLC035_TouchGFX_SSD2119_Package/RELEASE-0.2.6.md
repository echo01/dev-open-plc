# DLC035 0.2.6: Restore Asset Converters

0.2.5 packaging incorrectly excluded every directory named build, including vendor executables. This removed Middlewares/ST/touchgfx/framework/tools/imageconvert/build/win/imageconvert.out and fontconvert/build/win/fontconvert.out. The GUI code generation command and a firmware build with existing assets did not detect this. The failure was packaging, not LCD wiring or SPI settings.

0.2.6 restores the complete imageconvert/build and fontconvert/build directories. It retains the 0.2.5 template GUI and verified LCD configuration: AFCNTR enabled, 0x72EF, PG15 DC, PB7 RST, PB6 CS, PB3 SCK, PB5 MOSI, and the startup color test. No experimental LCD changes are included.

## Installation

Close Designer. Place DLC035_SSD2119_NUCLEO_H563ZI-0.2.6.tpa beside Install_DLC035_TouchGFX_Package.ps1. Open PowerShell there:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\Install_DLC035_TouchGFX_Package.ps1 -TouchGFXRoot 'E:\TouchGFX\4.26.1'
```

Use the actual installation root and the Windows account that runs Designer. The installer populates app/packages and the per-user Downloads cache with versioned and fallback filenames. Restart Designer and select version 0.2.6 under By Partners. The install ZIP contains the two required files and this guide. Installing does not repair existing projects automatically.

## Existing Projects

For a project made from 0.2.5, restore the entire imageconvert/build and fontconvert/build folders from a working 4.26.1 project to matching paths under Middlewares/ST/touchgfx/framework/tools. Do not overwrite the project's GUI or application files. MyApplication_8 was repaired this way and its previously failing make assets command passed. Generate again in Designer.

## Reproduce And Verify

Run scripts/package.ps1 from the development project. It derives the template from archived 0.2.5, restores the two vendor build directories from the local 4.26.1 middleware, and retains the hardware fixes. It refuses to overwrite an existing release. For a later release, review the baseline/overlays rather than assuming current application changes are included.

Only exclude exact application output paths, such as ./TouchGFX/build and ./STM32CubeIDE/Debug. NEVER use --exclude=build globally. The outer ZIP-format TPA contains package.json and the inner project ZIP; MD5 and Size describe the inner ZIP, not the TPA.

Before distribution, extract the FINISHED TPA into a fresh directory, verify both converters exist, run tgfx.exe generate with an absolute .touchgfx path, then run make -B -f simulator/gcc/Makefile assets -j8 from TouchGFX using the installed MinGW/MSYS/Ruby PATH. Finally clean-build the extracted project's STM32CubeIDE project with an isolated workspace. CLI generation alone is insufficient. scripts/build.ps1 now explicitly invokes the asset target. scripts/doctor.ps1 verifies converters inside the nested package.

Use the full installation workflow in RELEASE-0.2.5.md with version 0.2.6 substituted; its original blanket exclusion approach is superseded by this release and scripts/package.ps1. GUI-driven Create and hardware flash remain separate checks from CLI verification.
