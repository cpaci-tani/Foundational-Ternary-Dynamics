param(
  [string]$OutputDirectory,
  [switch]$Versioned
)

$ErrorActionPreference = 'Stop'
$project = Join-Path $PSScriptRoot 'Sift.csproj'
$elevationProject = Join-Path (Split-Path -Parent $PSScriptRoot) 'Sift.ElevationHost\Sift.ElevationHost.csproj'
$monitorProject = Join-Path (Split-Path -Parent $PSScriptRoot) 'Sift.MonitorHost\Sift.MonitorHost.csproj'
$usingDefaultOutput = [string]::IsNullOrWhiteSpace($OutputDirectory)

$version = '0.0.0'
$match = Select-String -Path $project -Pattern '<Version>([^<]+)</Version>' | Select-Object -First 1
if ($match) { $version = $match.Matches[0].Groups[1].Value }

if ($usingDefaultOutput) {
  $folderName = if ($Versioned) { "Sift-$version" } else { 'Sift' }
  $OutputDirectory = Join-Path $PSScriptRoot "dist\$folderName"
}

$distRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot 'dist')).TrimEnd(
  [IO.Path]::DirectorySeparatorChar, [IO.Path]::AltDirectorySeparatorChar)
$resolvedOutput = [IO.Path]::GetFullPath($OutputDirectory).TrimEnd(
  [IO.Path]::DirectorySeparatorChar, [IO.Path]::AltDirectorySeparatorChar)
$customMarker = Join-Path $resolvedOutput '.sift-release-output'

if ($usingDefaultOutput) {
  if (-not $resolvedOutput.StartsWith($distRoot + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to clean output outside the Sift dist directory: $resolvedOutput"
  }
} elseif (Test-Path -LiteralPath $resolvedOutput) {
  $existingItems = @(Get-ChildItem -LiteralPath $resolvedOutput -Force)
  if ($existingItems.Count -gt 0 -and -not (Test-Path -LiteralPath $customMarker -PathType Leaf)) {
    throw "Refusing to clean a non-empty custom output without the Sift release marker: $resolvedOutput"
  }
}

if (Test-Path -LiteralPath $resolvedOutput) {
  Remove-Item -LiteralPath $resolvedOutput -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $resolvedOutput | Out-Null
if (-not $usingDefaultOutput) {
  [IO.File]::WriteAllText($customMarker, "Sift release output $version`r`n")
}
$OutputDirectory = $resolvedOutput

dotnet publish $project `
  --configuration Release `
  --runtime win-x64 `
  --self-contained true `
  --source https://api.nuget.org/v3/index.json `
  -p:WindowsAppSDKSelfContained=true `
  -p:DebugType=None `
  -p:DebugSymbols=false `
  --output $OutputDirectory

if ($LASTEXITCODE -ne 0) { throw "Sift publish failed with exit code $LASTEXITCODE." }

# The helper project is a build dependency, not an in-process application payload.
# Remove its framework-dependent root artifacts before publishing the isolated self-contained helper below.
Get-ChildItem -LiteralPath $OutputDirectory -Filter 'Sift.ElevationHost*' -File -ErrorAction SilentlyContinue |
  Remove-Item -Force
Get-ChildItem -LiteralPath $OutputDirectory -Filter 'Sift.MonitorHost*' -File -ErrorAction SilentlyContinue |
  Remove-Item -Force

$elevationOutput = Join-Path $OutputDirectory 'ElevationHost'
if (Test-Path -LiteralPath $elevationOutput) { Remove-Item -LiteralPath $elevationOutput -Recurse -Force }
New-Item -ItemType Directory -Force -Path $elevationOutput | Out-Null
dotnet publish $elevationProject `
  --configuration Release `
  --runtime win-x64 `
  --self-contained true `
  --source https://api.nuget.org/v3/index.json `
  -p:PublishSingleFile=true `
  -p:IncludeNativeLibrariesForSelfExtract=true `
  -p:DebugType=None `
  -p:DebugSymbols=false `
  --output $elevationOutput
if ($LASTEXITCODE -ne 0) { throw "Sift elevation-helper publish failed with exit code $LASTEXITCODE." }

$monitorOutput = Join-Path $OutputDirectory 'MonitorHost'
if (Test-Path -LiteralPath $monitorOutput) { Remove-Item -LiteralPath $monitorOutput -Recurse -Force }
New-Item -ItemType Directory -Force -Path $monitorOutput | Out-Null
dotnet publish $monitorProject `
  --configuration Release `
  --runtime win-x64 `
  --self-contained true `
  --source https://api.nuget.org/v3/index.json `
  -p:WindowsAppSDKSelfContained=true `
  -p:DebugType=None `
  -p:DebugSymbols=false `
  --output $monitorOutput
if ($LASTEXITCODE -ne 0) { throw "Sift monitor-host publish failed with exit code $LASTEXITCODE." }

# dotnet publish currently omits application-owned XBF/PRI files for this unpackaged WinUI project.
# Copy only the application resources produced by the same Release build.
$buildOutput = Join-Path $PSScriptRoot 'bin\Release\net8.0-windows10.0.19041.0\win-x64'
foreach ($resourceFile in @('App.xbf', 'MainWindow.xbf', 'Sift.pri')) {
  $source = Join-Path $buildOutput $resourceFile
  if (-not (Test-Path -LiteralPath $source)) { throw "WinUI build resource is missing: $source" }
  Copy-Item -LiteralPath $source -Destination (Join-Path $OutputDirectory $resourceFile) -Force
}

foreach ($resourceDirectory in @('Controls', 'Views')) {
  $source = Join-Path $buildOutput $resourceDirectory
  if (-not (Test-Path -LiteralPath $source)) { throw "WinUI resource directory is missing: $source" }
  $destination = Join-Path $OutputDirectory $resourceDirectory
  New-Item -ItemType Directory -Force -Path $destination | Out-Null
  Copy-Item -Path (Join-Path $source '*') -Destination $destination -Recurse -Force
}

$executable = Join-Path $OutputDirectory 'Sift.exe'
if (-not (Test-Path -LiteralPath $executable)) { throw "Publish completed without the expected executable: $executable" }
$elevationExecutable = Join-Path $elevationOutput 'Sift.ElevationHost.exe'
if (-not (Test-Path -LiteralPath $elevationExecutable)) { throw "Publish completed without the elevation helper: $elevationExecutable" }
$monitorExecutable = Join-Path $monitorOutput 'Sift.MonitorHost.exe'
if (-not (Test-Path -LiteralPath $monitorExecutable)) { throw "Publish completed without the monitor host: $monitorExecutable" }
if (Get-ChildItem -LiteralPath $OutputDirectory -Filter 'Sift.MonitorHost*' -File -ErrorAction SilentlyContinue) {
  throw 'A framework-dependent monitor-host artifact leaked into the release root.'
}

$rebuildScript = Join-Path $PSScriptRoot 'scripts\REBUILD.bat'
if (-not (Test-Path -LiteralPath $rebuildScript)) { throw "Rebuild script template is missing: $rebuildScript" }
Copy-Item -LiteralPath $rebuildScript -Destination (Join-Path $OutputDirectory 'REBUILD.bat') -Force

Write-Host "Sift release: $executable (v$version)"
Write-Host "One-shot elevation helper: $elevationExecutable"
Write-Host "Optional per-user monitor: $monitorExecutable"
Write-Host "Rebuild this folder: $(Join-Path $OutputDirectory 'REBUILD.bat')"
Write-Host 'This is a self-contained folder deployment. Keep its files together.'
Write-Host 'The executable is unsigned; SmartScreen may warn until Authenticode signing is configured.'
