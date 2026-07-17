# Sift Disconnected Capability Wiring Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Safely connect Sift's scheduled-task, System Restore, persisted-history, and maintenance-persistence capabilities, then remove only audit-proven dead settings and services.

**Architecture:** Core owns typed task/restore policy, preflight tickets, immediate revalidation, bounded history aggregation, and all helper-side allowlists. Dedicated Task Manager, Optimize, and Health modules collect one explicit intent and present evidence; `WinUiAppServices` remains the only production composition root. The one-shot helper receives enums, booleans, fixed-size hashes, and existing bounded identifiers only—never task paths, task commands, WMI text, restore-point descriptions, scripts, or arbitrary shell payloads.

**Tech Stack:** .NET 8, C# 12, xUnit v3, WinUI 3, Windows App SDK, WinUI.TableView, PowerShell UI Automation, `schtasks.exe`, `System.Management`.

---

## Constraints and fixed decisions

- Scheduled-task actions remain single-selection only. The typed allowlist contains only the two exact Office updater identities already present in `ScheduledTaskMonitor`: `OfficeAutomaticUpdates` and `OfficeFeatureUpdates`. `\Microsoft\Windows\...`, Defender, Windows Update, security, maintenance, and caller-supplied task identities remain rejected.
- `ScheduledTaskIdentityCatalog` is the only enum-to-path/name map. `ScheduledTaskMonitor`, `ScheduledTaskController`, broker policy, and helper policy all call it; `ScheduledTaskInventory` stays parameterless. The broker request carries no task path. A fixed 64-character SHA-256 definition hash and expected enabled state bind confirmation to the inspected task definition and state.
- `ScheduledTaskChange` and `ServiceActionKind` replace action strings throughout `IGuardedSystemActions`, its implementation, broker, helper, module, and tests. Only `WindowsServiceMonitor.Act` converts `ServiceActionKind` to the two existing command strings. Existing tweak IDs, service names, backup filenames, request IDs, nonces, and task hashes retain strict count/length/character validation.
- Restore-point eligibility is exactly: `OfferSystemRestorePoint` is true and the already-confirmed Optimize selection contains either an HKLM registry tweak or an `Advanced` `AppPackage` tweak. Commands and HKCU-only batches do not qualify.
- The restore operation has a typed, non-mutating, five-minute, one-use `SystemRestorePointPreflight` ticket. Optimize displays its evidence in the first mutation confirmation. Execution consumes the ticket, immediately revalidates its expected environment identity/state, and only then performs the fixed parameter-free WMI operation. `SystemRestorePointController` owns the fixed description `Sift before Optimize changes`; neither UI nor broker can supply a description or WMI payload.
- Restore-point creation is best effort, never described as rollback or guaranteed recovery. Failure or UAC cancellation opens a second dialog titled `Continue without restore point?`. Closing that dialog ends the operation before machine or user mutation.
- Optimize keeps machine-phase-first behavior. A cancelled/rejected machine phase still prevents the local phase.
- Health becomes a dedicated module/view with separate `Checks` and `Activity / Recovery History` tabs. A later diagnostic-history plan adds a third tab; it does not replace or repurpose either tab in this plan.
- Native UI automation may inspect a naturally present allowlisted task and cancel its confirmation. It must not create, register, enable, or disable a task and must not confirm restore-point creation.
- `HomeWidgets` and `ChartSmoothing` remain unchanged for later plans.
- There are no commit steps in this plan.

## File map

### Create

- `apps/Sift.Core/Models/ScheduledTaskAction.cs` — typed task identity, desired change, preflight ticket, and result records.
- `apps/Sift.Core/Services/ScheduledTaskIdentityCatalog.cs` — sole exact enum/path/name resolver shared by inventory, controller, broker policy, and helper.
- `apps/Sift.Core/Services/ScheduledTaskActionService.cs` — injectable controller contract, bounded ticket policy, revalidation, and execution orchestration.
- `apps/Sift.Core/Services/ScheduledTaskActionWorkflow.cs` — non-WinUI confirmation/cancellation orchestration seam.
- `apps/Sift.Core/Services/SystemRestorePointService.cs` — eligibility policy, parameter-free controller/service contracts, and WMI implementation.
- `apps/Sift.Core/Services/OptimizeMutationWorkflow.cs` — non-WinUI Optimize preflight/confirmation/restore/apply ordering seam.
- `apps/Sift.Core/Services/HealthWorkspaceOrchestrator.cs` — non-WinUI latest-wins checks/history loader and cancellation seam.
- `apps/Sift.UnitTests/ScheduledTaskActionServiceTests.cs` — deterministic allowlist, non-mutation, stale-state, stale-identity, single-use, and broker-path tests.
- `apps/Sift.UnitTests/ScheduledTaskActionWorkflowTests.cs` — confirmation cancellation/close behavior with zero execution.
- `apps/Sift.UnitTests/SystemRestorePointServiceTests.cs` — eligibility, fixed-operation, cancellation, and failure tests.
- `apps/Sift.UnitTests/OptimizeMutationWorkflowTests.cs` — first/second confirmation, UAC cancellation, machine rejection, and dependent-mutation tests.
- `apps/Sift.UnitTests/HistoryServiceTests.cs` — merge, ordering, bounds, cancellation, and partial-source tests.
- `apps/Sift.UnitTests/HealthWorkspaceOrchestratorTests.cs` — stale-result, deactivate, independent failure, and partial-retention tests.
- `apps/Sift/Composition/HealthWorkspaceModule.cs` — latest-wins Health checks/history orchestration.
- `apps/Sift/Views/HealthWorkspaceView.xaml` — dedicated checks/history tabs and responsive visual states.
- `apps/Sift/Views/HealthWorkspaceView.xaml.cs` — independent filtering and explicit loading/empty/partial/error state presentation.

### Modify

- `apps/Sift.Core/Services/ScheduledTaskMonitor.cs`
- `apps/Sift.Core/Services/SystemInventoryServices.cs`
- `apps/Sift.Core/Services/GuardedSystemActions.cs`
- `apps/Sift.Core/Services/ElevationBroker.cs`
- `apps/Sift.ElevationHost/Program.cs`
- `apps/Sift.Core/Services/SystemHelpers.cs`
- `apps/Sift.Core/Services/HistoryAggregator.cs`
- `apps/Sift.Core/Models/HealthCheckRow.cs`
- `apps/Sift.Core/Models/AppSettings.cs`
- `apps/Sift/Composition/WinUiAppServices.cs`
- `apps/Sift/Composition/TaskManagerWorkspaceModule.cs`
- `apps/Sift/Views/TaskManagerWorkspaceView.xaml`
- `apps/Sift/Views/TaskManagerWorkspaceView.xaml.cs`
- `apps/Sift/Composition/OptimizeWorkspaceModule.cs`
- `apps/Sift/Views/OptimizeWorkspaceView.xaml.cs`
- `apps/Sift/Composition/MaintenanceWorkspaceModule.cs`
- `apps/Sift/Composition/WorkspaceSnapshotFactory.cs`
- `apps/Sift/MainWindow.xaml.cs`
- `apps/Sift.UnitTests/ElevationBrokerTests.cs`
- `apps/Sift.UnitTests/InfrastructureTests.cs`
- `apps/Sift.Tests/Program.cs`
- `apps/Sift/scripts/validate-ui.ps1`
- `apps/Sift/scripts/validate-feature-audit.ps1`
- `apps/Sift/docs/audits/sift-feature-audit.json`
- `apps/Sift/ARCHITECTURE.md`
- `apps/Sift/ROADMAP.md`
- `apps/Sift/CHANGELOG.md`

### Delete

