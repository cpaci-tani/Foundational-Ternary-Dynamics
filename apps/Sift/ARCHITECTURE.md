# Sift architecture

Sift is a WinUI 3 application over a presentation-neutral Windows policy and system-management core. There is one desktop client and one policy implementation.

Customer capabilities and setup are documented in `README.md`. Build and release procedures live in `docs/BUILD_AND_RELEASE.md`; the permission contract is summarized in `docs/SECURITY_AND_PERMISSIONS.md`. This document owns runtime and enforcement detail.

## Runtime layers

```text
App
  ├─ owns WinUiAppServices for the process/window lifetime
  └─ creates the workspace factory and shell-facing service record

MainWindow
  ├─ native title bar, NavigationView, keyboard routing, responsive console
  └─ resolves, activates, and deactivates registered workspaces

WorkspaceRegistryFactory / WorkspaceRegistry
  ├─ construct the complete typed workspace set in one composition location
  ├─ reject duplicate keys and forward shell-setting changes
  └─ own module event detachment and disposal

Workspace modules (IWorkspaceModule)
  ├─ own one view, refresh lifecycle, cancellation, and page-specific state
  ├─ activate/deactivate background work explicitly
  └─ call narrow services from WinUiAppServices

Sift.Core
  ├─ models and service contracts
  ├─ ActivityHub and OperationCoordinator
  ├─ settings and atomic persistence
  ├─ scanners, inventories, preflight/revalidation/rollback execution
  ├─ process, service, task, storage, and registry policy
  └─ Services/ feature folders (Optimize, Elevation, Dashboard, Storage, Apps, Scripts, Inventory, Maintenance, Guards, Health, Recovery) — namespaces remain Sift.Services

Sift.ElevationHost
  ├─ separate short-lived console process with a requireAdministrator manifest
  ├─ accepts only validated per-user request files and typed operation IDs
  └─ writes a nonce-bound response, then exits; it never creates or elevates the WinUI shell

Sift.MonitorHost
  ├─ optional single-instance, as-invoker process scoped to the current user
  ├─ owns read-only sampling, SQLite rollups, alert evaluation, notifications, and tray controls
  └─ exposes only versioned current-user snapshot/preferences/pause IPC; it has no action or elevation message
```

`MainWindow` owns shell integration and workspace activation, but it does not construct individual modules or know their dependencies. Dedicated modules own feature orchestration and views own presentation.

`WinUiAppServices` exposes feature facades (`Dashboard`, `Optimize`, `Inventory`, `Mutations`, `Scripting`, `Desktop`, `Infrastructure`) consumed by `WorkspaceRegistryFactory`. Dashboard telemetry for the WinUI fallback and MonitorHost is constructed through `DashboardRuntimeFactory` so slow-sample wiring cannot drift. `WinUiAppServices.Dispose` owns only the process-lifetime `IDisposable` Core services listed in `OwnedDisposables` (Operations, HardwareMonitor, DashboardHistory, SettingsPersistence, Log); the registry owns module disposal.

The shell stretches hosted controls in both dimensions. `WindowMinimumSize` owns the DPI-aware `WM_GETMINMAXINFO` subclass and enforces a 1100×720 DIP floor without leaking Win32 code into workspace modules.

Optimize, Task Manager, Performance, Hardware Monitor, Startup, Maintenance, Script Studio, Health, Recovery, Storage, Installed Apps, System Information, and Settings are dedicated vertical slices. Home uses `HomeDashboardWorkspaceModule`, `DashboardGridPanel`, identity-stable widget hosts, and a presentation-neutral packing engine. It no longer uses the former static overview table.

Home profiles are versioned documents with independent six-, four-, and two-column layouts. Below 480 DIPs the view derives a one-column accessibility layout without mutating Compact. Pointer edits use capture and compositor translation; keyboard edits commit one placement after Space/arrows/Shift+arrows/Enter and announce row, column, and span. Collision placement pushes only overlapping widgets downward; Tidy is the only automatic row-major compaction operation. The view reconciles hosts by instance ID, so samples, moves, resizes, and chart points update existing controls instead of recreating the widget tree.

