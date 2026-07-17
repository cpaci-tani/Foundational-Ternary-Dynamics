# Sift changelog

## Unreleased

### Release safeguard audit
- Audited product code for testing-only trust/mutation bypasses; none found — signature, elevation, and guard rails remain fail-closed.
- Removed the boot-era blanket `UnhandledException` swallow so public builds fail closed after logging to Activity, Serilog, and `winui-startup.log`.
- Reworded “unsigned development build” copy to trusted-signature language in Core, README, and security docs.
- Recorded the audit in `docs/audits/AUDIT_RELEASE_SAFEGUARDS.md`.

### Wave 2 measured telemetry
- Added disposable `PdhSystemSampler` for system CPU and physical-disk PDH counters; Performance opens it on Activate and disposes on Deactivate; Dashboard/MonitorHost own a process-lifetime query disposed with telemetry.
- Extended `SystemSnapshot` with optional `SystemCountersSnapshot`; Performance prefers PDH CPU/disk when primed.
- Enriched battery metrics via `BatteryReportReader` (power status + AggregateBattery capacity/rate/status) while keeping the no-battery short-circuit.
- Slowed Performance sampling to at least 10 seconds under Battery Saver.
- Added `Sift.Benchmarks` (BenchmarkDotNet) plus `scripts/run-benchmarks.ps1` and `scripts/measure-pdh-overhead.ps1` for the ≤15% median overhead gate.

### Wave 1 library foundation
- Added CsWin32-generated Win32 bindings for process I/O counters, WoW64 detection, and icon extraction (replacing hand-rolled `DllImport`s in those call sites).
- Added presentation-neutral `SiftResult` / `SiftResult<T>` and wired catalog-recipe elevation resolve through the typed API while keeping `TryResolve*` compatibility overloads.
- Centralized byte and relative-time formatting in `SiftDisplay` (Humanizer) for Storage, Installed Apps, Maintenance, and Home recovery widgets.
- Adopted CommunityToolkit SettingsExpander/SettingsCard on the Settings workspace without changing preference semantics.
- Added local-only Serilog file logging (`ISiftLog` → `%LOCALAPPDATA%\Sift\logs`) owned by `WinUiAppServices` disposal; set `SIFT_LOG_VERBOSE=1` for Debug level. No network sinks.
- Added `apps/nuget.config` so restores resolve nuget.org (not only the Visual Studio offline cache).

### Architecture audit hardening
- Split replaceable reads from confirmed mutations: confirmed Optimize, Maintenance, Task Manager, Recovery, Storage, Installed Apps, and Home actions now use a committed execution path that ignores navigation/view cancellation after the final confirmation and records a persistent terminal result.
- Pinned PowerShell, PowerCfg, DISM, SFC, and SchTasks to exact System32 paths with tokenized arguments, minimal child environments, bounded concurrent output drains, explicit timeouts, and process-tree termination. Process restart is blocked while Sift itself is elevated.
- Kept elevation request and restore inputs leased against mutation through helper completion, required matching trusted app/helper signatures, moved protected backup output back to the initiating user's Sift data root, and added an administrator-desktop typed operation confirmation. Unsigned builds remain read-only for protected machine changes.
- Made MonitorHost freshness and ownership explicit across host restarts, stale/incompatible samples, and in-process fallback. History and alerts consume changed metrics once; SQLite requeues failed flushes, versions schemas, bounds lock waits, prunes alerts physically, and quarantines corrupt stores.
- Required matching trusted app/monitor signatures for launch and current-user startup registration, preserved exact ownership when removing the Run value, and bounded every IPC connection's read/write lifetime.
- Stabilized Home, Performance, and Hardware Monitor refreshes by preserving control, row, series, and sensor-history identity; only changed values and chart points update. UI scaling and Compact one/two-column transitions now remeasure without blank space or clipping.

### Customizable lifecycle dashboard
- Replaced the static Home overview with independent Wide/Medium/Compact dashboard profiles, a custom span-aware WinUI panel, compositor drag/resize, keyboard editing, undo/redo, hide/duplicate/reset, import/export, and four shipped resettable profiles.
- Added identity-stable LiveCharts2 widgets and in-place list/value reconciliation, typed telemetry cadences, Battery Saver/sleep handling, SQLite 90-day rollups, editable lifecycle alerts, quiet hours, acknowledgement/snooze, and explicit history clearing.
- Added guarded Home actions through the existing Optimize, Maintenance, process, service, confirmation, revalidation, backup, and UAC workflows; destructive Recovery, Storage, and Installed Apps operations remain deep links.
- Added the optional as-invoker `Sift.MonitorHost`, current-user typed named-pipe IPC, tray controls, opt-in hardware sensors/notifications, disabled-by-default MSIX startup task, exact HKCU folder-deployment startup entry, and release/package validation.

