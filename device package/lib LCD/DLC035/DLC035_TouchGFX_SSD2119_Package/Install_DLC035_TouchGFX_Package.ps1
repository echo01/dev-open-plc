param(
    [string]$TouchGFXRoot = "C:\TouchGFX\4.26.1",
    [string]$PackageFile = "DLC035_SSD2119_NUCLEO_H563ZI-0.2.7.tpa",
    [string]$DesignerVersion = "4.26.1"
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$sourceTpa = if ([System.IO.Path]::IsPathRooted($PackageFile)) {
    $PackageFile
} else {
    Join-Path $scriptDir $PackageFile
}

if (-not (Test-Path -LiteralPath $sourceTpa)) {
    throw "Package file not found: $sourceTpa`nPlace the .tpa beside this script or pass its absolute path with -PackageFile."
}

$installPackagesDir = Join-Path $TouchGFXRoot "app\packages"
if (-not (Test-Path -LiteralPath $installPackagesDir)) {
    throw "TouchGFX packages folder not found: $installPackagesDir"
}

$packageName = "DLC035_SSD2119_NUCLEO_H563ZI"
Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [IO.Compression.ZipFile]::OpenRead($sourceTpa)
try {
    $entry = $archive.GetEntry('package.json')
    if (-not $entry) { throw 'Missing package.json in TPA' }
    $reader = [IO.StreamReader]::new($entry.Open())
    try { $metadata = $reader.ReadToEnd() | ConvertFrom-Json } finally { $reader.Dispose() }
    if ($metadata.Data.Name -ne $packageName) { throw 'Unexpected package identity' }
    $v = $metadata.Data.Version
    $packageVersion = '{0}.{1}.{2}' -f $v.Major, $v.Minor, $v.Build
    $payload = $archive.GetEntry("$packageName.zip")
    if (-not $payload) { throw 'Missing embedded payload' }
    if ($payload.Length -ne $metadata.Meta.Size) { throw 'Payload size mismatch' }
    $stream = $payload.Open()
    $md5 = [Security.Cryptography.MD5]::Create()
    try { $hash = [BitConverter]::ToString($md5.ComputeHash($stream)).Replace('-','') }
    finally { $stream.Dispose(); $md5.Dispose() }
    if ($hash -ne $metadata.Meta.MD5sum) { throw 'Payload checksum mismatch' }
} finally { $archive.Dispose() }

function Copy-Package([string]$Destination) {
    if ([IO.Path]::GetFullPath($sourceTpa) -ieq [IO.Path]::GetFullPath($Destination)) { return }
    Copy-Item -LiteralPath $sourceTpa -Destination $Destination -Force
}
$versionedName = "$packageName-$packageVersion.tpa"
$fallbackName = "$packageName.tpa"

Write-Host "Installing DLC035 TouchGFX package..."
Write-Host "Source      : $sourceTpa"
Write-Host "TouchGFX    : $TouchGFXRoot"

Copy-Package (Join-Path $installPackagesDir $versionedName)
Copy-Package (Join-Path $installPackagesDir $fallbackName)

$downloadsDir = Join-Path $env:APPDATA "TouchGFX-$DesignerVersion\Downloads\$packageName\$packageVersion"
New-Item -ItemType Directory -Path $downloadsDir -Force | Out-Null
Copy-Package (Join-Path $downloadsDir $versionedName)
Copy-Package (Join-Path $downloadsDir $fallbackName)

Write-Host ""
Write-Host "Installed files:"
Write-Host "  $(Join-Path $installPackagesDir $versionedName)"
Write-Host "  $(Join-Path $installPackagesDir $fallbackName)"
Write-Host "  $(Join-Path $downloadsDir $versionedName)"
Write-Host "  $(Join-Path $downloadsDir $fallbackName)"
Write-Host ""
Write-Host "Done. Close and reopen TouchGFX Designer $DesignerVersion before creating a new project."