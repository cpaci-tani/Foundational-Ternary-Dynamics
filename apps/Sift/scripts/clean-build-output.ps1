param(
  [Parameter(Mandatory = $true)]
  [string]$ProjectDirectory,
  [Parameter(Mandatory = $true)]
  [string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath($ProjectDirectory).TrimEnd(
  [IO.Path]::DirectorySeparatorChar,
  [IO.Path]::AltDirectorySeparatorChar)
$binRoot = [IO.Path]::GetFullPath((Join-Path $projectRoot 'bin')).TrimEnd(
  [IO.Path]::DirectorySeparatorChar,
  [IO.Path]::AltDirectorySeparatorChar)
$resolvedOutput = if ([IO.Path]::IsPathRooted($OutputDirectory)) {
  [IO.Path]::GetFullPath($OutputDirectory)
} else {
  [IO.Path]::GetFullPath((Join-Path $projectRoot $OutputDirectory))
}
$resolvedOutput = $resolvedOutput.TrimEnd(
  [IO.Path]::DirectorySeparatorChar,
  [IO.Path]::AltDirectorySeparatorChar)

if ($resolvedOutput.Equals($binRoot, [StringComparison]::OrdinalIgnoreCase) -or
    -not $resolvedOutput.StartsWith($binRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
  throw "Refusing to clean a build output outside the project bin directory: $resolvedOutput"
}

if (Test-Path -LiteralPath $resolvedOutput) {
  Remove-Item -LiteralPath $resolvedOutput -Recurse -Force
  Write-Host "Cleaned build output: $resolvedOutput"
}
