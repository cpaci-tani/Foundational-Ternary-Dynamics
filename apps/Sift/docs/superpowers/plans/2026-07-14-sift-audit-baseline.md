# Sift Audit Baseline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Establish a canonical, validated feature-wiring inventory and capture an honest pre-change validation baseline for all Sift roadmap work.

**Architecture:** A checked-in JSON inventory is the source for audit status and evidence. A PowerShell validator cross-checks that inventory against the shell routes, `AppSettings`, `WinUiAppServices`, elevation operations, and roadmap rows before the existing test/build/UI pipeline runs. Generated reports remain under ignored `artifacts/`; the checked-in baseline records commands and outcomes without claiming failed or skipped gates passed.

**Tech Stack:** PowerShell 7/Windows PowerShell, JSON, .NET 8, xUnit/integration console harness, WinUI 3 validation scripts.

---

## File map

- Create `apps/Sift/docs/audits/sift-feature-audit.json` — canonical feature, setting, service, elevation, and roadmap inventory.
- Create `apps/Sift/docs/audits/VALIDATION_BASELINE.md` — dated pre-change validation evidence.
- Create `apps/Sift/scripts/validate-feature-audit.ps1` — schema and source-coverage validator plus report renderer.
- Modify `apps/Sift/scripts/validate.ps1` — run the audit validator before restore/build/test work.
- Modify `apps/Sift.Tests/Program.cs` — assert the canonical audit artifacts and validation hook remain present.

### Task 1: Capture the untouched validation baseline

**Files:**
- Create: `apps/Sift/docs/audits/VALIDATION_BASELINE.md`
- Generated: `apps/Sift/artifacts/**`

- [ ] **Step 1: Run the existing complete gate and write observed evidence**

Run from `C:\Users\cpaci\Desktop\ftd`:

```powershell
$artifactRoot = 'apps\Sift\artifacts'
New-Item -ItemType Directory -Force -Path $artifactRoot | Out-Null
$logPath = Join-Path $artifactRoot 'validation-baseline.log'
$started = Get-Date
try {
    & apps\Sift\scripts\validate.ps1 *>&1 |
        Tee-Object -FilePath $logPath
    $exitCode = $LASTEXITCODE
    $failure = if ($exitCode -eq 0) { 'None' } else { "Process exit code $exitCode" }
} catch {
    $exitCode = 1
    $failure = $_.Exception.Message
    $_ | Out-String | Add-Content -LiteralPath $logPath
}
$elapsed = (Get-Date) - $started
$os = Get-ComputerInfo -Property OsName, OsVersion, OsBuildNumber
$revision = (git rev-parse HEAD).Trim()
$dotnetVersion = (dotnet --version).Trim()
$outcome = if ($exitCode -eq 0) { 'PASS' } else { 'FAIL' }
$lines = @(
    '# Sift validation baseline',
    '',
    "**Captured UTC:** $($started.ToUniversalTime().ToString('O'))",
    "**Repository revision:** $revision",
    "**Operating system:** $($os.OsName) $($os.OsVersion) build $($os.OsBuildNumber)",
    "**.NET SDK:** $dotnetVersion",
    '**Command:** `apps\Sift\scripts\validate.ps1`',
    "**Outcome:** $outcome",
    "**Exit code:** $exitCode",
    "**Elapsed seconds:** $([math]::Round($elapsed.TotalSeconds, 3))",
    '',
    '## Evidence',
    '',
    '- Console output: `apps/Sift/artifacts/validation-baseline.log`',
    '- UI artifacts: `apps/Sift/artifacts/`',
    "- First failure: $failure",
    '',
    'This document records observed evidence. A failed or not-reached gate is not a pass.'
)
New-Item -ItemType Directory -Force -Path apps\Sift\docs\audits | Out-Null
[IO.File]::WriteAllLines(
    'apps\Sift\docs\audits\VALIDATION_BASELINE.md',
    $lines,
    [Text.UTF8Encoding]::new($false))
```

