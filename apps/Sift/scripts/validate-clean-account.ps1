param(
  [Parameter(Mandatory = $true)]
  [string]$PackagePath,
  [string]$ExpectedPublisher,
  [switch]$InstallRoundTrip
)

$ErrorActionPreference = 'Stop'
$package = Get-Item -LiteralPath (Resolve-Path -LiteralPath $PackagePath)
if ($package.Extension -ne '.msix') { throw 'Clean-account validation requires an MSIX package.' }

$kitsRoot = 'C:\Program Files (x86)\Windows Kits\10\bin'
$kit = Get-ChildItem -LiteralPath $kitsRoot -Directory |
  Where-Object { $_.Name -match '^\d+\.\d+\.\d+\.\d+$' } |
  Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
$signTool = if ($null -ne $kit) { Join-Path $kit.FullName 'x64\signtool.exe' } else { $null }
if ([string]::IsNullOrWhiteSpace($signTool) -or -not (Test-Path -LiteralPath $signTool)) {
  throw 'A Windows SDK SignTool is required for release acceptance.'
}
& $signTool verify /pa /all /v $package.FullName
if ($LASTEXITCODE -ne 0) { throw 'The package signature is missing, invalid, or untrusted.' }

Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [IO.Compression.ZipFile]::OpenRead($package.FullName)
try {
  $manifestEntry = $archive.GetEntry('AppxManifest.xml')
  if ($null -eq $manifestEntry) { throw 'The package manifest is missing.' }
  $reader = [IO.StreamReader]::new($manifestEntry.Open())
  try { [xml]$manifest = $reader.ReadToEnd() } finally { $reader.Dispose() }
  $entries = @($archive.Entries.FullName)
  if ($entries -notcontains 'Sift.exe' -or
      $entries -notcontains 'ElevationHost/Sift.ElevationHost.exe' -or
      $entries -notcontains 'MonitorHost/Sift.MonitorHost.exe' -or
      @($entries | Where-Object { $_ -like 'Sift.ElevationHost*' }).Count -ne 0) {
    throw 'The package application/helper/monitor layout is invalid.'
  }
  if (@($entries | Where-Object { $_ -like 'Sift.MonitorHost*' }).Count -ne 0) {
    throw 'A framework-dependent monitor-host artifact leaked into the package root.'
  }
  if ($manifest.OuterXml -notmatch 'TaskId="SiftMonitor"' -or
      $manifest.OuterXml -notmatch 'Enabled="false"') {
    throw 'The optional monitor startup task is missing or enabled by default.'
  }
} finally { $archive.Dispose() }

$identity = $manifest.Package.Identity
$application = $manifest.Package.Applications.Application
if (-not [string]::IsNullOrWhiteSpace($ExpectedPublisher) -and
    -not [string]::Equals($identity.Publisher, $ExpectedPublisher, [StringComparison]::Ordinal)) {
  throw "Manifest publisher '$($identity.Publisher)' does not match '$ExpectedPublisher'."
}
if ($identity.ProcessorArchitecture -ne 'x64' -or $application.Executable -ne 'Sift.exe') {
  throw 'The package identity is not the expected x64 Sift full-trust application.'
}

Write-Host "PASS  Trusted package signature and bounded layout: $($identity.Name) $($identity.Version)"
if (-not $InstallRoundTrip) {
  Write-Host 'Static release acceptance passed. Use -InstallRoundTrip only from a disposable clean standard-user account.'
  return
}

$principal = [Security.Principal.WindowsPrincipal]::new([Security.Principal.WindowsIdentity]::GetCurrent())
if ($principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
  throw 'The install round trip must start from a non-elevated standard-user token.'
}
if ($null -ne (Get-AppxPackage -Name $identity.Name)) {
  throw 'The package is already installed. Use a disposable clean account with no prior Sift package.'
}

