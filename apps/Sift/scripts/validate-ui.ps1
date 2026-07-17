param(
  [ValidateSet('Debug', 'Release')]
  [string]$Configuration = 'Release',
  [switch]$NoBuild,
  [ValidateSet('All', 'Home', 'Optimize', 'Task Manager', 'Performance', 'Hardware monitor', 'Startup', 'Maintenance', 'Script studio', 'Health', 'Recovery', 'Storage', 'Installed apps', 'System information', 'Settings')]
  [string]$OnlyWorkspace = 'All'
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$project = Join-Path $projectRoot 'Sift.csproj'
$fixtureRoot = Join-Path ([IO.Path]::GetTempPath()) ("Sift-Storage-UiFixture-" + [Guid]::NewGuid().ToString('N'))
$orphanChildName = "Sift-UIFixture-" + [Guid]::NewGuid().ToString('N')
$orphanSubKey = "Software\Microsoft\Windows\CurrentVersion\Uninstall\$orphanChildName"
$orphanDisplayName = 'AcmeUiOrphan' + [Guid]::NewGuid().ToString('N')
$orphanRegistryCreated = $false
$uninstallChildName = "Sift-UIUninstallFixture-" + [Guid]::NewGuid().ToString('N')
$uninstallSubKey = "Software\Microsoft\Windows\CurrentVersion\Uninstall\$uninstallChildName"
$uninstallDisplayName = 'AcmeUiUninstallable' + [Guid]::NewGuid().ToString('N')
$uninstallRegistryCreated = $false
$uninstallExecutable = Join-Path $env:WINDIR 'System32\ping.exe'
$uninstallLeftoverPath = Join-Path $env:LOCALAPPDATA $uninstallDisplayName
$appDataLeftoverPath = Join-Path $env:LOCALAPPDATA $orphanDisplayName
$settingsPath = Join-Path $env:LOCALAPPDATA 'Sift\settings.json'
$settingsExisted = Test-Path -LiteralPath $settingsPath
$settingsBackup = if ($settingsExisted) { [IO.File]::ReadAllBytes($settingsPath) } else { $null }
$recoveryBackupDirectory = Join-Path $env:LOCALAPPDATA 'Sift\Backups'
$recoveryBackupName = 'backup-ui-recovery-' + [Guid]::NewGuid().ToString('N') + '.json'
$recoveryBackupPath = Join-Path $recoveryBackupDirectory $recoveryBackupName
$taskFixtureExecutable = Join-Path $fixtureRoot 'SiftTaskActionFixture.exe'
$taskFixtureProcess = $null

New-Item -ItemType Directory -Force -Path (Join-Path $fixtureRoot 'media\photos') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $fixtureRoot 'packages\cache') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $fixtureRoot 'documents') | Out-Null
[IO.File]::WriteAllBytes((Join-Path $fixtureRoot 'media\photos\photo-a.raw'), [byte[]]::new(380000))
[IO.File]::WriteAllBytes((Join-Path $fixtureRoot 'media\photos\photo-b.jpg'), [byte[]]::new(190000))
[IO.File]::WriteAllBytes((Join-Path $fixtureRoot 'packages\cache\archive.zip'), [byte[]]::new(270000))
[IO.File]::WriteAllBytes((Join-Path $fixtureRoot 'packages\app.bin'), [byte[]]::new(160000))
[IO.File]::WriteAllBytes((Join-Path $fixtureRoot 'documents\notes.txt'), [byte[]]::new(40000))
Copy-Item -LiteralPath (Join-Path $env:WINDIR 'System32\ping.exe') -Destination $taskFixtureExecutable
$taskFixtureProcess = Start-Process -FilePath $taskFixtureExecutable -ArgumentList '-t','127.0.0.1' -PassThru -WindowStyle Hidden
New-Item -ItemType Directory -Force -Path (Join-Path $appDataLeftoverPath 'cache') | Out-Null
[IO.File]::WriteAllText((Join-Path $appDataLeftoverPath 'cache\leftover.dat'), 'Sift UI preview fixture')
New-Item -ItemType Directory -Force -Path (Join-Path $uninstallLeftoverPath 'cache') | Out-Null
[IO.File]::WriteAllText((Join-Path $uninstallLeftoverPath 'cache\leftover.dat'), 'Sift verified-uninstall fixture')
$orphanInstallPath = Join-Path $fixtureRoot 'missing-app'
$orphanUninstaller = Join-Path $orphanInstallPath 'uninstall.exe'
$orphanKey = [Microsoft.Win32.Registry]::CurrentUser.CreateSubKey($orphanSubKey, $true)
try {
  $orphanKey.SetValue('DisplayName', $orphanDisplayName, [Microsoft.Win32.RegistryValueKind]::String)
  $orphanKey.SetValue('Publisher', 'Acme UI Test Vendor', [Microsoft.Win32.RegistryValueKind]::String)
  $orphanKey.SetValue('InstallLocation', $orphanInstallPath, [Microsoft.Win32.RegistryValueKind]::String)
  $orphanKey.SetValue('UninstallString', "`"$orphanUninstaller`"", [Microsoft.Win32.RegistryValueKind]::String)
  $orphanRegistryCreated = $true
} finally {
  $orphanKey.Dispose()
}
$uninstallKey = [Microsoft.Win32.Registry]::CurrentUser.CreateSubKey($uninstallSubKey, $true)
try {
  $uninstallKey.SetValue('DisplayName', $uninstallDisplayName, [Microsoft.Win32.RegistryValueKind]::String)
  $uninstallKey.SetValue('Publisher', 'Acme UI Test Vendor', [Microsoft.Win32.RegistryValueKind]::String)
  $uninstallKey.SetValue('DisplayVersion', '1.0-test', [Microsoft.Win32.RegistryValueKind]::String)
  $uninstallKey.SetValue('InstallLocation', $fixtureRoot, [Microsoft.Win32.RegistryValueKind]::String)
  $uninstallKey.SetValue('UninstallString', "`"$uninstallExecutable`" -n 3 127.0.0.1", [Microsoft.Win32.RegistryValueKind]::String)
  $uninstallRegistryCreated = $true
} finally {
  $uninstallKey.Dispose()
}
New-Item -ItemType Directory -Force -Path $recoveryBackupDirectory | Out-Null
$recoveryFixture = [ordered]@{
  SchemaVersion = 2
  OperationId = [Guid]::NewGuid().ToString('N')
  CreatedUtc = (Get-Date).ToUniversalTime().ToString('O')
  MachineName = $env:COMPUTERNAME
  WindowsVersion = [Environment]::OSVersion.VersionString
  Entries = @([ordered]@{
    TweakId = 'privacy.ad-id'; State = 'Applied'; AppliedUtc = (Get-Date).ToUniversalTime().ToString('O')
    RestoredUtc = $null; FailureDetail = $null; KeyExisted = $true; Existed = $true
    RegistryValue = [ordered]@{ Name = 'Enabled'; Kind = 'DWord'; Encoding = 'Int32'; Data = '1' }
    RegistryTree = $null; RegistryHive = $null; RegistrySubKey = $null; Value = $null
    RegistryKind = $null; AppliedSuccessfully = $true
  })
}
[IO.File]::WriteAllText($recoveryBackupPath, ($recoveryFixture | ConvertTo-Json -Depth 8))

if (-not $NoBuild) {
  dotnet build $project --configuration $Configuration
  if ($LASTEXITCODE -ne 0) { throw "Sift build failed with exit code $LASTEXITCODE." }
}

$exe = Join-Path $projectRoot "bin\$Configuration\net8.0-windows10.0.19041.0\win-x64\Sift.exe"
if (-not (Test-Path -LiteralPath $exe)) { throw "Sift executable is missing: $exe" }

Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class SiftUiCapture {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern bool ShowWindowAsync(IntPtr hWnd, int command);
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hWnd, IntPtr insertAfter, int x, int y, int width, int height, uint flags);
  [DllImport("dwmapi.dll")] public static extern int DwmFlush();
}
'@

