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
| `engine/docs/evidence/strict-recovery-wave3-exact.json` | `3c7c6a4679e5babe9bf8b222b3ea11367b3bbfe765d80265e995ef49fd404a3c` |
| `engine/docs/evidence/strict-hydro-throughput-2026-09.json` | `4c6f0c86fcfd3595e9dbac09bb465785291e9b1a560c0c2f0fbfc3b06a8de998` |
| `engine/docs/evidence/strict-hydro-response-v1.json` | `092d36821051b9f234ea24566d6b939eceaa5b1fba24f0ef8c41caa40b5dc4e6` |

The response-campaign hash matches the independently rehashed value recorded
in [AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md](AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md).

## Integrated evidence

From the worktree root, in Git Bash:

```bash
FTD_STRICT_CUDA_REQUIRED=1 python -m pytest scripts/tests/phi_v2_lattice -q -p no:cacheprovider
```

Result: **468 passed, 45 skipped in 114.67 s (0:01:54)** (pytest also reports
1 unrelated warning: an unknown `cache_dir` config option, not a test
result). `FTD_STRICT_CUDA_REQUIRED=1` turns a missing CUDA binary/device into
a hard failure for `test_cuda_parity.py` specifically; none of that suite's
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

## Disposition

No new law or physical identity is adopted by this wave. H1's exact-track
verdict is `"other"`, not NS-class isotropic. H2's registered acceptance was
not met (1 of 42 cells) and is retained as an obstruction at the registered
scope, per the preregistration's no-retuning rule; the post hoc diagnostic in
the audit characterizes the measurement's statistical power without
reinterpreting the registered outcome. Per spec section 4.1, because H1's
verdict is not NS-class isotropic, Phase 2 (a priced FCHC-class successor
candidate) is triggered; that plan is written separately.
