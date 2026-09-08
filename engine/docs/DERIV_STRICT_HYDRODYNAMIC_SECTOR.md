# Hydrodynamic limit of the staged field sector: Phase 1 booking

Date: 2026-09-08. Law: `phi-v2-staged-candidate-1` (`engine/strict`,
`scripts/phi_v2_lattice`). Program: strict-discrete stack, recovery wave 3.
Design: [Hydrodynamics from the strict Phi law: design](../../docs/superpowers/specs/2026-09-07-phi-hydrodynamics-design.md).
This document books Phase 1's three gates (H0, H1, H2) at their tags and
records the Phase 2 trigger decision. It introduces no new claim, promotes no
tag, and adds no physical unit calibration.

## Purpose and scope

In scope: the field sector of the current law on the homogeneous
doubly-occupied relation background (`s = 0`, uniform ell, both A9 slots of
every SC/FCC relation occupied by the same nonblank code), single polarity,
finite periodic torus. Exact linear-response (Boltzmann, product-closure)
hydrodynamics of that sector; one registered CUDA verification; a priced
successor law if the current law's hydrodynamics is not Navier-Stokes-class.

Boundaries that hold throughout this document. No Clay Millennium claim of
any kind (LEDGER FTD-0043 remains retracted). No physical unit calibration:
lattice spacing and microtick are the only units, and `a`, `tau` are external
comparison choices. Token count is not energy or mass in physical units. A
passing software suite is not a physical recovery gate. Phase 2 is an
adoption at a declared price, never a derivation.

## Gate H0: invariant census

**Tag: `[THEOREM — finite, exact, scoped to the field sector on the frozen
background]`.**

Source: `scripts/phi_v2_lattice/recovery_hydro_invariants.py`, evidence
`evidence/strict-recovery-wave3-exact.json` key `invariants`, law
`phi-v2-staged-candidate-1`.

Fixed-weight additive invariants of the three collision layers plus
streaming, per polarity: dimension 1 (the population). Layer covariance
`U C_q = C_{q-1} U`: 0 violations. Locked-schedule invariants
(`w_t = w_0 . U^{-t}`, layer `(l0 - t) mod 3`): dimension 7 per polarity for
every `l0` in {0, 1, 2}. The tangent 3-vector lies inside the locked span and
not inside the fixed span.

Fourth-rank isotropy table (`T_xxxx = 3 . T_xxyy` tested):

| Velocity set | T_xxxx | T_xxyy | T_xx | T_xy | isotropic (rank 2) | isotropic (rank 4) |
|---|---:|---:|---:|---:|---|---|
| body_diagonal_8 | 8 | 8 | 8 | 0 | yes | no |
| face_edge_18 | 10 | 4 | 10 | 0 | yes | no |
| moore_26 | 18 | 12 | 18 | 0 | yes | no |
| fchc_projected_18 | 12 | 4 | 12 | 0 | yes | yes |

Only the FCHC-projected 18-velocity set satisfies the rank-4 isotropy test;
the body-diagonal, face+edge, and Moore sets fail it.

## Gate H1: exact small-wavevector dispersion

**Tag: `[DERIVED — linearized Boltzmann (product) closure at the declared
reference; correlation leakage not bounded here]`.**

Contract and results:
[DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md](DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md).
Evidence: `evidence/strict-recovery-wave3-exact.json` keys `dispersion` and
`verdict`. Exact track computed at the declared proxy coupling
`r0 = 1/695`; certified track at the physical `r(p)` for
`p in {1/96, 1/192, 1/48}` using 256-bit ball arithmetic.

**Exact-track verdict** (`verdict.exact`): `label = "other"`. Rank of
`(I - P0)` on the 185-dimensional complement: 185. `block_dimension = 7`;
`block_dimension_first_order_only = 7`. Clause 1 of the NS-class-isotropic
rule fails: the smallest row space containing the density functional that is
invariant under `M1` and `M2` for all three directions spans all seven
conserved moments, and the first-order operators alone already close the
same seven-dimensional space, so no 4-dimensional closed block exists.

Per-direction characteristic polynomial of `M1` on the closure block
(coefficients low to high; identical to the full `M1` polynomial since the
block is the whole 7-dimensional space):

- n = (1,0,0): `[0, -256/3, 0, 176/3, 0, -40/3, 0, 1]`
- n = (1,1,0): `[0, -2048/3, 0, 704/3, 0, -80/3, 0, 1]`
- n = (1,1,1): `[0, -2304, 0, 528, 0, -40, 0, 1]`

**Certified-track statuses** (256-bit ball arithmetic), at every reference
density `p in {1/96, 1/192, 1/48}`: `block_dimension = 7`;
`sound_speed_direction_independent`, `transverse_isotropic`, and
`longitudinal_isotropic` each report `"NOT APPLICABLE (block dimension 7 !=
4)"`. The certified enclosures contain the exact-track rationals at `r0`, as
required.

Unit-modulus spectrum of `P(0)`: count 7 (float64 report, not an acceptance
criterion), matching the seven conserved modes.

Runtime: two foreground runs of
`PYTHONPATH=scripts python -m phi_v2_lattice.experiments.run_recovery_wave3 --output engine/docs/evidence/strict-recovery-wave3-exact.json`
produced byte-identical output files in 76.322 s and 76.597 s.

This is a linear-response prediction on the exact finite operators of the
law, not a theorem about Phi; gate H2 tests the closure assumption.

