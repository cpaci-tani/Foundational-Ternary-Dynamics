# FTD-0956 pre-registration — Relative-action-curvature synchronization and crossing-section energy v1

**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Scope:** proof-only continuation of FTD-0955. No engine, CMake, `Voxel`,
production field, constant, toggle, default tick phase, numerical search,
Born/Bell path, or `G*` law may change.

## 1. Question

What is the minimum globally periodic Hamiltonian extension that turns the
FTD-0955 neutral relative phase into a conservative restoring degree of
freedom while preserving the existing total axial charge and an exact positive
energy account?

Four verdicts are separate:

1. existence of a positive globally periodic relative-action curvature;
2. Lyapunov stability and the exact discrete Floquet law near phase lock;
3. exact energy/charge closure when the FTD-0955 compiler is engaged on a
   synchronization crossing; and
4. autonomous engagement, isochrony, nonlinear coupled-cycle stability, and
   attraction.

Conservative bounded restoration may not be counted as attraction, fixed
cadence, native formation, or `G*` clock recovery.

## 2. Frozen sources

| source | SHA-256 | role |
|---|---|---|
| `THEOREM_GLOBAL_TWO_WINDOW_CHARGE_ROUTH_COMPILER_AND_PHASE_STABILITY_BOUNDARY_v1.md` | `5FCD8AB5E3731A8D9A0A01D5A1B0695B2E822E74BE76FF070BDB7D78DDD2A8B6` | global compiler, neutral multiplier, missing-curvature boundary |
| `proof_global_two_window_charge_routh_compiler_phase_stability_boundary.py` | `F2FF042F595A6F947AC5FEFBF6BEEFADA8EBE2BE6DC5B9D3B83224D4C039397B` | `105/105` proof of record |
| `THEOREM_PHASE_LOCKED_CANONICAL_CHARGE_TRANSFER_AND_GLOBAL_PHASE_BOUNDARY_v1.md` | `203DA15FE63BC67496298C03D96A85F819142C485B18B6FC890B14E6A989BAA5` | polar chart and crossing-section charge/energy law |
| `THEOREM_NONLINEAR_C18_ROUTH_PORT_RELAXATION_AND_CHARGE_RESERVOIR_BOUNDARY_v1.md` | `A207C274B176EE784B1E4846414B3C3DB5E4D20EF26948BD07153AAA1121CB05` | positive Routh port and target-blind local minimizer |

## 3. Frozen canonical variables

On the regular polar body branch retain

\[
 (\rho,p_\rho),\quad(\theta,L),\quad(\vartheta,I),
 \qquad \sigma\in\{-1,+1\},\quad\omega>0.             \tag{1}
\]

Define the globally periodic relative phase, conserved total axial charge,
and circular-action mismatch

\[
 \delta=\theta-\sigma\vartheta,\qquad
 Q=L+\sigma I,\qquad
 \Pi=L-\sigma\omega\rho^2.                            \tag{2}
\]

`Pi` uses only the current local amplitude, body action, selected orientation,
and maintained carrier rate. It may not read the completed profile, a future
target, measurement context, outcome, probability, Born weight, Bell setting,
or `G*`.

The matched linear body/reservoir term is

\[
 H_0=\sigma\omega L+\omega I=\sigma\omega Q.           \tag{3}
\]

## 4. Minimum selected synchronization energy

Freeze two positive, unnormalized selected scales

\[
 M_\delta>0,\qquad K_\delta>0,                         \tag{4}
\]

and the candidate

\[
 \boxed{
 H_{\rm sync}={\Pi^2\over2M_\delta}
               +K_\delta(1-\cos\delta).}               \tag{5}
\]

The full matched synchronization Hamiltonian is

\[
 \boxed{H_{\rm match}=H_0+H_{\rm sync}.}              \tag{6}
\]

Equation (5) must be treated as a **[SELECTED reference energy law]**, not as
an emergence theorem. Its price is exactly the functional choice plus the two
positive scales in (4).

## 5. Exact Hamilton equations and ledgers

