# FTD-0950/0951 — Causal work-booked C18 finite-radius relaxation and mismatch-port theorem v1

**Date:** 2026-08-11  
**Status:** `[THEOREM — CAUSAL FINITE-RADIUS GEOMETRIC RELAXATION]` +
`[THEOREM — EXACT LOCAL MISMATCH EXPORT AND REVERSIBLE PORT LIFT]` +
`[THEOREM — FINITE WORK/CHARGE VARIATION WITH SIGNED LEDGERS]` +
`[SELECTED REFERENCE CONTROLLER]` +
`[OPEN — POSITIVE RESERVOIR, PORT RECYCLING, NATIVE ORIENTATION SOURCE, EXACT HAMILTONIAN TICK, STABILITY, PRODUCTION]`  
**Verdict:** `OUTCOME_A_REFERENCE_RELAXATION_AND_LEDGER_CLOSURE_PHYSICAL_FORMATION_OPEN`

## 1. Result

The exact exponentially tailed FTD-0949 recursive body can be approached by
a compact, causal, target-blind sequence with an explicit geometric error at
every finite depth.

The update reads only:

1. the compact registered core marker;
2. the current relative-field amplitude;
3. the selected onsite sextic; and
4. the eighteen face/edge C18 neighbours.

It does not read the exact final profile. If `phi_*` is the unique FTD-0949
solution and `phi_n` is the depth-`n` approximant, then

\[
 \boxed{
 \|\phi_n-\phi_*\|_w
 \le {11\over13022}
 \left({2489\over9000}\right)^n.}                            \tag{1}
\]

Each `phi_n` has finite support inside the depth-`n` C18 causal ball. Thus for
every declared finite accuracy there is a finite local computation that
witnesses it. Exact finite-time formation remains impossible, consistently
with FTD-0949.

The field-equation residual is not discarded. It is exactly the outgoing
datum of a reversible local mismatch port. Every change in field energy and
axial charge is also booked locally, with finite total absolute variation.

This closes the mathematical finite-radius relaxation and accounting debt.
It does **not** close physical formation: the controller is selected, the
work/charge reservoirs are signed ledgers rather than positive autonomous
systems, and every layer still requires a fresh zero port or a derived
recycling mechanism.

## 2. Local residual controller

Retain

\[
 \Lambda=\beta A_0^4\ge10^4,
 \qquad
 a^2={6\over5},
 \qquad
 \omega^2={26\Lambda\over25}.                               \tag{2}
\]

Let `c_x` be a compact body-core marker, equal to one at the marked center
and zero elsewhere. Define

\[
 \phi^{(0)}_x=ac_x,
 \qquad
 g(z)=2\Lambda z\left(3z^4-4z^2+1-{13\over25}\right),       \tag{3}
\]

and

\[
 \ell_x={24\Lambda\over25}(1+15c_x),
 \qquad (Lu)_x=\ell_xu_x.                                  \tag{4}
\]

The two values are exactly

\[
 g'(0)={24\Lambda\over25},
 \qquad
 g'(a)={384\Lambda\over25}.                                \tag{5}
\]

For the C18 graph Laplacian `K`, write

\[
 \mathcal F(\psi)=K\psi+g(\psi),                            \tag{6}
\]

and choose the **[SELECTED REFERENCE CONTROLLER]**

\[
 \boxed{
 u_0=0,
 \qquad
 u_{n+1}=\mathcal T(u_n)
 =u_n-L^{-1}\mathcal F(\phi^{(0)}+u_n),
 \qquad
 \phi_n=\phi^{(0)}+u_n.}                                   \tag{7}
\]

Equation (7) is exactly the FTD-0949 Banach map rewritten as residual
relaxation. The marked center replaces a global coordinate test: it is local
body data. No table of tail amplitudes occurs in the update.

## 3. Geometric convergence

FTD-0949 proves that `T` maps the locked radius-`10^-3` weighted ball into
itself and has Lipschitz constant

