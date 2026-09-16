param(
    [string]$Compiler = ''
)

$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot

try {
    if (-not $Compiler) {
        $gxx = Get-Command g++ -ErrorAction SilentlyContinue
        if (-not $gxx) {
            throw "Không tìm thấy g++. Hãy cài compiler C++ hoặc xem phần w64devkit trong README.md."
        }
        $Compiler = $gxx.Source
    }

    if (-not (Test-Path -LiteralPath $Compiler)) {
        throw "Không tìm thấy compiler tại: $Compiler"
    }

    New-Item -ItemType Directory -Path 'build' -Force | Out-Null
    $flags = @('-std=c++17', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-O2', '-Iinclude')
    $sharedSources = @(
        'src/config/Config.cpp',
        'src/controller/Controller.cpp',
        'src/input_sensor/InputValidator.cpp',
        'src/input_sensor/Sensor.cpp',
        'src/input_sensor/ConsoleInput.cpp'
    )

    Write-Host "Compiler: $Compiler"
    & $Compiler @flags 'src/main.cpp' @sharedSources '-o' 'build/traffic.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Build chương trình thất bại.' }

    & $Compiler @flags 'tests/test_main.cpp' @sharedSources '-o' 'build/traffic_tests.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Build smoke test thất bại.' }

    Write-Host 'Build OK.'
} finally {
    Pop-Location
}