### Product language and permission audit
- Added a chunked, evidence-backed audit covering customer copy, confirmations, Windows permission requests, Core execution ownership, composition, lifecycle, validation, and maintained documentation.
- Removed shell marketing badges and the persistent protection slogan; retained a single neutral Standard user/Administrator session indicator.
- Rewrote primary workspace subtitles around capabilities, changed primary buttons from “Review and …” to their actual actions, renamed visible “Safe” labels to “Standard” or “Recommended,” and replaced the Settings safety section with direct restore-point controls.
- Preserved consequence, privacy, and administrator-permission copy for actions where it materially affects the user.
- Prevented passive Hardware Monitor activation from opening LibreHardwareMonitor's Ring0 driver in administrator sessions. Driver-backed monitoring now requires a separate explicit design instead of being an implicit workspace side effect.
- Removed Optimize's duplicate “restore latest” mutation path. Its backup button now opens Recovery, which remains the sole owner of explicit backup selection, inspection, confirmation, reinspection, administrator permission, and restore execution.
- Corrected Optimize mutation coordination so one outer operation owns the machine and current-user phases, preserves Windows permission cancellation distinctly, publishes phase logs once, and honors cancellation between current-user actions.
- Bound Maintenance cleanup to a ten-minute, one-use Core review ticket containing the selected targets' live registry or filesystem-content identity. Any changed target consumes the ticket and leaves the complete selection untouched. Standard-user scans no longer expose machine-wide orphan registrations, and Core independently rejects caller-constructed HKLM cleanup without administrator permission.
- Bound Task Manager process actions to PID plus process start time, session, name, and executable path so PID reuse or stale selection cannot target a replacement process. Service Start/Restart now carries the reviewed Stopped/Running state through Core and the typed elevation request; state drift is rejected instead of turning Restart into Start.
- Reworked the README as a customer guide and moved permission, enforcement, build, packaging, and release-acceptance detail into maintained technical documents.
- Reconciled the historical program index and validation baseline with their provenance, aligned the application and elevation-helper versions, and hardened the feature-audit validator to resolve source, plan, roadmap, and visual-evidence references.
- Extracted workspace construction and module ownership from `MainWindow` into `WorkspaceRegistryFactory` and `WorkspaceRegistry`; the shell now receives narrow shell services plus the registry factory and rejects route/registry drift at startup.

### Hardware monitor
- Added a dedicated, read-only Hardware Monitor workspace with live CPU, GPU, memory, motherboard, storage, network, temperature, fan, voltage, clock, load, and power sensors.
- Added a presentation-neutral `IHardwareSensorProvider` extension seam and normalized device/sensor snapshots; provider failures and cleanup faults are isolated so optional sensor backends cannot take down the workspace.
- Integrated local LibreHardwareMonitor 0.9.6 discovery with no runtime downloads, plus device and sensor-type filtering, collapsible hardware groups, summary metrics, min/max values, copyable readings, and a bounded 180-sample LiveCharts2 history for the selected sensor.
- Sampling runs off the UI thread only while the workspace is active, supports pause/manual refresh, cancels during navigation, and reports standard-user or partial provider availability without requesting elevation.
- Added deterministic provider aggregation/failure/cleanup tests and native UI automation covering discovery, filtering, chart presence, expansion, and pause/resume lifecycle.
- Stabilized live refreshes with identity-keyed observable device and sensor rows. Routine samples now update only changed reading labels, summary values, provider/status text, and the selected chart series; they no longer rebuild expanders, filter options, selections, or scroll state, and background ticks no longer flash the busy indicator. Monitor inventory and chart panels now stretch to consume their complete allocated space.

### Sift identity
- Renamed the complete application identity from its former name to Sift across the WinUI shell, Core, elevation host, tests, namespaces, assemblies, executables, projects, package identity, CI workflow, assets, release folders, scripts, documentation, accessibility text, and local Script Studio host.
- Moved the canonical per-user data root to `%LOCALAPPDATA%\Sift`. First launch safely migrates an existing normal legacy directory when Sift data does not yet exist; if both roots exist, only missing settings, activity, startup-log, and exact top-level backup files are imported, with no overwrite and no reparse-point traversal.
- Bumped the application version to 0.15.0 for the product-identity transition.