- `apps/Sift.Core/Services/TelemetryHub.cs`

## Task 1: Build the typed scheduled-task Core boundary

**Files:**
- Create: `apps/Sift.Core/Models/ScheduledTaskAction.cs`
- Create: `apps/Sift.Core/Services/ScheduledTaskIdentityCatalog.cs`
- Create: `apps/Sift.Core/Services/ScheduledTaskActionService.cs`
- Create: `apps/Sift.Core/Services/ScheduledTaskActionWorkflow.cs`
- Modify: `apps/Sift.Core/Services/ScheduledTaskMonitor.cs`
- Modify: `apps/Sift.Core/Services/SystemInventoryServices.cs`
- Modify: `apps/Sift.Core/Services/GuardedSystemActions.cs`
- Test: `apps/Sift.UnitTests/ScheduledTaskActionServiceTests.cs`
- Test: `apps/Sift.UnitTests/ScheduledTaskActionWorkflowTests.cs`

- [ ] **Step 1: Write failing deterministic task-policy tests**

Add tests named:

```csharp
[Fact] public void Preflight_is_non_mutating_and_records_expected_state_and_identity();
[Fact] public async Task Execute_rejects_changed_expected_state_without_mutation();
[Fact] public async Task Execute_rejects_changed_definition_hash_without_mutation();
[Fact] public async Task Ticket_is_single_use_and_expired_tickets_do_not_mutate();
[Fact] public void Policy_rejects_windows_update_defender_unknown_and_multiple_targets();
[Fact] public async Task Standard_user_execution_sends_only_typed_identity_to_broker();
[Fact] public async Task Confirmation_decline_revokes_ticket_and_never_executes();
[Fact] public async Task Confirmation_close_revokes_ticket_and_never_executes();
```

Use a `RecordingScheduledTaskController` that exposes `InspectCount`, `SetCount`, mutable `Enabled`, and mutable `DefinitionHash`, plus a `RecordingElevationBroker`. Assert preflight leaves `SetCount == 0`; stale/expired/reused tickets leave both mutation counts at zero; the broker observes only `ScheduledTaskId`, `ScheduledTaskChange`, expected state, and the 64-character hash.

- [ ] **Step 2: Run the focused tests and observe red**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~ScheduledTaskActionServiceTests"
```

Expected: FAIL at compilation because the scheduled-task action types and service do not exist.

- [ ] **Step 3: Add the exact typed model**

Create these public types in `ScheduledTaskAction.cs`:

```csharp
namespace Sift.Models;

public enum ScheduledTaskId { OfficeAutomaticUpdates, OfficeFeatureUpdates }
public enum ScheduledTaskChange { Enable, Disable }

public sealed record ScheduledTaskIdentity(
    ScheduledTaskId Id,
    string DisplayName,
    bool Enabled,
    string State,
    string DefinitionHash);

public sealed record ScheduledTaskActionPreflight(
    Guid TicketId,
    ScheduledTaskId Id,
    string DisplayName,
    ScheduledTaskChange Change,
    bool ExpectedEnabled,
    string ExpectedState,
    string ExpectedDefinitionHash,
    DateTime ExpiresUtc,
    string Evidence);

public sealed record ScheduledTaskActionResult(
    bool Succeeded,
    bool Cancelled,
    string Summary,
    IReadOnlyList<string> Log);
```

- [ ] **Step 4: Implement the injectable controller and one-use service**

Define these exact contracts in `ScheduledTaskActionService.cs`:

```csharp
public interface IScheduledTaskController
{
    ScheduledTaskIdentity? Inspect(ScheduledTaskId id);
    ScheduledTaskActionResult SetEnabled(
        ScheduledTaskId id,
        ScheduledTaskChange change,
        bool expectedEnabled,
        string expectedDefinitionHash);
}

public interface IScheduledTaskActionService
{
    ScheduledTaskActionPreflight Preflight(ScheduledTaskId id, ScheduledTaskChange change);
    void Revoke(Guid ticketId);
    Task<ScheduledTaskActionResult> ExecuteAsync(Guid ticketId, CancellationToken cancellationToken = default);
}
```

`ScheduledTaskActionService` receives `IScheduledTaskController`, `IElevationBroker`, `Func<bool> isElevated`, `TimeProvider`, and an optional five-minute ticket lifetime. `Preflight` inspects exactly one enum identity, rejects a no-op state, verifies a 64-character uppercase hexadecimal hash, stores a private ticket, and returns evidence containing exact display name, current state, requested state, expiry, and “no change was made.” `ExecuteAsync` atomically consumes the ticket, checks expiry, re-inspects, compares ID/state/hash, and then uses the local controller when elevated or `IElevationBroker.ChangeScheduledTaskAsync` otherwise.

- [ ] **Step 5: Add the testable confirmation workflow**

Create:

```csharp
public interface IScheduledTaskConfirmation
{
    Task<bool> ConfirmAsync(
        ScheduledTaskActionPreflight preflight,
        CancellationToken cancellationToken);
}

public interface IScheduledTaskActionWorkflow
{
    Task<ScheduledTaskActionResult> RunAsync(
        ScheduledTaskId id,
        ScheduledTaskChange change,
        IScheduledTaskConfirmation confirmation,
        CancellationToken cancellationToken = default);
}
```

Implement `ScheduledTaskActionWorkflow(IScheduledTaskActionService actions) : IScheduledTaskActionWorkflow`. `RunAsync` calls `Preflight`, awaits the supplied confirmation, revokes and returns `Succeeded=false`, `Cancelled=true`, `Summary="Scheduled-task action cancelled; nothing was changed."`, and an empty log when confirmation returns false or closes. It calls `ExecuteAsync` only after true. `TaskManagerWorkspaceModule` later supplies a view-backed adapter; deterministic tests use a recording confirmation and assert zero controller/broker mutation.

- [ ] **Step 6: Replace broad task-folder matching with one exact resolver**

Create `ScheduledTaskIdentityCatalog` with the only two exact definitions and these methods:

```csharp
public sealed record ScheduledTaskDefinition(
    ScheduledTaskId Id,
    string TaskPath,
    string TaskName,
    string DisplayName);

public static ScheduledTaskDefinition Resolve(ScheduledTaskId id);
public static bool TryResolve(string taskPath, string taskName, out ScheduledTaskDefinition definition);
```

The catalog recognizes only:

```text
\Microsoft\Office\Office Automatic Updates
\Microsoft\Office\Office Feature Updates
```

Keep explicit early rejection for any full identity containing `Windows Defender`, `WindowsUpdate`, or beginning `\Microsoft\Windows\`. `ScheduledTaskMonitor` calls `ScheduledTaskIdentityCatalog.TryResolve`; `ScheduledTaskController` calls `Resolve`. The controller builds `/Query /TN "{definition.TaskPath.TrimEnd('\\')}\\{definition.TaskName}" /XML` only from that resolved definition, caps XML at 256 KiB, derives enabled state, and hashes the returned definition with SHA-256. Its mutation method re-inspects expected state/hash before invoking `/Change /ENABLE` or `/Change /DISABLE`. No second dictionary, switch, tuple table, or path/name constant is permitted.

Change `ScheduledTaskInfo` to include `ScheduledTaskId? ActionId`; derive `IsAllowlisted` from `ActionId.HasValue`. Keep `public sealed class ScheduledTaskInventory : IScheduledTaskInventory` parameterless, with `Enumerate()` delegating to `ScheduledTaskMonitor.Enumerate()`. No controller injection or conditional design remains.

- [ ] **Step 7: Retire the old task mutation path and type service actions**

Remove `ScheduledTaskActionTarget`, `PlanScheduledTaskChange`, and `SetScheduledTasksEnabled` from `IGuardedSystemActions` and `GuardedSystemActions`. Change:

```csharp
public enum ServiceActionKind { Start, Restart }

