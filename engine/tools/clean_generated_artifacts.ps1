# Clean selected, reproducible engine build output. Dry run is the default.
# Usage: ./engine/tools/clean_generated_artifacts.ps1 [-IncludeReleaseBinaries] [-IncludeWsl] [-Apply]
[CmdletBinding()]
param(
    [switch]$Apply,
    [switch]$IncludeReleaseBinaries,
    [switch]$IncludeWsl
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$engineRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..')).TrimEnd('\', '/')
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $engineRoot '..')).TrimEnd('\', '/')
$enginePrefix = $engineRoot + [System.IO.Path]::DirectorySeparatorChar
$reparsePoint = [System.IO.FileAttributes]::ReparsePoint
$protectedExtensions = @('.zip', '.csv', '.tsv', '.parquet', '.h5', '.hdf5')

function Assert-InEngine([string]$path) {
    $fullPath = [System.IO.Path]::GetFullPath($path)
    if (-not $fullPath.StartsWith($enginePrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing path outside engine: $fullPath"
    }
    return $fullPath
}

function Assert-NoReparsePath([string]$path) {
    $current = [System.IO.Path]::GetFullPath($path)
    while ($true) {
        $item = Get-Item -LiteralPath $current -Force
        if (($item.Attributes -band $reparsePoint) -ne 0) {
            throw "Refusing reparse point: $current"
        }
        if ([string]::Equals($current, $engineRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
            break
        }
        $parent = Split-Path -Path $current -Parent
        if (-not $parent -or [string]::Equals($parent, $current, [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "Could not reach engine root from: $path"
        }
        $current = $parent
    }
}

function Assert-IgnoredUntracked([string]$relativePath) {
    $gitPath = 'engine/' + $relativePath.Replace('\', '/')
    & git -C $repoRoot check-ignore -q -- $gitPath
    if ($LASTEXITCODE -ne 0) {
        throw "Refusing non-ignored path: $gitPath"
    }
    $tracked = @(& git -C $repoRoot ls-files --cached --full-name -- ":(literal)$gitPath")
    if ($LASTEXITCODE -ne 0 -or $tracked.Count -ne 0) {
        throw "Refusing path containing tracked files: $gitPath"
    }
}

function Inspect-Directory([string]$path, [switch]$PreserveResults, [switch]$AllowProtected) {
    $stack = [System.Collections.Generic.Stack[string]]::new()
    $stack.Push($path)
    [long]$fileCount = 0
    [long]$directoryCount = 1
    [long]$byteCount = 0
    $protectedPath = $null
    $preservedDirectories = [System.Collections.Generic.List[string]]::new()

    while ($stack.Count -gt 0) {
        $current = $stack.Pop()
        foreach ($child in Get-ChildItem -LiteralPath $current -Force) {
            $childPath = Assert-InEngine $child.FullName
            if (($child.Attributes -band $reparsePoint) -ne 0) {
                throw "Refusing reparse point inside candidate: $childPath"
            }
            if ($child.PSIsContainer) {
                if ($child.Name -ieq 'results' -and $PreserveResults) {
                    $preservedDirectories.Add($childPath)
                    continue
                }
                $directoryCount++
                if ($child.Name -ieq 'results' -and -not $AllowProtected) {
                    $protectedPath = $childPath
                    break
                }
                $stack.Push($childPath)
            } else {
                $fileCount++
                $byteCount += $child.Length
                if (-not $AllowProtected -and ($child.Extension -in $protectedExtensions -or
                    $child.Name -match '(?i)(^|[_-])results?([_.-]|$)')) {
                    $protectedPath = $childPath
                    break
                }
            }
        }
        if ($protectedPath) { break }
    }

    return [pscustomobject]@{
        Files = $fileCount
        Directories = $directoryCount
        Bytes = $byteCount
        ProtectedPath = $protectedPath
        PreservedDirectories = $preservedDirectories.ToArray()
    }
}

function Format-Bytes([long]$count) {
    if ($count -ge 1GB) { return ('{0:N2} GiB' -f ($count / 1GB)) }
    if ($count -ge 1MB) { return ('{0:N2} MiB' -f ($count / 1MB)) }
    if ($count -ge 1KB) { return ('{0:N2} KiB' -f ($count / 1KB)) }
    return "$count B"
}

Assert-NoReparsePath $engineRoot

# These are cold build directories, not active WSL builds, sealed studies, or runtime output.
$coldDirectories = @(
    'build_ci_source_gate_audit',
    'build_wasm',
    'build_wasm64',
    'build_wasm_mt',
    'build/Debug',
    'build/CMakeFiles',
    'build/native',
    'build/cuda',
    'build/Testing',
    'build/RelWithDebInfo',
    'build/__pycache__'
)

$candidates = [System.Collections.Generic.List[object]]::new()
$skipped = [System.Collections.Generic.List[string]]::new()
foreach ($relativePath in $coldDirectories) {
    $fullPath = Assert-InEngine (Join-Path $engineRoot $relativePath)
    if (-not (Test-Path -LiteralPath $fullPath)) { continue }
    Assert-NoReparsePath $fullPath
    if (-not (Get-Item -LiteralPath $fullPath -Force).PSIsContainer) {
        throw "Expected a directory: $fullPath"
    }
    Assert-IgnoredUntracked $relativePath
    $inventory = Inspect-Directory $fullPath
    if ($inventory.ProtectedPath) {
        $skipped.Add("$relativePath contains protected output: $($inventory.ProtectedPath)")
        continue
    }
    $candidates.Add([pscustomobject]@{
        RelativePath = $relativePath
        FullPath = $fullPath
        IsDirectory = $true
        Files = $inventory.Files
        Directories = $inventory.Directories
        Bytes = $inventory.Bytes
    })
}

$wslPreserved = [System.Collections.Generic.List[object]]::new()
if ($IncludeWsl) {
    $wslSource = Assert-InEngine (Join-Path $engineRoot 'build_wsl')
    $wslDestination = Assert-InEngine (Join-Path $engineRoot 'results/build_wsl_preserved_20260924')
    Assert-NoReparsePath (Join-Path $engineRoot 'results')
    Assert-IgnoredUntracked 'results/build_wsl_preserved_20260924'
    if (Test-Path -LiteralPath $wslSource) {
        if (Test-Path -LiteralPath $wslDestination) {
            throw "Preservation destination already exists: $wslDestination"
        }
        Assert-NoReparsePath $wslSource
        Assert-IgnoredUntracked 'build_wsl'
        $inventory = Inspect-Directory $wslSource -PreserveResults
        if ($inventory.ProtectedPath) {
            throw "Protected WSL output outside a results subtree: $($inventory.ProtectedPath)"
        }
        foreach ($source in $inventory.PreservedDirectories) {
            Assert-NoReparsePath $source
            $relativeResultsPath = $source.Substring($wslSource.Length + 1)
            $destination = Assert-InEngine (Join-Path $wslDestination $relativeResultsPath)
            $preservedInventory = Inspect-Directory $source -AllowProtected
            $wslPreserved.Add([pscustomobject]@{
                Source = $source
                Destination = $destination
                Files = $preservedInventory.Files
                Directories = $preservedInventory.Directories
                Bytes = $preservedInventory.Bytes
            })
        }
        $candidates.Add([pscustomobject]@{
            RelativePath = 'build_wsl'
            FullPath = $wslSource
            IsDirectory = $true
            Files = $inventory.Files
            Directories = $inventory.Directories
            Bytes = $inventory.Bytes
        })
    }
}

if ($IncludeReleaseBinaries) {
    $releasePath = Assert-InEngine (Join-Path $engineRoot 'build/Release')
    if (Test-Path -LiteralPath $releasePath) {
        Assert-NoReparsePath $releasePath
        Assert-IgnoredUntracked 'build/Release'
        foreach ($file in Get-ChildItem -LiteralPath $releasePath -Force -File) {
            if ($file.Extension -notin @('.exe', '.lib') -or $file.Name -ieq 'ws_server.exe') {
                continue
            }
            $fullPath = Assert-InEngine $file.FullName
            if (($file.Attributes -band $reparsePoint) -ne 0) {
                throw "Refusing reparse point: $fullPath"
            }
            $relativePath = 'build/Release/' + $file.Name
            $candidates.Add([pscustomobject]@{
                RelativePath = $relativePath
                FullPath = $fullPath
                IsDirectory = $false
                Files = 1L
                Directories = 0L
                Bytes = [long]$file.Length
            })
        }
    }
}

[long]$totalFiles = 0
[long]$totalDirectories = 0
[long]$totalBytes = 0
foreach ($candidate in $candidates) {
    $totalFiles += $candidate.Files
    $totalDirectories += $candidate.Directories
    $totalBytes += $candidate.Bytes
    Write-Output ("{0,-45} {1,7} files  {2,10}" -f $candidate.RelativePath, $candidate.Files, (Format-Bytes $candidate.Bytes))
}

Write-Output ("Selected: {0} paths, {1} files, {2} directories, {3}" -f
    $candidates.Count, $totalFiles, $totalDirectories, (Format-Bytes $totalBytes))
foreach ($reason in $skipped) { Write-Warning "Preserved $reason" }
if ($IncludeWsl) {
    [long]$preservedBytes = 0
    foreach ($item in $wslPreserved) {
        $preservedBytes += $item.Bytes
        Write-Output ("Preserve {0} -> {1} ({2} files, {3})" -f
            $item.Source, $item.Destination, $item.Files, (Format-Bytes $item.Bytes))
    }
    Write-Output ("WSL results preserved: {0} subtrees, {1}; reclaimable WSL bytes are included in Selected." -f
        $wslPreserved.Count, (Format-Bytes $preservedBytes))
} else {
    Write-Output 'Preserved: build_wsl (add -IncludeWsl to select it).'
}
Write-Output 'Also preserved: strict build/runtime output, sealed studies, .cache, results, ZIPs, deployed web/wasm, build/Release/engine/results, and ws_server.exe.'
if (-not $IncludeReleaseBinaries) {
    Write-Output 'Root build/Release .exe/.lib files are excluded; add -IncludeReleaseBinaries to select them (ws_server.exe always stays).'
}

if (-not $Apply) {
    Write-Output 'Dry run only. Pass -Apply to delete the selected paths.'
    return
}

# Recheck the full plan before the first deletion; an active build may have changed it.
if ($IncludeReleaseBinaries -and (Test-Path -LiteralPath (Join-Path $engineRoot 'build/Release'))) {
    Assert-IgnoredUntracked 'build/Release'
}
foreach ($candidate in $candidates) {
    if ($candidate.IsDirectory) {
        Assert-NoReparsePath $candidate.FullPath
        Assert-IgnoredUntracked $candidate.RelativePath
        if ($candidate.RelativePath -eq 'build_wsl') {
            Assert-NoReparsePath (Join-Path $engineRoot 'results')
            Assert-IgnoredUntracked 'results/build_wsl_preserved_20260924'
            if (Test-Path -LiteralPath $wslDestination) {
                throw "Preservation destination already exists: $wslDestination"
            }
            $fresh = Inspect-Directory $candidate.FullPath -PreserveResults
            $plannedPaths = @($wslPreserved | ForEach-Object Source | Sort-Object)
            $freshPaths = @($fresh.PreservedDirectories | Sort-Object)
            if (($plannedPaths -join "`n") -ne ($freshPaths -join "`n")) {
                throw 'WSL results subtrees changed during preflight.'
            }
            foreach ($item in $wslPreserved) {
                Assert-NoReparsePath $item.Source
                $result = Inspect-Directory $item.Source -AllowProtected
                if ($result.Files -ne $item.Files -or $result.Directories -ne $item.Directories -or
                    $result.Bytes -ne $item.Bytes) {
                    throw "WSL results changed during preflight: $($item.Source)"
                }
            }
        } else {
            $fresh = Inspect-Directory $candidate.FullPath
        }
        if ($fresh.ProtectedPath -or $fresh.Files -ne $candidate.Files -or
            $fresh.Directories -ne $candidate.Directories -or $fresh.Bytes -ne $candidate.Bytes) {
            throw "Candidate changed during preflight: $($candidate.FullPath)"
        }
    } else {
        $file = Get-Item -LiteralPath $candidate.FullPath -Force
        if (($file.Attributes -band $reparsePoint) -ne 0 -or $file.Length -ne $candidate.Bytes) {
            throw "Candidate changed during preflight: $($candidate.FullPath)"
        }
    }
}

if ($IncludeWsl -and $wslPreserved.Count -gt 0) {
    if (Test-Path -LiteralPath $wslDestination) {
        throw "Preservation destination already exists: $wslDestination"
    }
    $null = New-Item -ItemType Directory -Path $wslDestination
    foreach ($item in $wslPreserved) {
        $parent = Split-Path -Path $item.Destination -Parent
        if (-not (Test-Path -LiteralPath $parent)) {
            $null = New-Item -ItemType Directory -Path $parent -Force
        }
        Assert-NoReparsePath $parent
        Move-Item -LiteralPath $item.Source -Destination $item.Destination
        Write-Output "Preserved $($item.Source) at $($item.Destination)"
    }
}

foreach ($candidate in $candidates) {
    if ($candidate.IsDirectory) {
        if ($candidate.RelativePath -eq 'build_wsl') {
            $remaining = Inspect-Directory $candidate.FullPath
            if ($remaining.ProtectedPath -or $remaining.Files -ne $candidate.Files -or
                $remaining.Directories -ne $candidate.Directories -or $remaining.Bytes -ne $candidate.Bytes) {
                throw 'WSL build changed after results preservation; refusing to remove it.'
            }
        }
        Remove-Item -LiteralPath $candidate.FullPath -Recurse -Force
    } else {
        Remove-Item -LiteralPath $candidate.FullPath -Force
    }
    Write-Output "Removed $($candidate.RelativePath)"
}
Write-Output "Removed $($candidates.Count) selected paths, $(Format-Bytes $totalBytes)."
