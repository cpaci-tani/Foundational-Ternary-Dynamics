# Sift product language, permission, and architecture audit

**Status:** Active  
**Started:** 2026-07-16  
**Scope:** WinUI presentation, user-triggered actions, permission and elevation behavior, Core execution ownership, composition, lifecycle, validation, and maintained documentation.

## Objective

Make Sift direct, neutral, and customer-facing. Normal screens should describe capabilities, current state, actions, consequences, and required Windows permission without narrating implementation details.

This audit removes safety theater and defensive prose. It does not remove the project’s execution contract: explicit targets, automatic non-mutating review, confirmation for mutations, immediate target checking, prior-state capture where applicable, scoped administrator requests, and protected Windows-target exclusions remain enforced below the UI.

## Product-language standard

Customer copy follows this order:

1. capability;
2. current state;
3. material consequence, only when relevant;
4. Windows permission, only when required.

Normal UI must not expose architecture vocabulary such as `Core policy`, `one-shot helper`, `nonce`, `allowlisted`, `provider-isolated`, `bounded`, `exact target`, `automatic preflight`, or `live revalidation`. Technical evidence belongs in an optional details surface, the activity log, architecture documentation, or tests.

### Controls

- Buttons use a verb and object: `Apply selected`, `Clean selected`, `Move to Recycle Bin`.
- A button does not say `Review and …`; selecting it opens the review/confirmation surface when confirmation is required.
- Empty states explain the next available action, not an internal constraint.
- Status text uses present activity or result: `Checking 4 changes`, `Waiting for administrator permission`, `Applied 4 changes`.

### Confirmation dialogs

- Title: `[Verb] [target]?`
- Scope: the concrete target and proposed change.
- Consequence: only data loss, restart/sign-out, privacy exposure, duration, or inability to restore.
- Permission: `Windows will ask for administrator permission before this action starts.` only when true.
- Primary button: the exact action; secondary button: `Cancel`.
- Implementation evidence is optional details, not the lead paragraph.

### Terms

| Avoid in customer UI | Use instead |
|---|---|
| safe | Standard, Recommended, or High confidence |
| protected | View only or Administrator required |
| automatic preflight | Checking or Review |
| allowlisted | Supported or Available |
| blocked by Core policy | Action unavailable, followed by the concrete reason |
| exact target | Display the actual target |
| bounded | State the actual limit |
| one-shot elevation helper | Windows administrator permission |
| revalidate | Checking again, only when useful as status |
| no automatic undo | Cannot be restored by Sift |

## Execution invariants

These are acceptance requirements, not customer-facing slogans:

- The WinUI process remains `asInvoker`; administrator work uses a typed, scoped request.
- Every mutation retains non-mutating review, explicit confirmation, and immediate execution-time target checks.
- Registry changes retain prior value and kind before mutation.
- Process, service, task, storage, recovery, uninstall, and script actions retain their Core exclusions.
- Read-only actions do not receive redundant blocking confirmation dialogs.
- UAC is requested only for an action that needs administrator access, and the prompt follows the user’s selection.
- Cancellation before mutation is reported as cancellation, not failure or partial execution.
- Read-only workspaces and local preference changes do not request administrator access.

## Baseline findings

The prose inventory covered 17 XAML surfaces, 14 view code-behind files, 17 composition modules, customer-facing Core results, 117 Script Studio recipes, 64 Optimize actions, and the main maintained documents. Visible source currently contains approximately 120 `preflight`, 94 `confirm`, 72 `elevat*`, 66 `review`, 50 `exact`, and 32 `blocked` references. Many are legitimate internal evidence; the audit must remove them only from routine customer presentation.

### Permission and action matrix

| Area | Current permission model | Assessment | Required change |
|---|---|---|---|
| Shell, activity, Performance, Health, System Information, Settings | Current user; no confirmation | Appropriate | Keep permission-free; simplify copy |
| Hardware Monitor | LibreHardwareMonitor 0.9.6 opens Ring0 by default when privileges allow | Corrected for current passive mode | Block the provider in elevated sessions; design explicit driver-backed monitoring around the upstream opt-out before enabling it |
| Optimize per-user settings and Appx | Review + confirmation; current user | Appropriate | Simplify review and outcome copy |
| Optimize machine settings and repairs | Review + confirmation + scoped UAC | Appropriate boundary | Fix production self-cancellation; preserve distinct cancellation state |
| Optimize restore latest | Generic confirmation; direct current-token restore | Incorrect | Route to Recovery inspection, revalidation, and scoped UAC, or navigate to Recovery |
| Maintenance per-user cleanup | One-use reviewed-content ticket + confirmation; current user | Corrected | Preserve whole-selection drift rejection and ticket consumption |
| Maintenance HKLM orphan cleanup | Hidden from standard-user inventory; available only in an administrator session; Core independently enforces permission | Corrected without broadening elevation payloads | Preserve session boundary unless a typed exact-registration operation is added later |
| Installed-app uninstaller | Review + confirmation; vendor/Windows owns later UAC | Appropriate | Simplify copy; keep consequence and vendor handoff clear |
| Installed-app registration cleanup | Review + backup + confirmation; elevated-session HKLM | Appropriate under current session model | Decide whether to add on-demand UAC consistently |
| AppData leftovers | Review + Recycle Bin | Mostly appropriate | Decide whether folder-content drift must invalidate confirmation |
| End/restart process | Review + confirmation; current token | Corrected | PID, start time, session, name, and executable path are checked before review and execution |
| Start/restart service | Review + confirmation + scoped UAC | Corrected | Reviewed Stopped/Running state is typed through Core, broker, request, helper, and controller |
| Enable/disable scheduled task | Expiring ticket + confirmation + scoped UAC | Reference implementation | Preserve |
| Storage cleanup | Complete inventory ticket + confirmation + Recycle Bin | Reference implementation | Preserve |
| Recovery | Backup inspection + confirmation + scoped UAC | Corrected | Preserve distinct administrator-permission cancellation and Recovery ownership |
| Read-only Script recipes | Core check + direct execution | Corrected | Preserve catalog identity and command checks without a redundant dialog |
| State-changing Script recipes | Confirmation; administrator recipes use typed `RunCatalogRecipe` (ID + hash) | Corrected | Never send raw command text across elevation; authored scripts stay non-executable |