function Save-WindowCapture([IntPtr]$Handle, [string]$Path) {
  $rect = New-Object SiftUiCapture+RECT
  if (-not [SiftUiCapture]::GetWindowRect($Handle, [ref]$rect)) { throw 'Could not read the Sift window bounds.' }
  $width = $rect.Right - $rect.Left
  $height = $rect.Bottom - $rect.Top
  [SiftUiCapture]::ShowWindowAsync($Handle, 5) | Out-Null
  [SiftUiCapture]::SetWindowPos($Handle, [IntPtr](-1), 0, 0, 0, 0, 0x43) | Out-Null
  [SiftUiCapture]::SetForegroundWindow($Handle) | Out-Null
  [SiftUiCapture]::DwmFlush() | Out-Null
  Start-Sleep -Milliseconds 350
  $bitmap = New-Object System.Drawing.Bitmap($width, $height)
  $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
  try {
    for ($attempt = 1; $attempt -le 3; $attempt++) {
      try {
        $graphics.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bitmap.Size)
        break
      } catch {
        if ($attempt -eq 3) { throw }
        [SiftUiCapture]::SetForegroundWindow($Handle) | Out-Null
        [SiftUiCapture]::DwmFlush() | Out-Null
        Start-Sleep -Milliseconds 300
      }
    }
    $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
  } finally {
    $graphics.Dispose()
    $bitmap.Dispose()
    [SiftUiCapture]::SetWindowPos($Handle, [IntPtr](-2), 0, 0, 0, 0, 0x43) | Out-Null
  }
}

function Find-NamedControl(
  [System.Windows.Automation.AutomationElement]$Root,
  [string]$Name,
  [int]$TimeoutSeconds = 5) {
  $nameCondition = New-Object System.Windows.Automation.PropertyCondition(
    [System.Windows.Automation.AutomationElement]::NameProperty, $Name)
  $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
  do {
    $control = $Root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $nameCondition)
    if ($null -ne $control) { return $control }
    Start-Sleep -Milliseconds 200
  } while ((Get-Date) -lt $deadline)
  return $null
}