### Deterministic build output
- Every Sift project now removes its exact configuration/runtime directory below that project's `bin` folder before compilation. The cleaner refuses any path outside the owning project's `bin` directory.
- Release publishing now replaces the complete output folder on every run. Non-empty custom output directories require a Sift-owned marker before they can be cleaned, preventing an accidental recursive delete of unrelated files.
- Validation plants stale probes in both the application and elevation-host outputs and fails unless the next build removes them before copying the helper payload.

### Script Studio
- Evolved Command Center into a two-tab Script Studio while preserving the guarded library of more than 100 categorized exact-catalog commands and its Standard User/Administrator visibility rules.
- Added locally bundled Monaco 0.55.1 and xterm.js 6.0.0 assets with no CDN or runtime download, a graphite/clay/sage IDE surface, Problems navigation, selection/copy/clear/Open-in-Explorer terminal actions, and bounded terminal scrollback.
- Added presentation-neutral PowerShell, Python, Bash, CMD, JavaScript, and TypeScript document/runtime/diagnostic models plus finite exact-path and registered-Python runtime discovery without profile or drive sweeps. Core canonicalizes runtime IDs, rejects language substitution and reparse paths, requires local Windows signature trust, and revalidates immediately before launch.
- Added non-executing syntax adapters for PowerShell parser APIs, isolated Python AST parsing, `bash -n`, and `node --check`, with sanitized environments, bounded output, cancellation from document retrieval onward, analyzer timeouts spanning input/output, and verified process-tree termination.
- Added a shared Sift policy analyzer for opaque execution, remote retrieval, package installation, and state-changing APIs. Static analysis never enables execution; arbitrary authored documents remain blocked until they can produce a typed, reviewable Core plan.
- Locked the WebView2 bridge to a local HTTPS virtual host, denied navigation/new windows/permissions/downloads, disabled host-object access and GPU composition, sanitized terminal control characters, suspended browser workers while inactive, and restricted messages to document, diagnostic, clipboard, and fixed Explorer actions.
- Removed the authored Studio tab in elevated sessions and duplicated that guard in Core so per-user runtimes can never inherit an administrator token. The elevated exact-catalog command library remains available.
- Added a deterministic source/output hash manifest that every normal .NET build verifies, preventing stale or modified Monaco/xterm bundles from shipping silently.
- Added deterministic Core tests and native UI automation for local runtime discovery, no-execute parsing, dangerous-language diagnostics, library confirmation cancellation, editor initialization, and in-memory analysis.

### Command Center
- Added a searchable Command Center workspace with more than 100 bundled local CMD, PowerShell, and WSL/Bash troubleshooting recipes.
- Added Core-owned exact-catalog validation, blocked remote/destructive command tokens, automatic non-mutating preflight, immediate revalidation, cancellation, and streamed stdout/stderr.
- Added explicit read-only, state-changing, and advanced risk labels plus unit and native UI coverage.
- Split commands into Standard User and Administrator sections; the Administrator section is not created in a standard-user session.
- Pinned shell hosts to trusted Windows paths, sanitized child `PATH`/module lookup and working directory, authenticated complete recipe metadata, and verified process-tree exit on cancellation.
- Replaced obsolete WMIC recipes, corrected BitLocker/audit-policy administrator metadata and localized Administrators-group lookup, restricted DISM repair to local sources, and removed the WinGet source-agreement recipe.
- Added recipe-specific network/privacy evidence, secret-output warnings, bounded batched terminal rendering, accessible live status, and correct cancellation activity records.
- Formalized all recipes into 19 ordered troubleshooting categories and rendered each access section as visible category groups with descriptions and command counts.

### Disconnected capability wiring
- Wired typed Office scheduled-task enable/disable through `ScheduledTaskActionService`, elevation broker, Task Manager confirmation, and immediate state/hash revalidation.
- Wired best-effort System Restore preflight for eligible Optimize batches through `SystemRestorePointService` and `OptimizeMutationWorkflow`, including `Continue without restore point?` continuation when creation fails or UAC is cancelled.
- Promoted Health from overview cards to a dedicated checks/history workspace with bounded `HistoryService` aggregation, partial-source warnings, and latest-wins orchestration.
- Maintenance scan completion now flushes `LastMaintenanceScanUtc` immediately through `SettingsPersistenceCoordinator.SaveNow`.
- Wired the persisted `ChartSmoothing` preference to the live Performance charts through a Core `ChartSmoothingPolicy` and a new Settings line-smoothing control (None/Light/Medium/High); the CPU and memory history lines now honor the saved curvature instead of a hardcoded value.
- Removed obsolete `AppSettings` legacy fields, deleted unused `TelemetryHub`, and made `StorageDeleter` an internal shared instance only.