Expected: exit code `0` only if unit tests, integration validation, Release build, native UI traversal, and `git diff --check` all pass. Preserve the exact failing command and message if any stage fails.

- [ ] **Step 2: Inspect the generated baseline and raw log**

Confirm `VALIDATION_BASELINE.md` exactly matches the observed outcome and the raw log is present. Do not hand-edit `FAIL` to `PASS`; rerun the evidence command after a separately authorized fix.

- [ ] **Step 3: Verify the baseline has complete metadata**

Run:

```powershell
$baseline = Get-Content apps\Sift\docs\audits\VALIDATION_BASELINE.md -Raw
foreach ($label in @(
    'Captured UTC', 'Repository revision', 'Operating system', '.NET SDK',
    'Command', 'Outcome', 'Exit code', 'Elapsed seconds', 'First failure'
)) {
    if ($baseline -notmatch [regex]::Escape("**$label`:**")) {
        throw "Validation baseline is missing $label."
    }
}
```

Expected: no output and exit code `0`.

### Task 2: Add a failing canonical-audit integration check

**Files:**
- Modify: `apps/Sift.Tests/Program.cs`

- [ ] **Step 1: Register the audit contract check**

In `Main`, call `ValidateFeatureAuditContract()` immediately before `ValidateCanonicalWinUiSource()`.

- [ ] **Step 2: Add the failing check**

Add these imports:

```csharp
using System.Text.Json;
```

Add this method beside `ValidateCanonicalWinUiSource`:

```csharp
private static void ValidateFeatureAuditContract()
{
    var root = FindRepositoryRoot();
    var app = Path.Combine(root, "apps", "Sift");
    var auditPath = Path.Combine(app, "docs", "audits", "sift-feature-audit.json");
    var validatorPath = Path.Combine(app, "scripts", "validate-feature-audit.ps1");
    Check(File.Exists(auditPath), "canonical Sift feature audit exists");
    Check(File.Exists(validatorPath), "feature-audit validator exists");
    if (!File.Exists(auditPath)) return;

    using var document = JsonDocument.Parse(File.ReadAllText(auditPath));
    var rootElement = document.RootElement;
    Check(rootElement.GetProperty("schemaVersion").GetInt32() == 1, "feature audit uses schema version 1");
    var entries = rootElement.GetProperty("entries").EnumerateArray().ToArray();
    var ids = entries.Select(entry => entry.GetProperty("id").GetString()).ToArray();
    Check(ids.All(id => !string.IsNullOrWhiteSpace(id)) &&
          ids.Distinct(StringComparer.OrdinalIgnoreCase).Count() == ids.Length,
        "feature audit IDs are present and unique");

    foreach (var route in new[]
    {
        "Home", "Optimize", "TaskManager", "Performance", "Startup", "Maintenance",
        "Health", "Recovery", "Storage", "Apps", "SystemInfo", "Settings"
    })
        Check(entries.Any(entry =>
                entry.GetProperty("kind").GetString() == "route" &&
                string.Equals(entry.GetProperty("route").GetString(), route, StringComparison.Ordinal)),
            $"feature audit covers route {route}");

    var statuses = new HashSet<string>(StringComparer.Ordinal)
    {
        "wired", "intentionally-internal", "future", "disconnected",
        "blocked-external", "obsolete"
    };
    Check(entries.All(entry => statuses.Contains(entry.GetProperty("status").GetString() ?? "")),
        "feature audit statuses use the declared vocabulary");
    Check(entries.All(entry =>
            entry.GetProperty("automatedEvidence").ValueKind == JsonValueKind.Array &&
            entry.GetProperty("visualStates").ValueKind == JsonValueKind.Array),
        "feature audit records automated and visual evidence arrays");
}
```

- [ ] **Step 3: Run integration validation and verify it fails**

Run:

```powershell
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
```

Expected: FAIL with `canonical Sift feature audit exists` and `feature-audit validator exists`.

### Task 3: Create the canonical audit inventory

**Files:**
- Create: `apps/Sift/docs/audits/sift-feature-audit.json`

- [ ] **Step 1: Add the schema header and status vocabulary**

Use this root structure:

```json
{
  "schemaVersion": 1,
  "generatedUtc": "2026-07-14T00:00:00.0000000Z",
  "statusVocabulary": [
    "wired",
    "intentionally-internal",
    "future",
    "disconnected",
    "blocked-external",
    "obsolete"
  ],
  "entries": []
}
```

Every entry has exactly these fields:

```json
{
  "id": "route.home",
  "kind": "route",
  "route": "Home",
  "presentation": ["MainWindow.xaml:NavigationViewItem Tag=Home"],
  "composition": ["MainWindow.xaml.cs:BuildWorkspaceRegistry"],
  "coreBoundary": ["Sift.Core/Services/ProcessSampler.cs"],
  "persistence": [],
  "mutationContract": "read-only",
  "automatedEvidence": ["Sift/scripts/validate-ui.ps1:workspace traversal"],
  "visualStates": ["normal"],
  "status": "wired",
  "roadmapId": "roadmap.home",
  "ownerPlan": "2026-07-14-sift-shell-home-settings.md"
}
```

- [ ] **Step 2: Populate all route records**

Add one `kind: "route"` record for each exact route:

```text
Home
Optimize
TaskManager
Performance
Startup
Maintenance
Health
Recovery
Storage
Apps
SystemInfo
Settings
```

Record current evidence, not desired future evidence. At baseline:

- `Home` and `Health` are `wired` routes with incomplete visual-state arrays.
- All other routes are `wired`.
- Missing route-specific automation is represented by absent visual states, not by changing a functioning route to `disconnected`.

- [ ] **Step 3: Populate all `AppSettings` records**

Add one `kind: "setting"` record for each exact property from `Sift.Core/Models/AppSettings.cs`:

```text
OptimizeCategory
OptimizeRiskFilter
RefreshInterval
ChartFps
ChartHistory
ChartSmoothing
VisibleColumns
HomeWidgets
CpuFilterIndex
MemoryFilterIndex
StatusFilterIndex
ArchitectureFilterIndex
PriorityFilterIndex
OfferSystemRestorePoint
LastWorkspace
LastMaintenanceScanUtc
LastStorageScanUtc
StorageRoots
ConsoleVisible
ConsoleWidth
PendingOptimizeSelectionIds
```

Use these initial classifications:

- `RefreshInterval`, `ChartHistory`, `LastWorkspace`, `LastStorageScanUtc`, `StorageRoots`, `ConsoleVisible`, and `ConsoleWidth`: `wired`.
- `HomeWidgets`, `ChartSmoothing`, and `OfferSystemRestorePoint`: `disconnected`, each with its owning plan.
- `LastMaintenanceScanUtc`: `disconnected` because mutation is not immediately persisted.
- `OptimizeCategory`, `OptimizeRiskFilter`, `ChartFps`, `VisibleColumns`, five filter-index properties, and `PendingOptimizeSelectionIds`: `obsolete`, with migration-safe removal assigned to the disconnected-wiring plan.

- [ ] **Step 4: Populate all composition-service records**

Add one `kind: "service"` record for each required property in `WinUiAppServices`:

```text
Tweaks
Processes
MaintenanceScanner
MaintenanceCleaner
StorageScanner
StorageDeleter
StorageDeletion
Health
Services
Tasks
Startup
InstalledApps
InstalledAppManager
InstalledAppTrust
AppLeftovers
SystemInformation
GuardedActions
Elevation
Recovery
SettingsStore
Activity
Operations
SettingsPersistence
```

Classify `StorageDeleter` as `obsolete` public exposure while recording that its shared instance remains internally required. Classify all others as `wired`; separately add `kind: "capability"` records for disconnected methods or stores so the service itself is not mislabeled.

- [ ] **Step 5: Populate elevation and disconnected-capability records**

Add one `kind: "elevation"` record for each exact enum member:

```text
ApplyMachineTweaks
RestoreMachineBackup
ValidateElevation
ManageService
```

Add `kind: "capability"` records for:

- scheduled-task enable/disable: `disconnected`;
- restore-point helper: `disconnected`;
- activity/backup history aggregation: `disconnected`;
- process CSV export: `future`;
- telemetry hub: `obsolete`;
- Optimize latest-restore shortcut: `wired` with future Recovery migration.

- [ ] **Step 6: Populate every roadmap row**

Add one `kind: "roadmap"` record for each exact area:

```text
Core execution and safety
Shell and navigation
Activity console
Home
Optimize
Task Manager
Performance
Startup
Maintenance
Health
Recovery
Storage
Installed Apps
System Information
Settings and dialogs
Packaging
```

Use `future` when the row's completion gate is not complete. Use `blocked-external` only for trusted publisher signing/clean-account trust prerequisites, not for implementation work.

- [ ] **Step 7: Validate JSON syntax**

Run:

```powershell
Get-Content apps\Sift\docs\audits\sift-feature-audit.json -Raw |
    ConvertFrom-Json -Depth 20 | Out-Null
