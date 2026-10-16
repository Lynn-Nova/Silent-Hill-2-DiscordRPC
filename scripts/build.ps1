param([string]$ZigPath = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $projectRoot 'build'
New-Item -ItemType Directory -Path $buildDirectory -Force | Out-Null

function Invoke-Checked {
    param([string]$Program, [string[]]$Arguments)
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "Build command failed: $Program" }
}

if ($ZigPath) {
    $compiler = (Resolve-Path -LiteralPath $ZigPath).Path
    $compilerDirectory = Split-Path -Parent $compiler
    $common = @('c++', '-target', 'x86_64-windows-gnu', '-std=c++17', '-O2', '-static', '-s', '-DNDEBUG', "-ffile-prefix-map=$projectRoot=.", "-ffile-prefix-map=$compilerDirectory=zig", '-DUNICODE', '-D_UNICODE', '-DWIN32_LEAN_AND_MEAN', '-DNOMINMAX', '-D_WIN32_WINNT=0x0a00', '-I', (Join-Path $projectRoot 'src'), '-I', (Join-Path $projectRoot 'third_party'))
    $core = Join-Path $projectRoot 'src\core.cpp'
    $windows = Join-Path $projectRoot 'src\windows.cpp'
    Invoke-Checked $compiler ($common + @('-shared', $core, $windows, (Join-Path $projectRoot 'src\mod.cpp'), '-o', (Join-Path $buildDirectory 'SH2DiscordPresence.asi')))
    Invoke-Checked $compiler ($common + @($core, (Join-Path $projectRoot 'tests\core_tests.cpp'), '-o', (Join-Path $buildDirectory 'core_tests.exe')))
    Invoke-Checked $compiler ($common + @($core, $windows, (Join-Path $projectRoot 'tests\ipc_client.cpp'), '-o', (Join-Path $buildDirectory 'ipc_test_client.exe')))
    Invoke-Checked $compiler ($common + @('-shared', (Join-Path $projectRoot 'tests\steam_api_stub.cpp'), '-o', (Join-Path $buildDirectory 'steam_api_stub.dll')))
    Invoke-Checked $compiler ($common + @($core, $windows, (Join-Path $projectRoot 'tests\steam_reader.cpp'), '-o', (Join-Path $buildDirectory 'steam_reader_test.exe')))
    Invoke-Checked $compiler ($common + @((Join-Path $projectRoot 'tests\mod_host.cpp'), '-o', (Join-Path $buildDirectory 'SHProto-Win64-Shipping.exe')))
} else {
    Invoke-Checked 'cmake' @('-S', $projectRoot, '-B', $buildDirectory, '-A', 'x64')
    Invoke-Checked 'cmake' @('--build', $buildDirectory, '--config', 'Release', '--parallel')
    Invoke-Checked 'ctest' @('--test-dir', $buildDirectory, '-C', 'Release', '--output-on-failure')
}
