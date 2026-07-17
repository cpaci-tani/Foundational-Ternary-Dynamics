# Sift Full-Roadmap Audit and Completion Design

**Date:** 2026-07-14  
**Status:** Approved design; program index and completion agent created  
**Scope:** Audit every presented feature and complete every item currently recorded in `ROADMAP.md`, while preserving the safety and architecture boundaries in `AGENTS.md`.

## 1. Objective

Produce an evidence-backed feature inventory, repair disconnected existing capabilities, implement the remaining roadmap as isolated vertical slices, and finish with a repeatable acceptance gate covering behavior, safety, accessibility, packaging, and visual states.

Completion does not relax Sift's deliberate exclusions. Permanent deletion, broad or bulk mutation, unsafe service/task operations, remote execution, runtime downloads, preview-mode toggles, and weakened elevation checks remain out of scope.

## 2. Initial audit findings

The application already has a coherent twelve-route WinUI shell, a presentation-neutral Core, a sole production composition root, bounded background operations, and strong mutation guards. Existing validation includes unit tests, a live-Windows console harness, native UI Automation traversal, release publishing scripts, MSIX construction, and clean-account package validation.

The initial static audit identified these disconnected or incomplete surfaces:

- Core scheduled-task mutation has no Task Manager presentation path.
- `OfferSystemRestorePoint` is persisted, and a restore-point helper exists, but no production operation consumes the setting.
- Activity persistence and `HistoryAggregator` exist without a history presentation path.
- Several legacy settings and `TelemetryHub` are not consumed by production behavior.
- Process CSV export exists only in Core/tests.
- Maintenance scan time is not flushed immediately.
- Home, Performance, Startup, Health, Settings, activity-console behavior, keyboard routing, and performance lifecycle have shallow native automation coverage.
- Normal validation does not currently prove self-contained publish or package layout.
- Screenshot capture exists, but visual acceptance is manual and has no automated assertion layer.

These findings are starting evidence, not the final audit inventory.

## 3. Program structure

### Stage A: Baseline and traceability

Create a feature matrix covering every route, control, command, setting, Core capability, persistence path, elevation operation, and roadmap row. For each item, record its composition path, owning module/view, policy boundary, test evidence, visual states, and status: wired, intentionally internal, future work, disconnected, blocked, or obsolete.

Run the existing unit, integration, Release build, native UI, and diff gates before changes. Preserve unrelated repository changes.

### Stage B: Existing wiring repair

Connect capabilities already implemented below the UI when doing so is consistent with the roadmap and safety policy:

- expose conservative scheduled-task actions through Task Manager with explicit single selection, automatic preflight, reviewed confirmation, and live revalidation;
- consume the restore-point preference only at appropriate guarded mutation boundaries, with failure presented as non-fatal evidence rather than a safety guarantee;
- expose persisted activity/backup history through an owned workspace surface;
- make maintenance scan persistence deterministic;
- either wire each legacy setting to documented behavior or remove it through migration-safe cleanup;
- remove redundant composition exposure without changing service lifetime.

### Stage C: Roadmap feature slices

Implement each roadmap row as its own Core-to-view vertical slice:

- shell accessibility completion;
- activity export and direct resize affordance;
- configurable Home cards, sparklines, and workspace click-through;
- migration of Optimize's legacy restore shortcut into Recovery;
- Task Manager process-tree grouping and export;
- persisted Performance controls and process click-through;
- Startup narrow/accessibility automation;
- Maintenance progress, cancellation, and cleanup history;
- Health deep links and persisted diagnostic history;
- Storage scan-error summaries and picker automation;
- Installed Apps history/export under an explicit privacy and retention contract;
- System Information versioned signed JSON export and privacy-reviewed EDID decoding;
- Settings/UAC-cancellation coverage;
- packaging and clean-account release acceptance.

Bulk Task Manager mutation and permanent Storage deletion remain excluded as required by the roadmap and Core contract.

### Stage D: Privileged and release hardening

Design a versioned privileged-integrity envelope for eligible machine-level recovery entries. The helper must authenticate the envelope, independently revalidate the machine and allowlisted operation, reject replay/stale/foreign inputs, and retain protected-phase-first atomicity. This is a separate security-sensitive slice with adversarial tests.

Integrate self-contained publish and unsigned MSIX layout checks into normal validation. Signed package and clean-account trust acceptance remain mandatory release gates, but they report a precise external blocker if a trusted publisher certificate is unavailable. The project must never claim an artifact is signed unless verification succeeds.