\[
 c={2489\over9000}<{1\over2}.                               \tag{8}
\]

Its first increment obeys

\[
 \|u_1-u_0\|_w\le b={11\over18000}.                         \tag{9}
\]

Therefore contraction gives inductively

\[
 \|u_{n+1}-u_n\|_w\le bc^n.                               \tag{10}
\]

Summing the remaining increments proves

\[
 \|u_*-u_n\|_w
 \le\sum_{j=n}^{\infty}bc^j
 ={b\over1-c}c^n
 ={11\over13022}c^n,                                      \tag{11}
\]

which is equation (1).

For an operational tolerance `epsilon>0`, define `N_epsilon` to be the least
integer satisfying

\[
 {11\over13022}c^{N_\epsilon}\le\epsilon.                  \tag{12}
\]

This definition uses only an exact rational inequality. It needs neither an
infinite-volume limit nor a fitted convergence threshold.

## 4. Finite support and local-net consistency

Let `B_n` be the C18 graph ball of depth `n` around the core. If `phi_n`
vanishes outside `B_n`, then at any site outside `B_{n+1}`:

- every current C18 neighbour is zero;
- `K phi_n=0`;
- `g(0)=0`; and
- the core marker is zero.

Equation (7) therefore leaves that site zero. By induction,

\[
 \boxed{\operatorname{supp}\phi_n\subseteq B_n.}            \tag{13}
\]

Every layer expands the dependency cone by at most one C18 step. Two
finite-region computations with the same data agree at a site whenever its
complete depth-`n` cone lies in both regions. The approximants are therefore
restriction-consistent.

Equations (1) and (13) sharpen the FTD-0949 formation statement:

\[
 \text{exact finite-tick formation: impossible},
 \qquad
 \text{finite-error causal preparation: constructive}.     \tag{14}
\]

## 5. Exact mismatch export

Define the layer residual

\[
 r_n=\mathcal F(\phi_n).                                    \tag{15}
\]

Equation (7) immediately gives

\[
 \boxed{r_n=L(u_n-u_{n+1}).}                                \tag{16}
\]

Since

\[
 \|L\|={384\Lambda\over25},                               \tag{17}
\]

equation (10) yields

\[
 \boxed{
 \|r_n\|_w
 \le {88\Lambda\over9375}
 \left({2489\over9000}\right)^n.}                          \tag{18}
\]

The residual has finite support in `B_(n+1)` and tends geometrically to zero.
It is simultaneously the local equation defect and the exact information
needed to recover the overwritten incoming field.

## 6. Local energy work

On the phase-covariant rotating section, use the local density

\[
 h_x(\phi)=A_0^2\left[
 {\omega^2\over2}\phi_x^2
 +\Lambda\phi_x^2(\phi_x^2-1)^2
 +{1\over4}\sum_{y\sim x}w_{xy}(\phi_x-\phi_y)^2
 \right],                                                   \tag{19}
\]

where `w=1/9` on a face bond and `w=1/18` on an edge bond. Summing (19) counts
each bond twice at one-quarter weight and gives the selected Hamiltonian
energy of

\[
 q_x=A_0\phi_xv_\theta,
 \qquad
 p_x=\omega A_0\phi_xJ_ev_\theta.                           \tag{20}
\]

Define the local work and the signed `FormationWorkLedger` by

\[
 w_{n,x}=h_x(\phi_{n+1})-h_x(\phi_n),
 \qquad
 R^E_{n+1,x}=R^E_{n,x}-w_{n,x}.                             \tag{21}
\]

Then pointwise, and hence after summation,

\[
 h_x(\phi_{n+1})+R^E_{n+1,x}
 =h_x(\phi_n)+R^E_{n,x}.                                   \tag{22}
\]

The transaction is finite, not merely telescoping conditionally. Every
iterate satisfies

\[
 \|\phi_n\|_2<\rho={1101\over1000},
 \qquad
 \|\phi_n\|_\infty<{6\over5}.                              \tag{23}
\]

