# Strict recovery wave 3: hydrodynamic limit of the staged field sector

Date: 2026-09-08. Status: **WAVE AUDITED; registered closure test not met;
candidate law unchanged**. This continues the independently audited
[second wave](PROGRAM_STRICT_RECOVERY_WAVE_2.md). Frozen wave-1/wave-2
sources and evidence remain unchanged. Design of record:
[Hydrodynamics from the strict Phi law: design](../../docs/superpowers/specs/2026-09-07-phi-hydrodynamics-design.md).
Booking of record: [DERIV_STRICT_HYDRODYNAMIC_SECTOR.md](DERIV_STRICT_HYDRODYNAMIC_SECTOR.md).

## Ownership and gates

| Gate | Implementer | Independent reviewer | Controller |
|---|---|---|---|
| H0: invariant census | A Sonnet subagent | A different Sonnet subagent | Fable 5.1 |
| H1 contract (dispersion definitions, verdict rule, written before computation) | A Sonnet subagent | A different Sonnet subagent | Fable 5.1 |
| H1 evidence (exact and certified dispersion computation) | A Sonnet subagent | A different Sonnet subagent | Fable 5.1 |
| H2 instrument (`hydro_main.cpp`, CMake) | A Sonnet subagent | A different Sonnet subagent | Fable 5.1 |
| H2 runner (`recovery_hydro_campaign.py`) | A Sonnet subagent | A different Sonnet subagent | Fable 5.1 |
| H2 prereg (`PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md`) | A Sonnet subagent | A different Sonnet subagent | Fable 5.1 |
| H2 campaign (lock/run/summarize, WSL2 CUDA) | A Sonnet subagent | A different Sonnet subagent | Fable 5.1 |
| H2 audit (`AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md`) | A Sonnet subagent | A different Sonnet subagent | Fable 5.1 |

Per the design's execution model (section 5), the implementer of a gate never
reviews that gate's own work; the reviewer reproduces the result rather than
reading it; Fable 5.1 writes instructions, resolves rulings, integrates
documents, and checks every epistemic tag before booking. Contracts precede
validation and GPU execution. No canonical adoption, production migration, or
public deployment is part of this research wave.

## Gate table and dispositions

| Gate | Deliverable | Disposition |
|---|---|---|
| H0 | `scripts/phi_v2_lattice/recovery_hydro_invariants.py` + tests | PASS. Fixed dimension 1; 0 covariance violations; locked dimension 7 per polarity for every `l0`; tangent inside locked span; isotropy table computed for four velocity sets, only the FCHC-projected set isotropic at rank 4. Tag `[THEOREM — finite, exact, scoped to the field sector on the frozen background]` |
| H1 | `scripts/phi_v2_lattice/recovery_hydro_dispersion.py` + `DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md` | COMPLETE. Exact-track verdict label `"other"`; certified track agrees with the exact track at `r0` and reports block dimension 7 (not applicable to the NS-class clauses) at the three physical reference densities. Tag `[DERIVED — linearized Boltzmann (product) closure at the declared reference; correlation leakage not bounded here]` |
| H2 throughput probe | `scripts/phi_v2_lattice/experiments/probe_strict_cuda_throughput.py` | PASS. `L = 32` selected for the registered campaign within the two-hour GPU budget per case set |
| H2 preregistration | `PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md` | LOCKED before execution; `L = 32`, 23 stroboscopes, 336 cases |
| H2 campaign | `scripts/phi_v2_lattice/recovery_hydro_campaign.py`, `engine/strict/recovery_hydro/hydro_main.cpp` | REGISTERED ACCEPTANCE NOT MET. 1 of 42 cells passed; `boltzmann_closure_verified_at_registered_scope = false`. Retained per the preregistration as an obstruction, not corrected. Tags `[MEASURED — instrument and provenance verified]` for the campaign, `[CLOSED NEGATIVE at the registered scope]` for the closure hypothesis |
| H2 audit | `AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md` | PASS. Reproduces the retained report exactly on all seven checked keys; replay agreement on 2 of 336 cases; post hoc diagnostic computed and clearly marked non-acceptance |

