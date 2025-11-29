$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$dist = Join-Path $root 'dist'
$x64Out = Join-Path $root 'out\x64\Release'
$x86Out = Join-Path $root 'out\x86\Release'
$hdrSrc = Join-Path $root 'src\petag_api.h'
$licenseSrc = Join-Path $root 'LICENSE'

New-Item -ItemType Directory -Force -Path $dist | Out-Null
$x64Pkg = Join-Path $dist 'win-x64'
$x86Pkg = Join-Path $dist 'win-x86'
New-Item -ItemType Directory -Force -Path (Join-Path $x64Pkg 'bin') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $x64Pkg 'include') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $x86Pkg 'bin') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $x86Pkg 'include') | Out-Null

if (Test-Path $hdrSrc) { Copy-Item $hdrSrc (Join-Path $x64Pkg 'include') -Force }
if (Test-Path $hdrSrc) { Copy-Item $hdrSrc (Join-Path $x86Pkg 'include') -Force }
if (Test-Path $licenseSrc) { Copy-Item $licenseSrc $x64Pkg -Force }
if (Test-Path $licenseSrc) { Copy-Item $licenseSrc $x86Pkg -Force }

$x64Files = @('petag2.exe','petag.dll','petag2.pdb','petag.pdb') | ForEach-Object { Join-Path $x64Out $_ }
foreach ($f in $x64Files) { if (Test-Path $f) { Copy-Item $f (Join-Path $x64Pkg 'bin') -Force } }

$x86Files = @('petag2.exe','petag.dll','petag2.pdb','petag.pdb') | ForEach-Object { Join-Path $x86Out $_ }
foreach ($f in $x86Files) { if (Test-Path $f) { Copy-Item $f (Join-Path $x86Pkg 'bin') -Force } }

$x64Zip = Join-Path $dist 'win-x64.zip'
$x86Zip = Join-Path $dist 'win-x86.zip'
if (Test-Path $x64Zip) { Remove-Item $x64Zip -Force }
if (Test-Path $x86Zip) { Remove-Item $x86Zip -Force }
if (Get-ChildItem (Join-Path $x64Pkg 'bin') -ErrorAction SilentlyContinue) { Compress-Archive -Path $x64Pkg -DestinationPath $x64Zip -Force }
if (Get-ChildItem (Join-Path $x86Pkg 'bin') -ErrorAction SilentlyContinue) { Compress-Archive -Path $x86Pkg -DestinationPath $x86Zip -Force }

Write-Host "x64: " (Test-Path $x64Zip)
Write-Host "x86: " (Test-Path $x86Zip)