GuardedActionResult ActOnServices(
    IEnumerable<ServiceActionTarget> targets,
    ServiceActionKind action);
```

Update `GuardedSystemActions.ActOnServices`, all call sites, fakes, and tests to use the enum. It rejects undefined enum values. Change the monitor signature to `WindowsServiceMonitor.Act(string name, ServiceActionKind action)`; only inside that method may a local string become `"Start"` or `"Restart"` for existing service-control/log behavior. Update task tests to call `ScheduledTaskIdentityCatalog.TryResolve` and the new service/workflow.

- [ ] **Step 8: Run focused and guard tests**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~ScheduledTaskAction|FullyQualifiedName~GuardedSystemActionsTests"
```

Expected: PASS; no test invokes real `schtasks.exe /Change`.

## Task 2: Extend one-shot elevation with bounded typed operations

**Files:**
- Modify: `apps/Sift.Core/Services/ScheduledTaskActionService.cs`
- Modify: `apps/Sift.Core/Services/ElevationBroker.cs`
- Modify: `apps/Sift.ElevationHost/Program.cs`
- Test: `apps/Sift.UnitTests/ElevationBrokerTests.cs`

- [ ] **Step 1: Add failing envelope and broker-runner tests**

Add tests proving:

```csharp
[Fact] public void Task_request_round_trip_contains_no_task_path_or_command();
[Fact] public void Restore_point_request_has_no_operation_parameters();
[Fact] public void Envelope_policy_rejects_cross_operation_fields_and_unbounded_strings();
[Fact] public async Task Injected_runner_reports_uac_cancellation_and_deletes_request();
[Fact] public async Task Injected_runner_failure_reports_failure_and_deletes_request();
[Fact] public void Task_response_mapping_preserves_success_cancel_failure_and_log();
```

The serialized task request must not contain `TaskPath`, `TaskName`, `Command`, `Arguments`, or `Wmi`. The restore request must have empty tweak IDs and null service, backup, and task fields.

- [ ] **Step 2: Run red**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~ElevationBrokerTests"
```

Expected: FAIL because the new operation kinds, typed fields, and runner seam are absent.

- [ ] **Step 3: Make service actions typed and add the two operations**

Reuse Task 1's `ServiceActionKind` and add:

```csharp
public enum ElevatedOperationKind
{
    ApplyMachineTweaks,
    RestoreMachineBackup,
    ValidateElevation,
    ManageService,
    ChangeScheduledTask,
    CreateSystemRestorePoint
}
```

Change `IGuardedSystemActions.ActOnServices`, `GuardedSystemActions.ActOnServices`, `IElevationBroker.ManageServiceAsync`, `ElevationBroker.ManageServiceAsync`, `ElevatedOperationPolicy.TryResolveServiceAction`, `TaskManagerWorkspaceModule.ActOnServiceAsync`, `Sift.ElevationHost/Program.cs:ManageService`, and all corresponding tests/fakes to accept `ServiceActionKind`. `TaskManagerWorkspaceModule` converts button intent directly to the enum. The helper passes the enum to `GuardedSystemActions`. Convert to `"Start"`/`"Restart"` only in the final call inside `WindowsServiceMonitor.Act`; no other service action string remains. Update existing round-trip fixtures to use a 32-character lowercase request ID and a 64-character uppercase nonce. Add:

```csharp
Task<ElevatedOperationResponse> ChangeScheduledTaskAsync(
    ScheduledTaskId id,
    ScheduledTaskChange change,
    bool expectedEnabled,
    string expectedDefinitionHash,
    CancellationToken cancellationToken = default);

Task<ElevatedOperationResponse> CreateSystemRestorePointAsync(
    CancellationToken cancellationToken = default);
```

Extend `ElevatedOperationRequest` only with nullable typed task fields, `bool? ExpectedTaskEnabled`, and `string? ExpectedTaskDefinitionHash`. Do not add task path/name or restore description fields.

- [ ] **Step 4: Enforce operation-specific request shapes**

Add `ElevatedOperationPolicy.TryValidateRequestShape`. It must reject:

- request IDs not exactly 32 lowercase hexadecimal characters;
- nonces not exactly 64 uppercase hexadecimal characters;
- more than 64 tweak IDs, blank IDs, service names over 256 characters, backup names over 160 characters, and task hashes not exactly 64 uppercase hexadecimal characters;
- any field populated for an operation that does not own it;
- any undefined enum value;
- `CreateSystemRestorePoint` unless every payload field is empty/null.

Call this policy before writing a request and immediately after reading it in the helper.

- [ ] **Step 5: Add the deterministic process-runner seam**

Define:

```csharp
internal sealed record ElevationHostRunResult(bool Started, bool Cancelled, string? Error);

internal interface IElevationHostRunner
{
    Task<ElevationHostRunResult> RunAsync(
        string helperPath,
        string requestPath,
        CancellationToken cancellationToken);
}
```

Move current `Process.Start(... Verb = "runas")` and waiting behavior into `ElevationHostRunner`. Inject it into an internal `ElevationBroker` constructor. The default public construction uses the real runner. A fake cancelled result maps to the existing no-mutation cancellation response; a fake error maps to failure. The broker `finally` always deletes request and response files.

- [ ] **Step 6: Implement helper-side task execution**

For `ChangeScheduledTask`, the helper validates the request shape, creates its own `ScheduledTaskController`, re-inspects the enum identity, compares expected state and definition hash, and calls `SetEnabled`. It never trusts UI inventory or broker-side allowlisting alone. Map the result to one response with `Applied` equal to one only on success.

Leave `CreateSystemRestorePoint` routed to Task 3's parameter-free controller.

- [ ] **Step 7: Define exact response mappings**

Add the internal pure mapping method used by the scheduled-task service:

```csharp
internal static ScheduledTaskActionResult MapScheduledTaskResponse(
    ElevatedOperationResponse response);
```

The scheduled-task mapping is exact:

- `Succeeded=true`, `Cancelled=false`, `Applied=1`, `Failed=0` maps to `Succeeded=true`, `Cancelled=false`, `Summary=response.Message`, and a copied `response.Log`;
- `Cancelled=true` maps to `Succeeded=false`, `Cancelled=true`, the response message, and copied log regardless of counts;
- every other response, including inconsistent success counts, maps to `Succeeded=false`, `Cancelled=false`, the response message, and copied log.

- [ ] **Step 8: Run broker tests**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~ElevationBrokerTests|FullyQualifiedName~ScheduledTaskActionServiceTests"
```

Expected: PASS, including deterministic cancellation/failure with no UAC prompt.

## Task 3: Create the parameter-free System Restore policy/service

**Files:**
- Create: `apps/Sift.Core/Services/SystemRestorePointService.cs`
- Modify: `apps/Sift.Core/Services/SystemHelpers.cs`
- Modify: `apps/Sift.ElevationHost/Program.cs`
- Test: `apps/Sift.UnitTests/SystemRestorePointServiceTests.cs`

- [ ] **Step 1: Write failing policy and controller tests**

Add tests for these exact cases:

```csharp
[Theory]
[InlineData(false, "privacy.activity", false)]
[InlineData(true, "privacy.ad-id", false)]
[InlineData(true, "privacy.activity", true)]
[InlineData(true, "apps.clipchamp", true)]
[InlineData(true, "power.hibernate", false)]
public void Eligibility_is_limited_to_enabled_hklm_or_advanced_package_batches(
    bool setting, string tweakId, bool expected);

[Fact] public async Task Elevated_session_calls_parameter_free_controller_once();
[Fact] public async Task Standard_user_uses_parameter_free_broker_once();
[Fact] public async Task Cancellation_and_failure_are_best_effort_results_not_success();
[Fact] public void Preflight_is_non_mutating_and_returns_reviewable_evidence();
[Fact] public async Task Execute_consumes_ticket_once_and_revalidates_environment();
[Fact] public async Task Execute_rejects_expired_changed_and_reused_tickets_without_wmi_mutation();
[Fact] public void Elevated_response_mapping_preserves_success_cancel_failure_and_log();
```

- [ ] **Step 2: Run red**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~SystemRestorePointServiceTests"
```

Expected: FAIL because the service does not exist.

- [ ] **Step 3: Add exact contracts and eligibility**

Create:

```csharp
public sealed record SystemRestorePointResult(
    bool Succeeded,
    bool Cancelled,
    string Message,
    IReadOnlyList<string> Log);

public enum SystemProtectionState
{
    Available,
    Disabled,
    Unavailable,
    Unknown
}

public sealed record SystemRestorePointInspection(
    string MachineIdentityHash,
    bool SystemRestoreAvailable,
    SystemProtectionState ProtectionState,
    string Evidence);

public sealed record SystemRestorePointPreflight(
    Guid TicketId,
    DateTime ExpiresUtc,
    string ExpectedMachineIdentityHash,
    bool ExpectedSystemRestoreAvailable,
    SystemProtectionState ExpectedProtectionState,
    string Evidence);

public interface ISystemRestorePointController
{
    SystemRestorePointInspection Inspect();
    SystemRestorePointResult Create(SystemRestorePointInspection expected);
}

public interface ISystemRestorePointService
{
    bool IsEligible(bool offerEnabled, IReadOnlyList<Tweak> confirmedSelection);
    SystemRestorePointPreflight Preflight(
        bool offerEnabled,
        IReadOnlyList<Tweak> confirmedSelection);
    void Revoke(Guid ticketId);
    Task<SystemRestorePointResult> ExecuteAsync(
        Guid ticketId,
        CancellationToken cancellationToken = default);
}
```

`IsEligible` returns true only for an HKLM registry tweak or an `Advanced` `AppPackage` tweak when the setting is enabled. `Preflight` calls only `Inspect`, requires a 64-character uppercase hexadecimal machine hash, creates a private five-minute ticket, and returns evidence stating eligibility, current typed System Restore/System Protection state, fixed description, expiry, and `No restore point was created during preflight.` It throws a typed policy exception for ineligible input.

`ExecuteAsync` atomically consumes the ticket, rejects expiry/reuse, calls `Inspect` again, and compares machine hash, availability, and protection state. In an elevated process it calls `Create(revalidatedInspection)`; in a standard process it then calls the parameter-free `IElevationBroker.CreateSystemRestorePointAsync`. The helper independently performs `Inspect`, immediately passes that exact inspection into `Create`, and `Create` rechecks it before invoking WMI. Thus the elevation payload stays parameter-free while both sides enforce immediate expected-state/identity revalidation.

- [ ] **Step 4: Move WMI behind the interface**

Move all code from static `SystemRestoreHelper.TryCreateRestorePoint` into `SystemRestorePointController`. `Inspect` reads only local machine identity and bounded System Restore/System Protection availability without mutation. `Create(expected)` immediately repeats `Inspect`, rejects any change, then supplies the literal `Sift before Optimize changes`, restore point type `0`, and event type `100`. Remove the static helper. Return wording must include:

```text
Sift requested a best-effort System Restore point. Windows may decline creation when System Protection is unavailable, disabled, rate-limited, or blocked by policy.
```

No message may claim the restore point guarantees rollback.

- [ ] **Step 5: Define exact elevated-response mapping**

Add:

```csharp
internal static SystemRestorePointResult MapElevatedResponse(
    ElevatedOperationResponse response);
```

`Succeeded=true`, `Cancelled=false`, `Applied=1`, `Failed=0` maps to successful/noncancelled with `Message=response.Message` and a copied log. `Cancelled=true` maps to unsuccessful/cancelled with message/log preserved regardless of counts. Every other response, including inconsistent success counts, maps to unsuccessful/noncancelled with message/log preserved. Tests cover all branches and prove log order/content is unchanged.

The helper returns `Applied=1`, `Failed=0` only when Windows returns restore code zero; `Applied=0`, `Failed=1` for WMI/policy failure; and zero/zero for cancellation.

- [ ] **Step 6: Complete helper routing**

In `Sift.ElevationHost/Program.cs`, route `CreateSystemRestorePoint` by creating one controller, calling `Inspect`, then `Create(inspection)`. The request supplies no description, inspection, machine identifier, or protection state. Convert results using Task 2's exact response-count contract.

- [ ] **Step 7: Run focused tests**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~SystemRestorePointServiceTests|FullyQualifiedName~ElevationBrokerTests"
```

Expected: PASS without WMI mutation or UAC.

## Task 4: Wire Task Manager's single-task action path

**Files:**
- Modify: `apps/Sift/Composition/TaskManagerWorkspaceModule.cs`
- Modify: `apps/Sift/Views/TaskManagerWorkspaceView.xaml`
- Modify: `apps/Sift/Views/TaskManagerWorkspaceView.xaml.cs`
- Modify: `apps/Sift.Tests/Program.cs`
- Modify: `apps/Sift/scripts/validate-ui.ps1`

- [ ] **Step 1: Add failing canonical-source checks**

Extend `ValidateCanonicalWinUiSource` to require these symbols/text:

```text
Enable selected scheduled task
Disable selected scheduled task
ConfirmScheduledTaskActionAsync
IScheduledTaskActionWorkflow
ExpectedDefinitionHash
ChangeScheduledTaskAsync
```

Also assert the view contains neither a bulk-selection mode nor a task-path input.

- [ ] **Step 2: Run integration red**

Run:

```powershell
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
```

Expected: FAIL at the new Task Manager scheduled-task wiring check.

- [ ] **Step 3: Add the selected-task action bar**

Wrap the Scheduled tasks tab in the same two-row pattern as Services. Add `SelectedTaskText`, `EnableTaskButton`, and `DisableTaskButton` above `TaskTable`. Use exact automation names:

```text
Enable selected scheduled task
Disable selected scheduled task
```

Both buttons start disabled. The selected summary shows task display name, exact inventory path, current state, and policy label. The table remains `SelectionMode="Single"`.

- [ ] **Step 4: Add view intent and confirmation**

Add `EnableTaskRequested` and `DisableTaskRequested` events, `_canEnableTask`/`_canDisableTask` fields, `SetTaskActionAvailability`, and:

```csharp
public Task<bool> ConfirmScheduledTaskActionAsync(
    ScheduledTaskActionPreflight preflight,
    bool requiresElevation)
```

The dialog title is `Enable this exact scheduled task?` or `Disable this exact scheduled task?`; close text is `Leave task unchanged`. Show all preflight evidence, expected state, target state, and the immediate-revalidation statement. Mention one-shot administrator confirmation only when required.

- [ ] **Step 5: Adapt Task Manager to the tested workflow**

Inject `IScheduledTaskActionWorkflow` into `TaskManagerWorkspaceModule` with a view-backed `IScheduledTaskConfirmation` adapter. Selection availability requires `SelectedTask.ActionId.HasValue` and a state-changing action. The event handler calls `RunAsync` for exactly one enum ID, publishes returned success/failure/cancellation activity, and refreshes only on success. It does not duplicate preflight, revoke, confirmation, or execute branching already covered by `ScheduledTaskActionWorkflowTests`.

