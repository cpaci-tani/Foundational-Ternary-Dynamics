# PREREG — Full-state irreversibility of the current engine update map

**Prospective claim ID:** FTD-0395 (registry rechecked at lock time; FTD-0394 is the current maximum).  
**Tag:** [PRE-REGISTRATION — ENGINE THEOREM TEST] · LOCK-STD v1 · git tag `preregister-full-state-irreversibility-v1`.  
**Scope:** the current `RenderBridge` CPU update map on public-API-admissible finite states. FC-2 remains `[AXIOM]`; no framework type, calibration, or production API is changed.

## 1. Frozen question and maps

- `R` is the site readout `J -> (state,color,spin)`. FTD-0394 already witnesses that `R` is non-injective while the three post-genesis flux vectors remain distinct.
- `F` is one complete public `RenderBridge::tick()` update, including tick/time and public audit bookkeeping.
- An observer history is a sequence of `R` values. It is neither `R` nor `F` and cannot establish full-state loss by itself.

Question: do two distinct public-API-admissible engine states have exactly the same image under `F`?

## 2. Frozen construction and effective protocol

Instrument: `engine/tests/test_full_state_irreversibility.cpp` (frozen with this lock; SHA256 in `REF_PREREGISTER_MANIFEST.md`).

Create two `RenderBridge(8)` instances through the public API. On both: call `force_cpu()`, `toggles.disable_all()`, enable only `evaporation`, and call `seed_rng(20260422)`. Inject at `(3,3,3)` with the public

```text
inject_particle(x,y,z,+1,{1e-5,0,0},spin,color)
```

using `(spin,color)=(+1,1)` in arm A and `(-1,3)` in arm B. All lattice data, flux/wave fields, state, assigned particle ID, injector counters, seed, tick, `dt`, SOR count, and effective toggles are otherwise identical.

The negative-control pair is identical except `evaporation=false`. The existing `campaign_manifestation_readout_collision` target is run separately as correctness gate G7; it must reproduce FTD-0394's identical discrete readout with distinct `J` magnitudes.

Platform of record: WSL2 Ubuntu-22.04, canonical `engine/build_wsl`, CPU-forced reference execution, `FTD_FORCE_GPU` unset. The Windows CPU build may be used only as a compile/smoke cross-check and cannot supply the verdict of record.

## 3. Frozen complete-state comparator

The comparator enumerates fields; it does not `memcmp` `Voxel` padding. IEEE-754 scalar components are compared bit-for-bit.

For every voxel it compares: ternary state; `flux`, `wave_vel`; `flux_L`, `flux_R`, `wave_vel_L`, `wave_vel_R`; velocity and remainder; latency, tau, phase; locked; particle/pair IDs; spin, color, flavor; acceleration magnitude; strong flux/wave fields; weak flux/wave fields.

Public global state compares: lattice size, backend kind, current tick, physical time, `dt`, SOR count, injector particle/pair counters, charge sum, genesis/evaporation event counters, every `EnergyAudit` field, and every `EnergyLedger` field.

After the collision tick the comparator is rerun after each of sixteen additional complete ticks. This tail is the hidden-cache detector: any hidden difference that later reaches persistent public state defeats exact collision.

## 4. Correctness, validity, and vacuity gates

All gates precede outcome adjudication.

| Gate | Frozen requirement | Failure |
|---|---|---|
| G1 | census GREEN at tag cut | lock cannot be cut |
| G2 | backend is CPU and effective toggles match §2 | INVALID |
| G3 | pre-tick states differ in exactly the requested center-site spin and color fields; all comparator fields otherwise agree | INVALID |
| G4 | both arms evaporate on the same first full tick; each reports one evaporation event | INVALID |
| G5 | negative control retains state, particle ID, spin, and color, and its arms remain different | INVALID |
| G6 | the test enumerates every persistent `Voxel` member named in the source-of-record `voxel.h` | INVALID |
| G7 | FTD-0394 target passes and retains distinct `J` magnitudes | INVALID |
| G8 | two executions of each target are bit-identical in verdict/check output | INVALID |

Vacuity witnesses: G3 fails if both arms use the same labels; G4 fails when evaporation is disabled; G5 fails if the label difference is accidentally omitted; the exact comparator fails on the admissible pre-tick pair by construction. Therefore collision is not obtained from a constant comparator. G7 is a witness that readout collision does not imply full-state collision.

Quantifier audit: a FULL result proves existence of one colliding pair only. It supports non-injectivity on the stated domain; it says nothing about every manifestation, mathematical reversibility outside the engine, or FC-2's normative adoption.

## 5. Frozen outcomes and partition proof

Correctness gates have absolute precedence. Among valid runs, apply the following ordered decision list:

1. **FULL-NONINJECTIVE:** the complete comparator is equal immediately after the collision tick and after every one of the sixteen tail ticks.
2. **PHASE-ONLY:** both center records are erased on the same tick, but the complete comparator differs either immediately or in the tail.
3. **READOUT-ONLY:** the evaporation pair does not establish even the phase-local collision condition; only G7's `R` collision remains.
4. **INVALID:** any G2-G8 gate fails. INVALID is evaluated first despite being printed fourth to retain the requested outcome names.

Partition proof: after gates, `complete_equal` is Boolean. If true, outcome 1 fires. If false, `phase_record_equal_after_evaporation` is Boolean: true fires outcome 2 and false fires outcome 3. No valid dataset can fire two rows, and every valid dataset fires one. Ties do not exist; exact bit equality is the tie-break. Normative criteria and this precedence list outrank explanatory prose.

## 6. Licensed interpretation and reconciliation

FULL-NONINJECTIVE licenses only `[THEOREM — current engine update map on the public-API-admissible domain]`: two different admissible states have the same complete future under the tested map. PHASE-ONLY licenses only a phase-scoped statement. READOUT-ONLY leaves FTD-0394 as a theorem/cardinality fact about `R`, not `F`.

Under no outcome may prose say that genesis erases `J` or that manifestation destroys all information. On FULL, canonical arrow documents must say: genesis readout is lossy; a distinct evaporation witness establishes full-map non-injectivity. FC-2 stays `[AXIOM]`.

The result commit must update the LEDGER, open-items tracker, relevant canonical arrow document, META index, and manifest; it must record command line, git SHA, binary SHA256, platform, and effective toggles. Lock, result, and reconciliation are separate commits.

## 7. Execution window and executor

Executor: the current Codex repository session on branch `codex/invariant-quotient-roadmap-2026-07-20`. Window: from tag creation through `2026-07-24T00:39:19Z` (72 hours). A missed run or unbooked verdict creates F10 debt and blocks the next lock.

**LOCKED CONTENT ENDS HERE.** The immutable git commit/tag plus the preregistration SHA256 recorded in the manifest bind this text. Any normative edit requires v2.
