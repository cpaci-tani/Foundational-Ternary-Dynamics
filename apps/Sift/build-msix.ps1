param(
  [string]$Publisher = 'CN=Sift Development',
  [string]$CertificateThumbprint,
  [switch]$Unsigned,
  [switch]$NoBuild
)

$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$project = Join-Path $projectRoot 'Sift.csproj'
$releaseRoot = Join-Path $projectRoot 'dist\Sift'
$stageRoot = Join-Path $projectRoot 'dist\msix-stage'
$packageRoot = Join-Path $projectRoot 'dist\packages'
$template = Join-Path $projectRoot 'Packaging\AppxManifest.xml.template'

if ($Unsigned -and -not [string]::IsNullOrWhiteSpace($CertificateThumbprint)) {
  throw 'Choose either -Unsigned or -CertificateThumbprint, not both.'
}
if (-not $Unsigned -and [string]::IsNullOrWhiteSpace($CertificateThumbprint)) {
  throw 'A signed package requires -CertificateThumbprint. Use -Unsigned only for local package-layout validation.'
}

if (-not $NoBuild) { & (Join-Path $projectRoot 'build-release.ps1') }
if (-not (Test-Path -LiteralPath (Join-Path $releaseRoot 'Sift.exe'))) {
  throw 'The self-contained Sift release is missing. Run build-release.ps1 first.'
}
if (-not (Test-Path -LiteralPath (Join-Path $releaseRoot 'ElevationHost\Sift.ElevationHost.exe'))) {
  throw 'The one-shot elevation helper is missing from the release.'
}
if (-not (Test-Path -LiteralPath (Join-Path $releaseRoot 'MonitorHost\Sift.MonitorHost.exe'))) {
  throw 'The per-user monitor host is missing from the release.'
}

$versionText = ([regex]::Match((Get-Content $project -Raw), '<Version>([^<]+)</Version>')).Groups[1].Value
$parts = @($versionText.Split('.') | ForEach-Object { [int]$_ })
while ($parts.Count -lt 4) { $parts += 0 }
$msixVersion = '{0}.{1}.{2}.{3}' -f $parts[0],$parts[1],$parts[2],$parts[3]

$kitsRoot = 'C:\Program Files (x86)\Windows Kits\10\bin'
$kit = Get-ChildItem -LiteralPath $kitsRoot -Directory | Where-Object { $_.Name -match '^\d+\.\d+\.\d+\.\d+$' } |
  Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
if ($null -eq $kit) { throw 'A Windows 10/11 SDK with MakeAppx and SignTool is required.' }
$makeAppx = Join-Path $kit.FullName 'x64\makeappx.exe'
$signTool = Join-Path $kit.FullName 'x64\signtool.exe'
if (-not (Test-Path -LiteralPath $makeAppx)) { throw "MakeAppx is missing: $makeAppx" }
if (-not $Unsigned -and -not (Test-Path -LiteralPath $signTool)) { throw "SignTool is missing: $signTool" }

$thumbprint = $null
if (-not $Unsigned) {
  $thumbprint = $CertificateThumbprint.Replace(' ', '').ToUpperInvariant()
  $certificate = @(Get-ChildItem Cert:\CurrentUser\My,Cert:\LocalMachine\My -ErrorAction SilentlyContinue |
    Where-Object { $_.Thumbprint -eq $thumbprint }) | Select-Object -First 1
  if ($null -eq $certificate) { throw "Signing certificate was not found: $thumbprint" }
  if (-not $certificate.Subject.Equals($Publisher, [StringComparison]::OrdinalIgnoreCase)) {
    throw "Manifest publisher '$Publisher' must exactly match certificate subject '$($certificate.Subject)'."
  }
  foreach ($binary in @(
      (Join-Path $releaseRoot 'Sift.exe'),
      (Join-Path $releaseRoot 'ElevationHost\Sift.ElevationHost.exe'),
      (Join-Path $releaseRoot 'MonitorHost\Sift.MonitorHost.exe'))) {
    & $signTool sign /sha1 $thumbprint /fd SHA256 $binary
    if ($LASTEXITCODE -ne 0) { throw "Signing failed: $binary" }
    & $signTool verify /pa /all $binary
    if ($LASTEXITCODE -ne 0) { throw "Signature verification failed: $binary" }
  }
}

