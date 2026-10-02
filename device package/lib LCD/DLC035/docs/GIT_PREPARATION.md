# Git Preparation (2026-10-02)

The user approved removal of old release archives and generated build/test
directories. Published 0.2.7 TPA and installation ZIP are retained unchanged.
No remote repository, commit, or push was created as part of cleanup.

## Retained Dependencies

Commit Core, Drivers, Middlewares, TouchGFX assets/gui/target/simulator/config,
the .touchgfx and .ioc files, linker scripts, IDE project configuration,
scripts and documentation. The revised .gitignore retains vendor middleware,
imageconvert/build and fontconvert/build executables, simulator Makefiles,
target.config and gcc/components.mk. Do not ignore all directories named build.

Ignored paths include the Eclipse workspace, temporary cleanup validation,
package work/verify directories, TouchGFX generated/build output, IDE binaries
and machine-local editor/agent settings. Original assets and handwritten code
are retained. Generate with TouchGFX 4.26.1 before building on another machine;
install STM32CubeIDE separately. Review bundled vendor licenses before making
the repository public. Do not assume SDK redistribution rights from this test.

## Packaging

scripts/package.ps1 now reads the 0.2.7 TPA and package-0.2.7.json as its baseline.
It no longer reads any deleted 0.2.5 file or copies vendor tools from local source.
This preserves the template GUI rather than replacing it with the development UI.
Future driver changes require deliberate staging overlays and a version bump.

To test a repack without modifying published metadata:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -OutputDirectory .cleanup-check/repacked
```

The destination must not already contain the same TPA. A new release version
also requires RELEASE-<version>.md. Never replace a published TPA in place.

## Evidence

All 885 payload files in an isolated repack matched the published 0.2.7 contents
by SHA256. The original TPA SHA256 is:

```text
C8DD4AB947AC021129B1BC148338608A40DC77D0876BE35A62FE0A3459D9A482
```

docs/verification retains 0.2.6/0.2.7 generation, assets and build logs.
cleanup-2026-10-02.csv records the approved deleted paths and their sizes;
historical references to removed work folders describe past verification.
Historical markdown release notes remain available even though their archives
are removed. The installation ZIP contains the same original TPA and installer.

Before publishing, inspect git status and the staged file list for local/private
data, verify the intended repository visibility, and run scripts/build.ps1.
No hardware flashing is part of repository cleanup.
