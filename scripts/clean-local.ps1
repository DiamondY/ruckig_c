param(
    [switch]$Apply,
    [switch]$KeepReleaseBuilds
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptDir "..")
Push-Location $repoRoot
try {
    $safeDirectory = (Get-Item $repoRoot).FullName -replace "\\", "/"
    $mode = if ($Apply) { "-fdffX" } else { "-ndffX" }
    $args = @("-c", "safe.directory=$safeDirectory", "clean", $mode)

    if ($KeepReleaseBuilds) {
        $args += @(
            "-e", "build_release_check_ninja/",
            "-e", "build_release_check_shared/",
            "-e", "out/build/release/",
            "-e", "out/build/shared/"
        )
    }

    if (-not $Apply) {
        Write-Host "Dry run only. Re-run with -Apply to delete ignored local artifacts."
        Write-Host "Nested ignored Git checkouts, such as local tool caches, are included in the preview."
    }

    & git @args
}
finally {
    Pop-Location
}
