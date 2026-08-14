# FTD-0952 — Preregistration: nonlinear C18 Routh-port relaxation and charge-reservoir boundary v1

**Identifier:** `FTD-0952`  
**Date:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Scope:** nonlinear positive-port reference layer and physical energy/charge
boundary; no production integration

## 1. Question

Can the positive source-centered port mechanism already proved for quadratic
C18 relaxation be extended to the nonlinear FTD-0949 recursive body without
reading its target profile? If so, does that positive canonical port layer
also pay the physical field energy and axial charge, or is a distinct
co-rotating phase/action reservoir still required?

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_CAUSAL_WORK_BOOKED_C18_FINITE_RADIUS_RELAXATION_AND_MISMATCH_PORT_v1.md` | `B96254AA0C4A9C28015CF5978C9B9B219D371C332DC8DEDABB892BD45C964566` |
| `proof_causal_work_booked_c18_finite_radius_relaxation_v2.py` | `0F5D54576F5D3AD6045C93B25EF3A2277D1461429ECBB4E50E9A60D5151E3D8C` |
| `THEOREM_UNCONTAINED_C18_EXPONENTIALLY_TAILED_RECURSIVE_CHARGE_AND_FORMATION_BOUNDARY_v1.md` | `FC1F750CA5D5ABF52608F4789BE054B43919055FCB8A9EE674CD211B8E1B6356` |
| `proof_uncontained_c18_exponentially_tailed_recursive_charge.py` | `A9C72A3DB5B9E5E4F814470F5DB2DBA4CEFEB3FB125DD3B3BE9E7E26BC0D9536` |
| `THEOREM_EIGHT_COLOR_SOURCE_CENTERED_POSITIVE_PORT_RELAXATION_AND_MASSLESS_HALO_BOUNDARY_v1.md` | `EA70B9D7B16481B005F0FBF5DFF25893A27606A1186661677A7A944F1E301D09` |
| `proof_eight_color_source_centered_positive_port_relaxation_massless_halo_boundary.py` | `A7E338090EC10B141DC3E1336926E8B980DE348250DE0C48005498756240971E` |
| `THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md` | `0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F` |
| `proof_canonical_source_centered_gauss_gate_v2.py` | `6C35135A3B5B9345E6EA9A6EBFB61B32951EE07DDDB17188362B8B38A10F1816` |
| `THEOREM_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_AND_RECYCLING_BOUNDARY_v1.md` | `AF810B73322DE8521C8509792E09D549A10E1D8417C1B283A3630EB8B16D7BFC` |

Hash drift fails closed. No source repair is authorized by this protocol.

## 3. Frozen nonlinear Routh functional

Retain

\[
 \Lambda=\beta A_0^4\ge10^4,
 \quad a^2={6\over5},
 \quad \omega^2={26\Lambda\over25},
 \quad r_*={1\over1000}.                                   \tag{1}
\]

For scalar profile `phi`, define the dimensionless rotating-frame functional

\[
 \mathscr S(\phi)
 ={1\over2}\langle\phi,K\phi\rangle
 +\Lambda\sum_x\phi_x^2(\phi_x^2-1)^2
 -{\omega^2\over2}\sum_x\phi_x^2.                         \tag{2}
\]

Its gradient is exactly

\[
 \nabla\mathscr S(\phi)=K\phi+g(\phi),
 \qquad
 g(z)=2\Lambda z\left(3z^4-4z^2+1-{13\over25}\right).     \tag{3}
\]

On a specified finite grounded region containing the marked core, use the
local product branch

\[
 |\phi_0-a|\le r_* ,
 \qquad
 |\phi_x|\le r_*\quad(x\ne0),                              \tag{4}
\]

with zero exterior data. This is a finite operational branch, not a completed
infinite box.

## 4. Frozen convexity and finite-grounded target gates

FTD-0949 gives the nonlinear derivative bound

\[
 |g'(z)-g'(a c_x)|
 \le {6624\over25}\Lambda r_*                              \tag{5}
\]

on (4). Therefore the certificate must prove

\[
 \boxed{
 \nabla^2\mathscr S\succeq
 \mu I,
 \qquad
 \mu={2172\over3125}\Lambda>0.}                            \tag{6}
\]

The FTD-0949 fixed point obeys the stricter bound

\[
 \|\phi_* - a\delta_0\|_w
 \le {2249\over3000000}<r_*,                               \tag{7}
\]

so its restriction lies inside (4).

Let `R` be an `l1` radius and let `psi_R` be the unique minimizer of (2) on
the grounded branch (4). From the FTD-0949 tail and the C18 `l2` norm
`||K||<=16/9`, the certificate must prove

\[
 \boxed{
 \|\psi_R-\phi_*\|_2
 \le10^{-3}2^{-(R+1)}
 \left(1+{12500\over4887\Lambda}\right).}                  \tag{8}
\]

Equation (8), not an `R to infinity` claim, is the operational finite-region
approximation statement.

## 5. Frozen eight-color nonlinear coordinate relaxation

Color sites by their three coordinate parities. No C18 edge joins sites of
the same color. Holding inactive sites fixed, the active local functional is

\[
 U_x(z)={2\over3}z^2-h_xz
 +\Lambda z^2(z^2-1)^2-{\omega^2\over2}z^2,
 \qquad
 h_x=\sum_{y\sim x}w_{xy}\phi_y.                            \tag{9}
\]

The certificate must prove that `U_x` is strictly convex on its interval in
(4), its derivative points inward at both endpoints, and it has a unique
interior minimizer `z_x^*` determined only by the current C18 neighbours,
core flag, and selected parameters.

One color layer replaces every active `phi_x` by `z_x^*`. Eight colors form
one sweep. The update may not read `phi_*`, a fitted tail, future iterates,
measurement context, outcome, or probability.

For every specified finite grounded region, the certificate must prove:

1. every layer remains in (4);
2. `mathscr S` decreases strictly unless the active residual is zero;
3. the cyclic eight-color sequence converges to the unique minimizer
   `psi_R`; and
4. combined with (8), finite color depth reaches every declared finite
   accuracy.

No volume-independent convergence rate is required or permitted to be
inferred from compactness alone.

## 6. Frozen positive canonical Routh port

For one active cell define the source-centered energy coordinate

\[
 u_x(z)=\operatorname{sgn}(z-z_x^*)
 \sqrt{2A_0^2\,[U_x(z)-U_x(z_x^*)]}.                        \tag{10}
\]

Strict convexity must make (10) a monotone local coordinate chart with a
finite nonzero derivative at `z_x^*`. Restore its conjugate

\[
 \pi_u={p_z\over du/dz}                                    \tag{11}
\]

and one fresh complete port pair `(a,pi_a)`. The selected quarter-turn is

\[
 (u,a,\pi_u,\pi_a)
 \longmapsto(a,-u,\pi_a,-\pi_u).                            \tag{12}
\]

On the fresh zero-conjugate section, equation (12) sends `u` to zero and
therefore `z` to `z_x^*`; the outgoing port stores `-u`. It must preserve the
positive form

\[
 N={1\over2}(u^2+a^2+\pi_u^2+\pi_a^2),                     \tag{13}
\]

be symplectic, fourth order, exactly reversible, and admit the inherited
positive clocked Hamiltonian interpolation. Only the realized forward/reverse
chart segment is promoted; any global chart extension is imposed reference
structure and carries no physical claim.

For one fresh layer,

\[
 \boxed{
 A_0^2\Delta\mathscr S+\Delta E_{\rm port}=0.}              \tag{14}
\]

A finite bank supplies a declared finite horizon. An open/bilateral complete-
pair rail supplies an exact reference export. A finite cyclic bank may not be
called indefinitely fresh.

## 7. Frozen physical energy/charge audit

For orientation `sigma in {+1,-1}`, the rotating section has

\[
 H_{\rm rot}=A_0^2\left[
 {\omega^2\over2}\|\phi\|^2
 +{1\over2}\langle\phi,K\phi\rangle
 +\Lambda\sum_x\phi_x^2(\phi_x^2-1)^2\right],             \tag{15}
\]

\[
 Q=\sigma\omega A_0^2\|\phi\|^2.                         \tag{16}
\]

The certificate must prove the exact Routh identity

\[
 \boxed{A_0^2\mathscr S=H_{\rm rot}-\sigma\omega Q}.       \tag{17}
\]

Equations (14) and (17) imply

\[
 \boxed{\Delta H_{\rm rot}+\Delta E_{\rm port}
 =\sigma\omega\Delta Q.}                                  \tag{18}
\]

Thus a positive Routh port does not by itself conserve physical energy and
charge. A co-rotating action reservoir `(I,theta)` with

\[
 E_R=\omega I,
 \qquad Q_R=\sigma I                                      \tag{19}
\]

would close both ledgers algebraically if

\[
 \Delta I=-\sigma\Delta Q.                                 \tag{20}
\]

Every declared finite region and finite relaxation horizon has a finite
reserve sufficient to keep `I>0`. This is capacity, not dynamics.

For a symplectic system/port layer `F`, the phase-blind state-dependent map

\[
 (z,I,\theta)\mapsto
 (F(z),I-\sigma\Delta Q(z),\theta)                          \tag{21}
\]

must be tested against

\[
 \Omega+dI\wedge d\theta.                                 \tag{22}
\]

If `d DeltaQ !=0`, the extra term is

\[
 -\sigma\,d(\Delta Q)\wedge d\theta,                       \tag{23}
\]

so the phase-blind drain is not symplectic. Full closure requires phase
backreaction or a complete charge-transfer mode generated by a common
Hamiltonian.

## 8. Frozen outcomes

| Outcome | Required result | Interpretation |
|---|---|---|
| A | Positive nonlinear port relaxation, finite-grounded convergence/error, and an existing complete canonical charge reservoir all close under one local Hamiltonian | positive physical formation reference closes conditionally |
| B | Positive nonlinear Routh-port relaxation and finite-grounded convergence close, but the minimum phase-blind charge/action reservoir fails canonicality | exact positive Routh relaxation; full physical energy/charge reservoir remains open and its missing phase reaction is priced |
| C | Convexity, local chart, positive port, or finite-grounded convergence fails | reject nonlinear port extension |
| D | Hash, algebra, bound, scope, or classifier fails | no theorem |

The frozen expected classifier is Outcome B unless the certificate derives a
common phase-reacting Hamiltonian from the frozen sources without an added
type or law. A signed scalar ledger, post-hoc square-root battery, or
phase-blind action decrement cannot satisfy Outcome A.

## 9. Acceptance and scope gates

The exact certificate must check:

1. every frozen hash and required scope marker;
2. the gradient and Hessian of (2);
3. the exact strong-convexity constant (6);
4. the strict interior branch and fixed-point bound (7);
5. the truncation/strong-monotonicity estimate (8);
6. eight-color independence and inward endpoint derivatives;
7. the compact cyclic-coordinate convergence proof;
8. the coordinate-chart derivative and energy identity (10);
9. symplecticity, positivity, order four, inverse, and fresh-port reduction of
   (12);
10. finite-bank capacity and finite-cyclic freshness boundary;
11. the Routh/physical identities (17)--(18);
12. finite positive action capacity for every declared finite horizon;
13. the non-symplectic term (23);
14. explicit non-promotion to physical formation, native source, `G*`, Born,
    or production; and
15. the frozen outcome classifier.

No numerical search, fit, floating tolerance, empirical substitution, or
completed-infinity limit is permitted. Do not modify engine, CMake, `Voxel`,
production fields, constants, toggles, or default tick phases.

## 10. Promotion boundary

Outcome B would establish the simplest positive nonlinear environment found
for the recursive body: amplitude error becomes a complete outgoing canonical
port with positive Routh energy. It would also prove why that is not yet a
physical charged-body formation law.

Still open would be:

- a common Hamiltonian that reacts on the reservoir phase while transferring
  the exact charge/action;
- native preparation and orientation of that co-rotating reservoir;
- finite 3D port routing, congestion, return, and recycling;
- autonomous eight-color scheduling and stopping;
- exact full physical finite-tick energy/charge/reversal;
- perturbation recovery, mobility, collision/backpressure, mass, and
  production normalization;
- `gamma`, quartic-`G*` synchronization, Born/Bell, Lorentz hiding, and
  completeness.