## Architecture findings

### P0 — functional (corrected)

`OptimizeWorkspaceModule` previously nested `OperationCoordinator` calls under the same `workspace.optimize.mutate` key, so the inner phase cancelled the outer workflow. The outer module is now the sole coordinator owner; the production Core phase adapter awaits its machine and current-user execution boundaries directly, returns typed cancellation/failure state, and is covered through the real coordinator/workflow/adapter composition.

### P1 — permission and execution correctness

1. ~~Retire or reroute Optimize’s legacy `Restore latest` path.~~ Completed: Optimize now carries navigation intent only and Recovery owns restore.
2. ~~Correct Maintenance HKLM authorization and bind permanent cleanup to reviewed contents.~~ Completed: standard sessions exclude HKLM and cleanup uses one-use content-bound tickets.
3. ~~Bind service restart to the confirmed service state.~~ Completed: a typed expected state crosses Core and elevation; all execution paths reject drift.
4. ~~Preserve UAC cancellation separately from failure/partial execution.~~ Completed for Optimize, Recovery, service, task, and restore-point flows.
5. ~~Decide a just-in-time model for exact administrator Script recipes without elevating authored scripts or the whole shell.~~ Completed: typed `RunCatalogRecipe` elevation carries recipe ID + hash only; authored scripts remain non-executable.

### P1 — composition and lifecycle

- ~~`MainWindow` constructed every workspace and a concrete folder-picker adapter.~~ Corrected: a typed registry factory owns module construction while the window owns navigation and activation only.
- ~~`WinUiAppServices` exposed unused or duplicate services and had ambiguous disposal ownership.~~ Corrected: construction-only services remain local, App owns application services, MainWindow owns the registry, and the registry owns its modules.
- ~~Performance and Hardware Monitor use overlapping `async void` timer ticks.~~ Corrected: sampling is serialized, periodic ticks skip active work, and timers start after the initial sample.
- ~~Overview and Startup subscribe anonymous handlers that cannot be detached by their empty `Dispose` methods.~~ Corrected with named handlers and deterministic unsubscription.

### P2 — maintainability

- ~~Add WinUI clipboard and shell-launch adapters instead of duplicating `DataPackage` and `Process.Start` behavior.~~ Completed: narrow injected adapters now own text-copy and named Windows shell handoffs; views and modules no longer launch processes or call WinUI clipboard APIs directly.
- ~~Split Script Studio’s view host/bridge/library/terminal responsibilities.~~ Completed with responsibility-specific view partials and shared clipboard/shell adapters.
- ~~Split Installed Apps inventory, uninstall tracking, and leftover workflows.~~ Completed with three explicitly disposed controllers, a tested identity-bound continuation state, and view partials for inventory, dialogs, and leftovers.
- Follow-up: organize the flat Core service directory by feature or responsibility when a namespace-compatible migration can be reviewed separately.
- Follow-up: move static customer copy toward `.resw` resources and expand typed Core reason-code-to-presentation mapping. This is localization debt, not a reason to expose implementation terminology in current UI.

### Documentation and evidence drift

- Resolved: the feature-audit validator now checks roadmap IDs, source/evidence paths, owner-plan provenance, maintained-document links, current visual-state evidence, and application/helper version alignment.
- Resolved: stale service filenames, the stray `roadmap.settings` ID, the thirteen-route label, and unsupported current Health/Hardware Monitor visual-state claims were corrected in the manifest.
- Resolved: the 2026-07-14 validation baseline is explicitly retained as historical evidence and no longer represents the current application.
- Resolved: the program index records which proposed plans were never created and delegates current status to this audit, `ROADMAP.md`, and the feature manifest.
- Resolved: Sift and `Sift.ElevationHost` now share product version 0.15.0.
- Resolved: README is the customer guide; permission/enforcement and build/release procedures now have dedicated maintained documents.

## Implementation chunks

