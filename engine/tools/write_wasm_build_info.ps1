param(
    [Parameter(Mandatory = $true)][string]$StageDir,
    [Parameter(Mandatory = $true)][string]$SourceSha,
    [Parameter(Mandatory = $true)][string]$EmccVersion,
    [Parameter(Mandatory = $true)][string]$CmakeVersion
)

$ErrorActionPreference = 'Stop'

function Get-ArtifactFingerprint {
    param(
        [Parameter(Mandatory = $true)][string]$LiteralPath
    )
    $stream = [System.IO.File]::Open(
        $LiteralPath,
        [System.IO.FileMode]::Open,
        [System.IO.FileAccess]::Read,
        [System.IO.FileShare]::Read
    )
    $sha256 = $null
    try {
        $sizeBytes = [int64]$stream.Length
        $sha256 = [System.Security.Cryptography.SHA256]::Create()
        $hash = ([System.BitConverter]::ToString(
            $sha256.ComputeHash($stream))).Replace('-', '').ToLowerInvariant()
    } finally {
        if ($null -ne $sha256) {
            $sha256.Dispose()
        }
        $stream.Dispose()
    }
    [pscustomobject][ordered]@{
        sizeBytes = $sizeBytes
        sha256 = $hash
    }
}

$dirty = $SourceSha.EndsWith('-dirty', [System.StringComparison]::Ordinal)
$commit = if ($dirty) {
    $SourceSha.Substring(0, $SourceSha.Length - '-dirty'.Length)
} else {
    $SourceSha
}
if ($commit -notmatch '^[0-9a-f]{40}$') {
    throw "Source commit is not an exact 40-character Git SHA: $commit"
}

function New-ArtifactRecord {
    param(
        [Parameter(Mandatory = $true)][string]$Role,
        [Parameter(Mandatory = $true)][string]$File
    )
    $path = Join-Path $StageDir $File
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Required staged artifact is missing: $File"
    }
    $fingerprint = Get-ArtifactFingerprint -LiteralPath $path
    [ordered]@{
        role = $Role
        file = $File
        sizeBytes = $fingerprint.sizeBytes
        sha256 = $fingerprint.sha256
    }
}

function New-VariantRecord {
    param(
        [Parameter(Mandatory = $true)][string]$Id,
        [Parameter(Mandatory = $true)][int]$PointerBits,
        [Parameter(Mandatory = $true)][bool]$Memory64,
        [Parameter(Mandatory = $true)][bool]$Threads,
        [Parameter(Mandatory = $true)][string]$Factory,
        [Parameter(Mandatory = $true)][string[]]$CmakeFlags,
        [Parameter(Mandatory = $true)][string]$Loader,
        [Parameter(Mandatory = $true)][string]$Module
    )
    [ordered]@{
        id = $Id
        factory = $Factory
        abi = [ordered]@{
            pointerBits = $PointerBits
            memory64 = $Memory64
            threads = $Threads
            sharedMemory = $Threads
        }
        cmakeFlags = @($CmakeFlags)
        artifacts = @(
            (New-ArtifactRecord -Role 'loader' -File $Loader),
            (New-ArtifactRecord -Role 'module' -File $Module)
        )
    }
}

$variants = @(
    (New-VariantRecord -Id 'wasm32' -PointerBits 32 -Memory64 $false -Threads $false `
        -Factory 'createFTDModule' `
        -CmakeFlags @('-DCMAKE_BUILD_TYPE=Release', '-DFTD_MEMORY64=OFF', '-DFTD_WASM_THREADS=OFF') `
        -Loader 'ftd_core.js' -Module 'ftd_core.wasm'),
    (New-VariantRecord -Id 'wasm64' -PointerBits 64 -Memory64 $true -Threads $false `
        -Factory 'createFTDModule64' `
        -CmakeFlags @('-DCMAKE_BUILD_TYPE=Release', '-DFTD_MEMORY64=ON', '-DFTD_WASM_THREADS=OFF') `
        -Loader 'ftd_core64.js' -Module 'ftd_core64.wasm'),
    (New-VariantRecord -Id 'wasm32-threads' -PointerBits 32 -Memory64 $false -Threads $true `
        -Factory 'createFTDModuleMT' `
        -CmakeFlags @('-DCMAKE_BUILD_TYPE=Release', '-DFTD_MEMORY64=OFF', '-DFTD_WASM_THREADS=ON') `
        -Loader 'ftd_core_mt.js' -Module 'ftd_core_mt.wasm')
)

$canonical = [System.Text.StringBuilder]::new()
[void]$canonical.Append("ftd-wasm-bundle-v1`n")
foreach ($variant in $variants) {
    foreach ($artifact in $variant.artifacts) {
        [void]$canonical.Append($artifact.file)
        [void]$canonical.Append([char]0)
        [void]$canonical.Append(
            $artifact.sizeBytes.ToString([System.Globalization.CultureInfo]::InvariantCulture)
        )
        [void]$canonical.Append([char]0)
        [void]$canonical.Append($artifact.sha256)
        [void]$canonical.Append("`n")
    }
}
$sha256 = [System.Security.Cryptography.SHA256]::Create()
try {
    $bundleBytes = [System.Text.Encoding]::UTF8.GetBytes($canonical.ToString())
    $bundleSha256 = ([System.BitConverter]::ToString(
        $sha256.ComputeHash($bundleBytes))).Replace('-', '').ToLowerInvariant()
} finally {
    $sha256.Dispose()
}

$manifest = [ordered]@{
    schemaVersion = 1
    bundleSha256 = $bundleSha256
    source = [ordered]@{
        commit = $commit
        dirty = $dirty
        scope = 'engine/** excluding generated engine/web/wasm{,.next,.previous}/**'
    }
    toolchain = [ordered]@{
        emcc = $EmccVersion
        cmake = $CmakeVersion
        generator = 'MinGW Makefiles'
        powershell = $PSVersionTable.PSVersion.ToString()
        buildType = 'Release'
    }
    variants = $variants
}

$json = ($manifest | ConvertTo-Json -Depth 8) -replace "`r`n?", "`n"
$output = Join-Path $StageDir 'build_info.json'
[System.IO.File]::WriteAllText(
    $output,
    $json + "`n",
    [System.Text.UTF8Encoding]::new($false)
)
