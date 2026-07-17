# Historical Sift program index

**Status:** Superseded on 2026-07-16 by the [product language, permission, and architecture audit](../../audits/AUDIT_PRODUCT_LANGUAGE_PERMISSION_ARCHITECTURE.md).

This index preserves the original full-roadmap decomposition. It is not the current execution ledger: only Plans 1 and 2 were authored, while later filenames were proposed but never created. Current capability status lives in `ROADMAP.md` and `docs/audits/sift-feature-audit.json`.

**Design spec:** [Full-roadmap audit design](../specs/2026-07-14-full-roadmap-audit-design.md)

| # | Plan | Status | Owner agent | Notes |
|---|---|---|---|---|
| 1 | `2026-07-14-sift-audit-baseline.md` | Complete | sift-completion | Canonical audit JSON, validator, baseline evidence, and validation hook are present. |
| 2 | `2026-07-14-sift-disconnected-wiring.md` | Implemented | sift-completion | The typed workflows were implemented; later permission and copy corrections are tracked by the active audit. |
| 3 | `2026-07-14-sift-shell-home-settings.md` | Not created | — | Proposed shell accessibility, activity export/resize, Home cards, and Settings coverage. |
| 4 | `2026-07-14-sift-taskmanager-startup.md` | Not created | — | Proposed process-tree grouping, export, and Startup coverage. |
| 5 | `2026-07-14-sift-performance-maintenance-health.md` | Not created | — | Proposed Performance, Maintenance, and Health follow-up work. |
| 6 | `2026-07-14-sift-recovery-integrity.md` | Not created | — | Proposed recovery integrity and Optimize migration work. Optimize now links to Recovery; the integrity envelope remains on the roadmap. |
| 7 | `2026-07-14-sift-storage-installed-apps.md` | Not created | — | Proposed Storage and Installed Apps follow-up work. |
| 8 | `2026-07-14-sift-system-information.md` | Not created | — | Proposed signed export and EDID work. |
| 9 | `2026-07-14-sift-release-acceptance.md` | Not created | — | Proposed release acceptance and final visual/accessibility work. |
| 10 | `2026-07-15-sift-hardware-monitor.md` | Not created | — | Historical owner label added when Hardware Monitor was implemented outside the original plan sequence. |

## Current focus

Use `docs/audits/AUDIT_PRODUCT_LANGUAGE_PERMISSION_ARCHITECTURE.md` for current chunks and completion gates.

## External blockers

- Trusted publisher certificate required for signed MSIX and clean-account trust acceptance.
- Do not claim signing success without verified `signtool` results.

## Stage gates

- Plan 1 gate: `validate-feature-audit.ps1` passes and baseline evidence exists.
- Plan 2 gate: recorded as implemented in the changelog and feature audit.
- Current completion gate: the active audit's requirement-by-requirement evidence, including `validate.ps1`, release layout verification, required screenshots, and documentation reconciliation.
