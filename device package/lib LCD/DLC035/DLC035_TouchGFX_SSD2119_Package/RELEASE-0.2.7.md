# DLC035 0.2.7: Normal GUI On Startup

The package defaults LCD_COLOR_TEST_HOLD_MS to 0U in TouchGFX/target/TouchGFXHAL.cpp. The startup color drawing and its 5-second delay are compiled out. LCD initialization still runs normally, followed by the template GUI. The diagnostic function remains available for explicit troubleshooting; set a nonzero duration only when needed.

Scope: package template only. The development project's TouchGFXHAL.cpp is unchanged. Existing projects and flashed boards are not modified by installing a package; regenerate, build, and flash a newly created project to use this default. For an existing project, set LCD_COLOR_TEST_HOLD_MS to 0U in its own TouchGFX/target/TouchGFXHAL.cpp and rebuild/reflash.

All LCD pin mappings, AFCNTR retention, SSD2119 0x72EF, and converter restoration from 0.2.6 are preserved. Both imageconvert/build and fontconvert/build remain in the payload.

## Install

Extract the installation ZIP. Keep DLC035_SSD2119_NUCLEO_H563ZI-0.2.7.tpa and Install_DLC035_TouchGFX_Package.ps1 together. Close Designer and run PowerShell in that folder:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\Install_DLC035_TouchGFX_Package.ps1 -TouchGFXRoot 'E:\TouchGFX\4.26.1'
```

Adjust the root to the installed path. Use the same Windows account as Designer. Restart Designer, choose By Partners -> NUCLEO-H563ZI + DLC035 SSD2119 SPI -> 0.2.7, then create a new project, generate, build, and flash.

## Reproduce

scripts/package.ps1 defaults to 0.2.7. It preserves the previous baseline/asset repair, explicitly sets the staged diagnostic duration to zero, and excludes only application output folders. It does not change the development GUI or diagnostic setting. Future payload changes require a version bump and release note file; existing archives are not overwritten.

Verify the final TPA by extracting it into a new folder, checking the zero-duration macro and both converters, running tgfx generate and make -B -f simulator/gcc/Makefile assets -j8, then clean-building in CubeIDE. Hardware restart testing is separate from these software checks.