### Optimize Control Panel grid
- Restyled Optimize as a flat multi-column category picker (no per-category cards): checkbox + larger body labels, row hover states, and rich tooltips (description, risk/kind/state, undo, elevation, exact action target, catalog ID). Risk filter replaces the old category combo; presets and Review/apply unchanged.
- Expanded the allowlisted catalog with a Tron-inspired Advanced pack: additional consumer Appx removals, a few Moderate privacy policies, and elevated one-shot Repair jobs (`DISM` component cleanup, `sfc /scannow`). No Tron binaries, downloads, AV scanners, or Defender/Update/firewall/Store disable. Minimal/Balanced still exclude Advanced. Repair commands cross the one-shot elevation helper by exact catalog ID only.

### System information pane
- Replaced collapsible category expanders with an always-visible specs sheet: About snapshot, filters, cozy section headers, and two-column property rows (label + wrapping value) in one scrollable pane, plus selected-property detail for copy.

### Typography formalization
- Added a Sift type scale in `App.xaml` (`TypeWorkspaceTitleStyle`, `TypeSectionTitleStyle`, `TypeBodyStyle`, `TypeMetaStyle`, `TypeMetricValueStyle`, `TypePanelTitleStyle`, and related keys). Existing Workspace/Section/Eyebrow styles alias the scale. Shell chrome and KPI cards on Performance, Task Manager, Recovery, Storage, and Installed Apps now use the formal styles.
- Added a Settings **UI size** preference (`Compact` / `Default` / `Large`) persisted as `UiScale`, applied as a uniform scale on shell content. Optimize picker rows show the setting name only (full detail remains in the hover tooltip).

### Release tooling
- Added `REBUILD.bat` next to the published `Sift.exe`. Each `build-release.ps1` run copies it into the output folder so a double-click rebuilds that release in place (close Sift first if it is running from the folder).

### Table selection & app icons
- Switched every dense inventory table (processes, services, scheduled tasks, recovery backups, system information, installed apps, startup) to full-row selection (`SelectionUnit="Row"`) so clicking selects the whole row and its target instead of a single cell.
- Added a per-app icon column to Installed Apps. Icons are extracted in Core from each entry's registered `DisplayIcon` reference (`AppIconExtractor` — verbatim path plus optional index, existence-checked, no directory sweeps) and rendered through a `PngToImageSourceConverter`.
- Added the same icon column to Task Manager processes, services, and scheduled tasks where an exact executable path is available (process module path, service ImagePath, or task "Task To Run"); missing/protected paths leave a blank cell. Extraction is cached by path to keep inventory refresh cheap.
- Extended app icons to Startup (Run keys / Startup folders) and Performance top CPU/memory consumers so every tab that lists applications shows icons when a path can be resolved.
- Defaulted the Installed Apps inventory to uninstallable-first ordering (then leftover registrations, then name) so actionable apps and their guarded single-app operations surface at the top; header-click sorting still overrides the default.

### Confirmation workflow
- Removed the Optimize, Maintenance, Installed Apps, and app-leftover preview toggles and their persisted default settings. Every mutation now enters mandatory automatic non-mutating preflight rather than a user-selectable mode.
- Standardized mutations on automatic Core preflight, a reviewed confirmation dialog, and live Core revalidation immediately before confirmed execution.
- Standardized every confirmation dialog on Sift's warm clay/graphite button styles, removed the system accent as an implicit default, and gave multi-line preflight evidence explicit bordered space instead of collapsing it into one line.
- Added native dialog automation for Optimize, Task Manager, Maintenance, Storage, uninstall handoff, leftover-registration cleanup, and file-leftover cleanup; cancellation is asserted to leave fixtures unchanged.

### Task Manager
- Promoted Task Manager from read-only inventory to guarded single-selection controls: End task, Restart app, Start service, and Restart service. Every action runs automatic Core policy review, explicit confirmation, and live identity/path/registration revalidation.
- Added independently collapsible Process, Service, and Scheduled Task inventories with native keyboard/automation support, selected-target summaries, disabled-state guidance, and compact action bars.
- Hardened process actions to require the current interactive session, start-time identity, and a readable executable path while blocking cross-user/session-0 targets, Sift, named critical processes, PID 0–4, and every executable under the Windows or Sift directories. Execution rechecks the live PID, start time, session, name, and full path immediately before ending the process tree.
- Added a nonce-bound service operation to `Sift.ElevationHost`. It accepts only an exact service name, typed `Start`/`Restart` action, and matching reviewed Stopped/Running state; independently re-enumerates the service; rejects Windows/protected/disabled/missing registrations and state drift; and never elevates the WinUI shell.
- Added deterministic Core/elevation policy coverage and a harmless copied-process native fixture proving selection, collapse/expand, confirmation, and cancellation without terminating user software.