### Stage E: Final acceptance

Rerun the complete feature matrix and all automated gates. Visually inspect every route and every popup/dialog at wide and narrow widths. Reconcile `ROADMAP.md`, `ARCHITECTURE.md`, automation names, release documentation, and user-facing labels with the implemented state.

## 4. Architecture contract

- `MainWindow` owns only shell, navigation, title-bar, keyboard, and activity-console concerns.
- Each workspace implements `IWorkspaceModule`, owns one view, and explicitly starts/stops its work.
- `WinUiAppServices` remains the only default production composition root.
- Core owns models, scanners, persistence, guards, preflight tickets, execution, rollback, and revalidation.
- Modules own activation, latest-wins cancellation, orchestration, and UI-thread application of results.
- Views own presentation, explicit intent collection, accessibility metadata, and confirmation dialogs.
- Read-only providers are finite, cancellable, and tolerant of optional partial failure.
- New persistence has a bounded schema plus explicit privacy, retention, and migration behavior.

## 5. Mutation data flow

Every mutation follows this fixed sequence:

1. The user selects one explicit target or reviewed bounded batch.
2. Core performs a non-mutating preflight and returns typed evidence.
3. The UI presents that exact evidence in a confirmation dialog.
4. Cancellation leaves the target unchanged.
5. Core immediately revalidates identity, policy, and current state.
6. Prior state is captured before mutation.
7. Execution publishes typed activity and either succeeds or reports bounded rollback/recovery evidence.

No roadmap feature may bypass this sequence or expose a preview-mode toggle.

## 6. Failure handling

- Cancelled or superseded work cannot publish stale results.
- Optional scanner/provider failures produce partial results with visible warnings.
- Persistence uses bounded schemas and atomic same-directory replacement.
- Export or signing failure leaves no artifact labeled successful.
- UAC cancellation prevents any dependent unprivileged phase from applying.
- Elevation requests contain only allowlisted typed identifiers and independently revalidated bounded data.
- Background errors stop owned timers/work and remain observable without crashing the shell.
- Empty, loading, filtered-empty, partial, error, and cancellation states use explicit user-facing text.

## 7. Validation contract

A feature is complete only when normal, empty, loading, filtered, error, confirmation, cancellation, narrow, and accessibility states are implemented and verified where applicable.

Required gates:

- deterministic Core tests for every new policy and persistence contract;
- tests proving preflight non-mutation, confirmation cancellation, stale-target rejection, prior-state capture, backup ordering, and rollback for every mutation;
- module/lifecycle coverage for timers, latest-wins cancellation, history bounds, and stale-result suppression;
- route-specific native UI assertions for all twelve routes, the activity console, dialogs, keyboard routing, settings propagation, and narrow layouts;
- Release build with zero warnings;
- self-contained publish layout verification;
- unsigned MSIX structure verification in normal CI/local validation;
- signed MSIX and clean-account install/elevation/launch/upgrade/uninstall verification for release acceptance;
- `git diff --check`;
- manual visual inspection of generated wide/narrow screenshots, including popups and dialogs.

## 8. Deliverable decomposition

The full roadmap will be implemented through multiple plans, each producing working, testable software:

1. audit matrix and validation baseline;
2. disconnected capability wiring;
3. shell, activity, Home, and Settings;
4. Task Manager and Startup;
5. Performance, Maintenance, and Health;
6. Recovery integrity envelope and Optimize migration;
7. Storage and Installed Apps;
8. System Information exports and EDID;
9. packaging, clean-account validation, full visual/accessibility acceptance, and documentation reconciliation.

Dependencies may refine this ordering, but safety-sensitive recovery work and release claims cannot be merged into unrelated UI batches.

## 9. External blocker policy

A trusted publisher certificate is not generated or simulated by the application. If none is available, implementation can complete signing support, unsigned package verification, and negative-path tests, but signed-artifact and clean-account trust gates remain explicitly blocked. No other roadmap item is considered complete merely because it depends on unavailable external credentials.

## 10. Success criteria

- Every presented control and setting has a traceable production behavior or is deliberately removed.
- Every Core capability is consumed, intentionally internal, or documented as retired.
- Every roadmap row is implemented and validated, or carries a precise external blocker.
- Safety boundaries remain enforced below the UI.
- All automated gates pass with zero warnings.
- All required visual states are inspected.
- Architecture and roadmap documentation match the shipped application.
