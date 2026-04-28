# Audit Ledgers — Index

**Audience: LLM agents and humans tracing the history of audit sweeps.**
**Update trigger: every audit sweep merge moves its ledger here and adds a row.**

Audit ledgers track findings discovered during structured sweeps (single-pass
or multi-agent reviews). Each ledger uses the `[x]/[~]/[d]/[n]` legend:

- `[x]` fixed
- `[~]` partial
- `[d]` deferred (with reason)
- `[n]` not-a-bug after re-check

Active sweeps live at the project root as `AUDIT_LEDGER.md`. On merge,
they are renamed `AUDIT_<YYYY-MM>_<slug>.md` and moved here.

---

## Archive

| Date range | Slug | Findings | Resolved | Deferred | Not-a-bug |
|---|---|---:|---:|---:|---:|
| 2026-04-27 | [pre-refactor sweep](AUDIT_2026-04_pre-refactor.md) | 122 | 78 | 40 | 4 |

---

## Patterns observed (quarterly roll-up)

This section accumulates structural patterns across sweeps. Update on a
quarterly cadence so recurring drift becomes visible.

### 2026-Q2 (April)

- Energy convention drift across MockBridge / WasmBridge (½ factor missing
  on ~6 quantities) — single-source convention now in
  [CONTRACTS.md §6](../../CONTRACTS.md#6--energy-convention-contract)
- Toggle dependency violations on default state (`weak_transmutation=true`
  without `dual_substrate=true`) tripped validator — fixed by atomic-batch
  ordering in scenario-loader
- Constants drift in `resources/data/constants.json` (EPSILON wrong sign,
  `x_plus_tree` 47 ppm off) — JSON regenerated from `scripts/constants.py`
- Cross-scale constant duplication (Coulomb prefactor in 5 forms;
  strong-force constants hardcoded in 2 places) — promoted to
  `constants.js` exports

---

## Lifecycle policy

(See [META_PROJECT_ATLAS.md §6](../../META_PROJECT_ATLAS.md#6--audit_ledger-lifecycle))

- **Active sweep**: `AUDIT_LEDGER.md` at root.
- **On merge**: rename to `AUDIT_<YYYY-MM>_<slug>.md`, move here, add row above.
- **Retention**: indefinite.
- **Concurrent sweeps**: place under `docs/audits/active/<slug>/AUDIT.md`.
- **Quarterly roll-up**: append "Patterns observed" section.

---

## How to start a new sweep

1. Author `SPEC_REFACTOR_<name>.md` at root with scope, files, success criteria.
2. Create `AUDIT_LEDGER.md` skeleton at root with track headings.
3. Live-track findings during work using `[x]/[~]/[d]/[n]` legend.
4. On merge, rename + move + add row to this INDEX.

## Related docs

- [META_PROJECT_ATLAS.md](../../META_PROJECT_ATLAS.md) §6, §7
- [CONTRACTS.md §9](../../CONTRACTS.md#9--refactor-companion-contract) (refactor companion contract)
- [docs/adr/INDEX.md](../adr/INDEX.md)
