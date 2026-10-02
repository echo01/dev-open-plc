[CmdletBinding()]
param(
    [string]$TouchGFXRoot,
    [string]$CubeIDERoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$script:failures = 0
$script:warnings = 0

function Write-Check {
    param(
        [ValidateSet("PASS", "WARN", "FAIL")][string]$Status,
        [string]$Name,
        [string]$Detail
    )
    $color = switch ($Status) { "PASS" { "Green" } "WARN" { "Yellow" } "FAIL" { "Red" } }
    Write-Host ("[{0}] {1}: {2}" -f $Status, $Name, $Detail) -ForegroundColor $color
    if ($Status -eq "FAIL") { $script:failures++ }
    if ($Status -eq "WARN") { $script:warnings++ }
}

function Resolve-ExistingDirectory {
    param([string[]]$Candidates)
    foreach ($candidate in $Candidates) {
        if ([string]::IsNullOrWhiteSpace($candidate)) { continue }
        if (Test-Path -LiteralPath $candidate -PathType Container) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }
    return $null
}

function Get-TouchGFXCandidates {
    $items = New-Object System.Collections.Generic.List[string]
    if ($TouchGFXRoot) { $items.Add($TouchGFXRoot) }
    if ($env:TOUCHGFX_ROOT) { $items.Add($env:TOUCHGFX_ROOT) }
    $items.Add("C:\TouchGFX\4.26.1")
    $items.Add("E:\TouchGFX\4.26.1")
    return $items.ToArray()
}

function Get-CubeIDECandidates {
    $items = New-Object System.Collections.Generic.List[string]
    if ($CubeIDERoot) { $items.Add($CubeIDERoot) }
    if ($env:STM32CUBEIDE_ROOT) { $items.Add($env:STM32CUBEIDE_ROOT) }
    $items.Add("C:\ST\STM32CubeIDE_1.18.1\STM32CubeIDE")
    foreach ($base in @("C:\ST", "C:\Program Files\STMicroelectronics", "E:\ST")) {
        if (-not (Test-Path -LiteralPath $base -PathType Container)) { continue }
        Get-ChildItem -LiteralPath $base -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like "STM32CubeIDE_*" } |
            Sort-Object Name -Descending |
            ForEach-Object { $items.Add((Join-Path $_.FullName "STM32CubeIDE")) }
    }
    return $items.ToArray()
}

Write-Host "DLC035 environment doctor"
Write-Host "Project: $projectRoot"
Write-Host ""

$requiredFiles = @(
    "STM32H563ZI_NUCLEO_AZ2.ioc", "Core\Inc\main.h", "Core\Src\main.c",
    "Core\Src\ssd2119.c", "Core\Src\stm32h5xx_hal_msp.c",
    "TouchGFX\DLC035.touchgfx", "TouchGFX\target\TouchGFXHAL.cpp",
    "STM32CubeIDE\.project", "STM32CubeIDE\.cproject"
)
foreach ($relativePath in $requiredFiles) {
    if (Test-Path -LiteralPath (Join-Path $projectRoot $relativePath) -PathType Leaf) {
        Write-Check PASS "Project file" $relativePath
    } else {
        Write-Check FAIL "Project file" "Missing $relativePath"
    }
}

$resolvedTouchGFX = Resolve-ExistingDirectory (Get-TouchGFXCandidates)
if ($resolvedTouchGFX) { Write-Check PASS "TouchGFX root" $resolvedTouchGFX }
else { Write-Check FAIL "TouchGFX root" "TouchGFX 4.26.1 not found. Pass -TouchGFXRoot or set TOUCHGFX_ROOT." }

