param([string]$LoaderArchive = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $projectRoot 'build'
$plugin = Join-Path $buildDirectory 'Release\SH2DiscordPresence.asi'
if (-not (Test-Path -LiteralPath $plugin)) { $plugin = Join-Path $buildDirectory 'SH2DiscordPresence.asi' }
if (-not (Test-Path -LiteralPath $plugin)) { throw 'Build the plugin before packaging.' }

if (-not $LoaderArchive) {
    $LoaderArchive = Join-Path $buildDirectory 'Ultimate-ASI-Loader-v9.7.4-x64.zip'
    Invoke-WebRequest -Uri 'https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/v9.7.4/Ultimate-ASI-Loader-NoPDB_x64.zip' -OutFile $LoaderArchive -TimeoutSec 60
}
if ((Get-FileHash -LiteralPath $LoaderArchive -Algorithm SHA256).Hash -ne 'e5860e7d9a1805267535b65749575b5e406cc6ea3325c7392189c578815045d1') {
    throw 'Ultimate ASI Loader SHA-256 verification failed.'
}
$loaderDirectory = Join-Path $buildDirectory 'loader-v9.7.4'
Expand-Archive -LiteralPath $LoaderArchive -DestinationPath $loaderDirectory -Force
$releaseDirectory = Join-Path $projectRoot 'release'
$packageDirectory = Join-Path $releaseDirectory ('package-' + [guid]::NewGuid().ToString('N'))
$licensesDirectory = Join-Path $packageDirectory 'licenses'
New-Item -ItemType Directory -Path $licensesDirectory -Force | Out-Null
Copy-Item -LiteralPath $plugin -Destination (Join-Path $packageDirectory 'SH2DiscordPresence.asi') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'config\SH2DiscordPresence.json') -Destination $packageDirectory -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'config\winmm.ini') -Destination $packageDirectory -Force
Copy-Item -LiteralPath (Join-Path $loaderDirectory 'dinput8.dll') -Destination (Join-Path $packageDirectory 'winmm.dll') -Force
foreach ($document in @('README.md', 'README.pt-BR.md', 'LICENSE', 'THIRD_PARTY_NOTICES.md')) {
    Copy-Item -LiteralPath (Join-Path $projectRoot $document) -Destination $packageDirectory -Force
}
Copy-Item -LiteralPath (Join-Path $projectRoot 'docs') -Destination $packageDirectory -Recurse -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'third_party\nlohmann\LICENSE.MIT') -Destination (Join-Path $licensesDirectory 'nlohmann-json.LICENSE') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'third_party\Ultimate-ASI-Loader.LICENSE') -Destination $licensesDirectory -Force
Get-ChildItem -LiteralPath (Join-Path $projectRoot 'third_party\toolchain') -File | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination $licensesDirectory -Force
}
$archive = Join-Path $releaseDirectory 'SH2DiscordPresence-0.1.3-win64.zip'
Compress-Archive -Path (Join-Path $packageDirectory '*') -DestinationPath $archive -Force
$hash = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -LiteralPath ($archive + '.sha256') -Value ($hash + '  ' + (Split-Path -Leaf $archive)) -Encoding ascii
Write-Output $archive