`DashboardActionRouter` accepts only typed catalog actions and calls the same production `OptimizeMutationWorkflow`, maintenance review tickets, guarded process/service actions, and elevation broker used by their full workspaces. Recovery restore, Storage deletion, and Installed Apps uninstall remain navigation-only from Home. Neither profiles, widget definitions, alerts, notifications, nor MonitorHost can introduce commands, executable paths, URIs, scripts, or elevation requests.

Script Studio is a dedicated two-path vertical slice. Its Command Library path resolves only complete canonical `ScriptRecipe` records and retains automatic Core review, immediate recipe/token revalidation, trusted shell paths, sanitized environments, bounded output, and verified cancellation. Read-only recipes run without a redundant confirmation dialog; `ChangesState` and `Advanced` recipes require confirmation. Administrator catalog recipes are visible to standard users and cross elevation only as typed `RunCatalogRecipe` requests carrying `RecipeId` plus `ExpectedRecipeHash`; the helper re-resolves the catalog record and never accepts command text. Already-elevated sessions keep the in-process run path. Its authored-document path is intentionally analysis-only and is absent from elevated sessions: `IScriptStudioService` owns finite runtime discovery plus non-executing syntax and policy diagnostics for PowerShell, Python, Bash, CMD, JavaScript, and TypeScript. Core re-discovers the selected runtime ID, checks its language/path/reparse status and local Windows signature trust, and repeats those checks immediately before the analyzer process launch. Static analysis never grants execution authority.

The Studio view hosts locally bundled Monaco and xterm.js assets through WebView2. The browser is mapped to one local HTTPS virtual host with remote connections prohibited by content security policy. Navigation, new windows, permissions, downloads, host objects, and generic native proxies are disabled. A closed JSON bridge carries only document requests, diagnostics, terminal output, bounded clipboard text, and the fixed System32 Explorer handoff. The WebView is initialized only while the Studio tab is selected, disposed when leaving that tab, and closed again during module disposal.

Customer-facing policy outcomes use stable `SiftReasonCode` values with English fallbacks in Core (`ReasonMessages`) and optional WinUI `.resw` presentation through `ReasonPresenter`. Typed Core outcomes use `SiftResult` / `SiftResult<T>` (for example catalog-recipe elevation resolve); full UI string migration is not required for this foundation.

Inventory and dashboard size/age copy goes through `SiftDisplay` (Humanizer) so WinUI views do not keep parallel byte formatters. Local diagnostic logging uses `ISiftLog` / `SiftFileLog` (Serilog file sink under `%LOCALAPPDATA%\Sift\logs`, no network sinks); it complements `ActivityHub` and is disposed with `WinUiAppServices`. Win32 interop for process I/O and icon extraction uses CsWin32-generated bindings (`NativeMethods.txt`). Settings uses CommunityToolkit `SettingsExpander` / `SettingsCard` while keeping the graphite/clay/sage palette.

The Script Studio view contract is split into responsibility-specific partials: `Library` owns filtering, category expansion, selection, hover copy, and quick run; `Terminal` owns bounded output presentation; `Studio` owns WebView2 initialization, suspension, and teardown; `Bridge` owns the closed JSON protocol and pending document requests; and `Layout` owns tab and responsive layout behavior. The primary code-behind retains only the public module-facing contract and state-changing confirmation. Clipboard and Explorer handoffs use the shared WinUI interop adapters injected by the workspace factory.

Task Manager keeps inventory presentation in `TaskManagerWorkspaceView` and action orchestration in `TaskManagerWorkspaceModule`. The view exposes only explicit single selections, collapsible native inventory panels, confirmations, and status. `IGuardedSystemActions` owns process/service policy and repeats live identity checks at execution. A process target carries PID, start-time identity, session, name, and executable path from the reviewed inventory row. A service target carries its exact name and reviewed Running/Stopped state. Standard-user service Start/Restart crosses the nonce-bound elevation boundary as a typed action plus that expected state; both the broker and helper independently reject registration or state drift before the low-level controller checks the state once more and acts.