$installed = $null
$requestPath = $null
$responsePath = $null
try {
  Add-AppxPackage -Path $package.FullName -ErrorAction Stop
  $installed = Get-AppxPackage -Name $identity.Name | Select-Object -First 1
  if ($null -eq $installed) { throw 'Windows did not register Sift after Add-AppxPackage returned.' }

  $helper = Join-Path $installed.InstallLocation 'ElevationHost\Sift.ElevationHost.exe'
  $monitor = Join-Path $installed.InstallLocation 'MonitorHost\Sift.MonitorHost.exe'
  if (-not (Test-Path -LiteralPath $helper)) { throw 'The installed one-shot elevation helper is missing.' }
  if (-not (Test-Path -LiteralPath $monitor)) { throw 'The installed per-user monitor host is missing.' }
  & $signTool verify /pa /all /v (Join-Path $installed.InstallLocation 'Sift.exe')
  if ($LASTEXITCODE -ne 0) { throw 'The installed Sift executable signature is invalid.' }
  & $signTool verify /pa /all /v $helper
  if ($LASTEXITCODE -ne 0) { throw 'The installed elevation-helper signature is invalid.' }
  & $signTool verify /pa /all /v $monitor
  if ($LASTEXITCODE -ne 0) { throw 'The installed monitor-host signature is invalid.' }

  # Load a byte copy so this validation process does not retain a module mapping into the installed package.
  [Reflection.Assembly]::Load([IO.File]::ReadAllBytes(
    (Join-Path $installed.InstallLocation 'MonitorHost\Sift.Core.dll'))) | Out-Null
  $monitorProcess = Start-Process -FilePath $monitor -PassThru -WindowStyle Hidden
  try {
    Start-Sleep -Milliseconds 1200
    function Invoke-SiftMonitorProbe([string]$Command) {
      $options = [IO.Pipes.PipeOptions]::Asynchronous -bor [IO.Pipes.PipeOptions]::CurrentUserOnly
      $pipe = [IO.Pipes.NamedPipeClientStream]::new('.', [Sift.Services.DashboardMonitorProtocol]::PipeName,
        [IO.Pipes.PipeDirection]::InOut, $options)
      try {
        $pipe.Connect(5000)
        $envelope = [Sift.Services.DashboardMonitorEnvelope]::new(
          1, $identity.Version.ToString().Substring(0, $identity.Version.ToString().LastIndexOf('.')),
          $Command, $null, $null, $null, $false)
        [Sift.Services.DashboardMonitorProtocol]::WriteAsync(
          $pipe, $envelope, [Threading.CancellationToken]::None).GetAwaiter().GetResult() | Out-Null
        return [Sift.Services.DashboardMonitorProtocol]::ReadAsync(
          $pipe, [Threading.CancellationToken]::None).GetAwaiter().GetResult()
      } finally { $pipe.Dispose() }
    }
    $monitorStatus = Invoke-SiftMonitorProbe 'status'
    if ($monitorStatus.Message -ne 'running' -or $monitorStatus.Command -ne 'status') {
      throw 'The monitor current-user IPC status probe failed.'
    }
    $monitorShutdown = Invoke-SiftMonitorProbe 'shutdown'
    if ($monitorShutdown.Message -ne 'stopping') { throw 'The monitor shutdown probe failed.' }
  } finally {
    if (-not $monitorProcess.HasExited) { $monitorProcess.WaitForExit(5000) | Out-Null }
    if (-not $monitorProcess.HasExited) { $monitorProcess.Kill() }
  }
  Write-Host 'PASS  Current-user monitor IPC round trip (no actions or elevation API)'

  $requestId = [Guid]::NewGuid().ToString('N')
  $nonce = [Convert]::ToHexString([Security.Cryptography.RandomNumberGenerator]::GetBytes(32))
  $requestDirectory = Join-Path $env:LOCALAPPDATA 'Sift\Elevation'
  New-Item -ItemType Directory -Force -Path $requestDirectory | Out-Null
  $requestPath = Join-Path $requestDirectory "$requestId.request.json"
  $responsePath = Join-Path $requestDirectory "$requestId.response.json"
  $request = [ordered]@{
    RequestId = $requestId
    Nonce = $nonce
    Operation = 2 # ElevatedOperationKind.ValidateElevation
    TweakIds = @()
    BackupFileName = $null
  } | ConvertTo-Json -Depth 4
  $stream = [IO.FileStream]::new($requestPath, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write,
    [IO.FileShare]::None, 4096, [IO.FileOptions]::WriteThrough)
  try {
    $bytes = [Text.UTF8Encoding]::new($false).GetBytes($request)
    $stream.Write($bytes, 0, $bytes.Length)
    $stream.Flush($true)
  } finally { $stream.Dispose() }

  try {
    $probe = Start-Process -FilePath $helper -ArgumentList '--request', ('"{0}"' -f $requestPath) `
      -Verb RunAs -PassThru -Wait
  } catch [System.ComponentModel.Win32Exception] {
    if ($_.Exception.NativeErrorCode -eq 1223) { throw 'Administrator confirmation was cancelled during the elevation acceptance probe.' }
    throw
  }
  if ($probe.ExitCode -ne 0 -or -not (Test-Path -LiteralPath $responsePath)) {
    throw 'The installed helper did not complete the non-mutating elevation IPC probe.'
  }
  $response = Get-Content -LiteralPath $responsePath -Raw | ConvertFrom-Json
  if (-not $response.Succeeded -or $response.RequestId -ne $requestId -or $response.Nonce -ne $nonce) {
    throw 'The elevation probe response failed its success, request-id, or nonce check.'
  }
  Write-Host 'PASS  Standard-user to administrator one-shot IPC round trip (no settings changed)'

  if ($null -ne (Get-Process -Name Sift -ErrorAction SilentlyContinue)) {
    throw 'A Sift process was already running before packaged launch validation.'
  }
  Start-Process 'explorer.exe' -ArgumentList "shell:AppsFolder\$($installed.PackageFamilyName)!$($application.Id)"
  $deadline = (Get-Date).AddSeconds(20)
  do {
    Start-Sleep -Milliseconds 250
    $launched = Get-Process -Name Sift -ErrorAction SilentlyContinue | Select-Object -First 1
  } while ($null -eq $launched -and (Get-Date) -lt $deadline)
  if ($null -eq $launched) { throw 'The installed package did not create a Sift process.' }
  Stop-Process -Id $launched.Id -Force
  Write-Host 'PASS  Packaged full-trust launch from a standard-user account'
} finally {
  if ($null -ne $requestPath) { Remove-Item -LiteralPath $requestPath -Force -ErrorAction SilentlyContinue }
  if ($null -ne $responsePath) { Remove-Item -LiteralPath $responsePath -Force -ErrorAction SilentlyContinue }
  if ($null -ne $installed) {
    Remove-AppxPackage -Package $installed.PackageFullName -ErrorAction SilentlyContinue
  }
}

if ($null -ne (Get-AppxPackage -Name $identity.Name)) {
  throw 'Sift remained registered after the package uninstall round trip.'
}
if ($null -ne (Get-Process -Name Sift.MonitorHost -ErrorAction SilentlyContinue)) {
  throw 'Sift.MonitorHost remained running after the package uninstall round trip.'
}
if ((Get-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Run' -Name SiftMonitor -ErrorAction SilentlyContinue).SiftMonitor) {
  throw 'A folder-deployment SiftMonitor Run entry remained after the packaged uninstall round trip.'
}
Write-Host 'PASS  Clean-account install, elevation probe, launch, and uninstall round trip'