The adapter publishes preflight evidence as trace activity before calling `ConfirmScheduledTaskActionAsync`. Catch typed policy exceptions as blocked status without mutation. Detach both events in `Dispose`.

- [ ] **Step 6: Extend native automation without task mutation**

In the Task Manager block, select `Show scheduled task inventory`, assert both buttons are disabled before selection, then iterate visible rows. If a row enables one action, invoke that action, assert `Leave task unchanged` appears, capture `window-task-manager-scheduled-task-confirmation.png`, and close it. Print:

```text
SKIP  No naturally present allowlisted scheduled task is available.
```

when none exists. Do not create a fixture task and never press the primary action.

- [ ] **Step 7: Verify Task Manager**

Run:

```powershell
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
dotnet build apps\Sift\Sift.csproj --configuration Release
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace 'Task Manager'
```

Expected: all PASS; UI automation either captures and cancels a naturally available task dialog or emits the exact harmless skip.

## Task 5: Gate eligible confirmed Optimize batches on best-effort restore

**Files:**
- Create: `apps/Sift.Core/Services/OptimizeMutationWorkflow.cs`
- Test: `apps/Sift.UnitTests/OptimizeMutationWorkflowTests.cs`
- Modify: `apps/Sift/Composition/OptimizeWorkspaceModule.cs`
- Modify: `apps/Sift/Views/OptimizeWorkspaceView.xaml.cs`
- Modify: `apps/Sift.Tests/Program.cs`
- Modify: `apps/Sift/scripts/validate-ui.ps1`

- [ ] **Step 1: Add failing behavioral and ordering tests**

Create deterministic tests named:

```csharp
[Fact] public async Task First_confirmation_decline_revokes_restore_ticket_and_never_applies();
[Fact] public async Task Second_confirmation_decline_prevents_all_apply();
[Fact] public async Task Second_confirmation_close_prevents_all_apply();
[Fact] public async Task Restore_uac_cancellation_requires_continue_and_decline_prevents_apply();
[Fact] public async Task Machine_phase_rejection_prevents_local_phase();
[Fact] public async Task Accepted_best_effort_continuation_runs_machine_then_local();
```

Use recording delegates/services and assert call order and exact zero counts for machine/local apply in every blocked path. Require `OptimizeWorkspaceModule` to use `OptimizeMutationWorkflow`; assert the second confirmation's title and close text are `Continue without restore point?` and `Cancel all changes`.

- [ ] **Step 2: Run red**

Run:

```powershell
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
```

Expected: FAIL because `OptimizeMutationWorkflow` and the restore-ticket review path do not exist.

- [ ] **Step 3: Add the non-WinUI orchestration seam**

Define:

```csharp
public sealed record OptimizeMutationReview(
    ApplyResult TweakPreflight,
    SystemRestorePointPreflight? RestorePointPreflight);

public sealed record OptimizeMutationWorkflowResult(
    bool Succeeded,
    bool Cancelled,
    bool MutationStarted,
    string Summary);

public interface IOptimizeMutationInteraction
{
    Task<bool> ConfirmReviewedBatchAsync(
        OptimizeMutationReview review,
        CancellationToken cancellationToken);
    Task<bool> ConfirmContinueWithoutRestorePointAsync(
        SystemRestorePointResult failure,
        CancellationToken cancellationToken);
}

public interface IOptimizeMutationPhases
{
    Task<bool> ExecuteMachinePhaseAsync(CancellationToken cancellationToken);
    Task<bool> ExecuteLocalPhaseAsync(CancellationToken cancellationToken);
}

public interface IOptimizeMutationWorkflow
{
    Task<OptimizeMutationWorkflowResult> RunAsync(
        IReadOnlyList<Tweak> selection,
        bool offerSystemRestorePoint,
        IOptimizeMutationInteraction interaction,
        IOptimizeMutationPhases phases,
        CancellationToken cancellationToken = default);
}

```

Implement `OptimizeMutationWorkflow(ITweakExecutor executor, ISystemRestorePointService restorePoints) : IOptimizeMutationWorkflow`. `RunAsync` accepts the already validated selection, setting, interaction, and phases. It runs non-mutating tweak preflight, conditionally obtains the non-mutating restore ticket, passes both evidence objects to the first confirmation, revokes the restore ticket on decline/close, calls its injected `ISystemRestorePointService.ExecuteAsync(ticketId)` after approval, and invokes the second confirmation only on restore failure/UAC cancellation. False/close returns with `MutationStarted=false`. It then executes machine phase first; false/rejection/cancellation returns without local phase. Local phase runs only after machine success.

- [ ] **Step 4: Add both reviewed dialogs**

Add:

```csharp
public Task<bool> ConfirmContinueWithoutRestorePointAsync(string failureMessage)
```

It must show the failure/UAC-cancellation message, explain that no Optimize mutation has occurred, use primary text `Continue without restore point`, close text `Cancel all changes`, default to close, and avoid language implying automatic undo.

- [ ] **Step 5: Present restore preflight in the first confirmation**

Change `ConfirmMutationAsync` to accept `OptimizeMutationReview`. When `RestorePointPreflight` is present, append a separately bordered read-only section named `System Restore preflight evidence` containing the exact ticket evidence and best-effort wording. The first confirmation therefore reviews both the tweak preflight and restore preflight before either mutation.

- [ ] **Step 6: Adapt the module to the tested workflow**

Inject `IOptimizeMutationWorkflow`, `ISystemRestorePointService`, `AppSettings`, and `ActivityHub`. Supply view-backed `IOptimizeMutationInteraction` and phase delegates wrapping existing `ApplyConfirmedAsync` internals. Split `ApplyConfirmedAsync` into explicit machine and local delegates without changing their order or semantics. Publish restore success/failure and final workflow outcome. The module contains no independent duplicate branching.

- [ ] **Step 7: Keep native automation non-mutating**

The Optimize automation continues to cancel the first preflight confirmation. Add assertions that `Offer a System Restore point before eligible Optimize changes` is present in Settings and that no restore continuation dialog appears before the first confirmation is accepted. Do not accept the first confirmation in native automation.

- [ ] **Step 8: Verify Optimize and Core failure paths**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~SystemRestorePointServiceTests|FullyQualifiedName~OptimizeMutationWorkflowTests|FullyQualifiedName~ElevationBrokerTests"
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
dotnet build apps\Sift\Sift.csproj --configuration Release
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace Optimize
```

Expected: PASS; no restore point and no Optimize mutation are performed.

## Task 6: Refactor existing history aggregation into a bounded service

**Files:**
- Modify: `apps/Sift.Core/Services/HistoryAggregator.cs`
- Modify: `apps/Sift.Core/Models/HealthCheckRow.cs`
- Test: `apps/Sift.UnitTests/HistoryServiceTests.cs`

- [ ] **Step 1: Write failing history tests**

Add:

```csharp
[Fact] public async Task Load_merges_activity_optimize_and_registry_backup_rows_newest_first();
[Fact] public async Task Load_is_bounded_to_300_rows();
[Fact] public async Task Load_reports_activity_failure_as_partial_with_backup_rows();
[Fact] public async Task Load_reports_backup_failure_as_partial_with_activity_rows();
[Fact] public async Task Load_reports_unreadable_registry_backup_as_a_warning();
[Fact] public async Task Load_honors_cancellation();
```

Use temporary directories, a fake `ITweakExecutor`, and a fake `IActivityStore`. Do not read production LocalAppData.

- [ ] **Step 2: Run red**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~HistoryServiceTests"
```

Expected: FAIL because `IHistoryService` and `HistorySnapshot` do not exist.

- [ ] **Step 3: Define the injectable result**

Add to `HealthCheckRow.cs`:

```csharp
public sealed record HistorySnapshot(
    IReadOnlyList<HistoryRow> Rows,
    IReadOnlyList<string> Warnings)
{
    public bool IsPartial => Warnings.Count > 0;
}
```

In `HistoryAggregator.cs`, replace the static class with:

```csharp
public interface IHistoryService
{
    Task<HistorySnapshot> LoadAsync(CancellationToken cancellationToken = default);
}

public sealed class HistoryService(
    ITweakExecutor executor,
    IActivityStore activityStore,
    int maximumRows = 300) : IHistoryService
```

- [ ] **Step 4: Preserve and harden existing aggregation behavior**

Move the current Optimize backup, `backup-registry-*.json`, and persisted activity projections into `HistoryService.LoadAsync`. Load backup and activity sources independently, add a concise warning for each failed source/unreadable registry backup, sort descending, and apply `Take(maximumRows)`. Validate `maximumRows` is `1..300`, check cancellation between sources and during file loops, and perform file work in `Task.Run`.

- [ ] **Step 5: Run focused tests**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~HistoryServiceTests"
```

Expected: PASS with deterministic partial results and a hard 300-row ceiling.

## Task 7: Promote Health to a dedicated checks/history workspace

**Files:**
- Create: `apps/Sift.Core/Services/HealthWorkspaceOrchestrator.cs`
- Test: `apps/Sift.UnitTests/HealthWorkspaceOrchestratorTests.cs`
- Create: `apps/Sift/Composition/HealthWorkspaceModule.cs`
- Create: `apps/Sift/Views/HealthWorkspaceView.xaml`
- Create: `apps/Sift/Views/HealthWorkspaceView.xaml.cs`
- Modify: `apps/Sift/Composition/WorkspaceSnapshotFactory.cs`
- Modify: `apps/Sift/MainWindow.xaml.cs`
- Modify: `apps/Sift.Tests/Program.cs`
- Modify: `apps/Sift/scripts/validate-ui.ps1`

- [ ] **Step 1: Add failing Health orchestration tests and vertical-slice assertions**

Add deterministic tests:

```csharp
[Fact] public async Task Latest_refresh_suppresses_noncooperative_stale_result();
[Fact] public async Task Deactivate_cancels_owned_refresh();
[Fact] public async Task Checks_failure_retains_history_rows_and_warning();
[Fact] public async Task History_failure_retains_checks_rows_and_warning();
[Fact] public async Task History_partial_result_retains_rows_and_source_warnings();
```

Use controllable fake inventories/history services and `TaskCompletionSource` barriers; assert no stale `HealthWorkspaceResult` is published after a newer generation completes.

Require the dedicated files and exact automation names:

```text
Refresh health
Show health checks
Show activity and recovery history
Filter health checks
Filter activity and recovery history
Health checks
Activity and recovery history
```

Assert `MainWindow` constructs `HealthWorkspaceModule`; assert it no longer calls `AddWorkspace("Health"`; assert only two current tabs exist so a later diagnostic tab remains additive. Also require the exact state strings `Loading health`, `No checks returned`, `No matching checks`, `No activity or recovery history yet`, `No matching history`, `Partial history`, `Could not load checks`, and `Could not load history`. These source-contract assertions cover otherwise unsafe-to-force UI failure states without creating a WinUI test project.

- [ ] **Step 2: Run red**

Run:

```powershell
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
```

Expected: FAIL because Health still uses `WorkspaceOverview`.

- [ ] **Step 3: Implement the testable Health orchestration seam**

Define:

```csharp
public sealed record HealthWorkspaceResult(
    long Generation,
    IReadOnlyList<HealthCheckRow> Checks,
    HistorySnapshot History,
    IReadOnlyList<string> Warnings,
    bool Cancelled,
    bool Stale);

public interface IHealthWorkspaceOrchestrator
{
    Task<HealthWorkspaceResult> RefreshAsync(CancellationToken cancellationToken = default);
    void Deactivate();
}

```

Implement `HealthWorkspaceOrchestrator(IHealthInventory health, IHistoryService history) : IHealthWorkspaceOrchestrator`. The orchestrator owns a lock, monotonically increasing generation, and linked `CancellationTokenSource`. Each refresh cancels/disposes the prior source, starts checks and history independently off-thread, catches each source failure into a warning, preserves the successful/partial source, and marks any noncurrent generation `Stale=true`. `Deactivate` increments generation and cancels/disposes the active source. It never publishes through WinUI collections.

- [ ] **Step 4: Build the dedicated view**

Create a `TabView` with non-closable `Checks` and `Activity / Recovery History` items. Each tab owns its own filter box, count, list, and overlay state panel. Checks show title, detail, recommendation, and status. History shows local time, category, title, and detail; it is read-only and exposes no restore button.

Add `VisualStateManager` states `Wide` and `Narrow` at 980 effective pixels. Narrow state stacks metadata and preserves readable borders/padding. Every ambiguous control and both lists receive the automation names above.

- [ ] **Step 5: Implement explicit view states**

Expose:

```csharp
public void SetLoading();
public void BindChecks(IReadOnlyList<HealthCheckRow> checks);
public void BindHistory(HistorySnapshot history);
public void SetChecksError(string message);
public void SetHistoryError(string message);
public void SetStatus(string message);
public void FocusSearch();
```

Independent filters must distinguish:

- no source rows: `No checks returned` / `No activity or recovery history yet`;
- filtered zero: `No matching checks` / `No matching history`;
- warning-bearing history: visible `Partial history` panel plus retained rows;
- source failure: `Could not load checks` or `Could not load history`;
- loading: active progress and disabled refresh.

- [ ] **Step 6: Adapt the module to the tested orchestrator**

`HealthWorkspaceModule` injects `IHealthWorkspaceOrchestrator` and `ActivityHub`. It renders only results where `Cancelled == false` and `Stale == false`, applies checks/history on the UI context, and displays warnings as partial state. `Deactivate` calls the orchestrator's `Deactivate`. `FocusPrimarySearch` delegates to the active tab's filter. The module does not duplicate generation, linked-token, or source-isolation logic.

Remove only `WorkspaceSnapshotFactory.Health`; retain Home behavior.

- [ ] **Step 7: Extend harmless Health automation**

Before app launch, preserve any existing `activity.json`, create bounded activity and valid plus deliberately malformed registry-backup fixtures with unique `Sift-UIHistory-` names, and restore originals in `finally`. In Health:

1. assert both tabs and refresh exist;
2. verify checks remain visible;
3. select history, find both valid fixture categories, and assert `Partial history` is visible for the malformed fixture while valid rows remain;
4. filter to one fixture and verify the other disappears;
5. filter to a guaranteed non-match and assert `No matching history`;
6. clear the filter;
7. resize to the enforced minimum and capture `window-health-minimum.png`.

Do not add a WinUI test project.

- [ ] **Step 8: Verify Health**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~HistoryServiceTests|FullyQualifiedName~HealthWorkspaceOrchestratorTests"
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
dotnet build apps\Sift\Sift.csproj --configuration Release
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace Health
```

Expected: PASS; wide and minimum-width Health screenshots show Checks and Activity / Recovery History without replacing Checks.

## Task 8: Persist maintenance scan completion immediately

**Files:**
- Modify: `apps/Sift/Composition/MaintenanceWorkspaceModule.cs`
- Modify: `apps/Sift/MainWindow.xaml.cs`
- Modify: `apps/Sift.UnitTests/InfrastructureTests.cs`
- Modify: `apps/Sift.Tests/Program.cs`
- Modify: `apps/Sift/scripts/validate-ui.ps1`

- [ ] **Step 1: Add failing SaveNow and source-order tests**

Add a coordinator test that schedules an older snapshot, then calls `SaveNow` with a newer `LastMaintenanceScanUtc`, waits beyond the debounce interval, and asserts exactly one persisted save containing the newer timestamp.

Add an integration assertion that `LastMaintenanceScanUtc =` is followed by `_persistence.SaveNow(_settings)` in `MaintenanceWorkspaceModule`.

- [ ] **Step 2: Run red**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~InfrastructureTests.SettingsPersistence"
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
```

Expected: unit behavior passes or exposes a stale pending-save defect; integration fails because Maintenance does not call `SaveNow`.

- [ ] **Step 3: Inject and flush**

Add `SettingsPersistenceCoordinator persistence` to the Maintenance constructor, store `_persistence`, and immediately call:

```csharp
_settings.LastMaintenanceScanUtc = DateTime.UtcNow.ToString("O");
_persistence.SaveNow(_settings);
```

only after a successful, non-cancelled scan has been bound. Failed and cancelled scans must not update the timestamp.

- [ ] **Step 4: Prove persistence through native execution**

In Maintenance UI automation, after the initial scan completes, read `settings.json` and assert `LastMaintenanceScanUtc` is present and parseable before closing the app. Preserve and restore the original settings file as the script already does.

- [ ] **Step 5: Verify Maintenance**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release --filter "FullyQualifiedName~InfrastructureTests"
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
dotnet build apps\Sift\Sift.csproj --configuration Release
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace Maintenance
```

Expected: PASS and the on-disk timestamp exists before app shutdown.

## Task 9: Compose shared services and perform migration-safe cleanup

**Files:**
- Modify: `apps/Sift/Composition/WinUiAppServices.cs`
- Modify: `apps/Sift/MainWindow.xaml.cs`
- Modify: `apps/Sift.Core/Models/AppSettings.cs`
- Delete: `apps/Sift.Core/Services/TelemetryHub.cs`
- Modify: `apps/Sift.Tests/Program.cs`

- [ ] **Step 1: Add failing composition and migration assertions**

Update integration checks to require public production properties:

```text
Settings
TaskActions
ScheduledTaskWorkflow
RestorePoints
OptimizeWorkflow
History
HealthOrchestrator
```

and reject public `StorageDeleter`. Add a settings migration fixture containing all removed properties plus `"UnknownFutureProperty":{"nested":true}`; load it and assert retained settings still deserialize.

- [ ] **Step 2: Run red**

Run:

```powershell
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
```

Expected: FAIL at composition cleanup and old-settings tolerance assertions.

- [ ] **Step 3: Build one shared production graph**

In `CreateDefault`, construct exactly one each of `ActivityStore`, `AppSettings`, `StorageDeleter`, `ScheduledTaskController`, `TweakExecutor`, `ElevationBroker`, and `SettingsPersistenceCoordinator`. Add:

```csharp
public required AppSettings Settings { get; init; }
public required IScheduledTaskActionService TaskActions { get; init; }
public required IScheduledTaskActionWorkflow ScheduledTaskWorkflow { get; init; }
public required ISystemRestorePointService RestorePoints { get; init; }
public required IOptimizeMutationWorkflow OptimizeWorkflow { get; init; }
public required IHistoryService History { get; init; }
public required IHealthWorkspaceOrchestrator HealthOrchestrator { get; init; }
```

Retain every existing required property—`Tweaks`, `Processes`, `MaintenanceScanner`, `MaintenanceCleaner`, `StorageScanner`, `StorageDeletion`, `Health`, `Services`, `Tasks`, `Startup`, `InstalledApps`, `InstalledAppManager`, `InstalledAppTrust`, `AppLeftovers`, `SystemInformation`, `GuardedActions`, `Elevation`, `Recovery`, `SettingsStore`, `Activity`, `Operations`, and `SettingsPersistence`—unchanged. Remove only the public required `StorageDeleter` property. Pass the one local `StorageDeleter` instance to `StorageSelectionDeletionManager` and `InstalledAppManager`. Keep `ScheduledTaskInventory` parameterless. Build task/restore/workflow/history/Health orchestration objects from shared controller, broker, executor, activity store, and inventory instances only in `CreateDefault`.

Change `MainWindow` to use `_services.Settings`, pass new services into Task Manager, Optimize, Health, and Maintenance, and stop loading settings a second time.

- [ ] **Step 4: Remove only audited obsolete settings**

Delete exactly these properties from `AppSettings`:

```text
OptimizeCategory
OptimizeRiskFilter
ChartFps
VisibleColumns
CpuFilterIndex
MemoryFilterIndex
StatusFilterIndex
ArchitectureFilterIndex
PriorityFilterIndex
PendingOptimizeSelectionIds
```

Keep `HomeWidgets` and `ChartSmoothing`. Remove the old `VisibleColumns` integration assertion. Do not enable strict unknown-member handling; `System.Text.Json` must continue ignoring unknown old JSON properties.

- [ ] **Step 5: Delete only unused telemetry**

Delete `TelemetryHub.cs`. Confirm no production or test source references it. Do not change `ProcessSampler`, Performance's bounded history, chart cadence, or LiveCharts2.

- [ ] **Step 6: Run cleanup checks**

Run:

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
dotnet build apps\Sift\Sift.csproj --configuration Release
```

Expected: all PASS with zero warnings; old JSON loads retained values; shared storage deletion lifetime remains one instance.

## Task 10: Run targeted behavior and safety verification

**Files:**
- Verify all production and test files above.
- Generated only: `apps/Sift/artifacts/**`

- [ ] **Step 1: Run all unit tests**

```powershell
dotnet test apps\Sift.UnitTests\Sift.UnitTests.csproj --configuration Release
```

Expected: PASS, including task stale-state/hash/single-use checks, parameter-free restore policy, broker cancellation/failure, history partial/bounds, and persistence.

- [ ] **Step 2: Run live non-mutating integration validation**

```powershell
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
```

Expected: PASS; protected tasks remain rejected and no task/restore mutation occurs.

- [ ] **Step 3: Build Release**

```powershell
dotnet build apps\Sift\Sift.csproj --configuration Release
```

Expected: `Build succeeded.` with `0 Warning(s)` and `0 Error(s)`.

- [ ] **Step 4: Run targeted native UI checks**

Extend `validate-ui.ps1` so `Home`, `Performance`, `Startup`, `Task Manager`, `Optimize`, `Health`, and `Maintenance` each produce both their normal screenshot and a `window-<route>-minimum.png` screenshot after resizing to the enforced minimum, then restore 1500×920 before the next route. Dialog captures remain separate.

```powershell
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace Home
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace Performance
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace Startup
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace 'Task Manager'
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace Optimize
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace Health
apps\Sift\scripts\validate-ui.ps1 -Configuration Release -NoBuild -OnlyWorkspace Maintenance
```

Expected: all PASS. Task Manager may emit the documented natural-availability skip. No script confirms a task mutation, Optimize apply, or restore-point creation.

- [ ] **Step 5: Record targeted evidence for documentation**

Record exact passing command names and generated artifact paths for Task 11. Do not run the complete `validate.ps1`, release publish, or final `git diff --check` until the implementation audit/docs are updated.

## Task 11: Reconcile audit and implementation documentation

**Files:**
- Modify: `apps/Sift/docs/audits/sift-feature-audit.json`
- Modify: `apps/Sift/ARCHITECTURE.md`
- Modify: `apps/Sift/ROADMAP.md`
- Modify: `apps/Sift/CHANGELOG.md`
- Modify: `apps/Sift/scripts/validate-feature-audit.ps1`

- [ ] **Step 1: Update the canonical audit only after targeted behavior passes**

Make these exact audit changes:

- mark `capability.scheduled-task-actions`, `capability.restore-point-helper`, `capability.history-aggregation`, `setting.OfferSystemRestorePoint`, and `setting.LastMaintenanceScanUtc` as `wired`;
- add automated evidence from the new unit tests, integration checks, and targeted UI runs;
- add `normal`, `loading`, `empty`, `filtered-empty`, `partial`, `error`, `accessibility`, and `minimum-width` evidence to Health where implemented;
- remove setting entries for the ten deleted `AppSettings` properties;
- keep `service.StorageDeleter`, change its status to `intentionally-internal`, record that one shared local instance feeds `StorageDeletion` and `InstalledAppManager`, and attach source/integration evidence proving only its public composition property was removed;
- keep `capability.telemetry-hub` as `obsolete`, set its mutation contract to `resolved by removal`, replace its source path with resolved-removal evidence (`TelemetryHub.cs` absent and no references), and do not relabel it wired;
- add service entries `Settings`, `TaskActions`, `ScheduledTaskWorkflow`, `RestorePoints`, `OptimizeWorkflow`, `History`, and `HealthOrchestrator`;
- add elevation entries `ChangeScheduledTask` and `CreateSystemRestorePoint`;
- update Health route composition to `HealthWorkspaceModule`/`HealthWorkspaceView`;
- keep `HomeWidgets` and `ChartSmoothing` assigned to their later owner plans;
- keep roadmap rows `future` where later roadmap gates remain.

- [ ] **Step 2: Teach the audit validator about internal service evidence**

Keep public `required` properties compared exactly against audit `kind: service` entries whose status is not `intentionally-internal`. Separately require every `intentionally-internal` service entry to have nonempty composition/core-boundary/automated evidence and to be absent from the public required-property set. This permits `service.StorageDeleter` to remain auditable without recreating public exposure.

- [ ] **Step 3: Update architecture without claiming guarantees**

Document single-task typed identity/hash/state revalidation, parameter-free restore-point elevation, best-effort continuation, Health's additive tab model, bounded partial history, immediate maintenance metadata flush, and internal shared `StorageDeleter`. State explicitly that System Restore availability and restore success are not guaranteed.

- [ ] **Step 4: Update roadmap and changelog**

Update current-state text for Task Manager, Health, Maintenance, Settings/dialogs, and Core safety. Preserve later gates: process-tree/export, diagnostic history/deep links, maintenance progress/cancel/cleanup history, Home widgets, chart smoothing/persistence, and recovery integrity envelope.

Add concise Unreleased changelog sections for disconnected wiring and cleanup. Do not claim a task was changed or a restore point was created during validation.

- [ ] **Step 5: Run targeted documentation-aware checks**

```powershell
apps\Sift\scripts\validate-feature-audit.ps1
dotnet run --project apps\Sift.Tests\Sift.Tests.csproj --configuration Release
```

Expected: audit validator reports a positive entry count and integration passes. The complete gate and final formatting check remain Task 12.

## Task 12: Run final acceptance and self-review

- [ ] **Step 1: Build the self-contained release folder**

Run:

```powershell
apps\Sift\build-release.ps1
$release = 'apps\Sift\dist\Sift'
foreach ($required in @(
    'Sift.exe',
    'Sift.dll',
    'Sift.pri',
    'App.xbf',
    'MainWindow.xbf',
    'ElevationHost\Sift.ElevationHost.exe'
)) {
    if (-not (Test-Path -LiteralPath (Join-Path $release $required) -PathType Leaf)) {
        throw "Self-contained release file is missing: $required"
    }
}
if ((Get-ChildItem -LiteralPath (Join-Path $release 'ElevationHost') -Filter 'Sift.ElevationHost.runtimeconfig.json' -File).Count -ne 0) {
    throw 'Single-file helper output unexpectedly contains a framework-dependent runtimeconfig.'
}
```

Expected: publish succeeds, app resources exist, and the isolated self-contained one-shot helper executable exists. This is layout evidence only; do not describe the executable or folder as signed.

- [ ] **Step 2: Run complete validation after docs/audit**

```powershell
apps\Sift\scripts\validate.ps1
```

Expected: `Sift validation completed successfully.` after the updated audit and documentation are in place.

- [ ] **Step 3: Perform required visual inspection**

Inspect generated wide and narrow/minimum screenshots for `Home`, `Performance`, and `Startup` in addition to every affected state:

- Task Manager normal, no-selection disabled controls, selected allowlisted task when naturally present, task confirmation, and cancellation;
- Optimize tweak preflight confirmation, System Restore preflight evidence inside that dialog, deterministic restore failure/UAC-cancellation wording, and `Continue without restore point?` with both decline/close behavior;
- Health loading, Checks normal, history normal, empty, filtered-empty, partial, error, accessibility names/focus order, and narrow layout;
- Maintenance loading, successful scan timestamp/status, confirmation/cancellation, and narrow layout;
- Settings restore-point toggle and readable help text;
- all affected ContentDialogs at wide and narrow widths.

Verify graphite/clay/sage colors, explicit dark-theme foregrounds, deliberate borders/padding, text wrapping, keyboard focus, automation names, and no clipping. Use deterministic non-production orchestration/controller fakes to render restore failure and error states; do not accept any action that changes a real task, applies Optimize, or creates a real restore point.

- [ ] **Step 4: Run final formatting check**

```powershell
git diff --check
```

Expected: exit code `0`.

- [ ] **Step 5: Map every scope item to implemented evidence**

Confirm scheduled-task wiring maps to Tasks 1, 2, and 4; restore behavior to Tasks 2, 3, and 5; history/Health to Tasks 6 and 7; maintenance persistence to Task 8; cleanup to Task 9; post-verification docs to Task 11.

- [ ] **Step 6: Check type/signature consistency**

Confirm all call sites use `ScheduledTaskId`, `ScheduledTaskChange`, `ServiceActionKind`, `ScheduledTaskActionPreflight`, `SystemRestorePointInspection`, `SystemRestorePointPreflight`, `SystemRestorePointResult`, `OptimizeMutationReview`, `HistorySnapshot`, and `HealthWorkspaceResult` with the exact signatures declared earlier. Confirm service action strings exist only inside `WindowsServiceMonitor.Act`, no task path crosses elevation, and no restore method/request accepts a description.

- [ ] **Step 7: Check safety boundaries**

Confirm there is no bulk task API, arbitrary task path/name/command, WMI text payload, restore description payload, security/Windows Update allowlist entry, preview toggle, real-time priority, permanent deletion, remote script, runtime download, or weakened helper-side revalidation.

- [ ] **Step 8: Check later-plan boundaries**

Confirm this slice does not consume `HomeWidgets`, implement chart smoothing, add process-tree/export, add diagnostic history/deep links, move Optimize restore into Recovery, add recovery integrity envelopes, or introduce a WinUI test project.

- [ ] **Step 9: Review all ten repair findings**

Check each repair explicitly:

1. typed restore preflight ticket/evidence, one-use consumption, and revalidation;
2. non-WinUI cancellation/UAC/machine-rejection workflow tests;
3. testable Health latest-wins/deactivation/partial orchestration;
4. typed service action through every layer;
5. exact elevation response mappings and tests;
6. one identity catalog and parameterless task inventory;
7. all existing composition properties retained except public `StorageDeleter`;
8. internal StorageDeleter and obsolete-resolved TelemetryHub audit records retained;
9. targeted verification precedes docs, complete validation follows docs;
10. required route/dialog visuals plus unsigned self-contained release layout.

- [ ] **Step 10: Check document completeness**

Confirm every changed file and symbol is named, every test command has an expected result, no unfinished marker remains, and no commit command appears.
