param([string]$Version = "0.2.7", [string]$OutputDirectory)
$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$packageDir = Join-Path $root "DLC035_TouchGFX_SSD2119_Package"
$name = "DLC035_SSD2119_NUCLEO_H563ZI"
$work = Join-Path $packageDir ("work-" + $Version + "-" + (Get-Date -Format "yyyyMMdd-HHmmss"))
$stage = Join-Path $work "project"
$outer = Join-Path $work "outer"
$output = if ($OutputDirectory) { [IO.Path]::GetFullPath($OutputDirectory) } else { $packageDir }
New-Item -ItemType Directory -Path $output -Force | Out-Null
$tpa = Join-Path $output "$name-$Version.tpa"
if (Test-Path $tpa) { throw "Release already exists: $tpa. Bump version instead of overwriting." }
Add-Type -AssemblyName System.IO.Compression.FileSystem
New-Item -ItemType Directory -Path $outer -Force | Out-Null
$archive = [IO.Compression.ZipFile]::OpenRead((Join-Path $packageDir "$name-0.2.7.tpa"))
try {
    [IO.Compression.ZipFileExtensions]::ExtractToFile($archive.GetEntry("$name.zip"), (Join-Path $work "baseline.zip"))
} finally { $archive.Dispose() }
[IO.Compression.ZipFile]::ExtractToDirectory((Join-Path $work "baseline.zip"), $stage)
# The archived 0.2.7 baseline already contains both vendor converters.

# Production templates skip startup diagnostics; development source is unchanged.
$halPath = Join-Path $stage "TouchGFX\target\TouchGFXHAL.cpp"
$hal = [IO.File]::ReadAllText($halPath)
$enabled = "#define LCD_COLOR_TEST_HOLD_MS 5000U"
$disabled = "#define LCD_COLOR_TEST_HOLD_MS 0U"
if (-not $hal.Contains($enabled) -and -not $hal.Contains($disabled)) {
    throw "Unexpected LCD diagnostic setting; review before packaging."
}
[IO.File]::WriteAllText($halPath, $hal.Replace($enabled, $disabled), [Text.UTF8Encoding]::new($false))
Copy-Item (Join-Path $packageDir "RELEASE-$Version.md") $stage
$payload = Join-Path $outer "$name.zip"
# Exclude APPLICATION output paths only. Vendor tools/*/build must remain.
& tar.exe -a -cf $payload --exclude="./STM32CubeIDE/Debug" --exclude="./STM32CubeIDE/Release" --exclude="./TouchGFX/build" --exclude="./.cubeide_ws" --exclude="./.vscode" -C $stage .
if ($LASTEXITCODE -ne 0) { throw "Payload archive failed" }
$zip = [IO.Compression.ZipFile]::OpenRead($payload)
try {
    foreach ($tool in @("imageconvert", "fontconvert")) {
        $entry = $zip.GetEntry("./Middlewares/ST/touchgfx/framework/tools/$tool/build/win/$tool.out")
        if (-not $entry -or $entry.Length -eq 0) { throw "Missing packaged converter: $tool" }
    }
} finally { $zip.Dispose() }
$m = Get-Content (Join-Path $packageDir "package-0.2.7.json") -Raw | ConvertFrom-Json
$v = [version]$Version
$m.Data.Version.Major = $v.Major
$m.Data.Version.Minor = $v.Minor
$m.Data.Version.Build = $v.Build
$m.Meta.Url = $m.Meta.Url.Replace("0.2.7", $Version)
$m.Meta.CreatedAt = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
$m.Meta.MD5sum = (Get-FileHash $payload -Algorithm MD5).Hash
$m.Meta.Size = (Get-Item $payload).Length
$json = $m | ConvertTo-Json -Depth 20
foreach ($file in @((Join-Path $outer "package.json"), (Join-Path $output "package.json"), (Join-Path $output "package-$Version.json"))) {
    [IO.File]::WriteAllText($file, $json, [Text.UTF8Encoding]::new($false))
}
$outerZip = Join-Path $work "release.zip"
& tar.exe -a -cf $outerZip -C $outer package.json "$name.zip"
if ($LASTEXITCODE -ne 0) { throw "Outer archive failed" }
Copy-Item $outerZip $tpa
Write-Host "Created $tpa"
Write-Host "Work directory: $work"
Write-Host "REQUIRED: extract final TPA into a fresh directory; generate, make assets, then CubeIDE build before distribution."