Installed Apps uses `InstalledAppsWorkspaceModule` only as its lifetime boundary. `InstalledAppsInventoryController` owns inventory refresh, selected-uninstaller trust inspection, and the Windows Settings handoff; `InstalledAppUninstallController` owns confirmation, launch monitoring, completion checks, and the short-lived cleanup continuation; `InstalledAppLeftoverController` owns finite AppData scans and Recycle Bin execution. The shared `InstalledAppUninstallState` binds a continuation to the original registry identity and display name and revokes it when a new uninstall begins or verification fails. The view is split into inventory/selection, dialogs, and leftover-list partials and releases candidate change handlers when the module is disposed.

## Workspace lifecycle

Every module exposes:

- `ActivateAsync` — load its current state and start only the background work it owns;
- `RefreshAsync` — run a latest-wins operation through `OperationCoordinator`;
- `Deactivate` — stop timers and cancel obsolete work;
- `FocusPrimarySearch` — shell-level Ctrl+F routing;
- `Dispose` — detach events and release timers.

A module's sampling timer stops on deactivation unless a floating or embedded dock window explicitly retains its session: Hardware Monitor keeps sampling while `_dock.IsRetained` so detached live graphs stay current, and otherwise the active module is the only one sampling. Results are applied to WinUI-bound collections after the awaited operation returns to the UI context.

Performance and Hardware Monitor use a single UI-thread sampling gate around manual and periodic refreshes. A timer starts only after initial activation sampling completes, stops on deactivation, and is detached on disposal. Every workspace-module event subscription has a matching disposal-time detachment.

## Composition

`Composition/WinUiAppServices.cs` is the sole default construction site for runtime services. `WorkspaceRegistryFactory` is the sole construction site for workspace modules. `MainWindow` receives only `WinUiShellServices` plus `IWorkspaceRegistryFactory`; it does not know each module's constructor dependencies. Views do not instantiate scanners, executors, persistence stores, or protected-action services.

At startup the shell compares every `NavigationView` route with the registry in both directions. A missing module, missing route, or duplicate module key stops construction with a direct registration error instead of leaving a dead navigation item.

Lifetime ownership is explicit:

- `App` disposes `WinUiAppServices` when the main window closes;
- `MainWindow` disposes its `IWorkspaceRegistry`, activity console, and native window-size adapter;
- `WorkspaceRegistry` detaches its Settings bridge and disposes every module exactly once;
- each module stops owned timers, cancels keyed work, and detaches its view events;
- `WinUiAppServices` disposes provider, persistence, and operation infrastructure after module shutdown.

Services needed only while constructing another service stay local to `CreateDefault`; they are not exposed as unused container properties. The shell-facing record exposes only settings, activity, settings persistence, and the clipboard adapter consumed by the activity console.

`Sift.Core` physically owns shared source. It must not reference `System.Windows`, `Microsoft.UI.Xaml`, chart controls, or other presentation objects.

Desktop integration is composed at the same boundary. `IClipboardService` is the only owner of WinUI clipboard APIs, and `IWindowsShellLauncher` is the only owner of Explorer, Windows Settings, and `msinfo32` process handoffs. `WinUiAppServices` constructs both adapters once, while `WorkspaceRegistryFactory` injects them into only the modules and views that use them. The launcher exposes named Windows capabilities rather than an arbitrary command or URI API; it never participates in Core execution or elevation.

## Charts

Performance uses `LiveChartsCore.SkiaSharpView.WinUI` through a dedicated `PerformanceWorkspaceView`. The module owns sampling; the view owns observable history and chart styling. Charts use the graphite/clay/sage palette and a bounded 120-sample rolling window. While the workspace is active it opens a disposable `PdhSystemSampler` (CPU + physical-disk PDH counters), merges results into `SystemSnapshot.Counters`, and closes the query on `Deactivate`. Battery Saver stretches the refresh interval to at least 10 seconds. Dashboard telemetry also samples PDH into `cpu.pdh_percent` / `disk.*` and prefers PDH for `cpu.percent` when available; the PDH query is disposed with `IDashboardTelemetrySource`. Battery metrics use `BatteryReportReader` (`GetSystemPowerStatus` + WinRT AggregateBattery enrichment). Overhead is gated by `Sift.Benchmarks` and `scripts/measure-pdh-overhead.ps1` (target: ≤15% median vs ProcessSampler alone).

