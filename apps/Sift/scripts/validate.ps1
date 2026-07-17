param([switch]$SkipUi)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$appsRoot = Split-Path -Parent $projectRoot
$repositoryRoot = Split-Path -Parent $appsRoot

& (Join-Path $PSScriptRoot 'validate-feature-audit.ps1')

dotnet restore (Join-Path $appsRoot 'Sift.UnitTests\Sift.UnitTests.csproj') --source https://api.nuget.org/v3/index.json
if ($LASTEXITCODE -ne 0) { throw 'Unit-test restore failed.' }
dotnet restore (Join-Path $appsRoot 'Sift.Tests\Sift.Tests.csproj') --source https://api.nuget.org/v3/index.json
if ($LASTEXITCODE -ne 0) { throw 'Integration restore failed.' }
dotnet restore (Join-Path $projectRoot 'Sift.csproj') --source https://api.nuget.org/v3/index.json
if ($LASTEXITCODE -ne 0) { throw 'Application restore failed.' }

dotnet test (Join-Path $appsRoot 'Sift.UnitTests\Sift.UnitTests.csproj') --configuration Release --no-restore
if ($LASTEXITCODE -ne 0) { throw 'Unit tests failed.' }
dotnet run --project (Join-Path $appsRoot 'Sift.Tests\Sift.Tests.csproj') --configuration Release --no-restore
if ($LASTEXITCODE -ne 0) { throw 'Integration validation failed.' }
$appOutput = Join-Path $projectRoot 'bin\Release\net8.0-windows10.0.19041.0\win-x64'
$hostOutput = Join-Path $appsRoot 'Sift.ElevationHost\bin\Release\net8.0-windows10.0.19041.0\win-x64'
$appStaleProbe = Join-Path $appOutput 'stale-build-output.probe'
$hostStaleProbe = Join-Path $hostOutput 'stale-build-output.probe'
New-Item -ItemType Directory -Force -Path $appOutput, $hostOutput | Out-Null
[IO.File]::WriteAllText($appStaleProbe, 'This file must be removed by the next build.')
[IO.File]::WriteAllText($hostStaleProbe, 'This file must be removed before the elevation host is copied.')
dotnet build (Join-Path $projectRoot 'Sift.csproj') --configuration Release --no-restore
if ($LASTEXITCODE -ne 0) { throw 'Application build failed.' }
if (Test-Path -LiteralPath $appStaleProbe) { throw 'The application build left stale output in place.' }
if (Test-Path -LiteralPath $hostStaleProbe) { throw 'The elevation-host build left stale output in place.' }
if (Test-Path -LiteralPath (Join-Path $appOutput 'ElevationHost\stale-build-output.probe')) {
  throw 'A stale elevation-host file was copied into the application output.'
}

if (-not $SkipUi) { & (Join-Path $PSScriptRoot 'validate-ui.ps1') -Configuration Release -NoBuild }

git -C $repositoryRoot diff --check
if ($LASTEXITCODE -ne 0) { throw 'git diff --check failed.' }
Write-Host 'Sift validation completed successfully.'
