# engine/archive/ — Provenance-preserving graveyard

Files here are **archived**, not deleted. The CLAUDE.md *Documentation Cleanup Discipline* requires `git mv` to an archive rather than `git rm` for content with closure provenance value.

## Policy

- **Not built.** `engine/CMakeLists.txt` does not glob this tree; no `ftd_add_test(...)` or `add_executable(...)` here.
- **Not consumed.** Active source must not `#include` from `engine/archive/**` or import from it. The compiler should never see these files.
- **Not deleted.** The point of this tree is that closure has provenance value — re-attempting an already-closed-negative hypothesis or re-running a superseded campaign without consulting its archived record wastes effort.
- **Read git history for the move record.** `git log --follow engine/archive/<file>` traces back to the file's original location prior to archival.

## Sub-directories

| Path | Origin | Closure provenance |
|------|--------|---------------------|
| `link8_closed/tests/` | `engine/tests/archive/link8_closed/` | The "master quadratic as RG-step characteristic polynomial" hypothesis. Closed negative 2026-04-20 (LEDGER FTD-0050 / `docs/theory/10_eft_program/archive/closed_negative/AUDIT_LINK8_CLOSURE.md`). Three independent tests all rejected the hypothesis for structurally consistent reasons. FTD-0001/0013/0014 unaffected. |
| `phase_b_2026-04/` | `engine/tests/campaign_*_2026-04-*.cpp` (4 date-stamped April-2026 campaigns) | Phase B diagnostic arc closed 2026-05-04 (FTD-0136 retractions; commit `08c517e` removed 30 superseded Phase B exploratory tests, kept 9 load-bearing files). The 4 campaigns here were not in the deletion batch but are date-stamped to closed arc content; archived in the 2026-05-27 engine cleanup. See SPEC_ENGINE.md §5.6.21–§5.6.27. |
| `dumps_non_load_bearing/` | `engine/tests/dump_a1g_decay.cpp` | One-shot diagnostic dump for the A1g cluster-decay investigation; not referenced in SPEC_ENGINE.md §5.6.23–27 load-bearing test list. Archived in the 2026-05-27 engine cleanup. The load-bearing dumps (`dump_full_physics*.cpp`, `dump_toggle_bisection.cpp`) remain in `engine/tests/`. |

## Restoration

To restore an archived file:
1. `git mv engine/archive/<sub>/<file> engine/tests/<file>`
2. Re-register in `engine/CMakeLists.txt` with the appropriate `ftd_add_test(...)` or `add_executable(...)` block.
3. Note the rationale for restoration in the commit message — the historic closure is now a known precedent and re-opening it deliberately should be justified.