$tgfxExe = if ($resolvedTouchGFX) { Join-Path $resolvedTouchGFX "designer\tgfx.exe" } else { $null }
$rubyExe = if ($resolvedTouchGFX) { Join-Path $resolvedTouchGFX "env\MinGW\msys\1.0\Ruby30-x64\bin\ruby.exe" } else { $null }
if ($tgfxExe -and (Test-Path -LiteralPath $tgfxExe -PathType Leaf)) { Write-Check PASS "TouchGFX generator" $tgfxExe }
else { Write-Check FAIL "TouchGFX generator" "designer\tgfx.exe not found" }
if ($rubyExe -and (Test-Path -LiteralPath $rubyExe -PathType Leaf)) { Write-Check PASS "TouchGFX Ruby" $rubyExe }
else { Write-Check FAIL "TouchGFX Ruby" "Bundled Ruby runtime not found" }

$resolvedCubeIDE = Resolve-ExistingDirectory (Get-CubeIDECandidates)
$cubeIDECli = if ($resolvedCubeIDE) { Join-Path $resolvedCubeIDE "stm32cubeidec.exe" } else { $null }
if ($cubeIDECli -and (Test-Path -LiteralPath $cubeIDECli -PathType Leaf)) { Write-Check PASS "STM32CubeIDE CLI" $cubeIDECli }
else { Write-Check FAIL "STM32CubeIDE CLI" "stm32cubeidec.exe not found. Pass -CubeIDERoot or set STM32CUBEIDE_ROOT." }

foreach ($commandName in @("tar.exe", "robocopy.exe")) {
    $command = Get-Command $commandName -ErrorAction SilentlyContinue
    if ($command) { Write-Check PASS $commandName $command.Source }
    else { Write-Check WARN $commandName "Not found; firmware build works, but package creation may fail." }
}

$iocPath = Join-Path $projectRoot "STM32H563ZI_NUCLEO_AZ2.ioc"
if (Test-Path -LiteralPath $iocPath) {
    $ioc = Get-Content -LiteralPath $iocPath -Raw
    $pins = [ordered]@{
        "SPI1_SCK PB3" = "PB3(JTDO/TRACESWO).Signal=SPI1_SCK"
        "SPI1_MOSI PB5" = "PB5.Signal=SPI1_MOSI"
        "LCD_CS PB6" = "PB6.GPIO_Label=LCD_CS"
        "LCD_RST PB7" = "PB7.GPIO_Label=LCD_RST"
        "LCD_DC PG15" = "PG15.GPIO_Label=LCD_DC"
    }
    foreach ($entry in $pins.GetEnumerator()) {
        if ($ioc.Contains($entry.Value)) { Write-Check PASS "Pin mapping" $entry.Key }
        else { Write-Check FAIL "Pin mapping" "Missing $($entry.Key) in IOC" }
    }

    $misoMapping = "PB4(NJTRST).Signal=SPI1_MISO"
    if ($ioc.Contains($misoMapping)) {
        Write-Check PASS "Pin mapping" "SPI1_MISO PB4"
    } elseif ($ioc.Contains("SPI1.Direction=SPI_DIRECTION_1LINE")) {
        Write-Check WARN "Pin mapping" "SPI1_MISO PB4 is not configured; optional in 1LINE write-only mode."
    } else {
        Write-Check FAIL "Pin mapping" "Missing SPI1_MISO PB4 in IOC"
    }
}
$mainHeaderPath = Join-Path $projectRoot "Core\Inc\main.h"
if (Test-Path -LiteralPath $mainHeaderPath) {
    $header = Get-Content -LiteralPath $mainHeaderPath -Raw
    $macros = [ordered]@{
        "LCD_DC PG15" = @("#define LCD_DC_Pin GPIO_PIN_15", "#define LCD_DC_GPIO_Port GPIOG")
        "LCD_CS PB6" = @("#define LCD_CS_Pin GPIO_PIN_6", "#define LCD_CS_GPIO_Port GPIOB")
        "LCD_RST PB7" = @("#define LCD_RST_Pin GPIO_PIN_7", "#define LCD_RST_GPIO_Port GPIOB")
    }
    foreach ($entry in $macros.GetEnumerator()) {
        $valid = $true
        foreach ($expected in $entry.Value) { if (-not $header.Contains($expected)) { $valid = $false } }
        if ($valid) { Write-Check PASS "GPIO macro" $entry.Key }
        else { Write-Check FAIL "GPIO macro" "Incorrect $($entry.Key) macros" }
    }
}