## Gate H2: registered CUDA verification

**Tags: `[MEASURED — instrument and provenance verified]` for the campaign
itself; `[CLOSED NEGATIVE at the registered scope]` for the Boltzmann-closure
hypothesis at this preparation.**

Preregistration:
[PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md](PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md).
Audit: [AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md](AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md).
Evidence: `evidence/strict-hydro-response-v1.json`.

Registered matrix: periodic `L = 32`, single polarity, 23 stroboscopes of 48
microticks each (1,104 microticks per case), 3 directions x 2 wavenumbers x 7
stage-0 conserved modes x 8 seeds = 336 cases, 370,944 physical microticks
total. Executed on the WSL2 CUDA backend (NVIDIA GeForce RTX 5090, compute
capability 12.0) in 3388.7405394000234 s elapsed (execution receipt).

Registered acceptance (relative RMS deviation of the seed-mean trajectory
from the locked prediction at most 1/10, jointly with the measured e-folding
rate inside a 2-standard-error band of the prediction) was met in 1 of 42
cells; `boltzmann_closure_verified_at_registered_scope = false`. Relative RMS
across the 42 cells ranges from 0.077985 to 0.880760. The measured rate is
below the predicted rate in 41 of 42 cells.

The independent audit reproduces the retained campaign report (all seven
checked keys match the committed evidence exactly, including
`boltzmann_closure_verified_at_registered_scope = false`); an
out-of-pipeline recomputation of the locked prediction for one case matches
`predictions.json` exactly; the pure-Python reference matches an independent
CUDA invocation after 8 microticks for two replayed cases; and an
independent, out-of-campaign 48-microtick CUDA run reproduces the campaign
trace's first stroboscope hash for the same two cases.

The audit also computes a post hoc diagnostic on the retained report — never
a change to the registered acceptance rule. Of the 42 registered cells, the
seed-scatter noise-floor ratio `F = rms(stderr)/rms(|predicted|)` is at or
above the registered relative-RMS tolerance (`F >= 0.1`) in 28 of 42 cells,
and the worst-point-to-worst-standard-error ratio `D` is at or below 2 in 1
of 42 cells. As the audit states, a noise floor at or above the registered
tolerance means the registered criteria had low statistical power at this
signal-to-noise ratio for the affected cells, and this neither verifies the
Boltzmann closure nor retroactively passes or fails any cell under the
registered acceptance rule.

## Regime diagnostic (not an acceptance; float64 report)

The horizon rule used in the preregistration is set by the slowest of the
seven registered modes' predicted per-period decay rates at the registered
wavevectors, computed at `L in {16, 32, 48, 64}`:
0.01034575894892908, 0.01018756463675881, 0.010154744789741202,
0.010139334707868304 respectively — essentially independent of `L` (hence of
`k`), which is the signature of a collisional relaxation rate rather than a
vanishing hydrodynamic one. The `k = 0` spectrum of the same period map at
`p = 1/96` has its top 7 eigenvalue moduli exactly at 1 (the conserved
modes) and its eighth-largest modulus at 0.97392369, giving a per-period
decay rate of 0.026422330418528987 for the fastest-decaying mode adjacent to
the conserved sector. At the registered wavevectors
(`2 pi m / 32`, `m in {1, 2}`) the hydrodynamic modes decay faster than this
collisional relaxation, so the campaign probed the kinetic regime of the
linearized period map, not a hydrodynamic limit; a hydrodynamic window would
require wavevectors small enough that the slow-mode decay falls below this
rate, which the throughput probe places beyond the feasible lattice sizes.

## Phase 2 trigger decision

H1 verdict is "other"; per spec section 4.1 Phase 2 is built. Had the verdict
instead been NS-class isotropic, Phase 2 tasks would have been skipped and
gate H2 would have been extended with a registered shear-wave viscosity
measurement on the current law instead. The Phase 2 plan itself is written
separately.

## Reproduction

H0 and H1 (exact census and dispersion, single command, from the repository
root):

```text
PYTHONPATH=scripts python -m phi_v2_lattice.experiments.run_recovery_wave3 --output engine/docs/evidence/strict-recovery-wave3-exact.json
```

H2 throughput probe (WSL2 CUDA CLI must be built; see
`engine/docs/PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md`):

```text
PYTHONPATH=scripts python -m phi_v2_lattice.experiments.probe_strict_cuda_throughput --output engine/docs/evidence/strict-hydro-throughput-2026-09.json
```

H2 campaign (lock, run, summarize; `L = 32`, `stroboscopes = 23` as
registered):

```text
PYTHONPATH=scripts python -c "from phi_v2_lattice import recovery_hydro_campaign as C; print(C.prepare_campaign('engine/build_strict_hydro/campaign_v1', L=32, stroboscopes=23))"
PYTHONPATH=scripts python -c "from phi_v2_lattice import recovery_hydro_campaign as C; print(C.run_campaign('engine/build_strict_hydro/campaign_v1'))"
PYTHONPATH=scripts python -c "from phi_v2_lattice import recovery_hydro_campaign as C; r=C.summarize_campaign('engine/build_strict_hydro/campaign_v1'); print(r['groups_passing'], '/', r['groups'])"
```

H2 independent replay:

```text
PYTHONPATH=scripts python -m phi_v2_lattice.experiments.replay_hydro_case
```
