# Sift agent instructions

Sift is a self-contained Windows 11 control center built with .NET 8, WinUI 3, and the Windows App SDK. `Sift.Core` is the presentation-neutral policy and execution layer. There is no WPF client or compatibility project.

Current version: **0.15.0**.

## Non-negotiable safety boundaries

- Do not add remote-script execution, runtime downloads, analytics, or phone-home behavior.
- Do not disable Defender, Windows Update, recovery, firewall protections, BitLocker, Microsoft Store, or security-critical services.
- Do not add broad service, scheduled-task, registry, Appx, driver, OEM-software, or filesystem sweeps.
- Do not present folklore registry changes as performance improvements.
- Do not expose preview-mode toggles. Every mutation must run an automatic non-mutating Core preflight, show reviewed evidence in a confirmation dialog, and revalidate immediately before execution.
- Every registry change must capture its prior value and kind before mutation.
- Balanced and Minimal presets must remain automatically reversible and exclude `Advanced` actions.
- Package removal remains explicit, `Advanced`, unselected by default, and labeled as lacking automatic undo.
- Never expose real-time process priority.
- Keep termination guards for Sift and critical shell, session, security, and service-host processes.
- Treat “harmless” as a design objective, not a guarantee; explain tradeoffs plainly.

## Architecture rules

- `MainWindow` owns shell concerns only.
- Each workspace implements `IWorkspaceModule` and owns a single view.
- Construct default services only in `WinUiAppServices`.
- Put models, scanners, persistence, execution, and guards in `Sift.Core`.
- Never reference `System.Windows`, `UseWPF`, or WPF-specific packages.
- Never update WinUI-bound collections from worker threads.
- Stop timers and cancel obsolete work in `Deactivate`.
- Use `OperationCoordinator` for latest-wins background work and `ActivityHub` for observable operations.
- Use LiveCharts2 for charts; do not add another graph framework or custom canvas chart.
- Use WinUI.TableView for dense sortable inventories when ListView is insufficient.

## UI rules

- Continue the graphite, clay, and sage palette; do not introduce blue/cyan AI-dashboard styling.
- Every composed surface needs deliberate borders, padding, margins, alignment, type scale, empty state, loading state, and error state.
- Keep explicit readable foregrounds on the dark theme.
- Popups and ContentDialogs require separate visual verification.
- Keep the activity console usable at wide and narrow widths.
- Add `AutomationProperties.Name` to icon-only or ambiguous controls.
- Do not attach initialization-sensitive handlers until named controls exist.
- Use native WinUI controls before introducing custom drawing or behavior.

## Charts and telemetry

- `ProcessSampler.Sample` runs off the UI thread.
- Performance owns one bounded rolling history and one timer, active only while the workspace is visible.
- Chart render state belongs in the view; sampling and cancellation belong in the module.
- Measure overhead before adding PDH, ETW, GPU, network, or temperature telemetry.

## Build and validation

From the repository root:

```powershell
apps\Sift\scripts\validate.ps1
```

Individual commands:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
dotnet build apps\Sift\Sift.csproj --configuration Release
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild
git diff --check
```

Visual artifacts are written to `apps/Sift/artifacts/` and ignored. Inspect Home, Performance, and Startup after any theme, typography, table, chart, shell, or XAML change.

## Release

```powershell
apps\Sift\build-release.ps1
apps\Sift\build-release.ps1 -Versioned
```

The output is a self-contained folder under `apps/Sift/dist/`. Keep its files together. Do not claim the executable is signed unless `signtool verify` succeeds.

## Completion checklist

- [ ] Safety boundaries remain intact.
- [ ] Automatic preflight produces no mutation and confirmation cancellation leaves the target unchanged.
- [ ] Registry changes preserve prior values.
- [ ] Background sampling stays off the UI thread.
- [ ] Inactive workspace timers are stopped.
- [ ] Protected processes, services, and tasks remain blocked below the UI.
- [ ] All affected states were visually inspected.
- [ ] Core, tests, app build, publish, and native smoke validation pass with zero warnings.
- [ ] `git diff --check` passes.
- [ ] Unrelated workspace changes remain untouched.
