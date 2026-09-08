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

## Implementation-plan provenance

Current scoped review: [Scale 0–4 discrete causal physics](AUDIT_2026-09_SCALE0_SCALE4_DISCRETE_CAUSAL_PHYSICS.md)
(2026-09-04). Two reproduced Scale-2/3 defects remain open; the report separates
transport limits from dependency locality and records native/browser checks.
Follow-up: [one discrete evolution with coarse-grained observations](../../engine/docs/PLAN_STRICT_DISCRETE_COARSE_GRAINING.md)
records the requested architecture, first exact block implementation, and open
closure/continuum/production gates.
Execution: [strict-discrete stack program ledger](../../engine/docs/PROGRAM_STRICT_DISCRETE_STACK.md)
tracks the successor candidate, independent audits and blocked physical gates.
Evidence: [integrated foundation audit](../../engine/docs/AUDIT_STRICT_STACK_FOUNDATION.md)
records independent review, backend parity and local laboratory qualification.
Recovery: [strict recovery wave 1](../../engine/docs/PROGRAM_STRICT_RECOVERY_WAVE_1.md)
tracks exact prepared transport, full finite ensembles, registered constituent
campaigns and independently reviewed material/macroscopic obstructions.
Continuation: [strict recovery wave 2](../../engine/docs/PROGRAM_STRICT_RECOVERY_WAVE_2.md)
tracks collisional response, interacting pair transport, exact inventory currents
and a registered mixed-record CUDA intervention campaign.

Production review: [Scale 0 logic, physics and overlays audit](../../engine/docs/AUDIT_SCALE0_COMPREHENSIVE_2026-09-07.md)
records full enumerated source coverage, bounded repairs, independent reviews,
and separate hardware and physical-recovery release gates (2026-09-07).

Correctness follow-up: [Scale 0 production repair ledger](../../engine/docs/PROGRAM_SCALE0_CORRECTNESS_WAVE_2026-09-07.md)
tracks typed native command validation, telemetry scheduling/publication,
finite JSON readouts and mutation-free tick preflight. Its bounded engineering
acceptance leaves full replay, performance and physical recovery gates explicit.

Performance continuation: [Scale 0 performance repair v2](../../engine/docs/PROGRAM_SCALE0_PERFORMANCE_V2_2026-09-07.md)
registers asynchronous Spectrum analysis, bounded worker observations, retained
readouts and the complete panel/overlay hardware matrix. Its own evidence
disposition preserves earlier failures and keeps physical recovery separate.
The [v3 follow-up plan](../../engine/docs/PLAN_SCALE0_PERFORMANCE_V3_FOLLOWUP_2026-09-07.md)
separates the newly found Wave Lab component defect from further performance
work. The [Time/Gravity implementation contract](../../engine/docs/PLAN_SCALE0_TIME_GRAVITY_PERFORMANCE_V3_2026-09-08.md)
preserves clock sampling and full-volume normalization while specifying the
next retained-readout and bounded-slab candidates.
The [Wave Lab correctness v3 ledger](../../engine/docs/PROGRAM_SCALE0_WAVE_CORRECTNESS_V3_2026-09-08.md)
tracks the integrated component, availability and directional-zero repairs,
their independent numerical/browser checks and a distinct hardware follow-up.
The [Time/Gravity v4 program](../../engine/docs/PROGRAM_SCALE0_TIME_GRAVITY_V4_2026-09-08.md)
tracks retained Time rendering, same-publication Gravity slabs, independent
equivalence reviews and a separately registered 16-row hardware follow-up.
The [WaveLab timing diagnostic](../../engine/docs/PROGRAM_SCALE0_WAVE_ATTRIBUTION_V1_2026-09-08.md)
registers invocation-associated component measurements before further cost
changes, with unchanged production sources and separate acceptance gates.
The [alignment-law proposal](../../engine/docs/SPEC_PHI_ALIGNMENT_SUCCESSOR_PROPOSAL_V1.md)
records a separately reviewed formation candidate, its explicit information
loss and the continuum and binding gates it does not pass.

Transport follow-up: [Scale 0 native connection repair](../../engine/docs/PROGRAM_SCALE0_TRANSPORT_REPAIR_2026-09-07.md)
tracks bounded network operations, early frame admission and reconnect ownership.
It records the accepted deferral of full production replay while preserving the
strict candidate's existing checkpoint requirements.

