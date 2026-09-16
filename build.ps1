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

    $flags = @(
        '-std=c++17',
        '-Wall',
        '-Wextra',
        '-Wpedantic',
        '-Werror',
        '-O2',
        '-Iinclude'
    )

    # Tự động lấy toàn bộ file .cpp trong src
    $sourceFiles = Get-ChildItem `
        -Path 'src' `
        -Filter '*.cpp' `
        -Recurse |
        ForEach-Object { $_.FullName }

    Write-Host "Compiler: $Compiler"

    Write-Host "Source files:"
    $sourceFiles | ForEach-Object {
        Write-Host "  $_"
    }

    # Build chương trình chính
    & $Compiler @flags @sourceFiles '-o' 'build/traffic.exe'

    if ($LASTEXITCODE -ne 0) {
        throw 'Build chương trình thất bại.'
    }

    # Build test
    $testSources = @(
        'tests/test_main.cpp'
    )

    # Thêm các source cần cho test nhưng bỏ main.cpp
    $librarySources = $sourceFiles | Where-Object {
        $_ -notmatch '[\\/]main\.cpp$'
    }

    & $Compiler `
        @flags `
        @testSources `
        @librarySources `
        '-o' `
        'build/traffic_tests.exe'

    if ($LASTEXITCODE -ne 0) {
        throw 'Build smoke test thất bại.'
    }

    Write-Host 'Build OK.'
}
finally {
    Pop-Location
}