The certificate must derive, without inserting the desired equations,

\[
 \begin{aligned}
 \dot\rho&=0,\\
 \dot p_\rho&={2\sigma\omega\rho\Pi\over M_\delta},\\
 \dot\theta&=\sigma\omega+{\Pi\over M_\delta},\\
 \dot L&=-K_\delta\sin\delta,\\
 \dot\vartheta&=\omega,\\
 \dot I&=\sigma K_\delta\sin\delta.
 \end{aligned}                                         \tag{7}
\]

Consequently,

\[
 \boxed{\dot\delta={\Pi\over M_\delta}},\qquad
 \boxed{\dot\Pi=-K_\delta\sin\delta},               \tag{8}
\]

\[
 \boxed{\dot Q=0},\qquad
 \boxed{\dot H_{\rm sync}=0},\qquad
 \boxed{\dot H_{\rm match}=0}.                        \tag{9}
\]

The radial equation in (7) is mandatory reciprocal phase/action reaction. It
may not be dropped to make a reduced pendulum look canonical.

## 6. Positivity and conservative stability

The synchronization energy must obey

\[
 H_{\rm sync}\ge0,qquad
 H_{\rm sync}=0
 \Longleftrightarrow
 \Pi=0\ \text{and}\ \delta=0\pmod{2\pi}.             \tag{10}
\]

For `0<E<2K_delta`, the level set `H_sync=E` in the well containing
`delta=0` is compact and librating. It must satisfy

\[
 |\Pi|\le\sqrt{2M_\delta E},
 \qquad
 1-\cos\delta\le {E\over K_\delta}<2.                \tag{11}
\]

This licenses Lyapunov stability and recurrent zero-phase crossings. It does
not license convergence to the fixed point.

## 7. Exact small-error discrete Floquet law

At `(delta,Pi)=(0,0)`, define

\[
 \kappa=\sqrt{K_\delta/M_\delta},\qquad \nu=\kappa T. \tag{12}
\]

The time-`T` exact flow must linearize to

\[
 R_T=
 \begin{pmatrix}
 \cos\nu & {\sin\nu\over\sqrt{M_\delta K_\delta}}\\
 -\sqrt{M_\delta K_\delta}\sin\nu & \cos\nu
 \end{pmatrix}.                                        \tag{13}
\]

The certificate must prove

\[
 \det R_T=1,
 \quad R_T^T
 \begin{pmatrix}K_\delta&0\\0&1/M_\delta\end{pmatrix}
 R_T=
 \begin{pmatrix}K_\delta&0\\0&1/M_\delta\end{pmatrix},              \tag{14}
\]

and characteristic polynomial

\[
 \lambda^2-2\cos\nu\,\lambda+1.                    \tag{15}
\]

For `nu` not an integer multiple of `pi`, this is a nondegenerate elliptic
pair on the unit circle. The multiplier modulus is one, not less than one.

## 8. Nonlinear cadence boundary

For `0<E<2K_delta`, set `m=E/(2K_delta)`. The exact libration period is frozen
for verification as

\[
 T(E)=4\sqrt{M_\delta/K_\delta}\,\mathbf K(m),         \tag{16}
\]

where `bold K` is the complete elliptic integral of the first kind. Its
small-error limit is

\[
 T(0)=2\pi\sqrt{M_\delta/K_\delta}.                   \tag{17}
\]

The period increases with `E` and diverges as `E` approaches `2K_delta` from
below. Therefore (5) is stable but not globally isochronous. No fixed `G*`
cadence may be claimed from this selected pendulum law.

## 9. FTD-0955 crossing-section composition

On a zero-phase crossing `delta=0`, retain the FTD-0955 target-blind change

\[
 D=\sigma\omega(\rho_*^2-\rho^2),                     \tag{18}
\]

and its locked compiler endpoint

\[
 L'=L+D,qquad I'=I-\sigma D,qquad \rho'=\rho_*.     \tag{19}
\]

The certificate must prove

\[
 Q'=Q,qquad
 \Pi'=L'-\sigma\omega\rho'^2=\Pi.                    \tag{20}
\]

