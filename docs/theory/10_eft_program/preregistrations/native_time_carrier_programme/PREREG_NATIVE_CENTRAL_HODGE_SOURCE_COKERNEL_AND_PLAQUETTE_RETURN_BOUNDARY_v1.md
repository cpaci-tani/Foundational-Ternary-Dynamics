# FTD-0920 — Native central-Hodge source cokernel and plaquette-return boundary v1

**Identifier:** `FTD-0920`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact range of the unchanged production density/current source,
the unique source needed to close a prescribed circulation doublet, and the
elementary-plaquette obstruction; no numerical search and no engine change

## 1. Question

FTD-0919 proved that the free `C18` field has global conserved modal
circulation but no nonzero compact finite-dimensional invariant body. Before
pricing a new nonlinear confinement law, can the already-coded reciprocal
density/current source supply the exact boundary return that closes the
elementary `C4` plaquette?

The production source is

\[
 \mathcal H(\rho,j)=-G_C\nabla_c\rho+G_C\operatorname{curl}_c j,
 \qquad \rho=s\in\{-1,0,+1\},\quad j=s v.
\]

The protocol first enlarges the source domain to independent continuous
`rho` and `j`. Failure in that relaxed domain is a fortiori failure for the
live ternary source. Success in the relaxed domain is not counted as a live
production realization.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `AUDIT_NATIVE_FIELD_DISCRETE_ACTION.md` | `5EDC7F8C81456BEE4EEB061168154E8EF4D8347B8948C429BB40B8306FFC8AD8` |
| `AUDIT_NATIVE_HODGE_ENERGY_CONTINUITY.md` | `033985919FAC722F47B09311D51B47E5DDB4E5A3A47D0A3F36B736CFAF481D08` |
| `THEOREM_MINIMAL_MOORE_COMPATIBILITY_COAT.md` | `49F41E31DFA9542B2BD7AB0A224808C48D06164967A71139D9C4B7BFB5EBA7B7` |
| `THEOREM_NATIVE_PLAQUETTE_C4_CIRCULATION_AND_EMBEDDED_LEAKAGE_BOUNDARY_v1.md` | `3CD336B101BDA6A4F0E56CBFFC9428C203C5A68E037943408D762900FF58451F` |
| `THEOREM_NATIVE_C4_MODAL_CIRCULATION_AND_COMPACT_SUPPORT_OBSTRUCTION_v1.md` | `CA05D786A73775B398F90EE33E207E2A4D3522D49ECA86B9BF5774E2D6B1A285` |

The certificate fails closed on source drift.

## 3. Frozen return-source identity

Let `K=-C_WAVE^2 Delta_18` and let a fixed body subspace have embedding `B`.
To make its desired internal stiffness `K_b` exact under the production-order
kick

\[
 P^+=P-KJ+U,
\]

every body state `J=Bq` must obey

\[
 P^+=P-BK_bq.
\]

The certificate must derive the unique required feedback impulse

\[
 \boxed{U_{\rm ret}(Bq)=(KB-BK_b)q.}
\]

For an isotropic `C4` doublet, `K_b=kappa I`, hence

\[
 \boxed{U_{\rm ret}=(K-\kappa I)J.}
\]

This is a boundary/leakage return law. It is not assumed to arise from the
live source and is not an autonomous energy reservoir.

## 4. Frozen central-Hodge range theorem

On an even periodic quotient, remove the irrelevant nonzero factor `G_C` and
write

\[
 d(k)=(\sin k_x,\sin k_y,\sin k_z).
\]

Up to a common factor of `i`, the source symbol is

\[
 H(d)=\begin{bmatrix}-d&C(d)\end{bmatrix},
 \qquad C(d)j=d\times j.
\]

The certificate must prove

\[
 \boxed{H(d)H(d)^T=|d|^2I_3.}
\]

Therefore:

1. `rank H=3` at every mode with `d!=0`;
2. `rank H=0` at the eight corner modes
   `k_epsilon=pi epsilon`, `epsilon in {0,1}^3`;
3. on an even `L^3` periodic quotient,

   \[
   \boxed{\operatorname{Ran}\mathcal H
   =\{U:\widehat U(\pi\epsilon)=0\text{ for all }\epsilon\};}
   \]

4. for `L=4`, the target dimension is `192`, the source-symbol rank is
   `168`, and the cokernel dimension is `24`.

In real space the eight conditions are the componentwise parity moments

\[
 \boxed{
 M_\epsilon(U)=\sum_x(-1)^{\epsilon\cdot x}U(x)=0,
 \qquad \epsilon\in\{0,1\}^3.}
\]