### Storage
- Added native Windows folder picking while preserving the editable explicit-root field and scan-only-on-command behavior.
- Added exact selected-child cleanup through the Recycle Bin. Core completely inventories the selected file/folder, compares it with the current map, issues a bounded five-minute one-use ticket, and repeats the complete live inventory after confirmation before any shell operation.
- Blocked scanned-root deletion, path escape, protected targets, selected or nested reparse points, incomplete/inaccessible trees, stale maps, changed confirmations, and inventories above the two-million-entry safety bound.
- Replaced pointer-only treemap tiles with native keyboard-focusable buttons while retaining the warm extension palette, hover outlines, full-path automation help, and rich tooltips.
- Removed the dormant permanent-delete mode from the Core storage deletion contract. Storage and app-leftover cleanup can request only Recycle Bin operations by construction.

### Scoped elevation
- Added a separate `Sift.ElevationHost` process with a `requireAdministrator` manifest while keeping the WinUI application `asInvoker`; Sift never elevates its full shell.
- Added a nonce-bound one-shot request/response broker for standard-user HKLM Optimize work. The helper independently resolves bounded tweak IDs against `TweakCatalog` and accepts no raw command, registry path/value, script, or arbitrary filesystem payload.
- Split mixed Optimize batches so UAC-protected machine changes run first and per-user changes do not run when administrator confirmation is cancelled or the elevated phase is rejected.
- Added exact request-directory, GUID-name, size, and reparse-point validation, deterministic allowlist/contract tests, canonical-source checks, and release publishing for the self-contained helper.
- Extended the same one-shot helper to protected Recovery entries. Requests carry only an exact sibling backup filename; the helper independently revalidates machine identity, schema, entry bounds, allowlisted tweak IDs, bounded prior DWORD values, and the exact hibernation undo action. UAC cancellation stops before any current-user restore.

### Recovery
- Added a dedicated twelfth Recovery workspace with bordered summary cards, search/status filtering, a native backup table, selected-backup details, hover guidance, exact-path folder access, and explicit ready/protected/blocked states.
- Added automatic recovery preflight and a reviewed confirmation dialog for the exact selected backup. The file is re-read before confirmation and again before execution; nothing is restored on selection or cancellation.
- Added bounded Core backup inventory and restore orchestration with file-size/schema/entry limits, top-level path and reparse checks, cross-machine blocking, current-user/protected classification, protected-phase-first execution, persisted activity, and deterministic forged-input/cancellation tests.
- Kept HKLM uninstall-tree restoration blocked across the elevation boundary until backups have a privileged integrity envelope; the UI surfaces this as a concrete blocked reason rather than accepting an untrusted privileged payload.

### Packaging foundation
- Added a desktop full-trust MSIX manifest with native Sift package assets and a deterministic `build-msix.ps1` pipeline.
- Added explicit unsigned layout validation through MakeAppx and a signed-release path that requires a matching trusted certificate subject, signs and verifies both executables before packing, then signs and verifies the MSIX.
- Kept signing claims honest: the current local package is marked unsigned and is not treated as clean-account release evidence until a trusted publisher certificate is supplied and every SignTool verification succeeds.
- Added a clean-account release acceptance script that refuses unsigned/untrusted packages and existing installs, verifies installed app/helper signatures, performs install, non-mutating UAC/nonce IPC (including over-the-shoulder administrator credentials), packaged launch, uninstall, and final registration checks.

### System Information
- Added an eleventh dedicated System Information workspace with typed summary cards and a detailed read-only property inventory across Windows, security, processor, memory, firmware, graphics/displays, physical and logical storage, active network adapters/configuration, battery, and audio devices.
- Added finite exact-provider collection in Core with progress, cancellation boundaries, partial results, optional-provider warnings, unit normalization, and no shell-script or broad filesystem/registry sweep.
- Added full-text search, category filtering, full-value row inspection, copy-selected and copy-visible reports, explicit serial/MAC/IP sharing guidance, `msinfo32` handoff, Ctrl+I navigation, and structured activity events.
- Added deterministic parser/cancellation tests, real Windows integration assertions, eleventh-route source checks, native control automation, and System Information screenshot coverage.