$process = Start-Process -FilePath $exe -PassThru
try {
  $deadline = (Get-Date).AddSeconds(15)
  do {
    Start-Sleep -Milliseconds 250
    $process.Refresh()
  } while (-not $process.HasExited -and $process.MainWindowHandle -eq 0 -and (Get-Date) -lt $deadline)

  if ($process.HasExited -or $process.MainWindowHandle -eq 0) { throw 'Sift did not create a main window.' }
  $root = [System.Windows.Automation.AutomationElement]::FromHandle($process.MainWindowHandle)
  $condition = New-Object System.Windows.Automation.PropertyCondition(
    [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
    [System.Windows.Automation.ControlType]::ListItem)
  $items = $root.FindAll([System.Windows.Automation.TreeScope]::Descendants, $condition)
  $artifacts = Join-Path $projectRoot 'artifacts'
  New-Item -ItemType Directory -Force -Path $artifacts | Out-Null

  $workspaceSequence = @('Home', 'Optimize', 'Task Manager', 'Performance', 'Hardware monitor', 'Startup', 'Maintenance', 'Script studio', 'Health', 'Recovery', 'Storage', 'Installed apps', 'System information', 'Settings')
  if ($OnlyWorkspace -ne 'All') { $workspaceSequence = @($OnlyWorkspace) }
  foreach ($workspace in $workspaceSequence) {
    $item = $items | Where-Object { $_.Current.Name -eq $workspace } | Select-Object -First 1
    if ($null -eq $item) { $item = Find-NamedControl $root $workspace 5 }
    if ($null -eq $item) { throw "Navigation item is missing: $workspace" }
    $pattern = $item.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern)
    $pattern.Select()
    Start-Sleep -Milliseconds $(if ($workspace -in @('Optimize', 'Task Manager', 'Performance', 'Installed apps')) { 5200 } elseif ($workspace -eq 'Hardware monitor') { 6500 } elseif ($workspace -eq 'System information') { 4500 } elseif ($workspace -eq 'Maintenance') { 1800 } elseif ($workspace -eq 'Home') { 3500 } else { 900 })
    if ($workspace -eq 'Home') {
      foreach ($controlName in @('Dashboard profile', 'Add dashboard widget', 'Customize dashboard',
          'Dashboard widget options')) {
        if ($null -eq (Find-NamedControl $root $controlName 8)) {
          throw "Home dashboard control is missing: $controlName"
        }
      }
      $customize = Find-NamedControl $root 'Customize dashboard'
      $customize.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 300
      foreach ($controlName in @('Preview dashboard layout', 'Move dashboard widget', 'Resize dashboard widget',
          'Tidy', 'Undo', 'Redo', 'Save', 'Cancel')) {
        if ($null -eq (Find-NamedControl $root $controlName 8)) {
          throw "Home customization control is missing: $controlName"
        }
      }
      $preview = Find-NamedControl $root 'Preview dashboard layout'
      foreach ($breakpoint in @('Wide', 'Medium', 'Compact')) {
        $preview = Find-NamedControl $root 'Preview dashboard layout'
        $preview.GetCurrentPattern([System.Windows.Automation.ExpandCollapsePattern]::Pattern).Expand()
        Start-Sleep -Milliseconds 150
        $choice = Find-NamedControl $root $breakpoint 3
        if ($null -eq $choice) { throw "Home breakpoint preview is missing: $breakpoint" }
        $choice.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
        Start-Sleep -Milliseconds 120
      }
      $cancelCustomize = Find-NamedControl $root 'Cancel'
      $cancelCustomize.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 200
      if ($null -eq (Find-NamedControl $root 'Customize dashboard' 3)) {
        throw 'Home customization did not cancel back to the stable dashboard.'
      }
      Write-Host 'PASS  Home dashboard profiles, breakpoint previews, and keyboard/pointer edit controls'
    }
    if ($workspace -eq 'Optimize') {
      $balancedButton = Find-NamedControl $root 'Select balanced optimization preset'
      $optimizeButton = Find-NamedControl $root 'Apply selected optimizations'
      if ($null -eq $balancedButton -or $null -eq $optimizeButton) {
        throw 'Optimize confirmation controls are missing.'
      }
      $restoreButton = Find-NamedControl $root 'Open Recovery backups' 8
      if ($null -eq $restoreButton) { throw 'Optimize Recovery navigation control is missing.' }
      $readyDeadline = (Get-Date).AddSeconds(30)
      do {
        if ($restoreButton.Current.IsEnabled) { break }
        Start-Sleep -Milliseconds 250
        $restoreButton = Find-NamedControl $root 'Open Recovery backups' 2
        if ($null -eq $restoreButton) { throw 'Optimize Recovery navigation control disappeared while waiting for inventory refresh.' }
      } while ((Get-Date) -lt $readyDeadline)
      if (-not $restoreButton.Current.IsEnabled) { throw 'Optimize inventory refresh did not finish.' }
      $balancedButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 400
      $optimizeButton = Find-NamedControl $root 'Apply selected optimizations' 2
      if ($null -eq $optimizeButton -or -not $optimizeButton.Current.IsEnabled) { throw 'Balanced optimization preset did not select any settings.' }
      $optimizeButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      $cancelButton = Find-NamedControl $root 'Cancel' 8
      if ($null -eq $cancelButton) { throw 'Automatic optimization preflight did not open a confirmation dialog.' }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-optimize-confirmation.png')
      $cancelButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 300
      $restoreButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      if ($null -eq (Find-NamedControl $root 'Sift recovery backups' 8)) {
        throw 'Optimize did not navigate to the Recovery backup inventory.'
      }
      Write-Host 'PASS  Optimize confirmation and Recovery navigation'
    }
    if ($workspace -eq 'Task Manager') {
      foreach ($controlName in @(
          'End selected task', 'Restart selected process',
          'Collapse or expand process inventory', 'Show services inventory')) {
        if ($null -eq (Find-NamedControl $root $controlName 8)) {
          throw "Task Manager control is missing: $controlName"
        }
      }
      $endTaskButton = Find-NamedControl $root 'End selected task'
      if ($endTaskButton.Current.IsEnabled) { throw 'End task is enabled without an explicit process selection.' }

      $processExpander = Find-NamedControl $root 'Collapse or expand process inventory'
      $expandPattern = $processExpander.GetCurrentPattern([System.Windows.Automation.ExpandCollapsePattern]::Pattern)
      $expandPattern.Collapse()
      Start-Sleep -Milliseconds 150
      if ($expandPattern.Current.ExpandCollapseState -ne [System.Windows.Automation.ExpandCollapseState]::Collapsed) {
        throw 'The process inventory did not collapse.'
      }
      $expandPattern.Expand()
      Start-Sleep -Milliseconds 200

      $taskSearch = Find-NamedControl $root 'Filter Task Manager inventory'
      $taskSearch.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue($taskFixtureProcess.Id.ToString())
      Start-Sleep -Milliseconds 400
      $fixturePidElement = Find-NamedControl $root $taskFixtureProcess.Id.ToString() 5
      if ($null -eq $fixturePidElement) { throw 'The deterministic process fixture is missing from Task Manager.' }
      $fixtureRow = $fixturePidElement
      $walker = [System.Windows.Automation.TreeWalker]::ControlViewWalker
      while ($null -ne $fixtureRow -and $fixtureRow.Current.ControlType -notin @(
          [System.Windows.Automation.ControlType]::DataItem,
          [System.Windows.Automation.ControlType]::ListItem)) {
        $fixtureRow = $walker.GetParent($fixtureRow)
      }
      if ($null -eq $fixtureRow) { throw 'Could not resolve the deterministic process fixture row.' }
      $fixtureRow.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
      Start-Sleep -Milliseconds 250
      if (-not $endTaskButton.Current.IsEnabled) { throw 'Selecting a safe exact process did not enable End task.' }
      $endTaskButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      $keepProcessButton = Find-NamedControl $root 'Keep process running' 8
      if ($null -eq $keepProcessButton) { throw 'End task policy review did not open its confirmation dialog.' }
      $taskFixtureProcess.Refresh()
      if ($taskFixtureProcess.HasExited) { throw 'End task preflight terminated the fixture before confirmation.' }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-task-manager-end-confirmation.png')
      $keepProcessButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 250
      $taskFixtureProcess.Refresh()
      if ($taskFixtureProcess.HasExited) { throw 'Cancelling End task terminated the fixture.' }

      $taskSearch.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue('')
      Start-Sleep -Milliseconds 350
      $servicesTab = Find-NamedControl $root 'Show services inventory'
      $servicesTab.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
      Start-Sleep -Milliseconds 250
      foreach ($controlName in @('Start selected service', 'Restart selected service', 'Collapse or expand service inventory')) {
        $control = Find-NamedControl $root $controlName 5
        if ($null -eq $control) { throw "Task Manager service control is missing: $controlName" }
        if ($controlName -ne 'Collapse or expand service inventory' -and $control.Current.IsEnabled) {
          throw "$controlName is enabled without an explicit service selection."
        }
      }
      $startServiceControl = Find-NamedControl $root 'Start selected service'
      $restartServiceControl = Find-NamedControl $root 'Restart selected service'
      $serviceTable = Find-NamedControl $root 'Windows services'
      $serviceRowCondition = New-Object System.Windows.Automation.OrCondition(
        (New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
          [System.Windows.Automation.ControlType]::DataItem)),
        (New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
          [System.Windows.Automation.ControlType]::ListItem)))
      $serviceActionButton = $null
      foreach ($serviceRow in $serviceTable.FindAll([System.Windows.Automation.TreeScope]::Descendants, $serviceRowCondition)) {
        try {
          $serviceRow.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
          Start-Sleep -Milliseconds 120
          if ($restartServiceControl.Current.IsEnabled) { $serviceActionButton = $restartServiceControl; break }
          if ($startServiceControl.Current.IsEnabled) { $serviceActionButton = $startServiceControl; break }
        } catch { }
      }
      if ($null -ne $serviceActionButton) {
        $serviceActionButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
        $leaveServiceButton = Find-NamedControl $root 'Leave service unchanged' 8
        if ($null -eq $leaveServiceButton) { throw 'Service policy review did not open its confirmation dialog.' }
        Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-task-manager-service-confirmation.png')
        $leaveServiceButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
        Start-Sleep -Milliseconds 200
      } else {
        Write-Host 'SKIP  No visible policy-approved service row is in a startable/restartable state.'
      }
      Write-Host 'PASS  Task Manager collapsible inventories and confirmed End task cancellation'
    }
    if ($workspace -eq 'Hardware monitor') {
      foreach ($controlName in @('Pause hardware sensor sampling', 'Refresh hardware sensors',
          'Filter hardware sensors', 'Filter hardware sensor type', 'Expand all hardware devices',
          'Collapse all hardware devices', 'Selected hardware sensor history', 'Hardware monitor status',
          'Hardware sensor inventory panel', 'Hardware sensor history panel')) {
        if ($null -eq (Find-NamedControl $root $controlName 12)) { throw "Hardware monitor control is missing: $controlName" }
      }
      $inventoryPanel = Find-NamedControl $root 'Hardware sensor inventory panel' 3
      $historyPanel = Find-NamedControl $root 'Hardware sensor history panel' 3
      $inventoryBounds = $inventoryPanel.Current.BoundingRectangle
      $historyBounds = $historyPanel.Current.BoundingRectangle
      if ($inventoryBounds.Width -lt 300 -or $historyBounds.Width -lt 300 -or
          $inventoryBounds.Height -lt 400 -or $historyBounds.Height -lt 400) {
        throw 'Hardware monitor panels are not filling the available workspace.'
      }
      if ([Math]::Abs($inventoryBounds.Top - $historyBounds.Top) -gt 4 -or
          [Math]::Abs($inventoryBounds.Bottom - $historyBounds.Bottom) -gt 4) {
        throw 'Hardware monitor panels do not share the full available row height.'
      }
      $pauseSensors = Find-NamedControl $root 'Pause hardware sensor sampling' 3
      $pauseSensors.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 200
      if ($pauseSensors.Current.Name -ne 'Pause hardware sensor sampling') { throw 'Hardware monitor pause control lost its accessible identity.' }
      $pauseSensors.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Write-Host 'PASS  Hardware monitor provider inventory and sampling lifecycle'
    }
    if ($workspace -eq 'Maintenance') {
      $selectRecommendedButton = Find-NamedControl $root 'Select recommended maintenance items'
      $maintenanceButton = Find-NamedControl $root 'Clean selected maintenance items'
      if ($null -eq $selectRecommendedButton -or $null -eq $maintenanceButton) {
        throw 'Maintenance confirmation controls are missing.'
      }
      $selectRecommendedButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 250
      if (-not $maintenanceButton.Current.IsEnabled) { throw 'Recommended maintenance selection did not select any findings.' }
      $maintenanceButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      $cancelButton = Find-NamedControl $root 'Cancel' 10
      if ($null -eq $cancelButton) { throw 'Automatic maintenance preflight did not open a confirmation dialog.' }
      if (-not (Test-Path -LiteralPath $fixtureRoot)) { throw 'Automatic maintenance preflight mutated the fixture.' }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-maintenance-confirmation.png')
      $cancelButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 300
      if (-not (Test-Path -LiteralPath $fixtureRoot)) { throw 'Cancelling maintenance cleanup mutated the fixture.' }
      Write-Host 'PASS  Maintenance automatic preflight and confirmation'
    }
    if ($workspace -eq 'Script studio') {
      $standardHeader = Find-NamedControl $root 'STANDARD USER COMMANDS' 5
      if ($null -eq $standardHeader) { throw 'Command Center standard-user section is missing.' }
      if ($null -ne (Find-NamedControl $root 'ADMINISTRATOR COMMANDS' 1)) {
        throw 'Command Center exposed its administrator section in the standard-user UI validation session.'
      }
      $commandSearch = Find-NamedControl $root 'Search command recipes' 5
      $runCommandButton = Find-NamedControl $root 'Run selected command' 5
      $expandAllCategories = Find-NamedControl $root 'Expand all command categories' 5
      $collapseAllCategories = Find-NamedControl $root 'Collapse all command categories' 5
      if ($null -eq $commandSearch -or $null -eq $runCommandButton -or
          $null -eq $expandAllCategories -or $null -eq $collapseAllCategories) {
        throw 'Command Center filtering or execution controls are missing.'
      }
      $commandSearch.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue('no-command-can-match-this-fixture')
      Start-Sleep -Milliseconds 250
      if ($null -eq (Find-NamedControl $root 'No matching commands' 3)) {
        throw 'Command Center filtered-empty state is missing.'
      }
      $commandSearch.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue('Windows version')
      Start-Sleep -Milliseconds 300
      $recipeText = Find-NamedControl $root 'Windows version' 5
      if ($null -eq $recipeText) { throw 'Command Center deterministic standard-user recipe is missing.' }
      if ($null -eq (Find-NamedControl $root 'System' 3)) {
        throw 'Command Center did not expose the System category for the filtered recipe.'
      }
      $systemCategory = Find-NamedControl $root 'System category' 5
      if ($null -eq $systemCategory) { throw 'Command Center System category expander is missing.' }
      $expandPattern = $systemCategory.GetCurrentPattern([System.Windows.Automation.ExpandCollapsePattern]::Pattern)
      if ($expandPattern.Current.ExpandCollapseState -ne [System.Windows.Automation.ExpandCollapseState]::Expanded) {
        throw 'Filtering did not reveal the matching command category.'
      }
      $expandPattern.Collapse()
      Start-Sleep -Milliseconds 150
      if ($expandPattern.Current.ExpandCollapseState -ne [System.Windows.Automation.ExpandCollapseState]::Collapsed) {
        throw 'Command category did not collapse.'
      }
      $expandPattern.Expand()
      Start-Sleep -Milliseconds 150
      if ($expandPattern.Current.ExpandCollapseState -ne [System.Windows.Automation.ExpandCollapseState]::Expanded) {
        throw 'Command category did not expand.'
      }
      $recipeText = Find-NamedControl $root 'Windows version' 3
      if ($null -eq $recipeText) { throw 'Expanding the System category did not restore its command rows.' }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-command-library.png')
      $recipeRow = $recipeText
      $walker = [System.Windows.Automation.TreeWalker]::ControlViewWalker
      while ($null -ne $recipeRow -and $recipeRow.Current.ControlType -ne [System.Windows.Automation.ControlType]::ListItem) {
        $recipeRow = $walker.GetParent($recipeRow)
      }
      if ($null -eq $recipeRow) { throw 'Could not resolve the Command Center recipe row.' }
      $recipeRow.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
      Start-Sleep -Milliseconds 200
      $runCommandButton = Find-NamedControl $root 'Run selected command' 3
      if (-not $runCommandButton.Current.IsEnabled) { throw 'Selecting a read-only recipe did not enable Run.' }
      $insertedCommand = Find-NamedControl $root 'Inserted command preview' 3
      if ($null -eq $insertedCommand -or
          $insertedCommand.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).Current.Value -ne 'ver') {
        throw 'Selecting a command did not insert its exact text into the terminal preview.'
      }
      $copyRecipeButton = Find-NamedControl $root 'Copy Windows version command' 3
      $quickRunRecipeButton = Find-NamedControl $root 'Insert and run Windows version command' 3
      if ($null -eq $copyRecipeButton -or $null -eq $quickRunRecipeButton) {
        throw 'Command hover copy or quick-run action is missing.'
      }
      $copyBounds = $recipeRow.Current.BoundingRectangle
      [SiftUiCapture]::SetCursorPos(
        [int]($copyBounds.X + ($copyBounds.Width / 2)),
        [int]($copyBounds.Y + $copyBounds.Height - 12)) | Out-Null
      Start-Sleep -Milliseconds 200
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-command-library-hover-actions.png')
      $copyRecipeButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 150
      if ((Get-Clipboard -Raw) -ne 'ver') { throw 'Command hover copy did not place the exact command on the clipboard.' }
      $quickRunRecipeButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 150
      if ($null -ne (Find-NamedControl $root 'Cancel' 1)) {
        throw 'Read-only command quick-run opened a redundant confirmation dialog.'
      }
      $terminalOutput = Find-NamedControl $root 'Command output terminal' 3
      if ($null -eq $terminalOutput) { throw 'Command output terminal is missing.' }
      $commandDeadline = (Get-Date).AddSeconds(8)
      do {
        $outputText = $terminalOutput.GetCurrentPattern([System.Windows.Automation.TextPattern]::Pattern).DocumentRange.GetText(-1)
        if ($outputText -match '\[exit 0\]') { break }
        Start-Sleep -Milliseconds 150
      } while ((Get-Date) -lt $commandDeadline)
      if ($outputText -notmatch '\[exit 0\]') { throw 'Read-only command did not execute without confirmation.' }
      Write-Host 'PASS  Script Studio read-only command ran without confirmation'
      $expandPattern.Collapse()
      Start-Sleep -Milliseconds 150
      $commandSearch.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue('')
      Start-Sleep -Milliseconds 300
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-command-library-categories.png')
      $studioTab = Find-NamedControl $root 'Studio' 5
      if ($null -eq $studioTab) { throw 'Script Studio editor tab is missing.' }
      $studioTab.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
      Start-Sleep -Milliseconds 1800
      foreach ($controlName in @('Script language', 'Script runtime', 'Analyze script without executing it', 'Monaco script editor and xterm analysis terminal')) {
        if ($null -eq (Find-NamedControl $root $controlName 8)) { throw "Script Studio control is missing: $controlName" }
      }
      $analyzeScript = Find-NamedControl $root 'Analyze script without executing it' 5
      $studioReadyDeadline = (Get-Date).AddSeconds(15)
      do {
        if ($null -ne $analyzeScript -and $analyzeScript.Current.IsEnabled) { break }
        Start-Sleep -Milliseconds 250
        $analyzeScript = Find-NamedControl $root 'Analyze script without executing it' 2
      } while ((Get-Date) -lt $studioReadyDeadline)
      if ($null -eq $analyzeScript -or -not $analyzeScript.Current.IsEnabled) {
        throw 'Script Studio local editor did not become ready for analysis.'
      }
      $analyzeScript.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 1200
      $analyzeScript = Find-NamedControl $root 'Analyze script without executing it' 5
      if ($null -eq $analyzeScript -or -not $analyzeScript.Current.IsEnabled) {
        throw 'Script Studio in-memory analysis did not complete.'
      }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-script-studio.png')
      $homeItem = $items | Where-Object { $_.Current.Name -eq 'Home' } | Select-Object -First 1
      $homeItem.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
      Start-Sleep -Milliseconds 50
      $item.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
      $resumeDeadline = (Get-Date).AddSeconds(10)
      do {
        Start-Sleep -Milliseconds 100
        $analyzeScript = Find-NamedControl $root 'Analyze script without executing it' 2
        if ($null -ne $analyzeScript -and $analyzeScript.Current.IsEnabled) { break }
      } while ((Get-Date) -lt $resumeDeadline)
      if ($null -eq $analyzeScript -or -not $analyzeScript.Current.IsEnabled) {
        throw 'Script Studio did not resume after a rapid workspace round trip.'
      }
      $previousAnalysisPass = $analyzeScript.Current.ItemStatus
      $analyzeScript.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      $completionDeadline = (Get-Date).AddSeconds(10)
      $postResumeAnalysisCompleted = $false
      do {
        Start-Sleep -Milliseconds 100
        $analyzeScript = Find-NamedControl $root 'Analyze script without executing it' 2
        if ($null -ne $analyzeScript -and $analyzeScript.Current.ItemStatus -ne $previousAnalysisPass -and
            $analyzeScript.Current.ItemStatus -like 'Analysis pass * completed.') {
          $postResumeAnalysisCompleted = $true
          break
        }
      } while ((Get-Date) -lt $completionDeadline)
      if (-not $postResumeAnalysisCompleted -or $null -eq $analyzeScript -or -not $analyzeScript.Current.IsEnabled) {
        throw 'Script Studio WebView did not emit a successful analysis event after resuming.'
      }
      Write-Host 'PASS  Script Studio read-only direct run, state-change policy, editor, and suspend/resume lifecycle'
    }
    if ($workspace -eq 'Recovery') {
      foreach ($controlName in @('Filter recovery backups', 'Recovery status filter', 'Restore selected recovery backup', 'Open Sift backup folder', 'Refresh recovery backups')) {
        if ($null -eq (Find-NamedControl $root $controlName 8)) { throw "Recovery control is missing: $controlName" }
      }
      $backupText = Find-NamedControl $root $recoveryBackupName 8
      if ($null -eq $backupText) { throw 'The deterministic recovery backup fixture is not visible.' }
      $backupRow = $backupText
      $walker = [System.Windows.Automation.TreeWalker]::ControlViewWalker
      while ($null -ne $backupRow -and $backupRow.Current.ControlType -notin @(
          [System.Windows.Automation.ControlType]::DataItem,
          [System.Windows.Automation.ControlType]::ListItem)) {
        $backupRow = $walker.GetParent($backupRow)
      }
      if ($null -eq $backupRow) { throw 'Could not resolve the recovery fixture row.' }
      $backupRow.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
      Start-Sleep -Milliseconds 300
      $restoreButton = Find-NamedControl $root 'Restore selected recovery backup'
      if ($null -eq $restoreButton -or -not $restoreButton.Current.IsEnabled) {
        throw 'The safe current-user recovery fixture is not restorable.'
      }
      $restoreButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      $cancelButton = Find-NamedControl $root 'Cancel' 8
      if ($null -eq $cancelButton) { throw 'Automatic recovery preflight did not open a confirmation dialog.' }
      if (-not (Test-Path -LiteralPath $recoveryBackupPath)) { throw 'Recovery preflight removed the fixture backup.' }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-recovery-confirmation.png')
      $cancelButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 300
      if (-not (Test-Path -LiteralPath $recoveryBackupPath)) { throw 'Cancelling recovery removed the fixture backup.' }
      Write-Host 'PASS  Recovery inventory, automatic preflight, and cancellation'
    }
    if ($workspace -eq 'Storage') {
      $nameCondition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::NameProperty, 'Storage scan root')
      $rootInput = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $nameCondition)
      if ($null -eq $rootInput) { throw 'Storage root input is missing.' }
      $editCondition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
        [System.Windows.Automation.ControlType]::Edit)
      $rootEditor = $rootInput.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $editCondition)
      if ($null -eq $rootEditor) { $rootEditor = $rootInput }
      $valuePattern = $rootEditor.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern)
      $valuePattern.SetValue($fixtureRoot)

      $scanCondition = New-Object System.Windows.Automation.OrCondition(
        (New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::NameProperty, 'Scan storage')),
        (New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::AutomationIdProperty, 'ScanButton')))
      $scanDeadline = (Get-Date).AddSeconds(5)
      $scanButton = $null
      do {
        $scanButton = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $scanCondition)
        if ($null -eq $scanButton) { Start-Sleep -Milliseconds 200 }
      } while ($null -eq $scanButton -and (Get-Date) -lt $scanDeadline)
      if ($null -eq $scanButton) { throw 'Storage scan button is missing.' }
      $invokePattern = $scanButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern)
      $invokePattern.Invoke()
      Start-Sleep -Milliseconds 1800
    }
    if ($workspace -eq 'Installed apps') {
      $installedAppControls = @{}
      foreach ($controlName in @('Filter installed apps', 'Uninstall or clean selected app', 'Scan selected app file leftovers', 'Find leftover app registrations', 'Open Windows Installed Apps')) {
        $controlCondition = New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::NameProperty, $controlName)
        $controlDeadline = (Get-Date).AddSeconds(5)
        $control = $null
        do {
          $control = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $controlCondition)
          if ($null -eq $control) { Start-Sleep -Milliseconds 250 }
        } while ($null -eq $control -and (Get-Date) -lt $controlDeadline)
        if ($null -eq $control) {
          throw "Installed Apps control is missing: $controlName"
        }
        $installedAppControls[$controlName] = $control
      }
      foreach ($removedPreviewControl in @('Installed Apps preview mode', 'App file leftover preview mode')) {
        $removedCondition = New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::NameProperty, $removedPreviewControl)
        if ($null -ne $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $removedCondition)) {
          throw "Removed preview control is still visible: $removedPreviewControl"
        }
      }

      $uninstallCondition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::NameProperty, $uninstallDisplayName)
      $uninstallText = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $uninstallCondition)
      if ($null -eq $uninstallText) { throw 'The deterministic uninstallable fixture is not visible.' }
      $uninstallRow = $uninstallText
      $walker = [System.Windows.Automation.TreeWalker]::ControlViewWalker
      while ($null -ne $uninstallRow -and $uninstallRow.Current.ControlType -notin @(
          [System.Windows.Automation.ControlType]::DataItem,
          [System.Windows.Automation.ControlType]::ListItem)) {
        $uninstallRow = $walker.GetParent($uninstallRow)
      }
      if ($null -eq $uninstallRow) { throw 'Could not resolve the uninstallable fixture row.' }
      $uninstallRow.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
      $trustSummary = Find-NamedControl $root 'Selected app uninstaller trust summary' 5
      if ($null -eq $trustSummary) { throw 'The selected-app uninstaller trust panel is missing.' }
      $trustDeadline = (Get-Date).AddSeconds(12)
      $trustResult = ''
      do {
        $trustResult = $trustSummary.GetCurrentPropertyValue(
          [System.Windows.Automation.AutomationElement]::HelpTextProperty)
        if ($trustResult -notlike 'Trusted signer:*') { Start-Sleep -Milliseconds 250 }
      } while ($trustResult -notlike 'Trusted signer:*' -and (Get-Date) -lt $trustDeadline)
      if ($trustResult -notlike 'Trusted signer:*Microsoft*') {
        throw "The catalog-signed fixture was not reported as a trusted Microsoft signer: $trustResult"
      }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-installed-apps-trust.png')
      $appAction = $installedAppControls['Uninstall or clean selected app']
      if (-not $appAction.Current.IsEnabled) { throw 'A valid interactive registered uninstaller is not uninstallable.' }
      $appAction.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 600
      $cancelCondition = New-Object System.Windows.Automation.AndCondition(
        (New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
          [System.Windows.Automation.ControlType]::Button)),
        (New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::NameProperty, 'Cancel')))
      $cancelButton = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $cancelCondition)
      if ($null -eq $cancelButton) { throw 'Automatic uninstall preflight did not open a confirmation dialog.' }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-installed-apps-uninstall-confirmation.png')
      $openUninstallerButton = Find-NamedControl $root 'Open uninstaller'
      if ($null -eq $openUninstallerButton) { throw 'The confirmed uninstall action is missing.' }
      $openUninstallerButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()

      # Simulate the vendor's final uninstall commit while Sift is tracking the registered process.
      Start-Sleep -Milliseconds 350
      [Microsoft.Win32.Registry]::CurrentUser.DeleteSubKeyTree($uninstallSubKey, $false)

      $uninstallDeadline = (Get-Date).AddSeconds(12)
      $registrationExists = $true
      do {
        Start-Sleep -Milliseconds 250
        $key = [Microsoft.Win32.Registry]::CurrentUser.OpenSubKey($uninstallSubKey)
        try { $registrationExists = $null -ne $key } finally { if ($null -ne $key) { $key.Dispose() } }
      } while ($registrationExists -and (Get-Date) -lt $uninstallDeadline)
      if ($registrationExists) { throw 'The deterministic registered uninstaller did not remove its exact fixture entry.' }

      $verifiedStatus = "Confirmed that the exact registration for $uninstallDisplayName was removed. Exact AppData leftover review is now available."
      $verifiedText = Find-NamedControl $root $verifiedStatus 15
      if ($null -eq $verifiedText) { throw 'Sift did not automatically verify the completed uninstall.' }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-installed-apps-uninstall-verified.png')
      $verifiedScan = Find-NamedControl $root 'Scan selected app file leftovers'
      if ($null -eq $verifiedScan -or -not $verifiedScan.Current.IsEnabled) {
        throw 'Verified uninstall completion did not unlock exact leftover review.'
      }
      $verifiedScan.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      $verifiedLeftover = Find-NamedControl $root $uninstallLeftoverPath 8
      if ($null -eq $verifiedLeftover) { throw 'Verified uninstall token did not expose the exact leftover fixture.' }
      $verifiedFileAction = Find-NamedControl $root 'Delete selected app leftovers'
      if ($null -eq $verifiedFileAction -or $verifiedFileAction.Current.IsEnabled) {
        throw 'Verified uninstall leftovers were selected automatically.'
      }
      $closeVerifiedLeftovers = Find-NamedControl $root 'Close app file leftovers'
      if ($null -eq $closeVerifiedLeftovers) { throw 'The verified-leftover back action is missing.' }
      $closeVerifiedLeftovers.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 300

      $reviewPattern = $installedAppControls['Find leftover app registrations'].GetCurrentPattern(
        [System.Windows.Automation.InvokePattern]::Pattern)
      $reviewPattern.Invoke()
      Start-Sleep -Milliseconds 500
      $orphanCondition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::NameProperty, $orphanDisplayName)
      $orphanText = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $orphanCondition)
      if ($null -eq $orphanText) { throw 'The deterministic leftover-registration fixture is not visible after filtering.' }

      $row = $orphanText
      $walker = [System.Windows.Automation.TreeWalker]::ControlViewWalker
      while ($null -ne $row -and $row.Current.ControlType -notin @(
          [System.Windows.Automation.ControlType]::DataItem,
          [System.Windows.Automation.ControlType]::ListItem)) {
        $row = $walker.GetParent($row)
      }
      if ($null -eq $row) { throw 'Could not resolve the leftover registration row.' }
      $selectionPattern = $row.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern)
      $selectionPattern.Select()
      Start-Sleep -Milliseconds 350
      if (-not $appAction.Current.IsEnabled) {
        throw 'The HKCU leftover-registration cleanup action is not enabled.'
      }
      $appAction.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 600
      $cancelButton = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $cancelCondition)
      if ($null -eq $cancelButton) { throw 'Automatic registration-cleanup preflight did not open a confirmation dialog.' }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-installed-apps-registration-confirmation.png')
      $cancelButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 300
      $orphanStillPresent = [Microsoft.Win32.Registry]::CurrentUser.OpenSubKey($orphanSubKey)
      try {
        if ($null -eq $orphanStillPresent) { throw 'Cancelling registration cleanup removed the fixture.' }
      } finally {
        if ($null -ne $orphanStillPresent) { $orphanStillPresent.Dispose() }
      }
      $scanPattern = $installedAppControls['Scan selected app file leftovers'].GetCurrentPattern(
        [System.Windows.Automation.InvokePattern]::Pattern)
      $scanPattern.Invoke()
      Start-Sleep -Milliseconds 900

      $fileActionCondition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::NameProperty, 'Delete selected app leftovers')
      $fileAction = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $fileActionCondition)
      if ($null -eq $fileAction) { throw 'The app file-leftover action is missing.' }
      if ($fileAction.Current.IsEnabled) { throw 'File leftovers were selected automatically.' }

      $folderChoiceCondition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::NameProperty, 'Select app leftover folder')
      $folderChoice = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $folderChoiceCondition)
      if ($null -eq $folderChoice) { throw 'The exact AppData leftover fixture was not discovered.' }
      $folderToggle = $folderChoice.GetCurrentPattern([System.Windows.Automation.TogglePattern]::Pattern)
      $folderToggle.Toggle()
      Start-Sleep -Milliseconds 250
      if (-not $fileAction.Current.IsEnabled) { throw 'Selecting the leftover folder did not enable cleanup review.' }
      $fileAction.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 600
      if (-not (Test-Path -LiteralPath $appDataLeftoverPath)) {
        throw 'Automatic app file-leftover preflight mutated the fixture folder.'
      }
      $cancelButton = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $cancelCondition)
      if ($null -eq $cancelButton) { throw 'The app file-leftover deletion confirmation did not open.' }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-installed-apps-delete-confirmation.png')
      $cancelButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 300
      if (-not (Test-Path -LiteralPath $appDataLeftoverPath)) {
        throw 'Cancelling app file-leftover deletion mutated the fixture folder.'
      }
      Write-Host 'PASS  Installed Apps trust, tracked uninstall verification, and file-leftover controls'
    }
    if ($workspace -eq 'System information') {
      $systemInfoControls = @{}
      foreach ($controlName in @('Filter system information', 'Filter system information category', 'Copy visible system information report', 'Copy selected system information property', 'Refresh system information', 'System information properties')) {
        $controlCondition = New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::NameProperty, $controlName)
        $controlDeadline = (Get-Date).AddSeconds(10)
        $control = $null
        do {
          $control = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $controlCondition)
          if ($null -eq $control) { Start-Sleep -Milliseconds 250 }
        } while ($null -eq $control -and (Get-Date) -lt $controlDeadline)
        if ($null -eq $control) { throw "System Information control is missing: $controlName" }
        $systemInfoControls[$controlName] = $control
        if ($controlName -eq 'Copy visible system information report') {
          $reportDeadline = (Get-Date).AddSeconds(20)
          while (-not $control.Current.IsEnabled -and (Get-Date) -lt $reportDeadline) {
            Start-Sleep -Milliseconds 300
          }
          if (-not $control.Current.IsEnabled) { throw 'System Information did not finish with a copyable report.' }
        }
      }
      $systemSearch = $systemInfoControls['Filter system information'].GetCurrentPattern(
        [System.Windows.Automation.ValuePattern]::Pattern)
      $systemSearch.SetValue('Processor')
      Start-Sleep -Milliseconds 350
      if (-not $systemInfoControls['Copy visible system information report'].Current.IsEnabled) {
        throw 'System Information search did not retain matching processor properties.'
      }
      $systemSearch.SetValue('')
      Start-Sleep -Milliseconds 250

      $dataItemCondition = New-Object System.Windows.Automation.OrCondition(
        (New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
          [System.Windows.Automation.ControlType]::DataItem)),
        (New-Object System.Windows.Automation.PropertyCondition(
          [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
          [System.Windows.Automation.ControlType]::ListItem)))
      $propertyDeadline = (Get-Date).AddSeconds(10)
      $firstProperty = $null
      do {
        $firstProperty = $systemInfoControls['System information properties'].FindFirst(
          [System.Windows.Automation.TreeScope]::Descendants, $dataItemCondition)
        if ($null -eq $firstProperty) { Start-Sleep -Milliseconds 250 }
      } while ($null -eq $firstProperty -and (Get-Date) -lt $propertyDeadline)
      if ($null -eq $firstProperty) { throw 'System Information property table has no selectable rows.' }
      $firstProperty.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern).Select()
      Start-Sleep -Milliseconds 250
      if (-not $systemInfoControls['Copy selected system information property'].Current.IsEnabled) {
        throw 'Selecting a System Information property did not enable its copy action.'
      }
      [SiftUiCapture]::DwmFlush() | Out-Null
      Start-Sleep -Milliseconds 700
      Write-Host 'PASS  System Information detailed read-only inventory controls'
    }
    $process.Refresh()
    if ($process.HasExited) { throw "Sift exited while opening $workspace." }
    $name = $workspace.ToLowerInvariant().Replace(' ', '-')
    Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts "window-$name.png")
    if ($workspace -eq 'Storage') {
      $browseControl = Find-NamedControl $root 'Choose storage scan folder'
      $deleteControl = Find-NamedControl $root 'Move selected storage item to Recycle Bin'
      if ($null -eq $browseControl -or $null -eq $deleteControl) {
        throw 'Storage folder-picker or deletion-review controls are missing.'
      }
      if ($deleteControl.Current.IsEnabled) {
        throw 'Storage deletion review is enabled while only the scan root is selected.'
      }
      $tileCondition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::NameProperty, 'File photo-a.raw, 371.1 KB')
      $tile = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $tileCondition)
      if ($null -eq $tile) { throw 'Storage treemap fixture tile is missing from the automation tree.' }
      if ($tile.Current.HelpText -notlike "*$fixtureRoot*") {
        throw 'Storage treemap tile is missing its full-path hover/accessibility detail.'
      }
      Write-Host 'PASS  Storage tile hover metadata'

      $tile.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 250
      if (-not $deleteControl.Current.IsEnabled) {
        throw 'Selecting an exact non-root storage tile did not enable deletion review.'
      }
      $deleteControl.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      $keepPathButton = Find-NamedControl $root 'Cancel' 10
      if ($null -eq $keepPathButton) {
        throw 'Automatic storage deletion preflight did not open a confirmation dialog.'
      }
      $storageDeleteFixture = Join-Path $fixtureRoot 'media\photos\photo-a.raw'
      if (-not (Test-Path -LiteralPath $storageDeleteFixture)) {
        throw 'Storage deletion preflight mutated the selected fixture before confirmation.'
      }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-storage-delete-confirmation.png')
      $keepPathButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
      Start-Sleep -Milliseconds 300
      if (-not (Test-Path -LiteralPath $storageDeleteFixture)) {
        throw 'Cancelling storage deletion mutated the selected fixture.'
      }
      Write-Host 'PASS  Storage exact-child automatic preflight and confirmation cancellation'

      [SiftUiCapture]::SetWindowPos($process.MainWindowHandle, [IntPtr]::Zero, 0, 0, 800, 500, 0x0006) | Out-Null
      [SiftUiCapture]::DwmFlush() | Out-Null
      Start-Sleep -Milliseconds 500
      $minimumRect = New-Object SiftUiCapture+RECT
      [SiftUiCapture]::GetWindowRect($process.MainWindowHandle, [ref]$minimumRect) | Out-Null
      $minimumWidth = $minimumRect.Right - $minimumRect.Left
      $minimumHeight = $minimumRect.Bottom - $minimumRect.Top
      if ($minimumWidth -lt 1000 -or $minimumHeight -lt 650) {
        throw "Sift minimum window size was not enforced: ${minimumWidth}x${minimumHeight}."
      }
      Save-WindowCapture $process.MainWindowHandle (Join-Path $artifacts 'window-storage-minimum.png')
      [SiftUiCapture]::SetWindowPos($process.MainWindowHandle, [IntPtr]::Zero, 0, 0, 1500, 920, 0x0006) | Out-Null
      [SiftUiCapture]::DwmFlush() | Out-Null
      Start-Sleep -Milliseconds 400
    }
    Write-Host "PASS  $workspace"
  }

  Write-Host "Sift native UI validation passed. Artifacts: $artifacts"
}
finally {
  if ($null -ne $taskFixtureProcess) {
    $taskFixtureProcess.Refresh()
    if (-not $taskFixtureProcess.HasExited) { Stop-Process -Id $taskFixtureProcess.Id -Force -ErrorAction SilentlyContinue }
  }
  $process.Refresh()
  if (-not $process.HasExited) {
    $null = $process.CloseMainWindow()
    if (-not $process.WaitForExit(5000)) { Stop-Process -Id $process.Id -Force }
  }
  if ($settingsExisted) {
    [IO.File]::WriteAllBytes($settingsPath, $settingsBackup)
  } elseif (Test-Path -LiteralPath $settingsPath) {
    Remove-Item -LiteralPath $settingsPath -Force
  }
  if ($orphanRegistryCreated -and $orphanChildName.StartsWith('Sift-UIFixture-', [StringComparison]::Ordinal)) {
    [Microsoft.Win32.Registry]::CurrentUser.DeleteSubKeyTree($orphanSubKey, $false)
  }
  if ($uninstallRegistryCreated -and $uninstallChildName.StartsWith('Sift-UIUninstallFixture-', [StringComparison]::Ordinal)) {
    [Microsoft.Win32.Registry]::CurrentUser.DeleteSubKeyTree($uninstallSubKey, $false)
  }
  if ([IO.Path]::GetFileName($recoveryBackupPath).StartsWith('backup-ui-recovery-', [StringComparison]::Ordinal) -and
      (Split-Path -Parent ([IO.Path]::GetFullPath($recoveryBackupPath))).Equals(
        [IO.Path]::GetFullPath($recoveryBackupDirectory), [StringComparison]::OrdinalIgnoreCase)) {
    Remove-Item -LiteralPath $recoveryBackupPath -Force -ErrorAction SilentlyContinue
  }
  $resolvedLeftover = [IO.Path]::GetFullPath($appDataLeftoverPath)
  $resolvedLocalAppData = [IO.Path]::GetFullPath($env:LOCALAPPDATA).TrimEnd('\')
  if ((Split-Path -Parent $resolvedLeftover).TrimEnd('\').Equals($resolvedLocalAppData, [StringComparison]::OrdinalIgnoreCase) -and
      [IO.Path]::GetFileName($resolvedLeftover).StartsWith('AcmeUiOrphan', [StringComparison]::Ordinal)) {
    Remove-Item -LiteralPath $resolvedLeftover -Recurse -Force -ErrorAction SilentlyContinue
  }
  $resolvedUninstallLeftover = [IO.Path]::GetFullPath($uninstallLeftoverPath)
  if ((Split-Path -Parent $resolvedUninstallLeftover).TrimEnd('\').Equals($resolvedLocalAppData, [StringComparison]::OrdinalIgnoreCase) -and
      [IO.Path]::GetFileName($resolvedUninstallLeftover).StartsWith('AcmeUiUninstallable', [StringComparison]::Ordinal)) {
    Remove-Item -LiteralPath $resolvedUninstallLeftover -Recurse -Force -ErrorAction SilentlyContinue
  }
  $resolvedFixture = [IO.Path]::GetFullPath($fixtureRoot)
  $resolvedTemp = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
  if ($resolvedFixture.StartsWith($resolvedTemp, [StringComparison]::OrdinalIgnoreCase) -and
      [IO.Path]::GetFileName($resolvedFixture).StartsWith('Sift-Storage-UiFixture-', [StringComparison]::Ordinal)) {
    Remove-Item -LiteralPath $resolvedFixture -Recurse -Force -ErrorAction SilentlyContinue
  }
}