Equivalently, the total vector impulse in each of the eight site-parity
classes must vanish. These conditions apply to every central gradient and
central curl separately.

The converse is a finite-periodic global range statement. It does not assert
a finite-support preimage, a uniformly local inverse, or live ternary
realizability.

## 5. Frozen elementary-plaquette discriminator

Use one scalar component of the FTD-0918 plaquette word

\[
 f=\delta_{(0,0,0)}-\delta_{(1,1,0)}.
\]

At the eight blind modes,

\[
 \widehat f(\pi\epsilon)
 =1-(-1)^{\epsilon_x+\epsilon_y}.
\]

Thus exactly four blind fibers are nonzero: those with
`epsilon_x xor epsilon_y=1`, independently of `epsilon_z`.

The production stiffness symbol at those fibers is frozen as

\[
 \kappa_{100}=\kappa_{010}={4\over3},
 \qquad
 \kappa_{101}=\kappa_{011}={16\over9}.
\]

For any desired scalar body stiffness `kappa`, the return-source blind
components are

\[
 \widehat U_{\rm ret}(\pi\epsilon)
 =(\kappa_\epsilon-\kappa)\widehat f(\pi\epsilon).
\]

The certificate must prove that no single `kappa` makes all four components
zero, because it would require both

\[
 \kappa={4\over3}
 \quad\text{and}\quad
 \kappa={16\over9}.
\]

Two fixed controls are required:

- at the bare internal stiffness `kappa=25/18`, the two blind return
  amplitudes are `-1/9` and `7/9` after multiplication by
  `f_hat=2`;
- at the one-tick quarter-turn stiffness `kappa=2`, they are `-4/3` and
  `-4/9`.

Hence the elementary plaquette cannot be closed by the live central-Hodge
source even after relaxing ternary density, support-gated current, locality,
continuity, reciprocity, and energy closure.

## 6. Frozen Moore-coat control

The already-selected FTD-0577 coupling coat has symbol

\[
 B_M(k)=\prod_i\cos^2{k_i\over2}.
\]

It vanishes at every nonzero blind corner. The uncoated plaquette already
vanishes at the zero mode. Therefore

\[
 \widehat{B_Mf}(\pi\epsilon)=0
 \quad\text{for all eight }\epsilon,
\]

and so does `(K-kappa I)B_Mf`. On a fixed even periodic quotient, the coated
return source is therefore in the relaxed global linear range of `H`.

This is a control, not production closure. The certificate and theorem must
retain all of these debts:

- `rho` must be the live ternary state, not an arbitrary continuous scalar;
- `j=rho v` vanishes wherever `rho=0`, rather than being independent;
- a global Fourier preimage is not a finite-range local source construction;
- central continuity and the FTD-0576 conditional energy ledger must hold;
- source reaction and a positive autonomous reservoir must be supplied;
- formation, stability, mobility, scale, `G*` synchronization, and preferred
  tick hiding remain open.

No new selected type is permitted by this protocol. The coat is inherited
from `FTD-0577` with its existing status.

## 7. Outcome rules

- **Outcome A — exact native-source boundary:** the eight-fiber cokernel,
  unique return law, elementary-plaquette obstruction, and Moore-coat relaxed
  range control all pass. Book the source obstruction and retain coated local
  hardware as open.
- **Outcome B — elementary direct closure:** a single `kappa` removes every
  blind return component or the uncoated return is otherwise shown to lie in
  the frozen source range. This falsifies the stated obstruction and requires
  an explicit exact witness.
- **Outcome C — invalid execution:** any source lock, exact identity, count,
  or scope firewall fails. Book no theorem.

## 8. Required certificate gates

The independent exact certificate must cover:

1. all frozen source hashes;
2. uniqueness of the return impulse;
3. the cross-product matrix identity and rank dichotomy;
4. all eight central-derivative blind modes;
5. the `L=4` rank/cokernel counts;
6. the parity-moment equivalence;
7. the four plaquette blind fibers and both exact stiffness values;
8. incompatibility for arbitrary `kappa`;
9. the `25/18` and `2` controls;
10. Moore-coat annihilation and relaxed periodic range;
11. unchanged production source markers; and
12. explicit non-use of `G*`, gamma, Born/Bell, measurement targets, fitting,
    or numerical near-miss search.

No theorem may be booked until every gate passes.

## 9. Frozen scope ceiling

Success does **not** derive an autonomous local clock, a production ternary
source realization, a local inverse, a reciprocal matter-field step, a
positive source battery, Born frequencies, Bell correlations, gamma, or the
`G*` gearbox. It only decides whether the existing source operator can close
the simplest plaquette and identifies the exact parity condition that any
source-balanced successor body must satisfy.