## Evidence identities

SHA256, computed with `python -c "import hashlib; print(hashlib.sha256(open('<path>','rb').read()).hexdigest())"`:

| File | SHA256 |
|---|---|
| `engine/docs/evidence/strict-recovery-wave3-exact.json` | `bcf3e79802b5acf8bffc6083993488ec24275ff0df2f2e41fe15f0d919604478` |
| `engine/docs/evidence/strict-hydro-throughput-2026-09.json` | `4c6f0c86fcfd3595e9dbac09bb465785291e9b1a560c0c2f0fbfc3b06a8de998` |
| `engine/docs/evidence/strict-hydro-response-v1.json` | `092d36821051b9f234ea24566d6b939eceaa5b1fba24f0ef8c41caa40b5dc4e6` |

The response-campaign hash matches the independently rehashed value recorded
in [AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md](AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md).

## Integrated evidence

From the worktree root, in Git Bash:

```bash
FTD_STRICT_CUDA_REQUIRED=1 python -m pytest scripts/tests/phi_v2_lattice -q -p no:cacheprovider
```

Result: **469 passed, 45 skipped in 115.04 s (0:01:55)** (pytest also reports
1 unrelated warning: an unknown `cache_dir` config option, not a test
result). The count rose from 468 to 469 passed with this fix wave's one new
regression test (`test_certified_verdict_uses_declared_precision_regardless_of_ambient`,
`test_recovery_hydro_verdict.py`, I-1). `FTD_STRICT_CUDA_REQUIRED=1` turns a
missing CUDA binary/device into a hard failure for `test_cuda_parity.py`
specifically; none of that suite's
tests skipped, confirming the CUDA CLI and device are present. The 45 skips
(`pytest -rs`) are all pre-existing, orthogonal-binary skips, none of them
new to this wave: 33 in `test_native_parity.py` ("strict native CLI not
built; configure engine/strict separately" — a separate CPU-only
`engine/build_strict_native/ftd_strict_cli` this booking pass did not build),
8 in `test_recovery_carriers.py` ("optional carrier runner required for
preflight binary-hash fixture"), and 4 in `test_wasm_parity.py` ("actual WASM
module not requested" — skips unless `FTD_STRICT_WASM_MODULE` is set; that
module was not built for this booking pass).

`python engine/strict/generate_tables.py --check` reports the frozen C++
table transcription unchanged.

## Provenance dependency

This wave's sources (`scripts/phi_v2_lattice/recovery_hydro_invariants.py`,
`recovery_hydro_dispersion.py`, `recovery_hydro_campaign.py`,
`engine/strict/recovery_hydro/`, and the documents in this wave) import the
strict-stack foundation and recovery waves 1-2
(`engine/docs/PROGRAM_STRICT_DISCRETE_STACK.md`,
`engine/docs/PROGRAM_STRICT_RECOVERY_WAVE_1.md`,
`engine/docs/PROGRAM_STRICT_RECOVERY_WAVE_2.md`, `engine/strict/`,
`scripts/phi_v2_lattice/{channels,staged,state,tick,geometry,recovery_kinetic_response}.py`
and related). Those foundation files are present in this worktree (mirrored
from the concurrent main working tree) but are **not yet committed to git
history on any branch**, including `main`. A clean checkout of this branch
therefore cannot build or run this wave's code until that foundation is
committed first; this booking commit stages and commits only this wave's own
three new documents, per the controller ruling below.

## Pending upstream edit

`engine/docs/PROGRAM_STRICT_DISCRETE_STACK.md` is uncommitted upstream and
owned by a concurrent session; it is not modified or staged by this wave's
booking commit. The following two additions are for the file's owner to
apply when that file is committed.

Append to the **C3** row's disposition cell (currently "OPEN; half-occupation
collision campaign infeasible under stated reference"):

