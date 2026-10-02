param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectRoot
)

$ErrorActionPreference = "Stop"

$root = (Resolve-Path -LiteralPath $ProjectRoot).Path
$packageRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path

Write-Host "Applying DLC035 SSD2119 TouchGFX port to $root"

$incDir = Join-Path $root "Core\Inc"
$srcDir = Join-Path $root "Core\Src"
$targetDir = Join-Path $root "TouchGFX\target"

Copy-Item -LiteralPath (Join-Path $packageRoot "source\Core\Inc\ssd2119.h") -Destination $incDir -Force
Copy-Item -LiteralPath (Join-Path $packageRoot "source\Core\Inc\Adafruit_GFX.h") -Destination $incDir -Force
Copy-Item -LiteralPath (Join-Path $packageRoot "source\Core\Src\ssd2119.c") -Destination $srcDir -Force

Copy-Item -LiteralPath (Join-Path $packageRoot "reference\TouchGFXHAL.cpp") -Destination (Join-Path $targetDir "TouchGFXHAL.cpp") -Force

$appFreeRtos = Join-Path $srcDir "app_freertos.c"
$text = Get-Content -LiteralPath $appFreeRtos -Raw
if ($text -notmatch "touchgfxSignalVSync") {
    $text = $text -replace "void MX_FREERTOS_Init\(void\);", "void MX_FREERTOS_Init(void);`r`nextern void touchgfxSignalVSync(void);"
}
$text = $text -replace "osDelay\(1\);", "touchgfxSignalVSync();`r`n    osDelay(16);"
Set-Content -LiteralPath $appFreeRtos -Value $text -Encoding ASCII

$keySampler = Join-Path $targetDir "KeySampler.cpp"
if (Test-Path -LiteralPath $keySampler) {
    $text = Get-Content -LiteralPath $keySampler -Raw
    $text = $text -replace "BUTTON_USER_GPIO_Port", "BUTTON_GPIO_Port"
    $text = $text -replace "BUTTON_USER_Pin", "BUTTON_Pin"
    Set-Content -LiteralPath $keySampler -Value $text -Encoding ASCII
}

Write-Host "Done. Re-check STM32CubeIDE/.project includes Core/Src/ssd2119.c and does not include unused FMC memory HAL sources."

