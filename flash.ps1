# flash.ps1 - Flash the nRF24 serial-chat firmware to an ESP32 board.
#
# Targets:  esp32   = classic ESP32 (DevKit / WROOM)
#           esp32c3 = ESP32-C3 Supermini
#
# Usage (run from this folder):
#   .\flash.ps1 esp32              # auto-detect COM port, flash classic ESP32
#   .\flash.ps1 esp32c3            # auto-detect COM port, flash ESP32-C3
#   .\flash.ps1 esp32 -Port COM9   # flash a specific port
#
# Requires arduino-cli in tools\ (download from
# https://arduino.github.io/arduino-cli/ and drop arduino-cli.exe in tools\).
# Compiles fresh and uploads. The classic ESP32 is flashed at a reliable 115200
# baud (its CP210x can corrupt uploads at the 921600 default).

param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('esp32', 'esp32c3')]
    [string]$Target,

    [string]$Port
)

$ErrorActionPreference = 'Stop'
$cli = Join-Path $PSScriptRoot 'tools\arduino-cli.exe'
if (-not (Test-Path $cli)) { throw "arduino-cli not found at $cli (see this script's header)." }

$sketch = Join-Path $PSScriptRoot 'nrf24_serial_chat'

if ($Target -eq 'esp32c3') {
    $fqbn = 'esp32:esp32:esp32c3'
    $wantVid = '0x303A'          # ESP32-C3 native USB (Espressif)
} else {
    $fqbn = 'esp32:esp32:esp32:UploadSpeed=115200'
    $wantVid = '0x10C4'          # classic ESP32 via CP210x (Silicon Labs)
}

$buildDir = Join-Path $PSScriptRoot ("build_" + $Target)

Write-Host "Compiling '$Target'..." -ForegroundColor Cyan
& $cli compile --fqbn $fqbn --output-dir $buildDir --clean $sketch

# Auto-detect the COM port by USB vendor ID when not supplied.
if (-not $Port) {
    $found = & $cli board list --format json | ConvertFrom-Json
    $serial = @($found.detected_ports | Where-Object { $_.port.protocol -eq 'serial' })
    if ($serial.Count -eq 0) { throw "No serial port detected. Plug in the board or pass -Port COMx." }

    $match = $serial | Where-Object { $_.port.properties.vid -ieq $wantVid } | Select-Object -First 1
    if (-not $match -and $serial.Count -eq 1) { $match = $serial[0] }
    if (-not $match) {
        $list = ($serial | ForEach-Object { "$($_.port.address) (vid $($_.port.properties.vid))" }) -join ', '
        throw "Could not pick a port for '$Target' by USB VID ($wantVid). Ports: $list. Pass -Port COMx."
    }
    $Port = $match.port.address
    Write-Host "Auto-detected port for '$Target': $Port"
}

Write-Host "Flashing '$Target' to $Port ..." -ForegroundColor Cyan
& $cli upload -p $Port --fqbn $fqbn --input-dir $buildDir
Write-Host "Done. Open the serial monitor at 115200 baud." -ForegroundColor Green