Hardware Monitor follows the same active-workspace sampling contract with a bounded 180-sample selected-sensor history. Core aggregates finite `IHardwareSensorProvider` implementations into a normalized device/sensor tree and isolates provider failures; the default local provider is LibreHardwareMonitor. Sampling is serialized, and the periodic timer starts only after the initial scan completes. The view owns filtering, expansion state, summary presentation, and LiveCharts2 rendering; it reconciles identity-keyed device and sensor collections in place so routine samples update reading properties without recreating panels or rows. Sensor collection is read-only, performs no downloads, and reports partial availability when the current token cannot access a driver-backed sensor.

No OxyPlot, WPF drawing surface, home-grown canvas graph, or duplicated telemetry timer belongs in the application.

Storage is not a telemetry chart. Its presentation-neutral `SquarifiedTreemap` layout consumes the Core `StorageTree`; the WinUI `StorageTreemapControl` renders bounded native keyboard-focusable Button elements recursively, limits visual count, and supports directory drill-down. Files remain colored by the existing warm extension palette. `FolderPickerService` is the narrow WinUI/Win32 window-handle adapter for the native folder picker; picker APIs do not leak into Core.

## Activity and background work

All observable work produces typed `ActivityEvent` records. `ActivityHub` fans them out to the bounded, filterable live console and opt-in persisted sinks. Sink failure may never interrupt the guarded operation being observed.

`OperationCoordinator` provides keyed latest-wins cancellation for replaceable reads and a separate committed-operation path for confirmed mutations. Once a mutation crosses its final confirmation boundary, navigation, duplicate clicks, and coordinator disposal cannot cancel it; the persistent activity stream records its terminal success or failure. Process and performance sampling run outside the UI thread.

System Information uses a presentation-neutral `ISystemInformationService` and a finite list of exact, read-only Windows Management Instrumentation and registry providers. Each provider is isolated so an unavailable optional namespace produces a visible partial-result warning instead of failing the report. The WinUI module owns progress, filtering, clipboard presentation, `msinfo32` handoff, and privacy guidance; WMI parsing and formatting remain in Core.

Dashboard sampling has one coordinator per active collection host, never per-widget timers: fast metrics at two seconds, medium inventories at 30 seconds, and slow inventories at five minutes. Battery Saver changes the fast delay to ten seconds, and a sleep/resume gap forces a slow refresh. Snapshots carry explicit changed-metric keys; widgets update existing values and append chart points only for those keys. The UI falls back to the in-process collector when MonitorHost is absent. A healthy compatible MonitorHost is the single history/alert owner; stale, restarted, incompatible, or absent hosts transfer explicit ownership to the fallback without duplicating samples.

`DashboardHistoryStore` batches fast readings in memory and writes UTC one-minute aggregates under SQLite WAL. Minutes are retained for seven days and rolled into fifteen-minute aggregates through the configured maximum of 90 days. Failed or cancelled flushes requeue unwritten aggregates; schema versions, busy timeouts, alert pruning, and corrupt-database quarantine are explicit. The metric policy rejects process, command, path, executable, and filename identity. Profile export never includes telemetry.

## Permission and execution boundary

All mutations use one interaction contract: the workspace collects an explicit selection, Core performs an automatic non-mutating preflight, the UI presents those reviewed results in a confirmation dialog, and Core revalidates the live target before confirmed execution. Preflight is mandatory infrastructure, not a user mode or persisted preference.