```text
; wave 3 records the exact linear-response dispersion verdict and its registered CUDA verification ([wave 3](PROGRAM_STRICT_RECOVERY_WAVE_3.md))
```

Append to the **M4** row's disposition cell (currently "OPEN; requires C4 and
appropriate matter gates"):

```text
; field-sector hydrodynamic verdict recorded in wave 3
```

## Deviations from the spec (ruled)

Three points where the executed work departs from design section 3, each
ruled at the final whole-branch review and recorded here rather than silently
matched to the design text.

1. **H2 prediction formula.** `recovery_hydro_campaign.predict` computes
   `m(n) = W P(k)^n h0` — the full 192-channel period map `P(k)` propagated
   `n` times and projected onto the 7 conserved weights `W` only at readout —
   instead of design section 3.3's `m(n) = P_eff(k)^n m(0)` with `P_eff(k)`
   the 7x7 block of `P(k)`. Ruled: gate H1 found `block_dimension = 7`, i.e.
   the seven-space is not invariant under `M1`/`M2`; propagating a 7x7 block
   power would have assumed exactly the moment closure H1 had just refuted.
   `m(n) = W P(k)^n h0` tests the linearized-Boltzmann product closure on its
   own terms (does the full linear dynamics, viewed through the 7 conserved
   projections, match this prediction), not a closure the evidence already
   rejects.
2. **The phi = 1 density-mode preparation arm.** Design section 3.3 specifies
   preparing with phi being "(a) the density mode (`phi_c = 1`) and (b) each
   momentum-like right eigenvector from H1". `recovery_hydro_campaign.cases`
   runs seven arms, `mode in range(7)`, each a column of the equilibrium
   basis `V` (`recovery_hydro_campaign.mode_shape`) — none of the seven is
   the uniform `phi_c = 1` density-mode preparation, since that mode is not
   itself a column of `V`. Not run in this wave; recorded here for inclusion
   in the Phase 2 (H2') campaign design.
3. **Per-mode outputs.** Design section 3.2's "Outputs" (first-order
   propagation speeds, second-order damping rates per unit `|k|^2`, the
   density-mode diffusion coefficient) are supplied by the float64 mode table
   (`verdict.mode_table_float64`; see
   [DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md](DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md#mode-table-float64-report))
   rather than as exact rationals, because the label `"other"` (block
   dimension 7, not 4) leaves no 4-dimensional NS-class block for those exact
   per-mode quantities to be defined on; the density-mode diffusion
   coefficient specifically does not exist for this law, since the density
   functional is not an eigenfunctional of `M1`.

## Disposition

No new law or physical identity is adopted by this wave. H1's exact-track
verdict is `"other"`, not NS-class isotropic. H2's registered acceptance was
not met (1 of 42 cells) and is retained as an obstruction at the registered
scope, per the preregistration's no-retuning rule; the post hoc diagnostic in
the audit characterizes the measurement's statistical power without
reinterpreting the registered outcome. Per spec section 4.1, because H1's
verdict is not NS-class isotropic, Phase 2 (a priced FCHC-class successor
candidate) is triggered; that plan is written separately.

Before its own H2'-class campaign is locked, the Phase 2 (and any H2-prime)
plan requires: a pre-lock statistical power calculation against both the
seed-scatter noise floor (`F = rms(stderr)/rms(|predicted|)`, per the H2
audit's post hoc diagnostic above) and the epsilon^2 amplitude budget (per
[DERIV_STRICT_HYDRODYNAMIC_SECTOR.md](DERIV_STRICT_HYDRODYNAMIC_SECTOR.md#gate-h2-registered-cuda-verification)'s
regime paragraph); a registered epsilon-scaling control arm (multiple
`epsilon` values, since this wave ran only `epsilon = 1/4` with no scaling
check); and a rate-band tolerance that does not shrink with seed count alone
(the registered `2`-standard-error band narrows as `1/sqrt(seeds)` regardless
of whether the underlying signal is resolvable, which is exactly the low-power
failure mode the H2 audit's diagnostic identified in 28 of 42 cells).
