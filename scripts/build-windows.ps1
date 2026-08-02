# SPDX-FileCopyrightText: 2026 YumiMDWriter contributors
#
# SPDX-License-Identifier: GPL-3.0-or-later

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$CraftRoot,

    [string]$BuildDirectory = (Join-Path $PSScriptRoot '..\build'),
    [string]$InstallDirectory = (Join-Path $PSScriptRoot '..\stage'),
    [switch]$SkipTests
)

$ErrorActionPreference = 'Stop'

$sourceDirectory = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$cmake = Join-Path $CraftRoot 'dev-utils\cmake-base\bin\cmake.exe'
$ctest = Join-Path $CraftRoot 'dev-utils\cmake-base\bin\ctest.exe'

if (-not (Test-Path -LiteralPath $cmake -PathType Leaf)) {
    throw "CMake was not found under CraftRoot: $cmake"
}

if (-not (Test-Path -LiteralPath $ctest -PathType Leaf)) {
    throw "CTest was not found under CraftRoot: $ctest"
}

& $cmake `
    -S $sourceDirectory `
    -B $BuildDirectory `
    -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DBUILD_TESTING=ON `
    "-DCMAKE_PREFIX_PATH=$CraftRoot" `
    "-DCMAKE_INSTALL_PREFIX=$InstallDirectory"

if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed with exit code $LASTEXITCODE."
}

& $cmake --build $BuildDirectory --config Release

if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE."
}

if (-not $SkipTests) {
    $env:Path = (Join-Path $CraftRoot 'bin') + ';' + $env:Path
    & $ctest `
        --test-dir $BuildDirectory `
        -C Release `
        --output-on-failure

    if ($LASTEXITCODE -ne 0) {
        throw "Tests failed with exit code $LASTEXITCODE."
    }
}

& $cmake --install $BuildDirectory --config Release

if ($LASTEXITCODE -ne 0) {
    throw "Installation failed with exit code $LASTEXITCODE."
}

Write-Host "Build complete: $(Join-Path $BuildDirectory 'bin\YumiMDWriter.exe')"
Write-Host "Install tree: $InstallDirectory"
