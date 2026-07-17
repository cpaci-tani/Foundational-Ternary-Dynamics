param(
    [string]$Filter = '*',
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$appsRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$project = Join-Path $appsRoot 'Sift.Benchmarks\Sift.Benchmarks.csproj'
$artifacts = Join-Path $PSScriptRoot '..\artifacts\benchmarks'
New-Item -ItemType Directory -Force -Path $artifacts | Out-Null

dotnet run --project $project --configuration $Configuration -- --filter $Filter --artifacts $artifacts
if ($LASTEXITCODE -ne 0) { throw 'BenchmarkDotNet run failed.' }
Write-Host "Benchmarks complete. Results under $artifacts"