The C18 spectral ceiling is `16/9`. For

\[
 P=3(6/5)^4+4(6/5)^2+1={8113\over625},                     \tag{24}
\]

the kinetic, stiffness, and onsite mean-value bounds give

\[
 \sum_x|w_{n,x}|
 \le A_0^2 C_E\|\phi_{n+1}-\phi_n\|_2,                    \tag{25}
\]

with

\[
 C_E=\rho\left({16\over9}+{16876\over625}\Lambda\right). \tag{26}
\]

Combining (10), (25), and the geometric sum gives

\[
 \boxed{
 \sum_x|w_{n,x}|\le A_0^2C_Ebc^n,
 \qquad
 \sum_{n=0}^{\infty}\sum_x|w_{n,x}|
 \le A_0^2C_E{11\over13022}.}                              \tag{27}
\]

Thus the reference computation has finite total local energy transaction.
The theorem does not say that `R^E` is nonnegative or dynamically
phase-complete.

## 7. Axial-charge transaction

For orientation sign `sigma=+1` or `-1`, the rotating field has onsite axial
charge

\[
 Q_{n,x}=\sigma\omega A_0^2\phi_{n,x}^2.                   \tag{28}
\]

Define

\[
 q_{n,x}=Q_{n+1,x}-Q_{n,x},
 \qquad
 R^Q_{n+1,x}=R^Q_{n,x}-q_{n,x}.                             \tag{29}
\]

Field plus ledger charge is exactly conserved. Cauchy--Schwarz and (23) give

\[
 \sum_x|q_{n,x}|
 \le2\omega A_0^2\rho b c^n,                              \tag{30}
\]

and therefore finite total charge variation

\[
 \sum_{n,x}|q_{n,x}|
 \le2\omega A_0^2\rho{11\over13022}.                       \tag{31}
\]

This books the charge required by preparation. It does not derive `sigma`,
the body axis, the transverse direction, or the native source that supplies
that charge.

## 8. Reversible mismatch port

Let `e` be one fresh field-shaped input port. Define

\[
 u^+=\mathcal T(u)+L^{-1}e,
 \qquad
 m^+=L(u-u^+).                                              \tag{32}
\]

This local coordinate map is exactly invertible:

\[
 \boxed{
 u=u^++L^{-1}m^+,
 \qquad
 e=L\left[u^+-\mathcal T(u)\right].}                       \tag{33}
\]

On the fresh section `e=0`, it performs one relaxation layer and

\[
 m^+=L(u-\mathcal T(u))=\mathcal F(\phi)=r.                \tag{34}
\]

For one scalar coordinate, if `t=D T`, the Jacobian block is

\[
 \begin{pmatrix}
 t&L^{-1}\\
 L(1-t)&-1
 \end{pmatrix},
 \qquad \det=-1.                                           \tag{35}
\]

The explicit inverse (33), rather than the determinant, proves the field-map
bijection. A cotangent lift is consequently symplectic. After recovering the
old field, equations (21) and (29) also recover the old ledger coordinates.

This separates three questions that must not be conflated:

- **profile computation:** equation (7), convergent;
- **information reversibility:** equations (32)--(34), exact with outgoing
  mismatch retained; and
- **physical positive-energy formation:** still open.

Each layer consumes a fresh zero port and outputs a generally nonzero
mismatch port. Reusing the same hardware requires a local reset, transport,
compression, or recycling law. None is supplied for free.

## 9. Epistemic accounting

Theorem-grade, conditional on the selected FTD-0948/0949 action and regime:

- geometric convergence and the exact finite-depth error (1);
- finite causal support and restriction consistency;
- the residual identity and geometric envelope;
- exact local energy and charge ledger conservation;
- finite total absolute work and charge variation;
- exact reversible mismatch-port inversion; and
- target/profile/context/Born blindness of the controller.

Selected or imposed:

- the sextic action and coarse sufficient regime `Lambda>=10^4`;
- the core marker, core amplitude, frequency, and residual Picard controller;
- the body axis, transverse phase direction, and orientation sign;
- fresh zero ports; and
- signed work and charge ledger coordinates.

Open:

- a positive phase-complete local reservoir and its Hamiltonian;
- native reservoir preparation, port transport/recycling, autonomous stopping,
  and irreversible erasure accounting;
- the ordered two-frame and pseudoscalar source of handed charge;
- an exact energy/charge/reversal-preserving finite-range physical tick;
- perturbation stability and recovery;
- mobility, collision/backpressure, body identity, mass, physical scale, and
  production normalization;
- `gamma`, separate quartic-`G*` synchronization, Born/Bell, context,
  Lorentz hiding, and completeness; and
- every production-engine integration.

## 10. Certificate provenance

The FTD-0950 protocol SHA-256 is
`12C21B138BCFFB0F8613194620F8D75A287E6DDD9E25EC40DF50E14B78220988`.
Its immutable parent certificate SHA-256 is
`A2690CAEAEA7363C5E14D492844B250874545EABC8AF029415B3671E69D45071`.
The first parent execution passed `78/80` and returned Outcome D because one
mathematically equal rational power was compared by structural rather than
simplified equality; the chained outcome classifier then failed as designed.

The FTD-0951 verifier-only repair protocol SHA-256 is
`776AA1FCA1126D4CA728C9A1FDC11C90CF3E9ED337742AB0608F1ED9C85A33E4`.
The repair wrapper SHA-256 is
`0F5D54576F5D3AD6045C93B25EF3A2277D1461429ECBB4E50E9A60D5151E3D8C`.
Its first execution passed inherited `80/80` plus repair integrity `9/9`,
Outcome A. The parent files remain unchanged; the repair performs one locked
in-memory `simplify(left-right)==0` normalization and changes no mathematics.

No numerical search, tolerance, fitted target, engine source, production
source, CMake file, type, constant, toggle, or default tick changed.

## 11. Next gate

The profile-computation front is now far enough. The next branch must not
invent another relaxation. It must attack the physical resource layer:

1. construct a positive local canonical reservoir that pays equations
   (21) and (29), or prove a scoped no-go for the minimum candidate;
2. transport and recycle mismatch ports without free erasure;
3. keep the reservoir/controller context-blind and profile-blind;
4. identify the minimum native ordered two-frame plus pseudoscalar source for
   handed charge independently of the reservoir; and
5. only then test perturbation recovery and exact finite-tick dynamics.

```text
CAUSAL_FINITE_RADIUS_RELAXATION=EXACT_REFERENCE_CONSTRUCTION
TARGET_PROFILE_READ=FALSE
DEPENDENCY_RADIUS_PER_LAYER=ONE_C18_STEP
WEIGHTED_ERROR_BOUND=11_OVER_13022_TIMES_2489_OVER_9000_TO_N
FINITE_SUPPORT_AT_EVERY_FINITE_LAYER=TRUE
RESTRICTION_CONSISTENT=TRUE
MISMATCH_EXPORTED=EXACT
REVERSIBLE_MISMATCH_PORT=EXACT
LOCAL_WORK_LEDGER=EXACT_SIGNED_ACCOUNT
LOCAL_CHARGE_LEDGER=EXACT_SIGNED_ACCOUNT
TOTAL_ABSOLUTE_WORK_VARIATION=FINITE
TOTAL_ABSOLUTE_CHARGE_VARIATION=FINITE
POSITIVE_PHASE_COMPLETE_RESERVOIR=OPEN
FRESH_PORT_RECYCLING=OPEN
NATIVE_HANDED_SOURCE=OPEN
EXACT_PHYSICAL_FINITE_TICK=OPEN
PERTURBATION_STABILITY=OPEN
PRODUCTION_INTEGRATION=FORBIDDEN
```
