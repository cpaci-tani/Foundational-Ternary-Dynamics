# FTD-0995 — Preregistration: crossing-matched formation energy and causal quartic-clock growth v1

**Identifier:** `FTD-0995`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — NOT YET EVIDENCE]`  
**Expected classifier:** **Outcome B — exact compliance-surface growth / autonomous matching open**

## 1. Question

FTD-0993/0994 proves that positive local formation work and retained temporal
orientation can start an existing zero Cartesian clock pair. It also proves
that a coherent extended clock cannot be written globally in one local tick.
This discriminator asks whether the local result composes into an exact
causal growth law at clock crossings.

Specifically:

1. what local condition is necessary and sufficient for a newly occupied
   site to inherit the donor site's complete Cartesian clock state;
2. whether the occupancy-flip work can pay the new site's clock energy with
   an exact inverse and no target-phase or `G*` read;
3. whether the enlarged uniform manifold is dynamically invariant under the
   occupancy membrane and identical onsite clocks;
4. whether a Moore-local independent frontier can grow coherence no faster
   than one shell per admitted crossing; and
5. whether the selected critical quartic clock then inherits its amplitude,
   orientation, period, and CM calendar without an additional gearbox.

No engine or production mutation is authorized.

## 2. Frozen sources

| source | SHA-256 |
|---|---|
| `THEOREM_ZERO_ACTION_CANONICAL_SEED_AND_CAUSAL_CLOCK_GROWTH_BOUNDARY_v1.md` | `897367658B339F074A78FEA017994EEA63AD7921BA4C597663EA123088E76306` |
| `THEOREM_LOCAL_OCCUPANCY_FLIP_FORMATION_WORK_AND_MINIMUM_ACTIVE_APERTURE_v1.md` | `E4D4BBCF2A0E09953EA2107FD80954E50BB2ED9BE45A9C9C6D2381DA018D7B9F` |
| `THEOREM_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_BODY_CLOCK_SPLIT_v1.md` | `A19593DACD2CE97A6B785F235AE5048EADC228680E07D2F90F4C4DB7BD15333C` |
| `DERIV_QUARTIC_CLOCK_CM_GEARBOX_v1.md` | `1B969544B065D576523235F40A20918C22E0C55978E52282E2FC623385BC2FDF` |
| `DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md` | `779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/transmutation_phases.cpp` | `4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043` |

Any mismatch invalidates execution. A repair must preserve this protocol and
the first certificate byte-for-byte.

## 3. Registered crossing transaction

Use mass-normalized Cartesian pairs. Let an occupied donor `x` lie on the
kinetic crossing

\[
 q_x=0,\qquad p_x\ne0,\qquad
 \sigma=\operatorname{sgn}(p_x).                       \tag{1}
\]

Let an adjacent prospective site `y` have

\[
 q_y=p_y=0.                                             \tag{2}
\]

For the proposed occupancy change, let `W_y` be the exact FTD-0992 local
formation work, including every incident cut/join bond and every registered
onsite load. Define the available released work

\[
 U_y=-W_y.                                              \tag{3}
\]

Admission requires `U_y>0`. The FTD-0994 Cartesian shear then gives

\[
 q_y'=0,\qquad p_y'=\sigma\sqrt{2U_y}.                 \tag{4}
\]

At the donor crossing its per-site clock energy is

\[
 e_x={p_x^2\over2}.                                    \tag{5}
\]

The registered **self-dual compliance scalar** is

\[
 \boxed{C_{xy}=2U_y-p_x^2.}                            \tag{6}
\]

The candidate growth event is admitted only when `C_xy=0`. The term
"self-dual" means only that the disappearing membrane work and appearing
clock energy are the same local scalar. It is not a new ontological duality
or a claim that production enforces equation (6).

## 4. Registered exact consequences

On equations (1)--(6), prove

\[
 \boxed{(q_y',p_y')=(q_x,p_x)}.                        \tag{7}
\]

Conversely, equation (7) under the admitted sign branch must imply
`C_xy=0`. Thus the equality of formation work and donor crossing energy is
necessary and sufficient in the registered seed class; it is not a fitted
coefficient.

If the membrane/source sector loses `U_y`, equation (4) must give the exact
ledger

\[
 \Delta H_{\rm membrane}=-U_y,
 \qquad \Delta H_{\rm new\ clock}=+U_y,
 \qquad \Delta H_{\rm total}=0.                        \tag{8}
\]

At a later identical crossing, the inverse `-sigma` shear must clear the
receiver momentum and the reverse occupancy flip must restore `U_y`. The
orientation/aperture record is retained until this reversal.

## 5. Uniform-manifold and causal-front tests

Let `S` be a connected occupied component carrying an exact uniform
Cartesian state

\[
 q_z=q,\qquad p_z=p\qquad(z\in S).                     \tag{9}
\]

For the FTD-0990 occupancy Laplacian `K_m`, prove `K_m 1_S=0`. Therefore,
for any identical onsite Hamiltonian

\[
 h(q,p)={p^2\over2}+V(q),                              \tag{10}
\]

the uniform manifold is invariant while occupancy is fixed: all sites obey
the same onsite equation and the membrane force vanishes.

If a prospective site is added by equation (7), the enlarged component
starts on the same uniform manifold. Induction must then establish that a
sequence of admitted local events grows exact coherence causally.

For simultaneous reference growth, restrict the admitted set `F_n` to a
Moore-independent frontier: no two sites of `F_n` share a C18 bond. Then the
affected edge sets are disjoint and

\[
 W_{F_n}=\sum_{y\in F_n}W_y.                            \tag{11}
\]

A fixed coordinate-parity color is an allowed deterministic scheduling
witness, not a derived controller. Every newly coherent site after `r`
growth events must lie within Moore graph distance `r` of the initial seed.

## 6. Critical-quartic inheritance and mismatch boundary

For the selected critical quartic onsite law

\[
 h_4(q,p)={p^2\over2m}+\lambda q^4,
 \qquad m,\lambda>0,                                  \tag{12}
\]

use the corresponding mass-normalized momentum in equations (4)--(6), or
set `m=1` in the certificate. At a kinetic crossing, `E=lambda A^4` and

\[
 TA=\sqrt\pi G^*\sqrt{m\over2\lambda}.                \tag{13}
\]

Equation (7) must imply equal energy, amplitude, Hamiltonian orientation,
period, and normalized CM energy shell for donor and receiver. This is
inheritance from identical state, not a new derivation of the quartic law or
of `G*`.

For `U_y\ne e_x` on the same sign branch, the receiver amplitude and period
obey

\[
 {A_y\over A_x}=\left({U_y\over e_x}\right)^{1/4},
 \qquad
 {T_y\over T_x}=\left({e_x\over U_y}\right)^{1/4}.     \tag{14}
\]

Thus generic work mismatch detunes a critical quartic clock. The event must
fail closed, export/repair the mismatch through a separately derived local
port, or accept loss of exact coherence. A harmonic isochronous control may
retain phase despite amplitude mismatch; it does not invalidate (14).

## 7. Exact gates

### G1 — source lock

- all nine frozen hashes match;
- the sources contain the exact cut-set work, Cartesian shear, occupancy
  membrane/uniform mode, and critical-quartic period law;
- production lacks the composed crossing admission law, compliance scalar,
  coherent frontier, and exact inverse transaction.

### G2 — necessity and sufficiency

Prove equations (4)--(7) in both directions for `sigma=+1` and `sigma=-1`.
Reject `U<=0`, `p_x=0`, sign mismatch, and `C_xy!=0` as exact coherent-growth
events.

### G3 — energy and inverse

Prove equation (8), exact clearing by the inverse shear on the registered
crossing, restored formation energy under the reverse flip, and retention of
the orientation record. Do not generalize the seam ledger off crossing.

### G4 — invariant uniform manifold

Prove the incidence/Laplacian kernel, identical onsite flow, enlarged
uniform-state induction, and zero membrane current on the exact manifold.

### G5 — local concurrency and causal cone

Prove equation (11) for a Moore-independent simultaneous frontier, exact
per-event accounting, one-shell support growth, and the graph-distance lower
bound. Book the frontier selection/scheduling rule as imposed reference
control.

### G6 — quartic gearbox and mismatch

Prove equations (13)--(14), equal CM normalization/orientation under exact
state inheritance, and the absence of any explicit `G*` or target-phase read
from the admission map. Separate the harmonic isochronous control from the
critical-quartic mismatch result.

### G7 — epistemic and production firewalls

Explicitly reject promotion to:

- an autonomous mechanism that drives `C_xy` to zero;
- a derivation of `U_y`, the selected membrane stiffness, quarticity,
  `m`, `lambda`, amplitude, or `G*`;
- a generic attractive synchronization law or robustness result;
- a production genesis/evaporation/front implementation;
- a Born/Bell, probability, measurement, mass, Lorentz-hiding,
  consciousness, biological, or completeness result.

No fit, numerical near-miss search, parameter scan, formula substitution, or
engine mutation is permitted.

## 8. Classifier

- **Outcome A — autonomous native clock growth:** every gate passes and the
  frozen production sources already generate positive `U_y`, enforce
  `C_xy=0`, schedule the frontier, retain the inverse record, and realize the
  quartic clock.
- **Outcome B — exact compliance-surface growth / autonomous matching open:**
  G2--G6 pass, but formation-energy matching, frontier control, quartic
  hardware, or production remain selected/absent.
- **Outcome C — phase-only growth:** the receiver obtains the donor phase but
  not exact energy, amplitude, inverse, causal accounting, or quartic cadence.
- **Outcome D — invalid:** a source hash or exact gate fails.

Outcome B is expected. Outcome A is forbidden unless the frozen production
sources contain the complete mechanism.