### Installed Apps
- Added a dedicated Installed Apps workspace with standard uninstall-registry inventory, an explicit Uninstallable column, search, policy filtering, selected-app details, and a Windows Installed Apps deep link for Store/MSIX packages.
- Added selected-only uninstaller trust details: local-only `WinVerifyTrust` validation for embedded and Windows catalog signatures, signer certificate and locally available chain status, registered-publisher resemblance, file version, certificate validity/thumbprint, and SHA-256. MSI product-code entries explicitly verify only the Windows Installer host and never misattribute that host signature to the product publisher.
- Fixed uninstall handoff being trapped in visible preview mode. A valid selected desktop entry now runs automatic preflight, asks for confirmation, revalidates the exact registry identity and command, and launches the registered interactive uninstaller.
- Added bounded handle-backed uninstall sessions. Sift now observes the registered process without blocking the UI, refreshes automatically when it exits, and offers a manual status recheck for vendor launchers that delegate to another process.
- Closed the early-leftover-authorization gap: opening or closing an uninstaller no longer creates a cleanup token. Core authorizes exact AppData review only after the original registry identity is confirmed absent; unchanged registrations remain locked and changed/replaced registrations are blocked.
- Added Core exact-entry revalidation and guarded command parsing. Script hosts, URI commands, silent launches, stale/forged commands, system components, updates, drivers, runtimes, and security software remain blocked.
- Kept uninstall scope to one explicitly selected desktop app and the vendor's interactive uninstaller; there is no bulk, silent, or automatic-undo path.
- Added conservative cleanup for leftover uninstall registrations. Candidates require both a missing explicit install directory and a missing non-MSI uninstaller; protected publishers/components are excluded, automatic preflight is required, exact values are revalidated, and a typed full-tree backup is written before removing only the registry record.
- Hardened the Maintenance orphan-registration path to use the same two-signal evidence and scan-time value revalidation instead of treating a missing install folder alone as sufficient.
- Added app file-leftover review after a verified orphan or a successful same-session uninstall/registration-cleanup action. Discovery uses finite exact top-level Local, Roaming, and LocalLow AppData paths; nothing is preselected, and execution is Recycle Bin only after automatic preflight, confirmation, and Core revalidation of the app, authorization, path, and complete tree.
- Retired the Maintenance workspace's broad AppData enumeration and token-overlap matching. Legacy `AppLeftover` findings are rejected by Core so the selected-app workflow cannot be bypassed.
- Added deterministic tests for exact-vs-near-match discovery, active-registration blocking, expiring continuation tokens, forged paths, exact registered-uninstaller launch handoff, exit-before-removal locking, verified-removal authorization, delegated-uninstaller rechecks, Recycle Bin-only execution, and legacy-policy bypass; native automation covers preflight non-mutation and each confirmation without deleting or uninstalling the fixture.
- Added Ctrl+9 for Installed Apps and Ctrl+0 for Settings, plus Core/source/UI validation coverage for the Installed Apps route.

## 0.12.0 — 2026-07-13

### Architecture and safety
- Added typed, atomic registry snapshots, crash-aware operation journals, unique backup names, exact allowlist validation, and restoration of orphan-uninstall registry trees.
- Added maintenance path policy enforcement, conservative third-party service classification, exact scheduled-task allowlists, live PID/path revalidation, latest-wins cancellation checks, and serialized process sampling.
- Added deterministic coverage for typed registry restoration, backup collisions, crash recovery boundaries, path rejection, service/task spoofing, and non-cooperative cancellation.
- Moved default service construction to the application composition boundary and connected unhandled WinUI exceptions to structured local activity.

### Native UI
- Promoted Optimize and Maintenance from passive overviews to dedicated preview-first selection, confirmation, execution, log, backup, and restore modules.
- Promoted Task Manager to dedicated Processes, Services, and Scheduled tasks inventories with Core policy classifications.
- Restored Storage as a dedicated WinDirStat-style vertical slice with explicit-root scanning, progress/cancel, tested squarified layout, bounded recursive tiles, drill-down, largest-child details, and extension legend.
- Fixed shell content measurement so workspaces fill available width/height, added DPI-aware 1100×720 window minimums, and added rich hover tooltips/outlines to Storage tiles and controls.
- Added a native Settings workspace that controls console visibility/width and Performance cadence/history.
- Expanded the activity console with search, severity filtering, copy, clear, auto-follow, bounded retention, narrow overlay behavior, and explicit empty state.
- Added loading, error, empty, and filtered-empty states to shared inventories; standardized graphite/clay/sage tokens, borders, type scale, spacing, and controls.
- Replaced the mint logo emphasis with warm clay and removed the remaining cyan-adjacent branding.

