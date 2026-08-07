# FTD-0427 — Projection-Free Matched Gauss Transport

**Status:** [THEOREM — SELECTED DISCRETE COMPLEX] +
[MEASURED — PRODUCTION MOVEMENT COMPATIBILITY] + [SELECTED MECHANISM]
**Verdict:** `A_SELECTED_LOCAL_PROJECTION_FREE_TRANSPORT`
**Date:** 2026-07-23
**Pre-registration:**
[`PREREG_MATCHED_GAUSS_TRANSPORT_v1.md`](../../10_eft_program/preregistrations/lorentz_recovery_causal_structure/PREREG_MATCHED_GAUSS_TRANSPORT_v1.md)

**Successor notice (FTD-0428, 2026-07-23):** the licensed default-off
integration was executed. Minimum-energy dressing, coupled transverse
face/edge evolution, production `Voxel::flux` mirroring, and modified energy
pass at the selected-engine-extension scope. Sections 6–7 below remain the
historical FTD-0427 boundary; see
[`AUDIT_MATCHED_MAXWELL_INTEGRATION.md`](AUDIT_MATCHED_MAXWELL_INTEGRATION.md)
for the current boundary. Reactions, forces, native `U(1)`, and common-cone
recovery remain unresolved/negative as stated there.

## 1. Result

One local mechanism now transports a Gauss source without repeatedly solving
a global Poisson problem. Put flux `J` on oriented positive-axis faces, extract
the integrated signed face current `K` from actual production movement, and
update

\[
J^{n+1}=J^n-K+C B.
\]

With the matched backward-difference divergence `D` and curl `C`,

\[
D C=0,
\qquad
\Delta s+D K=0
\quad\Longrightarrow\quad
D J^{n+1}=s^{n+1}
\]

whenever `D J^n=s^n`. The first identity is exact operator algebra; the second
was measured on production movement histories. Gauss projection was disabled
for every campaign tick.

This closes only the **mechanism-sufficiency** question. The sidecar is not the
production `Voxel::flux` update and does not establish native charge emergence.

## 2. Independent derivation

Define the backward difference

\[
\delta_x^- f(x,y,z)=f(x,y,z)-f(x-1,y,z)
\]

and cyclic analogues. The face divergence is

\[
D J=\delta_x^-J_x+\delta_y^-J_y+\delta_z^-J_z.
\]

The matched curl is

\[
C B=
\begin{pmatrix}
\delta_y^-B_z-\delta_z^-B_y\\
\delta_z^-B_x-\delta_x^-B_z\\
\delta_x^-B_y-\delta_y^-B_x
\end{pmatrix}.
\]

All backward differences commute on the periodic finite lattice, so the six
terms in `D C B` cancel pairwise. If a conservative movement history obeys
`s^{n+1}-s^n=-D K`, then

\[
D(J^n-K+C B)=s^n-DK=s^{n+1}.
\]

No continuum limit, gauge principle, or fitted coefficient enters this proof.
The theorem is conditional on the selected placement and update rule.

## 3. Campaign

Each arm contained one mobile sign `q` and one locked sign `-q`. An exact
oriented-face path supplied the initial dipole flux. This is a flux string,
not a Coulomb profile. Production movement then advanced the mobile polarity
at `0.99*C_SPEED` for 12 ticks along each of the six axial directions, followed
by 8 stationary ticks. Both `q=+1` and `q=-1` were run.

After each tick, the existing native finite-volume history extractor routed
the signed movement current. The sidecar applied `-K` and a fixed nonzero
matched-curl challenge of amplitude `10^-3`. Closed cubes at radii 2, 3, and 4
measured the transported charge. `gauss_projection` remained false.

Runs of record:

| compiler/platform | sizes | rows | result |
|---|---:|---:|---|
| Windows 11, MSVC 14.44 CPU | 32, 64 | 1,440 | PASS |
| WSL2 Ubuntu-22.04, GCC CPU | 32, 64 | 1,440 | PASS |

Each size contains 12 sign/direction arms and 720 rows. Every arm produced
seven actual movement events.

## 4. Frozen gates

All 73 result-verifier checks passed.

| quantity | worst observed | gate |
|---|---:|---:|
| transport continuity residual | `0` | `1e-12` |
| `max |D C B|` | `4.34e-19` | `1e-12` |
| `max |D J-s|` | `1.89e-15` | `1e-12` |
| surface/divergence telescope | `4.44e-16` | `1e-12` |
| surface-charge error from `q` | `1.44e-15` | `1e-12` |
| radius plateau | `4.44e-16` | `1e-12` |
| stationary current | `0` | exactly zero |
| reaction L1 | `0` | exactly zero |

MSVC and GCC rows align exactly and all numeric values agree within `1e-12`.
The source/protocol lock passes 21/21.

## 5. What changed relative to FTD-0426

FTD-0426 showed that the production Gauss projector could create a good
static surface readout but that the frozen live profile did not preserve it.
FTD-0427 identifies a sufficient local alternative: route the same current
that moves the source and use a matched operator complex. The resulting Gauss
relation propagates from the initial data and needs no repeated projection.

The improvement is architectural, not evidentiary promotion. FTD-0427's field
is an experimental sidecar and its update was selected to preserve the
constraint. Therefore it does not show that the native engine generated the
mechanism spontaneously.

## 6. Remaining defects

1. The initial field is a thin oriented path, not a relaxed `1/r^2` electric
   field. Coulomb dressing and energy minimization are untested.
2. The transverse challenge tests `D C=0`; it is not a coupled Faraday/Ampere
   wave evolution and supplies no photon pole.
3. The sidecar does not write `Voxel::flux`, affect forces, or participate in
   the production energy ledger.
4. The source dictionary is tested only for reaction-free movement. FTD-0421
   still proves that no nontrivial additive charge in the frozen basis survives
   genesis, evaporation, pair production, annihilation, and weak
   transmutation together.
5. No local gauge redundancy, Ward identity, common light/matter cone,
   radiative protection, or empirical charge-violation bound follows.

## 7. Licensed successor

The next campaign may integrate this matched face-current update behind a
default-off experimental toggle, initialize an extended minimum-energy dipole
once, and test whether coupled longitudinal/transverse evolution preserves
Gauss, a radius-independent static field, finite-speed signals, and positive
energy without projection. That integration is a new selected engine branch
and requires a new lock. Full production reactions remain excluded until an
emergent charge dictionary independent of primitive signed state is supplied.

## 8. Artifacts

- selected operator: `engine/include/ftd/eft/matched_gauss_transport.h`
- implementation: `engine/src/eft/matched_gauss_transport.cpp`
- exact unit test: `engine/tests/test_matched_gauss_transport.cpp`
- production-driven campaign: `engine/tests/campaign_matched_gauss_transport.cpp`
- source lock: `scripts/proofs/matched_gauss_transport_lock.json`
- lock verifier: `scripts/proofs/proof_matched_gauss_transport_lock.py`
- result verifier: `scripts/proofs/proof_matched_gauss_transport_results.py`
- run records: `engine/results/ftd_0427/`
