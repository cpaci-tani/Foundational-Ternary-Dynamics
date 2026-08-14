# FTD-0926 — Local remainder–velocity `C4` Hamiltonian and formation ledger v1

**Identifier:** `FTD-0926`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact onsite reversible generation of the FTD-0925 velocity orbit
from the already-existing subcell remainder and velocity variables; positive
quadratic storage, exact phase-read/force/movement ordering, zero-work and
formation-cost ledgers, and autonomy boundaries; no numerical search, fitted
coefficient, engine mutation, new ontology type, or `G*`/Born/Bell read

## 1. Question

FTD-0925 compiled the exact `C4` bridge current through a causal 20-site
ternary scaffold, but supplied its four velocity fields by hand. Does the
existing onsite pair

\[
 z_x=(r_x,v_x)
\]

of subcell remainder and velocity already have enough phase space to generate
that orbit by one homogeneous, target-blind, reversible local Hamiltonian?

The registered test must distinguish three questions:

1. whether an exact local generator exists;
2. whether it has a positive conserved storage functional and a finite
   preparation debit; and
3. whether production dynamics form, couple, or restore the resulting clock.

Success on the first two does not count as success on the third.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_RADIUS_TWO_CAUSAL_TERNARY_BRIDGE_SCAFFOLD_AFFINE_C4_FIELD_AND_AUTONOMY_BOUNDARY_v1.md` | `581D41914A0E60D1E2AAB5CC6D212FE8395F2AA20D52C91C9E6A01DB059CED39` |
| `proof_radius_two_causal_ternary_bridge_scaffold_affine_c4_field.py` | `62F7E3B5EA37FD8B00CC736CF2A507260313D8F5724E1A0562CEB4B870F9E1DC` |
| `THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md` | `982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6` |
| `proof_local_canonical_hamiltonian_parity_rail.py` | `B971DDA9A79AD53C340B00A4268EF9DA5BF089AF62DC37DE3D04757FAE03E326` |
| `engine/src/render_bridge.cpp` | `BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_forces.cpp` | `F7A855DC3ED3BF9882807CF7C8D1A35CF66864433B711CA5CA4B9CB836549322` |
| `engine/src/render_bridge_phases/phase_movement.cpp` | `6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB` |
| `engine/include/ftd/causal_kinematics.h` | `705501451985333D64128A0896216A137A2D836673AEB02E9ACE6DE4F2E53AA2` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |

The certificate fails closed on any source drift.

## 3. Frozen carrier orbit

The certificate must independently rebuild the FTD-0925 equal-five-channel
current, the 20-site neutral scaffold `h`, and the four live velocity fields

\[
 v_n={j_n\over h},\qquad
 v_2=-v_0,\qquad v_3=-v_1.
\]

It must reverify:

- current support 19 and scaffold support 20;
- exact live continuity on every arm;
- pointwise equality `|v_{n+1}(x)|^2=|v_n(x)|^2`;
- peak speed squared `8/25<1/3`; and
- `v_0` and `v_1` are globally orthogonal with equal norm.

No certificate value may be imported from console text.

## 4. Frozen local map

The phase at the start of the registered source-read arm is encoded by the
existing remainder

\[
 \boxed{r_0={v_0-v_1\over2}}.
\]

After the current has been read, apply the same onsite rule to every site and
every Cartesian component:

\[
 \boxed{v'=v-2r},\qquad
 \boxed{r'=r+v'=v-r}.                                      \tag{1}
\]

Thus, for `z=(r,v)^T`,

\[
 z'=\mathsf Mz,\qquad
 \mathsf M=
 \begin{pmatrix}-1&1\\-2&1\end{pmatrix}.                   \tag{2}
\]

The rule may read only the local `(r,v)` pair. It may not read site
coordinates, `h`, the desired arm, a stored phase label, `G*`, measurement
context, outcome, or Born weight.

The frozen order is source read with `v_n`, then (1), then movement with the
new `v_{n+1}`. This is the ordering already represented by production
`phase_read -> phase_forces -> phase_movement`, but equation (1) is not
presently a production force law.

## 5. Frozen Hamiltonian lift

Use the standard onsite symplectic form

\[
 \mathsf J=
 \begin{pmatrix}0&1\\-1&0\end{pmatrix}
\]

and the symmetric metric

\[
 \mathsf G=
 \begin{pmatrix}2&-1\\-1&1\end{pmatrix}.                   \tag{3}
\]

The certificate must prove exactly

\[
 \mathsf M^2=-I,\qquad
 \mathsf M^4=I,\qquad
 \mathsf M^T\mathsf J\mathsf M=\mathsf J,
\]

\[
 \mathsf M^T\mathsf G\mathsf M=\mathsf G,\qquad
 \mathsf J\mathsf G=\mathsf M,
\]

and

\[
 \operatorname{spec}(\mathsf G)
 =\left\{{3-\sqrt5\over2},{3+\sqrt5\over2}\right\}>0.
\]

For

\[
 \mathcal E(r,v)
 =|r|^2-r\cdot v+\frac12|v|^2
 ={1\over2}z^T\mathsf Gz,                                 \tag{4}
\]

define the onsite positive Hamiltonian

\[
 H_{\rm rv}
 =\omega\sum_{x\in\operatorname{supp}h}\mathcal E(r_x,v_x).
                                                                    \tag{5}
\]

Hamilton's equation is

\[
 \dot z_x=\omega\mathsf Mz_x.
\]

At a declared sampling duration `T` with

\[
 \omega T={\pi\over2},                                     \tag{6}
\]

its exact flow is equation (2). The dimensionful scale `omega` is imposed by
the sampling convention; no `G*` period follows.

## 6. Exact orbit and local stability gates

Starting from `(r_0,v_0)`, four applications of (1) must give

\[
\begin{array}{c|c|c}
n&r_n&v_n\\ \hline
0&(v_0-v_1)/2&v_0\\
1&(v_0+v_1)/2&v_1\\
2&(v_1-v_0)/2&-v_0\\
3&-(v_0+v_1)/2&-v_1.
\end{array}                                                \tag{7}
\]

The next update must return both fields exactly. Every registered remainder
component must remain strictly inside `(-1,+1)`.

Because `\mathsf M^4=I` on the full local phase space, every sufficiently
small remainder/velocity perturbation is bounded and returns to its own
perturbed state after four steps. This is neutral reversible stability. It
is not attraction to the registered orbit, repair of a perturbed ternary
record, or coupled field-source recovery.

Controls:

- the zero state is fixed;
- `\mathsf M^{-1}=-\mathsf M` gives exact reverse orientation;
- velocity alone is insufficient: the FTD-0925 orbit contains identical
  onsite velocities with different successors;
- forcing `r_0=0` does not generate the registered orbit under (1); and
- no site-dependent matrix or hidden phase table is permitted.

## 7. Energy, work, and preparation ledger

The certificate must derive

\[
 \sum_x|v_0(x)|^2={104\over25},\qquad
 \sum_x|r_0(x)|^2={52\over25},
\]

\[
 \boxed{\sum_x\mathcal E(r_n,v_n)={52\over25}}
 \quad\hbox{on every arm}.                                 \tag{8}
\]

For unit global-tick sampling, equation (6) gives the positive stored carrier
energy

\[
 \boxed{H_{\rm rv}={26\pi\over25}}.                         \tag{9}
\]

More generally the invariant is `52 omega/25`. Equation (9) is a selected
reference normalization, not a substrate energy prediction.

Pointwise equal speeds require

\[
 (v_{n+1}-v_n)\cdot{v_{n+1}+v_n\over2}=0,                  \tag{10}
\]

so every isotropic speed-only matter dispersion has zero endpoint kinetic
change. This is consistent with the FTD-0925 zero scalar matter-work ledger.
It does not derive the vector field impulse, reciprocal recoil, or a common
matter-field action.

Relative to the empty `(r,v)=(0,0)` carrier, preparation owes the positive
debit in (9), plus the already-booked static halo energy `E_h>0`. The ternary
manifestation energy, reservoir that pays the debit, and autonomous formation
transaction remain open. No “formation from zero” claim may be made.

## 8. Frozen outcomes

- **Outcome A — exact existing-type local generator:** all source, orbit,
  symplectic, positivity, ordering, energy, and firewall gates pass. Book an
  autonomous target-blind reference recurrence and positive storage ledger.
  Keep production insertion, field recoil/common action, formation,
  dissipation/reset, reference-orbit recovery, scale, and `G*` open.
- **Outcome B — phase datum still required:** a reversible generator exists
  only after adding a site-dependent equilibrium center, phase table, or new
  conjugate variable. Book the exact added datum and do not call the
  FTD-0925 remainder/velocity pair sufficient.
- **Outcome C — registered class closes negative:** no positive symplectic
  onsite map realizes the frozen orbit and ordering.
- **Invalid:** source drift, modified FTD-0925 current/scaffold, post-lock
  coefficient change, target-arm read, fitted tolerance, or failed combined
  gate.

## 9. Firewalls

The certificate must contain no parameter sweep, near-miss search,
formula-substitution discovery, or production execution. It changes no
engine source, CMake target, toggle, default, current type, import, or
selected ontology type.

Even Outcome A does not establish spontaneous scaffold formation, a native
field-derived force, reciprocal source recoil, asymptotic perturbation
recovery, mobility, physical scale, critical-quartic dynamics, `G*` cadence,
Born frequencies, Bell correlations, measurement context, or operational
hiding.