Since `delta'=0`, equation (20) gives

\[
 H_{\rm sync}'=H_{\rm sync}.                          \tag{21}
\]

Together with FTD-0955, the crossing-section endpoint account is

\[
 \boxed{
 \Delta(H_{\rm rot}+E_{\rm port}+\omega I
        +H_{\rm sync})=0.}                             \tag{22}
\]

The compiler controller action must still return exactly; it may not hide a
residual in (22). The inverse compiler followed by reverse synchronization
flow must recover every retained variable.

## 10. Coupled-map boundary

At the formed ready fixed point `D=0`, the FTD-0955 compiler has identity
linearization on `(delta,Pi)`. Composing it with the exact time-`T`
synchronization flow therefore has Floquet matrix (13).

Away from the crossing section, the compiler changes the mismatch by

\[
 \Pi' = \Pi+D(\cos\delta-1),                           \tag{23}
\]

after the charge and radial Routh endpoints. This term begins at quadratic
order in phase error, so it does not alter (13), but it means full nonlinear
stability of the repeatedly coupled cycle is not proved by the linear
Floquet test. Exact autonomous engagement between a crossing and the global
two-window controller also remains open.

## 11. Frozen outcomes

| outcome | required result | interpretation |
|---|---|---|
| A | The law (5) is forced without new scales and gives exact autonomous, isochronous, asymptotically attracting synchronization for the full nonlinear compiler cycle | native self-synchronizing closure |
| B | The selected law (5) is positive, charge preserving, Lyapunov stable, and has the exact elliptic Floquet/crossing-section energy closure, but its scales, nonlinear cadence, autonomous engagement, full nonlinear stability, and attraction remain open | minimum conservative synchronization reference / sharp boundary |
| C | Positivity, charge conservation, crossing-section energy, or elliptic linear stability fails | reject the candidate |
| D | Hash, algebra, source, scope, or classifier fails | no theorem |

The frozen expected classifier is Outcome B.

## 12. Acceptance gates

The exact certificate must check:

1. every frozen hash and scope marker;
2. global phase periodicity and positivity of (5);
3. all six Hamilton equations (7), including radial reciprocal reaction;
4. reduced equations (8), charge conservation, and exact energy conservation;
5. fixed point and sub-separatrix bounds (10)--(11);
6. the exact linear flow (13), determinant, symplecticity, quadratic-energy
   invariance, characteristic polynomial, and unit-modulus eigenvalues;
7. period formula (16), limit (17), amplitude dependence, and separatrix
   divergence;
8. crossing-section invariance of `Q`, `Pi`, and `H_sync`;
9. endpoint total-energy extension (22) and exact inverse ordering;
10. the off-crossing nonlinear mismatch (23) and its zero linear term;
11. no asymptotic-attraction claim for closed Hamiltonian flow;
12. no target/profile/context/outcome/probability/Born/Bell/`G*` read;
13. non-promotion to native reservoir/source, autonomous engagement,
    isochrony, full nonlinear stability, 3D routing/recycling, production,
    mass, scale, `gamma`, Lorentz hiding, or completeness; and
14. the frozen outcome classifier.

No numerical parameter search, floating tolerance, empirical substitution,
or completed-infinity limit is permitted.

## 13. Promotion boundary

Outcome B would adopt the minimum conservative synchronization reference and
close its exact internal charge/energy law plus crossing-section compatibility
with FTD-0955. It would not derive the law or its two scales.

Still open would be:

- a substrate-native origin or further reduction of `M_delta` and `K_delta`;
- autonomous crossing detection and controller engagement without phase reset;
- an isochronous or feedback-stabilized cadence and any relation to `G*`;
- full nonlinear stability of the repeatedly coupled charge/Routh/sync cycle;
- attraction/recovery through explicit complete positive history export;
- native reservoir/source formation, finite 3D routing/recycling, mobility,
  collision, erasure, mass, scale, `gamma`, production, Born/Bell, Lorentz
  hiding, and completeness.