### Delivery
- Expanded native UI traversal and screenshots to all nine routes.
- Kept the double-click `dist/BUILD-LATEST.bat` release builder and self-contained publish verification aligned with the canonical WinUI project.

## 0.11.0 — 2026-07-13

### Native application reset
- Made the Windows App SDK 2.2 / WinUI 3 project the sole canonical `apps/Sift/Sift.csproj` application.
- Removed the legacy WPF project, views, controls, dialogs, controllers, OxyPlot dependency, WPF sampler, visual harness, and stale release outputs.
- Moved presentation-neutral models, infrastructure, scanners, guarded actions, persistence, and process sampling physically into `Sift.Core`.
- Added a native custom-title-bar shell, all eight workspace routes, live system-backed modules, responsive navigation, and a structured activity side console.
- Completed native Startup and LiveCharts2 Performance vertical slices.
- Added Ctrl+1…Ctrl+8 navigation, Ctrl+F focus routing, persisted last-workspace restoration, and responsive console state.
- Standardized the graphite, clay, and sage visual system with explicit borders, spacing, typography, and alignment.
- Rewired release, integration, unit, CI, logo-generation, and native UI-smoke scripts around the canonical WinUI app.
- Replaced the migration document with `ROADMAP.md`, a native workspace completion ledger.

### Architecture
- Added a central `WinUiAppServices` composition root and injectable inventory/scanner/action interfaces.
- Added structured `ActivityEvent` observability with live-console and persisted-history sinks.
- Added keyed background-operation coordination with cancellation, elapsed timing, and explicit dispatcher handoff.
- Added snapshot-based debounced settings persistence and atomic JSON replacement.
- Added `IWorkspaceModule` activation, deactivation, refresh, focus, and disposal lifecycle boundaries.
- Moved process, service, and scheduled-task protection checks into a guarded application service below the UI layer.
- Added deterministic xUnit v3 policy tests, non-mutating Core integration validation, and native executable screenshot traversal.

### UX
- Activity console docks on wide windows and collapses at narrow widths.
- Console visibility and dock width persist across sessions.

## 0.10.0 — 2026-07-13

### Features
- **Health** workspace with **Checks** and **History** tabs.
- **Checks:** read-only diagnostics — disk free space, pending reboot, memory pressure, stopped Automatic services (manageable only), Windows Update service status, recent System log errors (24h), WMI disk health. Each row explains what was found and links to the right Windows/Sift screen; nothing auto-fixes.
- **History:** unified timeline of Optimize registry backups, orphan-uninstall registry snapshots, and persisted activity (`activity.json`) from Optimize, Maintenance, Storage, Task Manager, Settings, and Health actions.
- Restore Optimize backups from History with **two confirmation dialogs** (same rollback engine as sidebar Restore).
- Home **Activity** widget links to Health → History.

### UX
- Ctrl+7 Health · Ctrl+8 Storage (Storage moved from Ctrl+7).
- Settings last-workspace list includes Health.
- Home **Top CPU / Top Memory** lists jump to Task Manager on a process name; card footer links still open the workspace without stealing list clicks.
- Home **Services** card opens Task Manager → Services tab.

### Reliability
- Fixed crash when selecting rows on Task Manager → **Scheduled tasks** (grouped DataGrid binding + selection handling).
- Scheduled-task list loads via `schtasks` on a background thread; stale refreshes are dropped and selection is preserved across reloads.
- Services/Tasks bulk actions and selection handlers use safe `.OfType<>()` filtering instead of `.Cast<>()` on mixed selections.
- Group headers bind `{Binding Path=Name, Mode=OneWay}` to avoid WPF `GroupItem` / `FrameworkElement.Name` ambiguity; row virtualization disabled on grouped Services/Tasks grids.

## 0.9.0 — 2026-07-13

### Features
- **Home** dashboard (default landing): toggleable widgets for CPU/memory/disk sparklines, top processes, services summary, startup/storage/maintenance/optimize/activity cards. Customize via in-view checklist.
- **Task Manager** tabs: Processes | Services | Scheduled tasks with collapsible groups, multi-select, and bulk actions.
- Services: Start / Stop / Restart for non-critical services; hard-blocked Defender, Windows Update, firewall, RPC, and related protected names.
- Scheduled tasks: enable/disable only for a small OEM/updater allowlist; everything else is read-only with Open in Task Scheduler.
- Process Restart (single + bulk) when an executable path is known; End selected keeps existing protected-process guards.
- **OxyPlot** charts on Home + Performance with separate sample interval vs chart FPS (default 30), history length, and Light/Medium/Off smoothing.