$packagePath = Join-Path $projectRoot "DLC035_TouchGFX_SSD2119_Package\DLC035_SSD2119_NUCLEO_H563ZI-0.2.7.tpa"
if (Test-Path -LiteralPath $packagePath -PathType Leaf) {
    try {
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        $archive = [System.IO.Compression.ZipFile]::OpenRead($packagePath)
        try {
            $metadataEntry = $archive.GetEntry("package.json")
            $payloadEntry = $archive.GetEntry("DLC035_SSD2119_NUCLEO_H563ZI.zip")
            if (-not $metadataEntry -or -not $payloadEntry) { throw "TPA must contain package.json and payload ZIP" }
            $reader = New-Object System.IO.StreamReader($metadataEntry.Open())
            try { $metadata = $reader.ReadToEnd() | ConvertFrom-Json } finally { $reader.Dispose() }
            $md5 = [System.Security.Cryptography.MD5]::Create()
            $stream = $payloadEntry.Open()
            try { $actualMd5 = [BitConverter]::ToString($md5.ComputeHash($stream)).Replace("-", "") }
            finally { $stream.Dispose(); $md5.Dispose() }
            $version = "{0}.{1}.{2}" -f $metadata.Data.Version.Major, $metadata.Data.Version.Minor, $metadata.Data.Version.Build
            if ($version -ne "0.2.7") { throw "Expected v0.2.7, found v$version" }
            if ($actualMd5 -ne $metadata.Meta.MD5sum) { throw "Payload MD5 mismatch" }
            if ($payloadEntry.Length -ne $metadata.Meta.Size) { throw "Payload size mismatch" }
            $memory = [IO.MemoryStream]::new()
            $inputStream = $payloadEntry.Open()
            try { $inputStream.CopyTo($memory) } finally { $inputStream.Dispose() }
            $memory.Position = 0
            $payloadZip = [IO.Compression.ZipArchive]::new($memory, [IO.Compression.ZipArchiveMode]::Read)
            try {
                foreach ($tool in @("imageconvert", "fontconvert")) {
                    $relative = "Middlewares/ST/touchgfx/framework/tools/$tool/build/win/$tool.out"
                    $entry = $payloadZip.GetEntry("./" + $relative)
                    if (-not $entry) { $entry = $payloadZip.GetEntry($relative) }
                    if (-not $entry -or $entry.Length -eq 0) { throw "Missing packaged converter: $relative" }
                    if (-not (Test-Path (Join-Path $projectRoot $relative))) { throw "Missing local converter: $relative" }
                    Write-Check PASS "Asset converter" $tool
                }

                $halEntry = $payloadZip.GetEntry("./TouchGFX/target/TouchGFXHAL.cpp")
                if (-not $halEntry) { $halEntry = $payloadZip.GetEntry("TouchGFX/target/TouchGFXHAL.cpp") }
                if (-not $halEntry) { throw "Missing packaged TouchGFXHAL.cpp" }
                $halReader = [IO.StreamReader]::new($halEntry.Open())
                try { $halText = $halReader.ReadToEnd() } finally { $halReader.Dispose() }
                if (-not $halText.Contains("#define LCD_COLOR_TEST_HOLD_MS 0U")) {
                    throw "Package startup color test must be disabled"
                }
                Write-Check PASS "Package startup" "Color diagnostics disabled"
            } finally { $payloadZip.Dispose(); $memory.Dispose() }

            Write-Check PASS "TouchGFX package" "v$version, MD5 $actualMd5, $($payloadEntry.Length) bytes"
        } finally { $archive.Dispose() }
    } catch { Write-Check FAIL "TouchGFX package" $_.Exception.Message }
} else { Write-Check FAIL "TouchGFX package" "Missing v0.2.7 TPA" }

Write-Host ""
Write-Host ("Doctor summary: {0} failure(s), {1} warning(s)" -f $script:failures, $script:warnings)
if ($script:failures -gt 0) { exit 1 }
exit 0
