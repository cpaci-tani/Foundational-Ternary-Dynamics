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
$AuditPath = [IO.Path]::GetFullPath($AuditPath)
$ReportPath = [IO.Path]::GetFullPath($ReportPath)
if ([string]::Equals($AuditPath, $ReportPath, [StringComparison]::OrdinalIgnoreCase)) {
  throw "AuditPath and ReportPath resolve to the same path: '$AuditPath'."
}

function Get-SourceText {
  param(
    [Parameter(Mandatory = $true)]
    [string]$Path,
    [Parameter(Mandatory = $true)]
    [string]$Description
  )

  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    throw "$Description was not found at '$Path'."
  }
  return [IO.File]::ReadAllText((Resolve-Path -LiteralPath $Path).Path)
}

function Test-AuditFileReference {
  param(
    [Parameter(Mandatory = $true)]
    [string]$Value,
    [Parameter(Mandatory = $true)]
    [string]$Description
  )

  $reference = $Value.Trim()
  $filePattern = 'Sift(?:\.(?:Core|Tests|UnitTests|ElevationHost|MonitorHost))?/[^:]+?\.(?:cs|xaml|ps1|json|csproj|md|targets|xml|manifest|js|mjs)'
  if ($reference -match "^resolved removal:\s*(?<path>$filePattern)\s+absent$") {
    $removedPath = Join-Path $appsRoot ($Matches['path'] -replace '/', '\')
    if (Test-Path -LiteralPath $removedPath) {
      throw "$Description claims that '$($Matches['path'])' is absent, but the path exists."
    }
    return $true
  }
  if ($reference -notmatch "^(?<path>$filePattern)(?::.*)?$") {
    return $false
  }

  $relativePath = $Matches['path'] -replace '/', '\'
  $resolvedPath = Join-Path $appsRoot $relativePath
  if (-not (Test-Path -LiteralPath $resolvedPath -PathType Leaf)) {
    throw "$Description references missing file '$($Matches['path'])'."
  }
  return $true
}

function Test-MarkdownLocalLinks {
  param(
    [Parameter(Mandatory = $true)]
    [string]$Path
  )

  $sourceText = Get-SourceText -Path $Path -Description ([IO.Path]::GetFileName($Path))
  $directory = [IO.Path]::GetDirectoryName($Path)
  foreach ($match in [regex]::Matches($sourceText, '\[[^\]]+\]\((?<target>[^)]+)\)')) {
    $target = $match.Groups['target'].Value.Trim()
    if ($target.StartsWith('<') -and $target.EndsWith('>')) {
      $target = $target.Substring(1, $target.Length - 2)
    }
    if ($target -match '^(?:https?://|mailto:|#)') {
      continue
    }
    $target = ($target -split '#', 2)[0]
    if ([string]::IsNullOrWhiteSpace($target)) {
      continue
    }
    $linkedPath = [IO.Path]::GetFullPath((Join-Path $directory ($target -replace '/', '\')))
    if (-not (Test-Path -LiteralPath $linkedPath -PathType Leaf)) {
      throw "Markdown link '$target' in '$Path' does not resolve to a file."
    }
  }
}

function Compare-ExactMembers {
  param(
    [Parameter(Mandatory = $true)]
    [string]$Description,
    [Parameter(Mandatory = $true)]
    [object[]]$SourceMembers,
    [Parameter(Mandatory = $true)]
    [object[]]$AuditMembers
  )

  $source = @($SourceMembers | ForEach-Object { ([string]$_).Trim() } |
    Where-Object { -not [string]::IsNullOrWhiteSpace($_) } | Sort-Object)
  $invalidAuditPositions = @()
  for ($index = 0; $index -lt $AuditMembers.Count; $index++) {
    if ($null -eq $AuditMembers[$index] -or
        [string]::IsNullOrWhiteSpace([string]$AuditMembers[$index])) {
      $invalidAuditPositions += ($index + 1)
    }
  }
  if ($invalidAuditPositions.Count -gt 0) {
    throw "$Description cross-check failed; null or blank audit member(s) at position(s): $($invalidAuditPositions -join ', ')."
  }
  $audited = @($AuditMembers | ForEach-Object { ([string]$_).Trim() } | Sort-Object)
  $seenAuditMembers = @()
  $duplicateAuditMembers = @()
  foreach ($member in $audited) {
    if ($seenAuditMembers -ccontains $member) {
      if ($duplicateAuditMembers -cnotcontains $member) {
        $duplicateAuditMembers += $member
      }
    } else {
      $seenAuditMembers += $member
    }
  }
  if ($duplicateAuditMembers.Count -gt 0) {
    throw "$Description cross-check failed; duplicate audit member(s): $($duplicateAuditMembers -join ', ')."
  }

  $missing = @($source | Where-Object { $audited -cnotcontains $_ })
  $nonexistent = @($audited | Where-Object { $source -cnotcontains $_ })
  $problems = @()
  if ($missing.Count -gt 0) {
    $problems += "missing audit member(s): $($missing -join ', ')"
  }
  if ($nonexistent.Count -gt 0) {
    $problems += "nonexistent source member(s): $($nonexistent -join ', ')"
  }
  if ($source.Count -ne $audited.Count) {
    $problems += "cardinality mismatch: source has $($source.Count) member(s), audit has $($audited.Count)"
  }
  if ($problems.Count -gt 0) {
    throw "$Description cross-check failed; $($problems -join '; ')."
  }
}

$auditFile = Get-Item -LiteralPath $AuditPath -ErrorAction Stop
$maximumAuditBytes = 4 * 1024 * 1024
if ($auditFile.Length -gt $maximumAuditBytes) {
  throw "Feature audit exceeds the 4 MiB input limit: '$($auditFile.FullName)'."
}

try {
  $audit = [IO.File]::ReadAllText($auditFile.FullName) | ConvertFrom-Json
} catch {
  throw "Feature audit JSON could not be parsed: $($_.Exception.Message)"
}
if ($null -eq $audit) {
  throw 'Feature audit JSON is empty.'
}
if ($null -eq $audit.PSObject.Properties['schemaVersion'] -or $audit.schemaVersion -ne 1) {
  throw 'Feature audit schemaVersion must be 1.'
}
if ($null -eq $audit.PSObject.Properties['entries'] -or $null -eq $audit.entries -or
    @($audit.entries).Count -eq 0) {
  throw 'Feature audit entries must be nonempty.'
}

$requiredFields = @(
  'id',
  'kind',
  'route',
  'presentation',
  'composition',
  'coreBoundary',
  'persistence',
  'mutationContract',
  'automatedEvidence',
  'visualStates',
  'status',
  'roadmapId',
  'ownerPlan'
)
$allowedStatuses = @(
  'wired',
  'intentionally-internal',
  'future',
  'disconnected',
  'blocked-external',
  'obsolete'
)
$entries = @($audit.entries)
$ids = @{}
$resolvedReferenceCount = 0
$registeredPlanProvenanceCount = 0

for ($index = 0; $index -lt $entries.Count; $index++) {
  $entry = $entries[$index]
  if ($null -eq $entry) {
    throw "Feature audit entry at index $index must not be null."
  }
  foreach ($field in $requiredFields) {
    $property = $entry.PSObject.Properties[$field]
    if ($null -eq $property -or $null -eq $property.Value) {
      throw "Feature audit entry at index $index has a null or missing '$field' field."
    }
  }

  $id = [string]$entry.id
  if ([string]::IsNullOrWhiteSpace($id)) {
    throw "Feature audit entry at index $index has a blank id."
  }
  if ($ids.ContainsKey($id)) {
    throw "Feature audit id '$id' is duplicated case-insensitively."
  }
  $ids[$id] = $true

  $status = [string]$entry.status
  if ($allowedStatuses -cnotcontains $status) {
    throw "Feature audit entry '$id' has invalid status '$status'. Allowed statuses: $($allowedStatuses -join ', ')."
  }
  if (($status -eq 'disconnected' -or $status -eq 'obsolete') -and
      [string]::IsNullOrWhiteSpace([string]$entry.ownerPlan)) {
    throw "Feature audit entry '$id' with status '$status' requires a nonblank ownerPlan."
  }
  if ($status -eq 'wired') {
    $evidence = @($entry.automatedEvidence)
    $nonblankEvidence = @($evidence | Where-Object { -not [string]::IsNullOrWhiteSpace([string]$_) })
    if ($evidence.Count -eq 0 -or $nonblankEvidence.Count -eq 0) {
      throw "Feature audit entry '$id' with status 'wired' requires nonempty automatedEvidence."
    }
  }

  foreach ($evidenceField in @('presentation', 'composition', 'coreBoundary', 'persistence', 'automatedEvidence')) {
    $values = @($entry.$evidenceField)
    for ($valueIndex = 0; $valueIndex -lt $values.Count; $valueIndex++) {
      if ($null -eq $values[$valueIndex] -or
          [string]::IsNullOrWhiteSpace([string]$values[$valueIndex])) {
        throw "Feature audit entry '$id' has a blank $evidenceField reference at position $($valueIndex + 1)."
      }
      if (Test-AuditFileReference -Value ([string]$values[$valueIndex]) `
          -Description "Feature audit entry '$id' field '$evidenceField'") {
        $resolvedReferenceCount++
      }
    }
  }

  $visualStates = @($entry.visualStates)
  $duplicateVisualStates = @($visualStates | Group-Object | Where-Object Count -gt 1 | ForEach-Object Name)
  if ($duplicateVisualStates.Count -gt 0) {
    throw "Feature audit entry '$id' duplicates visual state(s): $($duplicateVisualStates -join ', ')."
  }
  if ($status -eq 'wired' -and $visualStates.Count -gt 0 -and
      $entry.kind -in @('route', 'capability', 'roadmap') -and
      -not (@($entry.automatedEvidence) -match '^Sift/scripts/validate-ui\.ps1(?::|$)')) {
    throw "Feature audit entry '$id' claims current visual states without native UI evidence."
  }
}

$roadmapIds = @($entries | Where-Object kind -eq 'roadmap' | ForEach-Object { [string]$_.id })
$historicalProgramIndexPath = Join-Path $projectRoot 'docs\superpowers\plans\2026-07-14-sift-program-index.md'
$historicalProgramIndex = Get-SourceText -Path $historicalProgramIndexPath -Description 'historical Sift program index'
foreach ($entry in $entries) {
  $id = [string]$entry.id
  $roadmapId = [string]$entry.roadmapId
  if ([string]::IsNullOrWhiteSpace($roadmapId) -or $roadmapIds -cnotcontains $roadmapId) {
    throw "Feature audit entry '$id' references unknown roadmapId '$roadmapId'."
  }

  $ownerPlan = ([string]$entry.ownerPlan).Trim()
  if ([string]::IsNullOrWhiteSpace($ownerPlan)) {
    continue
  }
  if ($ownerPlan -match '^Sift/') {
    if (-not (Test-AuditFileReference -Value $ownerPlan -Description "Feature audit entry '$id' ownerPlan")) {
      throw "Feature audit entry '$id' has invalid ownerPlan '$ownerPlan'."
    }
    $resolvedReferenceCount++
    continue
  }
  $planPath = Join-Path $projectRoot "docs\superpowers\plans\$ownerPlan"
  if (Test-Path -LiteralPath $planPath -PathType Leaf) {
    $resolvedReferenceCount++
    continue
  }
  $escapedPlan = [regex]::Escape($ownerPlan)
  if ($historicalProgramIndex -notmatch "(?m)^\|\s*\d+\s*\|\s*``$escapedPlan``\s*\|\s*Not created\s*\|") {
    throw "Feature audit entry '$id' references missing owner plan '$ownerPlan' without registered provenance."
  }
  $registeredPlanProvenanceCount++
}

foreach ($documentPath in @(
    (Join-Path $projectRoot 'README.md'),
    (Join-Path $projectRoot 'ARCHITECTURE.md'),
    (Join-Path $projectRoot 'ROADMAP.md'),
    (Join-Path $projectRoot 'docs\SECURITY_AND_PERMISSIONS.md'),
    (Join-Path $projectRoot 'docs\BUILD_AND_RELEASE.md'),
    $historicalProgramIndexPath
  )) {
  Test-MarkdownLocalLinks -Path $documentPath
}

$appProjectText = Get-SourceText -Path (Join-Path $projectRoot 'Sift.csproj') -Description 'Sift.csproj'
$hostProjectText = Get-SourceText -Path (Join-Path $appsRoot 'Sift.ElevationHost\Sift.ElevationHost.csproj') `
  -Description 'Sift.ElevationHost.csproj'
$monitorProjectText = Get-SourceText -Path (Join-Path $appsRoot 'Sift.MonitorHost\Sift.MonitorHost.csproj') `
  -Description 'Sift.MonitorHost.csproj'
$appVersion = [regex]::Match($appProjectText, '<Version>(?<version>[^<]+)</Version>').Groups['version'].Value
$hostVersion = [regex]::Match($hostProjectText, '<Version>(?<version>[^<]+)</Version>').Groups['version'].Value
$monitorVersion = [regex]::Match($monitorProjectText, '<Version>(?<version>[^<]+)</Version>').Groups['version'].Value
if ([string]::IsNullOrWhiteSpace($appVersion) -or [string]::IsNullOrWhiteSpace($hostVersion) -or
    [string]::IsNullOrWhiteSpace($monitorVersion) -or
    -not [string]::Equals($appVersion, $hostVersion, [StringComparison]::Ordinal) -or
    -not [string]::Equals($appVersion, $monitorVersion, [StringComparison]::Ordinal)) {
  throw "Application, elevation-host, and monitor-host versions must match (app '$appVersion', elevation '$hostVersion', monitor '$monitorVersion')."
}

$mainWindowPath = Join-Path $projectRoot 'MainWindow.xaml'
$mainWindowText = Get-SourceText -Path $mainWindowPath -Description 'MainWindow.xaml'
$routeMatches = [regex]::Matches(
  $mainWindowText,
  '<NavigationViewItem\b[^>]*\bTag\s*=\s*"(?<tag>[^"]+)"',
  [Text.RegularExpressions.RegexOptions]::IgnoreCase
)
$sourceRoutes = @($routeMatches | ForEach-Object { $_.Groups['tag'].Value }) + @('Settings')
$auditRoutes = @($entries | Where-Object { $_.kind -eq 'route' } |
  ForEach-Object { [string]$_.route })
Compare-ExactMembers -Description 'MainWindow route' -SourceMembers $sourceRoutes -AuditMembers $auditRoutes

$appSettingsPath = Join-Path $appsRoot 'Sift.Core\Models\AppSettings.cs'
$appSettingsText = Get-SourceText -Path $appSettingsPath -Description 'AppSettings.cs'
$settingMatches = [regex]::Matches(
  $appSettingsText,
  '^\s*public\s+[^\r\n{]+\s+(?<name>[A-Za-z_][A-Za-z0-9_]*)\s*\{\s*get;\s*set;\s*\}',
  [Text.RegularExpressions.RegexOptions]::Multiline
)
$sourceSettings = @($settingMatches | ForEach-Object { $_.Groups['name'].Value })
$auditSettings = @($entries | Where-Object { $_.kind -eq 'setting' } |
  ForEach-Object { ([string]$_.id) -replace '^setting\.', '' })
Compare-ExactMembers -Description 'AppSettings property' -SourceMembers $sourceSettings -AuditMembers $auditSettings

$servicesPath = Join-Path $projectRoot 'Composition\WinUiAppServices.cs'
$servicesText = Get-SourceText -Path $servicesPath -Description 'WinUiAppServices.cs'
$serviceMatches = [regex]::Matches(
  $servicesText,
  '^\s*public\s+required\s+[^\r\n{]+\s+(?<name>[A-Za-z_][A-Za-z0-9_]*)\s*\{\s*get;\s*init;\s*\}',
  [Text.RegularExpressions.RegexOptions]::Multiline
)
$sourceServices = @($serviceMatches | ForEach-Object { $_.Groups['name'].Value })
$auditPublicServices = @($entries | Where-Object {
    $_.kind -eq 'service' -and [string]$_.status -ne 'intentionally-internal'
  } | ForEach-Object { ([string]$_.id) -replace '^service\.', '' })
$auditInternalServices = @($entries | Where-Object {
    $_.kind -eq 'service' -and [string]$_.status -eq 'intentionally-internal'
  } | ForEach-Object { ([string]$_.id) -replace '^service\.', '' })
Compare-ExactMembers -Description 'WinUiAppServices property' -SourceMembers $sourceServices -AuditMembers $auditPublicServices
foreach ($internalService in $auditInternalServices) {
  if ($sourceServices -ccontains $internalService) {
    throw "Intentionally-internal service '$internalService' must not remain a public required WinUiAppServices property."
  }
  $entry = $entries | Where-Object { $_.id -eq "service.$internalService" } | Select-Object -First 1
  if ($null -eq $entry) {
    throw "Intentionally-internal service '$internalService' is missing from the feature audit."
  }
  foreach ($evidenceField in @('composition', 'coreBoundary', 'automatedEvidence')) {
    $values = @($entry.$evidenceField)
    $nonblank = @($values | Where-Object { -not [string]::IsNullOrWhiteSpace([string]$_) })
    if ($values.Count -eq 0 -or $nonblank.Count -eq 0) {
      throw "Intentionally-internal service '$internalService' requires nonempty $evidenceField evidence."
    }
  }
}

$elevationPath = Join-Path $appsRoot 'Sift.Core\Services\ElevationBroker.cs'
$elevationText = Get-SourceText -Path $elevationPath -Description 'ElevationBroker.cs'
$enumMatch = [regex]::Match(
  $elevationText,
  'public\s+enum\s+ElevatedOperationKind\s*\{(?<body>.*?)\}',
  [Text.RegularExpressions.RegexOptions]::Singleline
)
if (-not $enumMatch.Success) {
  throw 'ElevatedOperationKind enum could not be parsed from ElevationBroker.cs.'
}
$sourceElevation = @($enumMatch.Groups['body'].Value -split ',' | ForEach-Object {
    $member = ($_ -replace '//[^\r\n]*', '').Trim()
    if (-not [string]::IsNullOrWhiteSpace($member)) {
      ($member -split '=')[0].Trim()
    }
  })
$auditElevation = @($entries | Where-Object { $_.kind -eq 'elevation' } |
  ForEach-Object { ([string]$_.id) -replace '^elevation\.', '' })
Compare-ExactMembers -Description 'ElevatedOperationKind' -SourceMembers $sourceElevation -AuditMembers $auditElevation

$roadmapPath = Join-Path $projectRoot 'ROADMAP.md'
$roadmapText = Get-SourceText -Path $roadmapPath -Description 'ROADMAP.md'
$sourceRoadmapAreas = @()
foreach ($line in ($roadmapText -split "\r?\n")) {
  if ($line -notmatch '^\s*\|') {
    continue
  }
  $columns = @($line.Trim().Trim('|').Split('|'))
  if ($columns.Count -eq 0) {
    continue
  }
  $area = $columns[0].Trim()
  if ($area -ne 'Area' -and $area -notmatch '^:?-{3,}:?$') {
    $sourceRoadmapAreas += $area
  }
}
$auditRoadmapAreas = @($entries | Where-Object { $_.kind -eq 'roadmap' } | ForEach-Object {
    $presentation = @($_.presentation)
    if ($presentation.Count -eq 0 -or [string]::IsNullOrWhiteSpace([string]$presentation[0])) {
      throw "Roadmap audit entry '$($_.id)' requires a nonblank presentation[0]."
    }
    [string]$presentation[0]
  })
Compare-ExactMembers -Description 'ROADMAP area' -SourceMembers $sourceRoadmapAreas -AuditMembers $auditRoadmapAreas

$reportLines = New-Object 'System.Collections.Generic.List[string]'
$reportLines.Add('# Sift Feature Audit')
$reportLines.Add('')
$reportLines.Add("Generated UTC: $([DateTime]::UtcNow.ToString('o'))")
$reportLines.Add('')
$reportLines.Add("Validated entries: $($entries.Count)")
$reportLines.Add("Resolved existing file and plan references: $resolvedReferenceCount")
$reportLines.Add("Registered references to plans that were not created: $registeredPlanProvenanceCount")
$reportLines.Add("Release version: $appVersion")
$reportLines.Add('')
$reportLines.Add('## Status counts')
$reportLines.Add('')
$reportLines.Add('| Status | Count |')
$reportLines.Add('|---|---:|')
foreach ($status in $allowedStatuses) {
  $count = @($entries | Where-Object { $_.status -ceq $status }).Count
  $reportLines.Add("| $status | $count |")
}
$reportLines.Add('')
$reportLines.Add('## Follow-up entries')
$reportLines.Add('')
$followUp = @($entries | Where-Object {
    $_.status -eq 'disconnected' -or $_.status -eq 'obsolete' -or $_.status -eq 'blocked-external'
  } | Sort-Object { [string]$_.id })
if ($followUp.Count -eq 0) {
  $reportLines.Add('None.')
} else {
  $reportLines.Add('| ID | Status | Owner plan |')
  $reportLines.Add('|---|---|---|')
  foreach ($entry in $followUp) {
    $reportId = ([string]$entry.id) -replace '\|', '\|'
    $reportStatus = ([string]$entry.status) -replace '\|', '\|'
    $reportOwner = (([string]$entry.ownerPlan) -replace '\r?\n', ' ') -replace '\|', '\|'
    $reportLines.Add("| $reportId | $reportStatus | $reportOwner |")
  }
}

$reportDirectory = [IO.Path]::GetDirectoryName($ReportPath)
if ([string]::IsNullOrWhiteSpace($reportDirectory)) {
  throw "ReportPath '$ReportPath' has no parent directory."
}
[IO.Directory]::CreateDirectory($reportDirectory) | Out-Null
$utf8WithoutBom = New-Object Text.UTF8Encoding($false)
[IO.File]::WriteAllText($ReportPath, ($reportLines -join [Environment]::NewLine) + [Environment]::NewLine,
  $utf8WithoutBom)

Write-Host "Sift feature audit validated: $($entries.Count) entries."