### UX
- Ctrl+1 Home; Optimize→2 … Storage→7.
- Settings: telemetry sample interval, chart FPS/history/smoothing, workspace restore includes Home.

## 0.8.0 — 2026-07-13

### Features
- **Storage** workspace: WinDirStat-style disk analyzer with parallel `FindFirstFileEx` scanning, squarified treemap, folder drill-down, extension legend, and guarded delete.
- Delete toolbar toggle: **Move to Recycle Bin** (default) vs permanent (extra confirm). Drive roots, Windows paths, Sift files, and reparse points are blocked.
- Default scan roots: all fixed local drives; add folder / reset drives; Cancel during scan.
- Ctrl+6 opens Storage.

### Performance
- Parallel directory walk with long-path support and batched progress updates.
- Elevated NTFS USN journal probe (falls back to parallel walk when full MFT rebuild is unavailable).

## 0.7.0 — 2026-07-13

### UX
- Maintenance has its own activity log (no longer shares Optimize’s log).
- Honest cleanup summaries: cleaned / skipped / failed — locked files are never reported as cleaned.
- Optimize risk filter (All / Safe / Moderate / Advanced) plus select/clear visible rows.
- Settings dialog: restore-point offer, default preview modes, last-workspace restore, last Maintenance scan timestamp.
- Admin hint + **Relaunch elevated** when Delivery Optimization or Prefetch scans need elevation.
- Keyboard: Ctrl+5 opens Maintenance; Startup search via Ctrl+F when that workspace is active.

### Features
- Startup inventory expanded: RunOnce, WOW6432Node Run/RunOnce, Common Startup folder, StartupApproved Enabled/Disabled status (read-only); deep-link to Windows Startup settings.
- Maintenance scans: thumbnail cache, WER ReportQueue, user crash dumps, Prefetch (report-by-default; clean needs Advanced + elevated + confirm), stronger leftover matching with High/Medium confidence.
- Conservative **orphan uninstall** registry hygiene: uninstall keys with a missing InstallLocation folder, excluding SystemComponent and Microsoft/Windows publishers; JSON backup before delete.
- `power.hibernate` status detected via `powercfg /a` (ACTIVE when hibernation is off).
- Retired duplicate Optimize action `maintenance.temp` — use the Maintenance workspace instead.
- ProcessMonitor warm-up sample and path-stable icon reuse; elevation relaunch restores pending Optimize selections.

### Release
- Version footer shows major.minor.patch (`0.7.0`).

## 0.6.1 — 2026-07-13

### UX
- Optimize catalog is now a sortable table (Setting, Category, Risk, Status, Description).

### Features
- **Maintenance** workspace: scan temp folders, Recycle Bin, Delivery Optimization cache (admin), and AppData folders left after uninstalls; preview-first cleanup with user selection.

## 0.6.0 — 2026-07-13

### UX
- In-app activity log for preview/apply/restore (MessageBox kept for confirmations only)
- Version footer bound from assembly metadata
- Persisted Task Manager columns, filters, refresh interval, and Optimize category to `%LOCALAPPDATA%\Sift\settings.json`
- Empty states for filtered tweak and process lists
- Keyboard shortcuts: Ctrl+1/2/3/4 workspaces, Ctrl+F search, Ctrl+P preview toggle

### Reliability
- Transactional apply: backup JSON written before mutations and rewritten after each success/failure
- Structured apply/restore results with partial-failure restore prompt
- Backup picker dialog (restore any backup, not only latest)
- Optional System Restore point prompt before irreversible or HKLM batches
- GitHub Actions workflow `sift.yml` runs the validation harness on Windows

### Architecture
- Optimize / Task Manager / Performance / Startup split into UserControls under `Views/`
- Service interfaces: `ISettingsStore`, `ITweakExecutor`, `IProcessSampler`
- Manifest elevation changed to `asInvoker`; HKLM selections prompt to relaunch elevated
- Process icon cache capped (LRU 500)

### Features
- Read-only Startup apps workspace (Run keys + Startup folder)
- Export visible process list to CSV
- Click Performance bar charts to select/filter in Task Manager
- Chart hover tooltips; independent pause/resume sampling on Performance
- Catalog: silent-installed apps and preinstalled-app suggestion toggles; clarified temp cleanup scope

### Release
- `build-release.ps1 -Versioned` publishes to `dist/Sift-<version>/`
- Accessibility names on navigation and key controls
- Code signing remains optional/external (see AGENTS.md)

## 0.5.0

Initial public control-center release with Optimize, Task Manager, and Performance workspaces.
