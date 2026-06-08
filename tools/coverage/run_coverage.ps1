param(
    [string]$Preset = "windows-clang-ninja-coverage",
    [string]$CoverageLabel = "0.7.0-alpha.3",
    [string]$LlvmCov = "llvm-cov",
    [string]$LlvmProfdata = "llvm-profdata"
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$buildDir = Join-Path $repoRoot "out\build\$Preset"
$coverageRoot = Join-Path $repoRoot "out\coverage\$CoverageLabel"
$rawDir = Join-Path $coverageRoot "raw"
$htmlDir = Join-Path $coverageRoot "html"
$summaryFile = Join-Path $coverageRoot "coverage-summary.txt"
$filesFile = Join-Path $coverageRoot "coverage-files.txt"
$profileDataFile = Join-Path $coverageRoot "coverage.profdata"

function Resolve-Tool {
    param([string]$Name)
    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }
    $defaultLlvm = Join-Path "D:\Program Files\LLVM\bin" "$Name.exe"
    if (Test-Path $defaultLlvm) {
        return $defaultLlvm
    }
    throw "Required tool '$Name' was not found on PATH or under D:\Program Files\LLVM\bin."
}

function Assert-UnderRepo {
    param([string]$Path)
    $fullPath = [System.IO.Path]::GetFullPath($Path)
    if (-not $fullPath.StartsWith($repoRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to operate outside repository root: $fullPath"
    }
}

$llvmCovPath = Resolve-Tool $LlvmCov
$llvmProfdataPath = Resolve-Tool $LlvmProfdata

Assert-UnderRepo $coverageRoot
New-Item -ItemType Directory -Force -Path $coverageRoot | Out-Null
if (Test-Path $rawDir) {
    Assert-UnderRepo $rawDir
    Remove-Item -Recurse -Force -LiteralPath $rawDir
}
if (Test-Path $htmlDir) {
    Assert-UnderRepo $htmlDir
    Remove-Item -Recurse -Force -LiteralPath $htmlDir
}
New-Item -ItemType Directory -Force -Path $rawDir | Out-Null
New-Item -ItemType Directory -Force -Path $htmlDir | Out-Null

Push-Location $repoRoot
try {
    & cmake --preset $Preset
    & cmake --build --preset $Preset

    $env:LLVM_PROFILE_FILE = (Join-Path $rawDir "ruckig-c-%p-%m.profraw")
    & ctest --test-dir $buildDir --output-on-failure -E "ruckig_c_oracle_random_release|ruckig_c_oracle_random_development|ruckig_c_tracking_random_seed_.*"
    & (Join-Path $buildDir "ruckig_c_oracle_tests.exe") --random 10000 --seed 1
    & (Join-Path $buildDir "ruckig_c_oracle_tests.exe") --random-per-dof 10000 --seed 1
    & (Join-Path $buildDir "ruckig_c_tests.exe") --tracking-random 10000 --seed 1
}
finally {
    Pop-Location
}

$rawProfiles = @(Get-ChildItem -Path $rawDir -Filter "*.profraw" -File)
if ($rawProfiles.Count -eq 0) {
    throw "No .profraw files were generated under $rawDir."
}

& $llvmProfdataPath merge -sparse @($rawProfiles | ForEach-Object { $_.FullName }) -o $profileDataFile

$objects = @()
$objects += Get-ChildItem -Path $buildDir -Filter "*.exe" -File |
    Where-Object { $_.Name -eq "ruckig_c_tests.exe" -or $_.Name -eq "ruckig_c_oracle_tests.exe" }
$objects += Get-ChildItem -Path $buildDir -Filter "example-ruckig-c-*.exe" -File
$objects = $objects | Sort-Object FullName -Unique

if ($objects.Count -eq 0) {
    throw "No coverage objects were found under $buildDir."
}

$objects | ForEach-Object { $_.FullName } | Set-Content -Encoding ascii $filesFile

$firstObject = $objects[0].FullName
$extraObjectArgs = @()
if ($objects.Count -gt 1) {
    foreach ($object in $objects[1..($objects.Count - 1)]) {
        $extraObjectArgs += "-object=$($object.FullName)"
    }
}

$ignoreRegex = "([\\/](original|test|examples|bindings|out)[\\/])"

& $llvmCovPath report `
    $firstObject `
    @extraObjectArgs `
    "-instr-profile=$profileDataFile" `
    "-ignore-filename-regex=$ignoreRegex" `
    -show-branch-summary `
    | Tee-Object -FilePath $summaryFile

& $llvmCovPath show `
    $firstObject `
    @extraObjectArgs `
    "-instr-profile=$profileDataFile" `
    "-ignore-filename-regex=$ignoreRegex" `
    -format=html `
    "-output-dir=$htmlDir"

Write-Host ""
Write-Host "Coverage summary: $summaryFile"
Write-Host "Coverage HTML: $htmlDir"
Write-Host "Coverage objects: $filesFile"