Observation follow-up: [native sample provenance and remaining recovery gates](../../engine/docs/PROGRAM_NATIVE_OBSERVATION_V3_2026-09-07.md)
tracks versioned binary observations, exact identities, request ownership and
the original strict-stack phase dispositions. Scientific continuation:
[known pair-family formation obstruction](../../engine/docs/AUDIT_STRICT_PAIR_FORMATION_OBSTRUCTION.md)
records a separately implemented and independently reviewed scoped theorem.

| Collection | Scope | Disposition |
|---|---|---|
| [FTD engine agent plans](engine_agent_plans/README.md) | Six graph/engine-overlay plans plus the former root Scale-0 work plan; Plans 01–03 remain at the collection root | Preserved as implementation provenance; no claim status changed by relocation |
| [Scale-1 particle-context v4](../../engine/docs/PLAN_SCALE1_PARTICLE_CONTEXT_V4.md) | Native particle observatory, quantum/QED references, effective reference lab, and projection/loss contract | 39 executable scenarios across four live workspaces; disabled roadmap rows retired from the manifest |

---

## Archive

| Date range | Slug | Findings | Resolved | Deferred | Not-a-bug |
|---|---|---:|---:|---:|---:|
| 2026-08-28 | [product consolidation](AUDIT_2026-08_PRODUCT_CONSOLIDATION.md) | retired app/prototype/build-tree inventory | native + web boundary established | none | theory-side trit work retained |
| 2026-06-17 | [project cleanup plan](PLAN_PROJECT_CLEANUP_2026-06-17.md) | public GitHub/README cleanup plan | phase 1 complete | yes | n/a |
| 2026-04-11 | [documentation cleanup ledger](AUDIT_DOCUMENT_CLEANUP_LEDGER.md) | repo-wide documentation drift ledger | mixed | yes | n/a |
| 2026-04-27 | [pre-refactor sweep](AUDIT_2026-04_pre-refactor.md) | 122 | 78 | 40 | 4 |
| 2026-04-27 | [refactor sweep (8-phase)](AUDIT_2026-04_refactor-sweep.md) | 8 phases · 17 commits | 17 | 1 (WSL2 GPU parity) | 0 |
| 2026-07-02 | engine revision program (plan `i-want-you-to-clever-frost` + `engine/CHECKLIST_ENGINE.md` revision sections) | 38-agent audit → phased execution on `engine/revision-program-p0` | Phases 0–3 + 2.10/4.2/6.1 | 4.1, 5.1–5.6, 6.2 | 7 refuted + 5 stale claims (recorded) |
| 2026-07-29 | [Scale 1 particle engine](AUDIT_2026-07_scale1-particle-engine.md) | 76 confirmed (94 raised, 18 refuted) | 0 (superseded by full redesign, not fixed in place) | all — subsystem being retired | 18 refuted, recorded |

---

## Known UNAUDITED areas (revision 6.3 — no sweep has covered these)

| Area | Contents | Risk note |
|---|---|---|
| `engine/tools/*.sh` | WSL2 campaign shell runners (`op_mixing_sweep.sh`, `run_topological_production.sh`, …) | Research data-product paths, uncovered; failures surface only mid-campaign |
| `engine/tools/` Python utilities + `build_file_manifest.py` | Plot/manifest utilities | Low risk; manifest generator's output IS committed (`engine/docs/ENGINE_FILE_MANIFEST.*`) so drift is visible in diffs |

A scoped follow-up sweep of `engine/tools/` (shellcheck of the campaign runners
and smoke checks for the remaining Python utilities) is the recorded next step;
until then, treat results produced
via these tools as unaudited-pipeline outputs.

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
- [archive/ORIGINAL_REQUEST_2026-05-26_ENGINE_WEB_AUDIT.md](archive/ORIGINAL_REQUEST_2026-05-26_ENGINE_WEB_AUDIT.md) — historical multi-agent request provenance
- [archive/PHASE_8_PYTORCH_STATUS.md](archive/PHASE_8_PYTORCH_STATUS.md) — historical Python/PyTorch conversion status
