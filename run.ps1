param(
    [switch]$Test
)

$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot

try {
    if (-not (Test-Path -LiteralPath 'build/traffic.exe') -or
        -not (Test-Path -LiteralPath 'build/traffic_tests.exe')) {
        & './build.ps1'
    }

    if ($Test) {
        & './build/traffic_tests.exe'
    } else {
        & './build/traffic.exe'
    }

    if ($LASTEXITCODE -ne 0) {
        throw "Chương trình kết thúc với mã lỗi $LASTEXITCODE."
    }
} finally {
    Pop-Location
}