foreach ($path in @($stageRoot, $packageRoot)) {
  if (Test-Path -LiteralPath $path) { Remove-Item -LiteralPath $path -Recurse -Force }
  New-Item -ItemType Directory -Force -Path $path | Out-Null
}
Copy-Item -Path (Join-Path $releaseRoot '*') -Destination $stageRoot -Recurse -Force

$manifest = (Get-Content $template -Raw).Replace('__PUBLISHER__', [Security.SecurityElement]::Escape($Publisher)).Replace('__VERSION__', $msixVersion)
[IO.File]::WriteAllText((Join-Path $stageRoot 'AppxManifest.xml'), $manifest, [Text.UTF8Encoding]::new($false))

Add-Type -AssemblyName System.Drawing
function Save-SquareAsset([string]$Source, [string]$Destination, [int]$Size) {
  $sourceImage = [Drawing.Image]::FromFile($Source)
  $bitmap = [Drawing.Bitmap]::new($Size, $Size)
  $graphics = [Drawing.Graphics]::FromImage($bitmap)
  try {
    $graphics.Clear([Drawing.ColorTranslator]::FromHtml('#1A1816'))
    $graphics.InterpolationMode = [Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $graphics.DrawImage($sourceImage, 0, 0, $Size, $Size)
    $bitmap.Save($Destination, [Drawing.Imaging.ImageFormat]::Png)
  } finally {
    $graphics.Dispose()
    $bitmap.Dispose()
    $sourceImage.Dispose()
  }
}
$assets = Join-Path $stageRoot 'Assets'
New-Item -ItemType Directory -Force -Path $assets | Out-Null
$sourceLogo = Join-Path $projectRoot 'Assets\SiftLogo.png'
Save-SquareAsset $sourceLogo (Join-Path $assets 'Square44x44Logo.png') 44
Save-SquareAsset $sourceLogo (Join-Path $assets 'Square150x150Logo.png') 150
Save-SquareAsset $sourceLogo (Join-Path $assets 'StoreLogo.png') 50

$package = Join-Path $packageRoot "Sift-$msixVersion-x64.msix"
& $makeAppx pack /d $stageRoot /p $package /o
if ($LASTEXITCODE -ne 0) { throw "MakeAppx failed with exit code $LASTEXITCODE." }

Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [IO.Compression.ZipFile]::OpenRead($package)
try {
  $entries = @($archive.Entries.FullName)
  if ('ElevationHost/Sift.ElevationHost.exe' -notin $entries) {
    throw 'The packaged one-shot elevation helper is missing.'
  }
  if ('MonitorHost/Sift.MonitorHost.exe' -notin $entries) {
    throw 'The packaged per-user monitor host is missing.'
  }
  if ($entries | Where-Object { $_ -like 'Sift.ElevationHost*' }) {
    throw 'A framework-dependent elevation-helper artifact leaked into the package root.'
  }
  if ($entries | Where-Object { $_ -like 'Sift.MonitorHost*' }) {
    throw 'A framework-dependent monitor-host artifact leaked into the package root.'
  }
  $manifestEntry = $archive.GetEntry('AppxManifest.xml')
  $manifestReader = [IO.StreamReader]::new($manifestEntry.Open())
  try { $manifestText = $manifestReader.ReadToEnd() } finally { $manifestReader.Dispose() }
  if ($manifestText -notmatch 'TaskId="SiftMonitor"' -or $manifestText -notmatch 'Enabled="false"') {
    throw 'The optional monitor startup task is missing or enabled by default.'
  }
  if ($manifestText -notmatch 'windows.toastNotificationActivation' -or
      $manifestText -notmatch 'Executable="MonitorHost\\Sift.MonitorHost.exe"') {
    throw 'The packaged monitor notification activation contract is missing.'
  }
  if ('AppxManifest.xml' -notin $entries -or 'AppxBlockMap.xml' -notin $entries) {
    throw 'The package manifest or block map is missing.'
  }
} finally {
  $archive.Dispose()
}

if (-not $Unsigned) {
  & $signTool sign /sha1 $thumbprint /fd SHA256 $package
  if ($LASTEXITCODE -ne 0) { throw 'MSIX signing failed.' }
  & $signTool verify /pa /all $package
  if ($LASTEXITCODE -ne 0) { throw 'MSIX signature verification failed.' }
  Write-Host "Signed and verified MSIX: $package"
} else {
  Write-Host "Unsigned MSIX layout validation passed: $package"
  Write-Host 'This package is not installable on a normal clean account until signed with a trusted certificate.'
}

Remove-Item -LiteralPath $stageRoot -Recurse -Force