```

Expected: exit code `0`.

### Task 4: Implement the source-coverage validator

**Files:**
- Create: `apps/Sift/scripts/validate-feature-audit.ps1`

- [ ] **Step 1: Implement parameters and bounded parsing**

Start the script with:

```powershell
param(
    [string]$AuditPath,
    [string]$ReportPath
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$appsRoot = Split-Path -Parent $projectRoot

if ([string]::IsNullOrWhiteSpace($AuditPath)) {
    $AuditPath = Join-Path $projectRoot 'docs\audits\sift-feature-audit.json'
}
if ([string]::IsNullOrWhiteSpace($ReportPath)) {
    $ReportPath = Join-Path $projectRoot 'artifacts\feature-audit.md'
}
if (-not (Test-Path -LiteralPath $AuditPath -PathType Leaf)) {
    throw "Feature audit not found: $AuditPath"
}
if ((Get-Item -LiteralPath $AuditPath).Length -gt 4MB) {
    throw 'Feature audit exceeds the 4 MiB bound.'
}

$audit = Get-Content -LiteralPath $AuditPath -Raw | ConvertFrom-Json -Depth 20
if ($audit.schemaVersion -ne 1) { throw 'Unsupported feature-audit schema.' }
$entries = @($audit.entries)
if ($entries.Count -eq 0) { throw 'Feature audit contains no entries.' }
```

- [ ] **Step 2: Validate fields, IDs, statuses, decisions, and evidence**

Use:

```powershell
$statuses = @(
    'wired', 'intentionally-internal', 'future',
    'disconnected', 'blocked-external', 'obsolete'
)
$ids = [System.Collections.Generic.HashSet[string]]::new(
    [System.StringComparer]::OrdinalIgnoreCase)

foreach ($entry in $entries) {
    foreach ($field in @(
        'id', 'kind', 'route', 'presentation', 'composition', 'coreBoundary',
        'persistence', 'mutationContract', 'automatedEvidence', 'visualStates',
        'status', 'roadmapId', 'ownerPlan'
    )) {
        if ($null -eq $entry.$field) { throw "$($entry.id): missing $field" }
    }
    if ([string]::IsNullOrWhiteSpace($entry.id)) { throw 'Audit entry has a blank ID.' }
    if (-not $ids.Add([string]$entry.id)) { throw "Duplicate audit ID: $($entry.id)" }
    if ($statuses -notcontains $entry.status) { throw "$($entry.id): invalid status $($entry.status)" }
    if (($entry.status -eq 'disconnected' -or $entry.status -eq 'obsolete') -and
        [string]::IsNullOrWhiteSpace($entry.ownerPlan)) {
        throw "$($entry.id): disconnected/obsolete entries require ownerPlan"
    }
    if ($entry.status -eq 'wired' -and @($entry.automatedEvidence).Count -eq 0) {
        throw "$($entry.id): wired entries require automated evidence"
    }
}
```

- [ ] **Step 3: Cross-check routes**

Parse `MainWindow.xaml` tags and append `Settings`:

```powershell
$shell = Get-Content (Join-Path $projectRoot 'MainWindow.xaml') -Raw
$routes = @([regex]::Matches($shell, '<NavigationViewItem\b[^>]*\bTag="([^"]+)"') |
    ForEach-Object { $_.Groups[1].Value }) + 'Settings'
$auditedRoutes = @($entries | Where-Object kind -eq 'route' | ForEach-Object route)
foreach ($route in $routes) {
    if ($auditedRoutes -cnotcontains $route) { throw "Unaudited route: $route" }
}
foreach ($route in $auditedRoutes) {
    if ($routes -cnotcontains $route) { throw "Audit contains nonexistent route: $route" }
}
```

- [ ] **Step 4: Cross-check settings, services, and elevation kinds**

Use source regexes bounded to the canonical declarations:

```powershell
$settingsSource = Get-Content (Join-Path $appsRoot 'Sift.Core\Models\AppSettings.cs') -Raw
$settings = [regex]::Matches(
    $settingsSource,
    'public\s+(?:[\w?<>,\[\]\.]+)\s+(\w+)\s*\{\s*get;\s*set;'
) | ForEach-Object { $_.Groups[1].Value }
$auditedSettings = @($entries | Where-Object kind -eq 'setting' | ForEach-Object id) |
    ForEach-Object { $_ -replace '^setting\.', '' }

$servicesSource = Get-Content (Join-Path $projectRoot 'Composition\WinUiAppServices.cs') -Raw
$services = [regex]::Matches(
    $servicesSource,
    'public\s+required\s+[\w<>]+\s+(\w+)\s*\{\s*get;\s*init;'
) | ForEach-Object { $_.Groups[1].Value }
$auditedServices = @($entries | Where-Object kind -eq 'service' | ForEach-Object id) |
    ForEach-Object { $_ -replace '^service\.', '' }

$elevationSource = Get-Content (Join-Path $appsRoot 'Sift.Core\Services\ElevationBroker.cs') -Raw
$enumBody = [regex]::Match(
    $elevationSource,
    'public\s+enum\s+ElevatedOperationKind\s*\{(?<body>[^}]*)\}',
    [System.Text.RegularExpressions.RegexOptions]::Singleline
).Groups['body'].Value
$elevationKinds = $enumBody -split ',' |
    ForEach-Object { ($_ -replace '//.*', '').Trim() } |
    Where-Object { $_ }
$auditedElevation = @($entries | Where-Object kind -eq 'elevation' | ForEach-Object id) |
    ForEach-Object { $_ -replace '^elevation\.', '' }

foreach ($pair in @(
    @{ Name = 'setting'; Source = @($settings); Audit = @($auditedSettings) },
    @{ Name = 'service'; Source = @($services); Audit = @($auditedServices) },
    @{ Name = 'elevation operation'; Source = @($elevationKinds); Audit = @($auditedElevation) }
)) {
    foreach ($name in $pair.Source) {
        if ($pair.Audit -cnotcontains $name) { throw "Unaudited $($pair.Name): $name" }
    }
    foreach ($name in $pair.Audit) {
        if ($pair.Source -cnotcontains $name) { throw "Audit contains nonexistent $($pair.Name): $name" }
    }
}
```

- [ ] **Step 5: Cross-check roadmap rows and render the ignored report**

Use:

```powershell
$roadmap = Get-Content (Join-Path $projectRoot 'ROADMAP.md') -Raw
$roadmapAreas = [regex]::Matches($roadmap, '(?m)^\|\s*([^|]+?)\s*\|') |
    ForEach-Object { $_.Groups[1].Value.Trim() } |
    Where-Object { $_ -notin @('Area', '---') }
$auditedAreas = @($entries | Where-Object kind -eq 'roadmap' |
    ForEach-Object { $_.presentation[0] })
foreach ($area in $roadmapAreas) {
    if ($auditedAreas -cnotcontains $area) { throw "Unaudited roadmap area: $area" }
}

$reportDirectory = Split-Path -Parent $ReportPath
New-Item -ItemType Directory -Force -Path $reportDirectory | Out-Null
$lines = @(
    '# Sift feature audit report',
    '',
    "Generated UTC: $([DateTime]::UtcNow.ToString('O'))",
    '',
    '| Status | Count |',
    '|---|---:|'
)
foreach ($status in $statuses) {
    $count = @($entries | Where-Object status -eq $status).Count
    $lines += "| $status | $count |"
}
$lines += ''
$lines += '## Disconnected, obsolete, and blocked items'
foreach ($entry in $entries | Where-Object {
    $_.status -in @('disconnected', 'obsolete', 'blocked-external')
} | Sort-Object id) {
    $lines += "- **$($entry.id)** — $($entry.status); owner: $($entry.ownerPlan)"
}
[IO.File]::WriteAllLines($ReportPath, $lines, [Text.UTF8Encoding]::new($false))
Write-Host "Sift feature audit validated: $($entries.Count) entries."
```

- [ ] **Step 6: Run the validator**

Run:

```powershell
apps\Sift\scripts\validate-feature-audit.ps1
```

Expected: `Sift feature audit validated:` followed by a positive entry count, and exit code `0`.

### Task 5: Insert the audit gate into normal validation

**Files:**
- Modify: `apps/Sift/scripts/validate.ps1`

- [ ] **Step 1: Add the gate before package restore**

After repository-root calculation and before the first `dotnet restore`, add:

```powershell
& (Join-Path $PSScriptRoot 'validate-feature-audit.ps1')
if ($LASTEXITCODE -ne 0) { throw 'Feature-audit validation failed.' }
```

- [ ] **Step 2: Prove an uncovered source item fails validation**

Copy the audit to a temporary file, remove the `setting.ChartFps` entry, and invoke:

```powershell
$audit = Get-Content apps\Sift\docs\audits\sift-feature-audit.json -Raw |
    ConvertFrom-Json -Depth 20
$audit.entries = @($audit.entries | Where-Object id -ne 'setting.ChartFps')
$temp = Join-Path $env:TEMP 'sift-feature-audit-missing-setting.json'
$audit | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $temp -Encoding utf8
try {
    apps\Sift\scripts\validate-feature-audit.ps1 -AuditPath $temp
    if ($LASTEXITCODE -eq 0) { throw 'Validator accepted an unaudited setting.' }
} finally {
    Remove-Item -LiteralPath $temp -Force -ErrorAction SilentlyContinue
}
```

Expected: failure containing `Unaudited setting: ChartFps`.

- [ ] **Step 3: Run integration validation**

Run:

```powershell
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
```

Expected: PASS, including:

```text
[PASS] canonical Sift feature audit exists
[PASS] feature-audit validator exists
[PASS] feature audit IDs are present and unique
```

### Task 6: Verify the baseline delivery

**Files:**
- Verify all files listed in the file map.

- [ ] **Step 1: Run the audit gate alone**

```powershell
apps\Sift\scripts\validate-feature-audit.ps1
```

Expected: PASS and `apps/Sift/artifacts/feature-audit.md` exists.

- [ ] **Step 2: Run the complete Sift gate**

```powershell
apps\Sift\scripts\validate.ps1
```

Expected: `Sift validation completed successfully.` with zero build warnings. If the pre-change baseline contained a real failure, this task is complete only after recording that the same failure remains or after fixing it under a separately approved implementation plan.

- [ ] **Step 3: Verify formatting**

```powershell
git diff --check
```

Expected: exit code `0`.

- [ ] **Step 4: Review generated and checked-in evidence**

Confirm:

- `VALIDATION_BASELINE.md` contains no template markers and does not label skipped gates as passed.
- `feature-audit.md` lists every disconnected, obsolete, and externally blocked item with an owner plan.
- generated artifacts are ignored;
- the canonical JSON and baseline Markdown are tracked candidates;
- no application behavior or safety boundary changed in this plan.
