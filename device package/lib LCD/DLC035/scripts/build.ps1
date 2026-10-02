[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")][string]$Configuration = "Debug",
    [string]$TouchGFXRoot,
    [string]$CubeIDERoot,
    [string]$Workspace,
    [switch]$SkipGenerate,
    [switch]$SkipDoctor
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$projectName = "STM32H563ZI_NUCLEO_AZ2"
$cubeIDEProject = Join-Path $projectRoot "STM32CubeIDE"
$touchGFXProject = Join-Path $projectRoot "TouchGFX\DLC035.touchgfx"

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

function Resolve-TouchGFXRoot {
    return Resolve-ExistingDirectory @($TouchGFXRoot, $env:TOUCHGFX_ROOT, "C:\TouchGFX\4.26.1", "E:\TouchGFX\4.26.1")
}

function Resolve-CubeIDERoot {
    $items = New-Object System.Collections.Generic.List[string]
    foreach ($candidate in @($CubeIDERoot, $env:STM32CUBEIDE_ROOT, "C:\ST\STM32CubeIDE_1.18.1\STM32CubeIDE")) {
        if ($candidate) { $items.Add($candidate) }
    }
    foreach ($base in @("C:\ST", "C:\Program Files\STMicroelectronics", "E:\ST")) {
        if (-not (Test-Path -LiteralPath $base -PathType Container)) { continue }
        Get-ChildItem -LiteralPath $base -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like "STM32CubeIDE_*" } |
            Sort-Object Name -Descending |
            ForEach-Object { $items.Add((Join-Path $_.FullName "STM32CubeIDE")) }
    }
    return Resolve-ExistingDirectory $items.ToArray()
}

if (-not $SkipDoctor) {
    $doctorArgs = @{}
    if ($TouchGFXRoot) { $doctorArgs.TouchGFXRoot = $TouchGFXRoot }
    if ($CubeIDERoot) { $doctorArgs.CubeIDERoot = $CubeIDERoot }
    Write-Host "Running environment checks..." -ForegroundColor Cyan
    & (Join-Path $PSScriptRoot "doctor.ps1") @doctorArgs
    if ($LASTEXITCODE -ne 0) { throw "Environment doctor failed." }
}

$resolvedTouchGFX = Resolve-TouchGFXRoot
$resolvedCubeIDE = Resolve-CubeIDERoot
if (-not $resolvedTouchGFX) { throw "TouchGFX 4.26.1 not found. Pass -TouchGFXRoot or set TOUCHGFX_ROOT." }
if (-not $resolvedCubeIDE) { throw "STM32CubeIDE not found. Pass -CubeIDERoot or set STM32CUBEIDE_ROOT." }

$tgfxExe = Join-Path $resolvedTouchGFX "designer\tgfx.exe"
$cubeIDECli = Join-Path $resolvedCubeIDE "stm32cubeidec.exe"
if (-not (Test-Path -LiteralPath $tgfxExe -PathType Leaf)) { throw "Missing $tgfxExe" }
if (-not (Test-Path -LiteralPath $cubeIDECli -PathType Leaf)) { throw "Missing $cubeIDECli" }

if (-not $Workspace) { $Workspace = Join-Path $projectRoot ".cubeide_ws" }
New-Item -ItemType Directory -Path $Workspace -Force | Out-Null
$Workspace = (Resolve-Path -LiteralPath $Workspace).Path

Write-Host ""
Write-Host "DLC035 build" -ForegroundColor Cyan
Write-Host "Configuration : $Configuration"
Write-Host "TouchGFX      : $resolvedTouchGFX"
Write-Host "STM32CubeIDE  : $resolvedCubeIDE"
Write-Host "Workspace     : $Workspace"

if (-not $SkipGenerate) {
    Write-Host ""
    Write-Host "Generating TouchGFX code and assets..." -ForegroundColor Cyan
    & $tgfxExe generate -v -p $touchGFXProject
    if ($LASTEXITCODE -ne 0) { throw "TouchGFX generation failed with exit code $LASTEXITCODE." }
    $previousPath = $env:PATH
    try {
        $env:PATH = (Join-Path $resolvedTouchGFX "env\MinGW\bin") + ";" +
                    (Join-Path $resolvedTouchGFX "env\MinGW\msys\1.0\bin") + ";" +
                    (Join-Path $resolvedTouchGFX "env\MinGW\msys\1.0\Ruby30-x64\bin") + ";" + $previousPath
        Push-Location (Join-Path $projectRoot "TouchGFX")
        try {
            & make -f simulator/gcc/Makefile assets -j8
            if ($LASTEXITCODE -ne 0) { throw "TouchGFX asset generation failed." }
        } finally { Pop-Location }
    } finally { $env:PATH = $previousPath }

}

Write-Host ""
Write-Host "Running STM32CubeIDE headless clean build..." -ForegroundColor Cyan
$buildTarget = "$projectName/$Configuration"
$ideArgs = @(
    "--launcher.suppressErrors",
    "-nosplash",
    "-application", "org.eclipse.cdt.managedbuilder.core.headlessbuild",
    "-data", $Workspace,
    "-import", $cubeIDEProject,
    "-cleanBuild", $buildTarget
)
& $cubeIDECli @ideArgs
if ($LASTEXITCODE -ne 0) { throw "STM32CubeIDE build failed with exit code $LASTEXITCODE." }

$elfPath = Join-Path $cubeIDEProject "$Configuration\$projectName.elf"
if (-not (Test-Path -LiteralPath $elfPath -PathType Leaf)) {
    throw "Build reported success but ELF was not found: $elfPath"
}

$elf = Get-Item -LiteralPath $elfPath
Write-Host ""
Write-Host "Build succeeded." -ForegroundColor Green
Write-Host "ELF  : $($elf.FullName)"
Write-Host "Size : $($elf.Length) bytes"
