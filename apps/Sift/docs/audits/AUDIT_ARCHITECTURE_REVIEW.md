# Architecture review audit

Status: active. Scope: the full Sift solution (`Sift` WinUI shell, `Sift.Core`, `Sift.ElevationHost`,
`Sift.MonitorHost`). Method: a ten-dimension read-only review with every finding independently
re-verified against the code, followed by a direct re-read of the load-bearing files by the reviewer.

This document records the findings and their disposition. Companion documents:
[`ARCHITECTURE.md`](../../ARCHITECTURE.md) owns runtime and enforcement detail;
[`SECURITY_AND_PERMISSIONS.md`](../SECURITY_AND_PERMISSIONS.md) owns the permission contract.

## Executive summary

Sift is a soundly architected product with a security model that is real rather than cosmetic. The
privilege-separation design — an `asInvoker` shell that brokers machine mutations to a short-lived
`requireAdministrator` helper through a nonce-bound request/response file, with independent policy
re-resolution on both sides, typed operation identifiers (no raw command, registry target, script, or
path ever crosses the boundary), full-hash identity gates, deny-write leases, signed-signer matching,
and a helper-side consent prompt — holds up under scrutiny. Several of the review's most tempting attack
narratives were checked and found already neutralized by these overlapping guards, which is itself a
signal of deliberate defense in depth. The layering discipline (presentation-neutral `Sift.Core`, facade
records, transient request DTOs constructed at call sites) is coherent and mostly honored.

The real risk is not a single catastrophic hole; it is a small number of load-bearing defects clustered
in three places: the native-interop lifecycle, the async-void/timer error boundary, and one specific gap
in the elevation write-path validation. There are three high-severity findings and no critical. Two of
the three (H2, H3) are reliability failures reachable from ordinary user actions with no attacker present
— a Script Studio tab that dies permanently after one round-trip, and a whole-process crash triggerable
by a passive background timer. The third (H1) is a genuine privilege-boundary defect: a dangling-symlink
bypass that defeats the documented reparse-point guarantee and yields a medium-integrity → administrator
file-creation primitive, bounded by a create-only/structured-JSON constraint and a Developer-Mode
prerequisite.

Beneath the highs sits a longer tail of low findings that individually do not threaten the product but
collectively describe where the architecture's discipline is convention rather than enforcement: facades
that constrain nothing at the type level, a committed-mutation boundary drawn in the wrong place on two
paths, and several instances of documentation promising a guarantee the code delivers only conditionally.

Headline: the security boundary the product is built to defend is largely intact, but the code paths
around it — native resource teardown, background exception handling, and the "we validated this" claims
in the docs — need a focused hardening pass to reach the level the documentation already asserts.

Severity tally: 0 critical, 3 high, 3 medium, ~18 low.

## Systemic themes

**T1 — TOCTOU / validation-vs-open discipline on the privilege boundary.** The elevation path validates
a name and then opens it in a separate syscall, and the validation itself has a fail-open branch
(`ValidateExactPath` gates its reparse check behind `File.Exists`, which follows the link). Directory
trust is established by folder-name pattern rather than owner/SID/DACL, and response acceptance rests on
an echoed nonce that same-user processes can read through the `FileShare.Read` lease. Common root:
identity is inferred from strings and existence checks instead of from the OS's own ownership and handle
primitives, and validation is decoupled from the open it is meant to protect. Evidence: H1, ipc-2, ipc-4.

**T2 — Native/resource lifecycle ownership is under-specified.** Two subsystems tear down a native
resource on one path and assume it can be revived on another: WebView2 is `Close()`d on tab-leave then
reused, and `DashboardHistoryStore.Dispose` disposes its semaphore without holding it. Teardown and
reacquire are written as if idempotent when the underlying primitive is not. Evidence: H3,
resources-interop-2, concurrency-lifecycle-3.

**T3 — The async-void error boundary is inconsistently guarded.** `App.UnhandledException` deliberately
fails closed, which is defensible provided every async-void handler catches its own faults. `ScriptCenter`
does; the Home dashboard and the settings-flush timer do not, so a transient SQLite lock, disk-full, dead
monitor host, or timer-thread exception terminates the process. Evidence: H2.

**T4 — Cancellation / commit boundary placed too early.** `RunCommittedAsync` correctly makes mutations
uncancelable, and TaskManager confirms before committing. The Home dashboard and scheduled-task paths run
the whole cancelable scan/preview/confirmation phase inside the committed region, so navigation-driven
cancellation silently no-ops. Evidence: concurrency-lifecycle-2.

**T5 — Documentation/reality drift on user-facing safety guarantees.** Recycle-Bin-only deletion is
violated silently when a selection exceeds bin capacity; `AtomicFile` is atomic against readers but not
crash-durable despite its name; history compaction advertises rollups but overwrites instead of merging.
A name or doc line asserts a strong invariant the implementation satisfies only conditionally, and the
failure is silent. Evidence: M1, persistence-integrity-1/2, the doc facet of H1.