The main application manifest remains `asInvoker`. Standard-user HKLM Optimize batches and protected Recovery entries are sent first to `Sift.ElevationHost` through a one-shot request/response file under the exact per-user Sift elevation directory. The broker holds a deny-write lease over the request for the helper's complete lifetime, and the helper holds the read lease while dispatching it. Both sides independently resolve operations against `TweakCatalog`; the helper accepts no raw command, registry target, script, or arbitrary filesystem path. Recovery passes only an exact sibling backup filename, then the helper revalidates its machine identity, schema, entry count, tweak IDs, prior-value domains, and path. Requests are bounded, GUID-named, and reparse-point checked (validation and the create are one syscall via `FILE_FLAG_OPEN_REPARSE_POINT`, so a planted symlink — dangling or not — is rejected rather than followed). The 256-bit nonce correlates a response to its request and rejects a stale one; it is not a same-user authenticity secret, because the request file is readable by the invoking user. The integrity guarantees that actually bound a protected mutation are the independent policy re-resolution on both sides, the typed operation IDs, the signed-identity match, and the administrator-side consent prompt. The signed app/helper identities must match, and the administrator desktop shows a second typed operation summary before any mutation. UAC or administrator-side cancellation leaves the per-user portion unapplied, preventing a misleading partial batch.

External Windows tools used by protected workflows are resolved to exact System32 paths, receive tokenized arguments and a minimal sanitized environment, drain bounded output concurrently, and have operation-specific timeouts with process-tree termination. Sift never resolves DISM, SFC, PowerCfg, PowerShell, or SchTasks through the caller's working directory or inherited `PATH`. Process restart is unavailable while the Sift shell itself is elevated, preventing a selected user executable from inheriting the administrator token.

The UI may collect intent and present confirmation, but Core rechecks policy:

- Sift and critical Windows processes cannot be ended or restarted.
- Process actions require the current interactive session, a readable executable path, and a readable start-time identity; block cross-user/session-0 targets and every executable below the Windows and Sift roots; and recheck the live PID/start time/session/name/path after confirmation to reject PID reuse or stale-row execution.
- Defender, Windows Update, firewall, RPC, EventLog, and related services remain protected.
- Service controls expose Start for a reviewed Stopped service and Restart for a reviewed Running service only. The typed elevation request carries that expected state, and execution rejects drift instead of converting Restart into Start. The helper accepts no service command line, executable path, start-type change, Stop request, or arbitrary shell payload.
- Scheduled-task writes are limited to the two cataloged Microsoft Office update tasks.
- Real-time process priority is unavailable.
- `StorageSelectionDeletionManager` permits only an exact non-root child of the current map. It blocks path escape, Windows/Sift/protected paths, selected or nested reparse points, incomplete/inaccessible inventory, stale size/file counts, and over-bound selections. A complete content fingerprint produces a bounded five-minute one-use ticket; the exact tree is inventoried again after confirmation, and any difference consumes the ticket without mutation.
- `IStorageDeleter` exposes only `MoveToRecycleBin`; there is no permanent-delete mode for Storage or app leftovers anywhere below the UI.
- Installed Apps reads only the standard HKCU/HKLM uninstall roots, operates on one selected desktop entry, and re-reads the exact registry identity and values immediately before any handoff or cleanup.
- Script hosts, URI commands, silent commands, protected components, and stale/forged uninstall entries are blocked in Core. Store/MSIX management stays in Windows Settings.
- `InstalledAppTrustInspector` is a selected-item, read-only boundary. It revalidates the exact registration, parses the same guarded executable, verifies embedded Authenticode signatures or Windows catalog membership/signatures with `WinVerifyTrust`, disables network retrieval, builds only the locally available certificate chain, and reports signer/version/hash plus a non-authoritative publisher-name comparison. MSI product-code commands are labeled separately because verifying `msiexec.exe` cannot establish the product package signer.
- Core retains a bounded two-hour handle-backed uninstall session after a confirmed launch. The workspace waits asynchronously for the registered process to exit, refreshes automatically, and also supports an explicit status recheck for vendor launchers that delegate to child processes.
- Uninstaller launch or process exit alone grants no cleanup authority. A 30-minute continuation token is created only after Core confirms that the exact original uninstall registration is absent; an unchanged registration is treated as a running or cancelled uninstall, and a replaced identity is blocked.
- Leftover registration cleanup requires both a missing explicit install directory and a missing non-MSI uninstaller. Microsoft/system/update/driver/runtime entries are excluded; execution captures a typed full-tree backup before removing only the registration key.
- `AppLeftoverManager` owns file-leftover policy. It accepts a current verified orphan or a matching 30-minute in-memory continuation token created by verified registration removal/registration cleanup; an active registry entry always blocks the file scan.
- Candidate discovery probes only finite exact top-level AppData paths derived from the selected display name. It never enumerates AppData or authorizes token/fuzzy matches. Candidates start unselected, reparse points and incomplete trees are blocked, and deletion revalidates the full identity/path/tree before using `StorageDeleter` in Recycle Bin mode only.
- The older Maintenance token-overlap AppData sweep is retired, and `MaintenancePolicy` rejects legacy `AppLeftover` findings so that callers cannot bypass the Installed Apps authorization boundary.
- Maintenance review captures a registry-tree or filesystem metadata identity for every selected finding and returns a ten-minute, one-use ticket. Confirmed execution consumes the ticket, checks every target again before the first deletion, and rejects the entire batch when any reviewed content changed. Standard-user inventory excludes HKLM orphan registrations; Core also rejects a caller-constructed HKLM finding unless the current process has administrator permission.
- Registry mutation captures prior value and kind before applying.
- Authored Script Studio documents cannot execute, elevate, download runtimes/packages, or reach the elevation helper. Administrator catalog recipes are visible only in an elevated Sift session; neither recipe IDs nor raw commands cross the helper boundary. The Studio tab and analyzer launches are blocked while Sift is elevated. Language analysis consumes the in-memory document through standard input with profiles/site imports disabled where supported, a sanitized environment, bounded output, a timeout spanning launch/input/output, cancellation, and verified process-tree termination.
- Recovery inventory and execution are owned by `RecoveryManager`, not by the shell or Optimize view. It accepts only exact top-level Sift backup files, bounds schema/size/count, blocks reparse and cross-machine inputs, classifies protected entries, and re-inspects the selected file immediately before execution. In a standard-user session the one-shot elevated phase runs first; cancellation or rejection prevents the current-user phase, avoiding partial recovery batches.
- Cross-workspace links carry navigation intent only. Optimize can open Recovery, but it cannot select a backup, confirm a restore, or invoke recovery execution.
- Protected recovery is intentionally narrower than backup parsing. The helper accepts only an exact sibling filename and independently permits known HKLM tweak IDs with bounded prior DWORD domains plus the exact hibernation undo command. HKLM uninstall-tree snapshots are not elevated because the current backup format has no privileged integrity envelope; the UI reports them as blocked instead of weakening the boundary.

