# FTD-0946 — Preregistration: minimum nonlinear relative-field recursive charge and source-frame boundary v1

**Identifier:** `FTD-0946`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact algebraic and finite-dimensional variational audit of the
existing relative canonical vector pair after FTD-0943/0945; body-axis complex
structure; degree-minimum nonnegative radial sextic; conserved recursive
orientation charge; finite-region constrained minimizer and relative
equilibrium; local energy current; reversible local charge-preserving split
tick; exact energy defect; odd affine source, inverse, and reservoir debit;
signed-cubic source-frame obstruction; no numerical search, fitting,
production change, new primitive field, uncontained-localization theorem,
Born, Bell, measurement context, outcome, physical scale, `G*`, or gamma read

## 1. Question

FTD-0943 proves that the unchanged isolated linear `C18` relative field has no
finite-range scalar characteristic and no nonzero finite-support exact rigid
translator or recurrent complete state. FTD-0944/0945 proves that the existing
event stack preserves the relative-zero submanifold and does not write a
phase-complete reversible history into that field. FTD-0907 nevertheless
shows, conditionally, that two canonical coordinates with a conserved
antisymmetric wedge can form a bounded recursive orientation memory.

The lower-price branch before adopting a new port type is therefore:

> Can the existing relative canonical vector pair support a degree-minimum,
> bounded nonlinear recursive charge under a declared local action, and can
> an ordinary signed occupancy event create that charge covariantly and
> reversibly without importing an additional frame or handedness datum?

This protocol separates four claims that must not be conflated:

1. representability of a complex structure in the existing vector pair;
2. existence of a selected bounded recursive reference action;
3. a finite-tick local implementation with its actual invariants; and
4. a natural production source for nonzero charge.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_CONFIGURATION_SPACE_CARRIER_NECESSITY.md` | `9FCD2E7AA89C8B38339D730B04AAD2A9797F40E3EDD08ACA3B5C9CFCB4996FBD` |
| `THEOREM_NATIVE_TERNARY_DIPOLE_AXIS_AND_BILATERAL_PHASE_WEDGE_MEMORY_BOUNDARY_v1.md` | `8B07C26475A76E79C37B825B91EA174C0D1D8C13F06422483EE60B236DC14340` |
| `THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md` | `C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329` |
| `THEOREM_EXISTING_EVENT_MEDIATED_RELATIVE_HISTORY_CARRIER_BOUNDARY_v1.md` | `E9DC4C6325507523365C7483919FF88EFB4F1877DA2F1D5CAFC7ACEFE208F2ED` |

The certificate must fail closed on source drift.

## 3. Existing-field complex structure

Let a finite neutral actual body supply the already representable polar axis

\[
 e={d_\Lambda\over |d_\Lambda|},\qquad |e|=1,
\]

as in FTD-0907. For the existing relative canonical pair

\[
 D=J_L-J_R,\qquad P_D=P_L-P_R,
\]

define the transverse projector and quarter-turn

\[
 \Pi_e=I-ee^T,
 \qquad J_ev=e\times v.
\]

The certificate must establish

\[
 J_e^T=-J_e,
 \qquad J_e^2=-\Pi_e,
 \qquad J_e\Pi_e=\Pi_eJ_e=J_e.
\]

Thus `J_e` is an exact real realization of multiplication by `i` on the
two-dimensional plane perpendicular to `e`. This is a body-contextual complex
structure represented by existing real vectors. It is not a derivation of a
complex Hilbert space, Born weights, or a universal axis in the vacuum.

Write

\[
 q_x=\Pi_eD_x,\qquad p_x=\Pi_eP_{D,x}.
\]

## 4. Degree-minimum selected action

On a finite connected `C18` region, write the positive local stiffness as

\[
 {1\over2}\langle q,Kq\rangle
 ={1\over4}\sum_{x,y}w_{xy}|q_x-q_y|^2,
 \qquad w_{xy}=w_{yx}\ge0,
\]

where only registered face/edge neighbours have nonzero weight. Adopt the
**[SELECTED REFERENCE ACTION]**

\[
 H_e(q,p)=\sum_x{ |p_x|^2\over2}
       +{1\over2}\langle q,Kq\rangle
       +\sum_xV(|q_x|),                                      \tag{1}
\]

with

\[
 \boxed{V(r)=\beta r^2(r^2-A^2)^2},
 \qquad \beta>0,\quad A>0.                                  \tag{2}
\]

The registered polynomial class is: radial, even in `q`, polynomial in
`r^2`, nonnegative, with a minimum at `r=0` and a degenerate minimum at the
nonzero ring `r=A`. In that class, the certificate must prove:

- degree two cannot have both zero sets;
- degree four either lacks one minimum or changes sign between the two
  zeros; and
- degree six is the floor, attained uniquely up to a positive multiplier by
  `r^2(r^2-A^2)^2`.

Equation (2) is also the degenerate specialization of

\[
 {m^2\over2}r^2-{g\over4}r^4+{h\over6}r^6,
\]

with

\[
 m^2=2\beta A^4,\qquad g=8\beta A^2,\qquad h=6\beta,
 \qquad m^2={3g^2\over16h}.                                  \tag{3}
\]

The equality in (3) is a selected vacuum-degeneracy condition, not a derived
physical coupling.

The force must be recorded exactly:

\[
 -\nabla_qV=-2\beta q(r^2-A^2)(3r^2-A^2).                    \tag{4}
\]

Calling (2) a perfect-square or square-factorized potential is permitted.
Calling it gauge-theoretic self-duality or a Bogomolny equation is forbidden.

## 5. Recursive charge and finite-region existence

The global axial charge is

\[
 \boxed{Q_e=\sum_x e\cdot(q_x\times p_x)
             =\sum_x(J_eq_x)\cdot p_x.}                       \tag{5}
\]

The certificate must prove `{Q_e,H_e}=0` exactly. The onsite force is radial
and the pairwise `C18` torques cancel, so the result may not be asserted only
from a name such as “Noether charge.”

For fixed `Q!=0`, let

\[
 N[q]=\sum_x|q_x|^2.
\]

Minimizing the kinetic term over `p` at fixed charge must give

\[
 p_x={Q\over N[q]}J_eq_x,
 \qquad
 T_{\min}={Q^2\over2N[q]}.                                    \tag{6}
\]

The reduced functional is

\[
 \mathcal E_Q[q]
 ={Q^2\over2N[q]}+{1\over2}\langle q,Kq\rangle
  +\sum_x\beta|q_x|^2(|q_x|^2-A^2)^2.                         \tag{7}
\]

On every fixed finite region, equation (7) diverges as `N -> 0` and is
coercive as `N -> infinity`; hence it attains a minimum. Since `Q!=0`, the
constraint is regular. The constrained Euler equation makes every minimizer
a relative equilibrium generated by (5), i.e. a recursively rotating orbit
under the exact Hamiltonian flow. Energy and charge conservation give
stability of the compact minimizer set at fixed charge.

This is a finite-region variational theorem. It must **not** be promoted to:

- an exact compact-support solution on the uncontained substrate;
- an infinite-volume localized `Q`-ball theorem;
- a mobile particle, collision-separated identity, or production body;
- autonomous formation from `D=P_D=0`; or
- a production-stable object under damping, genesis, movement, or erasure.

For the broader sextic family, the algebraic nonlinear-core window is

\[
 m^2-{3g^2\over16h}<\omega^2<m^2.                              \tag{8}
\]

For (2), this becomes

\[
 0<\omega^2<2\beta A^4.                                      \tag{9}
\]

Equations (8)--(9) are a necessary reference localization window obtained
from `min_{r>0} 2V(r)/r^2 < omega^2 < V''(0)`. They are not, by themselves,
an uncontained existence proof.

## 6. Exact continuous local energy current

Use the local energy allocation

\[
 \varepsilon_x={|p_x|^2\over2}+V(|q_x|)
 +{1\over4}\sum_yw_{xy}|q_x-q_y|^2.
\]

The exact Hamiltonian equations must imply

\[
 \dot\varepsilon_x=-\sum_y\mathcal J_{x\to y},
 \qquad
 \boxed{\mathcal J_{x\to y}
 ={w_{xy}\over2}(p_x+p_y)\cdot(q_x-q_y)},                     \tag{10}
\]

with `J_{y->x}=-J_{x->y}`. This closes the reference continuous-time local
energy ledger, not the finite production tick.

## 7. Finite-tick reference map and its energy boundary

Let `U(q)` be the stiffness plus onsite part of (1). Register the symmetric
kick--drift--kick map

\[
 p_{n+1/2}=p_n-{\Delta n\over2}\nabla U(q_n),
\]
\[
 q_{n+1}=q_n+\Delta n\,p_{n+1/2},
\]
\[
 p_{n+1}=p_{n+1/2}-{\Delta n\over2}\nabla U(q_{n+1}).           \tag{11}
\]

The certificate must prove that (11) is finite-range local per substep,
symplectic, exactly reversible under `Delta n -> -Delta n`, and preserves
`Q_e` exactly. It must also exhibit an exact harmonic witness for which
`H_e(q_{n+1},p_{n+1}) != H_e(q_n,p_n)` at nonzero step. Therefore (11) is a
charge-preserving reversible candidate tick, not an exact energy-closed
production law. Exact-flow sampling preserves energy but is not to be called
a finite-range computable production update.

## 8. Gamma separation

On the transverse plane the linear generator decomposes as

\[
 L=\omega J_e-\gamma\Pi_e.                                    \tag{12}
\]

`J_e` is antisymmetric and supplies the oriented quarter-turn; `gamma` is a
real symmetric contraction. The certificate must show

\[
 e^{tL}=e^{-\gamma t}
  \bigl(\cos(\omega t)\Pi_e+\sin(\omega t)J_e\bigr),           \tag{13}
\]

and, for momentum damping `dot p=...-gamma p`,

\[
 \dot Q_e=-\gamma Q_e.                                        \tag{14}
\]

Thus `i`/`J_e` does not determine `gamma`. A stable conservative recursive
charge uses `gamma=0`; nonzero damping requires a separately specified bath,
feedback law, and energy/information account.

## 9. Odd reversible source and source-frame gate

Suppose a signed event `s in {-1,+1}` supplies, in addition to `e`, a retained
transverse vector `v` and a handed quarter-turn `J_e`. At one site the affine
source

\[
 q^+=q^-+a s v,
 \qquad
 p^+=p^-+bJ_ev                                             \tag{15}
\]

is symplectic when its source data are fixed and retained. From relative
vacuum it writes

\[
 \boxed{Q_e^+=abs|v|^2},                                    \tag{16}
\]

which is odd under `s -> -s`. Its inverse subtracts the same two impulses.
If

\[
 E_R^+=E_R^--\bigl(H_e^+-H_e^-\bigr),                         \tag{17}
\]

then source plus field energy closes exactly. Equation (17) is a reservoir
contract, not free creation and not a derivation of a production reservoir.

The natural-source gate is stricter. From one polar axis alone there is no
nonzero signed-cubic-equivariant transverse vector at a fourfold-symmetric
axis. For `e=e_z`, covariance under the quarter-turn `R_z(pi/2)` requires

\[
 v(e_z)=R_z(\pi/2)v(e_z),
\]

whose only fixed vectors are parallel to `e_z`; transverse projection then
gives zero. Moreover `e cross v` is axial under improper cubic operations.
Writing it into the polar momentum field requires an additional pseudoscalar
handedness datum, or restriction to the proper-rotation subgroup.

A second polar datum `a` can give `v=Pi_e a` when `a` is not parallel to `e`,
but it does not remove the improper-rotation handedness requirement and it
fails on the parallel branch. The certificate must therefore distinguish:

- conditional two-frame/proper-rotation source realization;
- universal full-signed-cubic source realization; and
- source representability versus autonomous production formation.

## 10. Frozen outcomes

| Outcome | Exact condition | Verdict |
|---|---|---|
| A | The existing relative pair, degree-minimum action, exact finite-tick energy/charge/inverse law, and a universal signed-cubic odd source all pass without an extra frame or reservoir type | same-field carrier and natural source close at reference level |
| B | The existing pair supports the complex structure, minimum sextic, conserved charge, finite-region recursive minimizer, local continuous energy current, and conditional reversible source, but finite-tick exact energy and/or universal source covariance remain open | stable recursive reference mechanism exists conditionally; production gearbox remains unclosed and its missing data are classified |
| C | The action supports only a bounded static/finite-region state or the source writes no nonzero conserved charge | partial nonlinear structure only; no recursive carrier promotion |
| D | source drift or exact gate failure | execution invalid; no theorem |

No tolerance, fit, numerical near-miss, target history, target probability,
measurement setting, outcome, `G*`, or post-hoc outcome change is permitted.

## 11. Acceptance and stop conditions

The certificate must report separately:

1. all frozen source hashes;
2. projector/quarter-turn identities and the scope of `i`;
3. sextic degree minimum, extrema, factorization, and parameter map;
4. exact axial-charge conservation including edge-torque cancellation;
5. fixed-charge kinetic reduction, coercivity, minimizer, and relative-orbit
   logic;
6. the algebraic localization window and its non-promotion firewall;
7. antisymmetric local energy current;
8. finite-tick locality, symplecticity, reversibility, exact charge, and
   non-exact-energy witness;
9. `J_e`/gamma separation and charge decay under damping;
10. odd affine source, inverse, charge deposit, and exact reservoir debit;
11. one-axis frame obstruction, improper-rotation parity price, and
    conditional two-frame survivor; and
12. all Born/Bell/G*/production/uncontained/localization firewalls.

Stop immediately on source drift. Do not modify production engine sources,
tests, CMake, toggles, constants, or ontology.

## 12. Promotion boundary

Outcome B would establish the smallest same-field stable recursive mechanism
found so far while preserving the key debt rather than hiding it. The next
gate would split:

1. prove or refute an uncontained exponentially localized charged droplet
   with the registered `C18` stiffness and finite propagation discipline;
2. identify a production-native ordered two-frame plus pseudoscalar source,
   or explicitly price that source interface; and
3. replace (11) with an exact energy/charge/reversibility update or book its
   controller/reservoir transaction before any production integration.

Failure of those gates would leave the FTD-0941 selected oriented-port branch
as the next honest type adoption. No result here recovers Hilbert space,
Born's rule, gamma, `G*` cadence, matter, or a complete framework.