**T6 — Layering is convention, not compile-time contract.** The facade records are presented as the
scoping mechanism, but `WorkspaceRegistryFactory` holds the whole service bag and reaches past the facades
for three services; the Composition router's contract is anchored to a type owned by a view's code-behind.
Intended boundaries exist in naming and structure but are not enforced by the type system. Evidence:
layering-composition-1/2/3.

## Findings

Severity is the verified/corrected severity; several reported severities were adversarially corrected
down with reasoning preserved in the remediation plan.

### High

| ID | Finding | Files |
|----|---------|-------|
| H1 (`elevation-ipc-1`) | Reparse-point guard in `ValidateExactPath` gates on `File.Exists`, which follows the link, so a **dangling** symlink skips the check; the elevated helper's `CreateNew` then follows it and creates an attacker-chosen file as administrator (medium-IL → admin file-creation primitive). Bounded by Developer-Mode / `SeCreateSymbolicLinkPrivilege` and create-only structured content. | `Sift.Core/Services/Elevation/ElevationBroker.cs:534,609-618`; `Sift.ElevationHost/Program.cs:67,78` |
| H2 (`concurrency-lifecycle-1`, `error-resilience-1`) | `App.UnhandledException` fails closed (no `Handled=true`), but `HomeDashboardWorkspaceModule.SampleAsync`/`View_WidgetActionRequested` post-sample awaits and the `SettingsPersistenceCoordinator` flush-timer callback run unguarded; a transient SQLite/IPC/IO fault crashes the whole app from a background timer. | `Sift/Composition/HomeDashboardWorkspaceModule.cs`; `Sift.Core/Infrastructure/Settings/SettingsPersistenceCoordinator.cs`; `Sift/App.xaml.cs:16-23` |
| H3 (`resources-interop-1`) | The intra-workspace tab toggle calls `DisposeStudio()` → `StudioWebView.Close()` (terminal) and re-inits on return, permanently bricking Script Studio after one Studio→Library→Studio round-trip; `_studioInitialized` is also not reset in the init catch. | `Sift/Views/ScriptCenterWorkspaceView.Layout.cs:32-34`; `.Studio.cs:23,44,124-140` |

### Medium

| ID | Finding | Files |
|----|---------|-------|
| M1 (`policy-guards-2`) | `SHFileOperation` with `FOF_ALLOWUNDO\|FOF_SILENT\|FOF_NOCONFIRMATION` permanently deletes (best-effort undo) when a selection exceeds Recycle Bin capacity or the bin is disabled; no byte-size guard, and success is reported as "Moved to the Recycle Bin" either way. | `Sift.Core/Services/Storage/StorageDeleter.cs:125`; `StorageSelectionDeletionManager.cs` |
| M2 (`build-packaging-hygiene-1`) | The entire `apps/` tree and `.github/workflows/sift.yml` are untracked in git — no history, no review trail, dead CI — for ~28K LOC of security-sensitive code. | repository root `.gitignore`; `apps/**` |
| M3 (`build-packaging-hygiene-3`) | The committed ~14 MB Monaco/xterm bundle has no reproducible source rebuild in CI; `verify-script-studio.ps1` checks the committed set against a manifest that is itself part of that set. | `Sift.csproj:53-55`; `scripts/verify-script-studio.ps1`; `WebAssets/dist/**` |

### Low