### Chunk 0 — baseline and language contract

**Deliverables:** this audit, complete action/permission inventory, visible-copy standard, explicit execution invariants.  
**Gate:** every action has an owner, permission model, confirmation rule, and authoritative enforcement location.

### Chunk 1 — shell and primary workspace copy

Remove marketing badges, safety slogans, implementation-led subtitles, `Review and …` button labels, visible `Safe` labels, and repeated read-only/confirmation narration. Preserve material consequence and permission sentences.  
**Gate:** Release build; source scan for forbidden visible terms; native screenshots of all changed workspaces.

### Chunk 2 — mutation correctness

Fix Optimize production coordination, retire/reroute legacy restore, correct Maintenance HKLM behavior and content binding, bind service expected state, and bind process instance identity.  
**Gate:** behavior-level tests using production adapters; cancellation and stale-target tests; native confirmation cancellation.

### Chunk 3 — permission experience

Remove blocking confirmation for read-only recipes; standardize the administrator sentence; preserve cancellation as a first-class outcome; decide exact-recipe just-in-time UAC; review double-UAC Optimize behavior.  
**Gate:** one permission decision per action, controlled UAC negative-path tests, no raw command/path elevation payloads.

### Chunk 4 — secondary copy and typed outcomes

Rewrite confirmations, statuses, empty states, activity summaries, and Core-originated customer messages. Introduce typed outcome/reason codes and presentation mapping; move static copy to resources.  
**Gate:** no routine UI exposes internal architecture terms; consequence/privacy/permission copy remains where required.

### Chunk 5 — composition and lifecycle

Extract workspace registry construction, shrink service roots, standardize non-overlapping sampling and handler disposal, add clipboard/shell adapters, and split Script Studio/Installed Apps hotspots.  
**Gate:** MainWindow is shell-only; each dependency has one owner; inactive workspaces have no live sampling or attached handlers.

### Chunk 6 — documentation and evidence

Separate customer guide from architecture and release docs; reconcile README, ARCHITECTURE, ROADMAP, CHANGELOG, program index, versions, and validation baseline. Harden audit validation to resolve references and require evidence for claimed states.  
**Gate:** every maintained claim resolves to current source, test, or artifact.

### Chunk 7 — completion audit

Run unit, integration, Release build, native UI, package-layout, copy-term, reference-resolution, and `git diff --check` gates. Inspect changed dialogs and all fourteen workspaces at normal and narrow width.  
**Gate:** requirement-by-requirement evidence demonstrates the objective; no completion claim is based only on source-string assertions.

## Progress

- [x] Parallel prose, permission, and architecture audits completed.
- [x] Language and execution contracts recorded.
- [x] Completed Chunk 1 shell badges, workspace subtitles, primary action labels, statuses, empty states, tooltips, Settings heading, and visible `Safe` risk-label cleanup.
- [x] Disabled passive Hardware Monitor sampling in elevated sessions so workspace activation cannot implicitly install/open LibreHardwareMonitor's Ring0 driver.
- [x] Completed the all-workspace native screenshot audit and the visible-source forbidden-term scan.
- [x] Corrected Optimize coordinator ownership, typed phase cancellation, cancellation boundaries, and production-composition coverage.
- [x] Removed the duplicate Optimize restore API and routed its backup link to Recovery.
- [x] Corrected Maintenance authorization and content binding: standard-user inventory excludes HKLM registrations, Core rejects forged HKLM findings, and permanent cleanup requires an unexpired one-use reviewed-content ticket with whole-selection drift rejection.
- [x] Bound Task Manager process actions to PID plus start time/session/name/path and service actions to the reviewed Stopped/Running state; added drift, cancellation, request-shape, and protected-target tests.
- [x] Reconciled Chunk 6 customer, architecture, roadmap, release, program-index, baseline, version, and feature-audit documentation; added reference and visual-evidence validation.
- [x] Extracted Chunk 5 workspace construction and ownership into a registry factory/owner; MainWindow now receives narrow shell services, validates route parity, and disposes the registry as one lifetime boundary.
- [x] Added narrow WinUI clipboard and Windows shell-launch adapters and routed their consumers through workspace composition.
- [x] Removed the read-only Script recipe confirmation and landed typed `RunCatalogRecipe` elevation for administrator catalog recipes.
- [x] Serialized Performance and Hardware Monitor sampling and corrected Overview/Startup handler disposal.
- [x] Split Installed Apps inventory, uninstall tracking, and leftover cleanup ownership; centralized its Windows Settings handoff and added continuation-identity behavior coverage.
- [x] Split Script Studio library, terminal, WebView bridge/lifecycle, and responsive layout ownership while preserving its public view contract and automation names.
- [x] Hardware Monitor panels stretch across the available workspace; identity-keyed device and sensor collections are reconciled in place, chart animation is disabled, and periodic samples notify only changed reading properties.
- [x] Completion evidence: 179 unit tests, integration/source validation, a warning-free Release build with stale-output probes, 93 feature-manifest entries, focused native UI for Hardware Monitor, Script Studio, Installed Apps, and System Information, and repository whitespace validation.
