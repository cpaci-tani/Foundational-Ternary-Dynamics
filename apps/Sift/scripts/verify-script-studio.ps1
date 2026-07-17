[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$distRoot = Join-Path $projectRoot 'WebAssets\dist'
$manifestPath = Join-Path $distRoot 'asset-manifest.json'

if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
  throw 'Script Studio asset manifest is missing. Run npm ci and npm run build from apps\Sift.'
}

$manifest = Get-Content -Raw -LiteralPath $manifestPath | ConvertFrom-Json
if ($manifest.schemaVersion -ne 1) { throw 'Script Studio asset manifest schema is unsupported.' }

function Assert-Hashes([object]$Entries, [string]$Root, [string]$Kind) {
  foreach ($entry in $Entries.PSObject.Properties) {
    $relative = $entry.Name.Replace('/', [IO.Path]::DirectorySeparatorChar)
    $path = Join-Path $Root $relative
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
      throw "Script Studio $Kind file is missing: $relative"
    }
    $stream = [IO.File]::OpenRead($path)
    try {
      $algorithm = [Security.Cryptography.SHA256]::Create()
      try { $bytes = $algorithm.ComputeHash($stream) }
      finally { $algorithm.Dispose() }
    } finally {
      $stream.Dispose()
    }
    $actual = ([BitConverter]::ToString($bytes)).Replace('-', '').ToLowerInvariant()
    if ($actual -ne [string]$entry.Value) {
      throw "Script Studio $Kind is stale or modified: $relative. Run npm ci and npm run build from apps\Sift."
    }
  }
}

Assert-Hashes $manifest.inputs $projectRoot 'input'
Assert-Hashes $manifest.outputs $distRoot 'output'

$actualOutputs = @(Get-ChildItem -LiteralPath $distRoot -Recurse -File |
  Where-Object Name -ne 'asset-manifest.json')
$expectedCount = @($manifest.outputs.PSObject.Properties).Count
if ($actualOutputs.Count -ne $expectedCount) {
  throw "Script Studio output inventory differs from its manifest ($($actualOutputs.Count) files, expected $expectedCount)."
}

Write-Host "Script Studio assets verified: $expectedCount bundled files."