| ID | Finding |
|----|---------|
| `elevation-ipc-2` | Request directory trusted by folder-name pattern, not owner/SID/DACL (not currently exploitable behind the `requireAdministrator` gate). |
| `elevation-ipc-4` / `doc-reality-gap-2` | Response accepted on echoed id+nonce; the nonce is readable via the `FileShare.Read` request lease, so same-user malware can pre-squat the response file and spoof success/failure to the UI. Docs overstate the nonce as authenticity. |
| `concurrency-lifecycle-2` | The uncancelable committed boundary wraps the whole scan/preview/confirm pipeline on the dashboard and scheduled-task paths, so navigation cancellation no-ops. |
| `concurrency-lifecycle-3` | Re-entrant navigation can call `Sample()` on a disposed PDH sampler (guarded to a cosmetic status flash by the sampler's own lock + `_disposed`). |
| `resources-interop-2` | `DashboardHistoryStore.Dispose` runs a synchronous SQLite flush on the UI thread and disposes its semaphore without holding it. |
| `persistence-integrity-1` | History compaction rollup uses REPLACE (`excluded.*`) not additive merge, dropping samples at the 7-day boundary. |
| `persistence-integrity-2` | `AtomicFile` is atomic for readers but not crash/power-loss durable (no flush before rename). |
| `persistence-integrity-4` | `busy_timeout` PRAGMA applied only to the init connection, not per-operation connections. |
| `policy-guards-3` | Scheduled-task "Unknown" state maps to enabled (contained by the hash+consent gate). |
| `layering-composition-1/2/3` | Facades do not enforce scope; Composition↔Views coupling via a view-owned intent type; optimize pipeline duplicated at two call sites. |
| `build-packaging-hygiene-4` | The "zero warnings / no `UseWPF` / no `System.Windows` in Core" contract holds but is not enforced (`TreatWarningsAsErrors`, `Directory.Build.props`). |
| `build-packaging-hygiene-5` | Dev-build copy deletes only `Sift.MonitorHost.*` root leaks, not `Sift.ElevationHost.*`; `TargetPlatformMinVersion` inconsistent with the MSIX floor. |
| `doc-reality-gap-1` | "Only the active module may keep a sampling timer running" is violated by the intentional retained-dock exception in Hardware Monitor. |
| `scriptstudio-analyzer-1` | Interpreter trust (reparse + Authenticode) is checked at discovery, not re-verified at spawn; auto-discovered runtimes may be user-writable (TOCTOU). |
| `scriptstudio-webview-2` | WebView2 args set by mutating the process-global `WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS` (replace, not append; race window) instead of `CoreWebView2EnvironmentOptions`. |

### Checked and dismissed (refuted)

| ID | Reported claim | Why dismissed |
|----|----------------|---------------|
| `elevation-ipc-3` | Helper shares `ReadWrite` and re-reads the request unsynchronized in an empty-catch path. | The mechanics are real but benign: the nonce, deny-write lease, and independent policy re-resolution neutralize any tampered re-read; no privilege or integrity crossing. |
| `policy-guards-1` | Maintenance cleanup permanently deletes files, contradicting the Recycle-Bin-only guarantee. | Misreads the doc: the Recycle-Bin-only guarantee is scoped to Storage and Installed-Apps leftovers; Maintenance temp-file cleanup is in-scope permanent deletion by design. |
| `build-packaging-hygiene-2` | The `.gitignore` rule enabling the committed bundle lives in a root-ignored file, breaking on clone. | Moot: subsumed by M2 — the whole `apps/` tree is untracked, so the sub-rule cannot matter until the tree is committed (at which point M2's `.gitignore` verification covers it). |

## Preserve (load-bearing and correct — do not restructure)

- The elevation broker trust model: typed operation IDs (helper accepts no raw command/registry
  target/script/path), independent policy re-resolution on both sides, 256-bit nonce binding, full
  SHA-256 identity gates, deny-write leases, and the SystemModal helper-side consent prompt. Fix H1
  *within* this model. This is why `elevation-ipc-2` and `policy-guards-3` collapse to low — the
  overlapping guards genuinely neutralize the exploit chains.
- `App.UnhandledException` failing closed. The fix for H2 is to guard the handlers, not to swallow faults
  globally.
- The cross-workspace WebView2 `Suspend`/`Resume` path. H3 is fixed by extending it to the tab toggle.
- `RunCommittedAsync`'s uncancelable-mutation contract and TaskManager's confirm-before-commit ordering.
- `AtomicFile` reader-atomicity and `WriteAggregatesAsync`'s additive merge — the fixes bring other code
  to match these, not to change them.
- The presentation-neutral `Sift.Core` split.

## Remediation status

All three highs, all three mediums, and most of the low tail are fixed in the audit remediation change
set; the solution builds with zero warnings under `TreatWarningsAsErrors`, and the unit suite (296),
the integration validator (103 canonical checks + inventory/preflight), and the WebAssets
reproducibility check all pass.

**Fixed:** H1 (reparse create-no-follow + name-based reparse rejection), H2 (guarded dashboard
async-void handlers and settings-flush timer), H3 (WebView2 suspend/resume on tab switch + retryable
init); M1 (Recycle-Bin availability guard + `FOF_WANTNUKEWARNING`), M2 (source tree + CI now tracked),
M3 (reproducible-bundle CI job); and the low items `elevation-ipc-2`, `persistence-integrity-1/2/4`,
`resources-interop-2`, `policy-guards-3`, `concurrency-lifecycle-3`, `layering-composition-1/2`,
`build-packaging-hygiene-4/5`, `doc-reality-gap-1/2`, `scriptstudio-analyzer-1`, and
`scriptstudio-webview-2`. A pre-existing icon-markup test bug and four stale canonical-source checks
(drift the untracked/un-CI'd tree had accumulated — exactly what M2 addresses) were also reconciled.

**Deferred (with rationale):**
- `elevation-ipc-4` (response-forgery pre-lease protocol) — the docs were corrected to describe the
  nonce as correlation, but the response-file pre-lease/HMAC protocol change is held back: it rewrites
  the elevated read/write path, carries real regression risk to the core elevation feature, and cannot
  be end-to-end verified here without an interactive UAC elevation. Severity is low (same-integrity UI
  spoofing, not privilege escalation).
- `concurrency-lifecycle-2` (move the confirm/preview phase out of the committed region on the dashboard
  and scheduled-task paths) — a real but low-severity, self-resolving contract deviation; the fix
  restructures the mutation pipeline and needs interactive verification of each action flow.
- `layering-composition-3` (optimize wiring duplicated at two call sites) — reviewed and left as-is: the
  shared wiring is facade-field passing to two different constructors, not extractable pipeline logic;
  a helper would add indirection without removing real duplication.

Recommended interactive confirmation for H2/H3 (not runnable headlessly here): `scripts/validate-ui.ps1`
plus a manual Script Studio Studio↔Library tab round-trip and a dashboard background-sampling cycle.
