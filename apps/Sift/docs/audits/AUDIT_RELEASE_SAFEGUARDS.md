# Release safeguard audit

**Date:** 2026-07-16  
**Scope:** `apps/Sift`, `Sift.Core`, `Sift.ElevationHost`, `Sift.MonitorHost`  
**Goal:** Identify testing-only soft gates before public release; keep production safety boundaries intact.

## Verdict

There were **no signature, elevation, or mutation bypasses** that let unsigned or DEBUG builds perform protected machine changes. Trust policy already fail-closes. The release prep work is hygiene: remove boot-era crash swallowing and replace “development build” copy with release-grade language.

## Kept (production safety — do not remove)

| Control | Location | Why it stays |
|---|---|---|
| Authenticode + matching-signer gate for elevation helper | `BinaryTrustPolicy`, `ElevationHelperTrust` | Blocks helper hop without trusted matching signatures |
| Trusted-host gate for machine Optimize mutations | `TweakExecutor.IsTrustedSignedMutationHost` | Unsigned/untrusted host cannot apply HKLM/command elevate path |
| Matching-signer gate for MonitorHost launch/startup | `DashboardMonitorController` | Background monitor cannot start from untrusted payload |
| Elevation nonce, request lease, shape validation, admin MessageBox consent | `ElevationBroker`, `ElevationHost` | Typed one-shot helper; default-No consent |
| Preflight → confirm → revalidate mutation flow | Optimize, Maintenance, Task Manager, Scripts, etc. | Core product contract |
| Process/service termination guards | `GuardedSystemActions` | Protects Sift/shell/security hosts |
| Hardware Ring0 block while elevated | `LibreHardwareSensorProvider` | Prevents implicit driver open in admin sessions |
| Script forbidden tokens + catalog identity | `ScriptCommandService`, `ScriptRecipeIdentity` | No remote/script download surface |
| Studio analysis blocked while elevated | `ScriptStudioService` | Language tools must not inherit admin token |
| asInvoker defaults; monitor startup disabled by default | manifests / Settings | No silent privileged background work |

## Removed or hardened in this pass

| Item | Was | Action |
|---|---|---|
| Blanket `UnhandledException` → `args.Handled = true` | Boot-debug resilience that kept the shell alive after any fault | **Removed.** Exceptions are logged to Activity, Serilog, and `winui-startup.log`, then allowed to fail closed |
| “Unsigned development builds…” user copy | Dev-phase wording | **Reworded** to trusted-signature language suitable for public builds |

## Explicitly not testing bypasses

| Item | Classification |
|---|---|
| Unsigned builds cannot elevate / mutate protected state | Production fail-closed gate (required for release) |
| `SIFT_LOG_VERBOSE` | Local diagnostic verbosity only; no network; no policy change |
| `InternalsVisibleTo("Sift.UnitTests")` | Test assembly access; not shipped to end users as a runtime switch |
| `Sift.Benchmarks` / measure scripts | Dev-only; not part of the app payload |
| `validate-ui.ps1` fixtures under artifacts | Validation harness; not product code |
| `args.Handled = true` on keyboard routing in `MainWindow` | Normal WinUI key handling, not exception swallowing |

## Residual release blockers (external)

These are not “testing safeguards” inside the product; they remain release gates:

1. Trusted publisher certificate + `SignTool verify` for app, ElevationHost, MonitorHost, and MSIX.
2. `scripts/validate-clean-account.ps1 -InstallRoundTrip` on a disposable standard-user account.
3. Visual pass of Settings / Performance / Home after Toolkit + PDH changes.

## Search coverage

Grep/manual review covered: `#if DEBUG`, trust/signature skips, `SIFT_*` env vars, elevation consent, Hardware elevated path, Script Studio elevated block, MonitorHost signer checks, UnhandledException handlers, InternalsVisibleTo, and release scripts. No `AllowUnsigned`, `SkipTrust`, or DEBUG-only mutation permits were found.