## Persistence

Settings and activity JSON use same-directory temporary files followed by replacement. UI settings are snapshotted and debounced before leaving the UI thread. Permission-handoff state may request an immediate flush.

Dashboard layouts use atomic `dashboard-profiles.json`; `AppSettings.HomeWidgets` is consumed only as a one-release first-profile migration input. Numeric trends and alert state use `dashboard.db`. Packaged background startup uses a disabled-by-default `windows.startupTask`; folder deployment uses one exact HKCU Run value. No Windows service is installed.

## Validation

- `Sift.UnitTests` — deterministic infrastructure and guard-policy tests.
- `Sift.Tests` — non-mutating Windows inventory, preflight, persistence, and canonical-source checks.
- `scripts/validate-ui.ps1` — launches the real WinUI executable, traverses all fourteen routes, and captures native screenshots.
- `scripts/validate-clean-account.ps1` — rejects unsigned/untrusted packages, then optionally performs a disposable standard-user install/elevation-IPC/launch/uninstall round trip. Its elevation probe changes no setting.
- `Sift.Build.targets` + `scripts/clean-build-output.ps1` — path-scoped clean-output contract imported by every Sift project; only the exact configuration/runtime directory below the owning project's `bin` root may be removed.
- `build-release.ps1` — full output-folder replacement, guarded custom-output ownership, self-contained publish, and application XBF/PRI verification.

The canonical project must build with zero warnings and must contain no `UseWPF`, `System.Windows`, or WPF chart dependency.
