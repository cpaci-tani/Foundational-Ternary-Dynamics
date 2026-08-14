# FTD-0952/0953 — Nonlinear C18 Routh-port relaxation and charge-reservoir boundary v1

**Date:** 2026-08-11  
**Status:** `[THEOREM — STRONGLY CONVEX NONLINEAR ROUTH BRANCH]` +
`[THEOREM — TARGET-BLIND EIGHT-COLOR FINITE-GROUNDED RELAXATION]` +
`[THEOREM — POSITIVE CANONICAL NONLINEAR ROUTH PORT]` +
`[THEOREM — EXPLICIT FINITE-RADIUS BODY ERROR]` +
`[CLOSED NEGATIVE — PHASE-BLIND STATE-DEPENDENT CHARGE/ACTION DRAIN]` +
`[OPEN — COMMON PHASE-REACTING CHARGE-TRANSFER HAMILTONIAN, NATIVE RESERVOIR, 3D PORT ROUTING, EXACT PHYSICAL TICK, STABILITY, PRODUCTION]`  
**Verdict:** `OUTCOME_B_POSITIVE_NONLINEAR_ROUTH_RELAXATION_PHYSICAL_CHARGE_RESERVOIR_OPEN`

## 1. Result

The selected nonlinear recursive body has a positive local canonical
relaxation mechanism at the rotating-frame, or Routh, level.

On every declared finite grounded region, the registered FTD-0949 branch is
strongly convex. An eight-color C18 schedule may therefore minimize every
active local coordinate without active-active interference. The updates read
only the current neighbours, core marker, and selected action parameters.
They converge to the unique grounded minimizer and never read the exact
uncontained profile.

Each local nonlinear minimization is exactly a quarter-turn between:

- the field's local Routh-energy coordinate; and
- one fresh complete canonical environment port.

The quarter-turn is symplectic, positive, reversible, and retains the removed
field distinction as the outgoing port. This is the nonlinear extension of
the FTD-0886/0930 positive source-centered gate.

The mechanism is not yet physical charged-body formation. It conserves

\[
 H_{\rm rot}-\sigma\omega Q+E_{\rm port},                   \tag{1}
\]

not physical field energy and axial charge separately. A co-rotating action
reservoir has enough finite capacity for every declared finite computation,
but the minimum phase-blind state-dependent action debit is not symplectic.
A common Hamiltonian must react on the reservoir phase or exchange a complete
charge-transfer mode.

## 2. Nonlinear Routh functional

Retain

\[
 \Lambda=\beta A_0^4\ge10^4,
 \qquad a^2={6\over5},
 \qquad \omega^2={26\Lambda\over25}.                       \tag{2}
\]

For the scalar profile define

\[
 \mathscr S(\phi)
 ={1\over2}\langle\phi,K\phi\rangle
 +\Lambda\sum_x\phi_x^2(\phi_x^2-1)^2
 -{\omega^2\over2}\|\phi\|_2^2.                          \tag{3}
\]

Its gradient is exactly the FTD-0949 stationary equation:

\[
 \nabla\mathscr S(\phi)=K\phi+g(\phi),                     \tag{4}
\]

\[
 g(z)=2\Lambda z\left(3z^4-4z^2+1-{13\over25}\right).     \tag{5}
\]

On a finite grounded region containing the marked core, take the local branch

\[
 |\phi_0-a|\le r_*={1\over1000},
 \qquad
 |\phi_x|\le r_*\quad(x\ne0).                              \tag{6}
\]

FTD-0949 supplies

\[
 g'(0)={24\Lambda\over25},
 \qquad
 g'(a)={384\Lambda\over25},                               \tag{7}
\]

and, throughout the branch,

\[
 |g'(z)-g'(ac_x)|
 \le {6624\over25}\Lambda r_*.                            \tag{8}
\]

Since `K` is positive semidefinite, equations (7)--(8) give

\[
 \boxed{
 \nabla^2\mathscr S\succeq\mu I,
 \qquad
 \mu={2172\over3125}\Lambda>0.}                           \tag{9}
\]

The registered branch is strongly convex.

The exact FTD-0949 fixed point is strictly inside it because

\[
 \|\phi_*-a\delta_0\|_w
 \le {2249\over3000000}<{1\over1000}.                       \tag{10}
\]

## 3. Finite-grounded approximation to the tailed body

Let `P_R` retain the `l1` ball of radius `R` and set the exterior to zero.
The FTD-0949 tail obeys

\[
 \|(I-P_R)\phi_*\|_2
 \le10^{-3}2^{-(R+1)}.                                     \tag{11}
\]

Let `psi_R` be the unique minimizer of (3) on the grounded branch (6). The
truncated exact profile has residual

\[
 \nabla\mathscr S_R(P_R\phi_*)
 =-P_RK(I-P_R)\phi_*.                                      \tag{12}
\]

Using `||K||_2<=16/9`, strong monotonicity, and the minimizer variational
inequality gives

\[
 \|\psi_R-P_R\phi_*\|_2
 \le {16\over9\mu}\|(I-P_R)\phi_*\|_2.                    \tag{13}
\]

Since

\[
 {16\over9\mu}={12500\over4887\Lambda},                   \tag{14}
\]

the finite operational error is

\[
 \boxed{
 \|\psi_R-\phi_*\|_2
 \le10^{-3}2^{-(R+1)}
 \left(1+{12500\over4887\Lambda}\right).}                 \tag{15}
\]

Equation (15) is a finite-radius statement, not a completed-infinity or
`R to infinity` ontology claim.

## 4. Eight-color target-blind relaxation

Color each site by its three coordinate parities. Every C18 face step changes
one parity bit and every C18 edge step changes two, so no coupled sites share
a color.

Holding inactive sites fixed, one active site has local functional

\[
 U_x(z)={2\over3}z^2-h_xz
 +\Lambda z^2(z^2-1)^2-{\omega^2\over2}z^2,                \tag{16}
\]

\[
 h_x=\sum_{y\sim x}w_{xy}\phi_y.                           \tag{17}
\]

Its derivative and curvature are

\[
 U'_x(z)={4\over3}z-h_x+g(z),
 \qquad
 U''_x(z)={4\over3}+g'(z)>0.                               \tag{18}
\]

The total neighbour coefficient is `4/3`, and all branch amplitudes are below
`6/5`, so `|h_x|<=8/5`. At `Lambda=10^4`, the minimum nonlinear endpoint
response exceeds the worst linear and neighbour contribution by

\[
 \mu r_*-\left({4\over3}{6\over5}+{8\over5}\right)
 ={2344\over625}>0.                                        \tag{19}
\]

Thus `U'_x` points inward at both interval endpoints and has one interior zero
`z_x^*`.

One color layer replaces every active value by this local minimizer. The
same-color updates commute. On a finite product branch:

1. every layer remains in the branch;
2. `mathscr S` decreases strictly unless the active residuals vanish;
3. its values converge because the branch is compact;
4. every cluster point must minimize all eight color blocks, otherwise a
   later visit would cause a fixed positive decrease; and
5. strong convexity makes that coordinatewise minimizer the unique `psi_R`.

Consequently the entire cyclic sequence converges to `psi_R`. Compactness
proves convergence, not a volume-independent rate. Combining it with (15)
gives a finite color depth for every declared finite error.

## 5. Positive nonlinear Routh port

At one active site define

\[
 u_x(z)=\operatorname{sgn}(z-z_x^*)
 \sqrt{2A_0^2[U_x(z)-U_x(z_x^*)]}.                          \tag{20}
\]

Strict convexity makes `u_x` monotone. Away from the minimum,

\[
 {du\over dz}={A_0^2U'_x(z)\over u},                       \tag{21}
\]

and at the minimum

\[
 {du\over dz}\longrightarrow A_0\sqrt{U''_x(z_x^*)}>0.   \tag{22}
\]

Hence (20) is a valid local coordinate chart on the realized branch. Its
canonical conjugate is

\[
 \pi_u={p_z\over du/dz}.                                    \tag{23}
\]

Introduce one fresh complete port pair `(a,pi_a)` and apply

\[
 \boxed{
 (u,a,\pi_u,\pi_a)\mapsto(a,-u,\pi_a,-\pi_u).}            \tag{24}
\]

The map is orthogonal, symplectic, determinant one, fourth order, and exactly
reversible. It preserves

\[
 N={1\over2}(u^2+a^2+\pi_u^2+\pi_a^2)>0                    \tag{25}
\]

away from the zero state.

On the fresh zero-conjugate section, `u` becomes zero, so the field moves to
`z_x^*`, while the outgoing port becomes `-u`. Therefore

\[
 \boxed{A_0^2\Delta\mathscr S+\Delta E_{\rm port}=0.}       \tag{26}
\]

The positive clocked Hamiltonian interpolation already established by
FTD-0886 applies in this canonical chart. The realized forward/reverse
quarter-cycle remains inside the chart segment between `z` and `z_x^*`.
Any global chart extension is imposed reference structure and carries no
additional physical claim.

## 6. Port supply and reduced unactualization

A used port contains the full outgoing coordinate, conjugate response, sign,
and positive energy. It is not blank.

- A bank of `C` initially blank complete pairs supplies `C` generic layers.
- A finite cyclic bank is not generically fresh on layer `C+1`.
- An open or bilateral rail transports complete pairs by a symplectic,
  energy-preserving permutation.

Thus finite-error preparation needs only finite port capacity, while
indefinite repeated operation requires growing/outgoing history or a new
compression/recycling theorem. Exporting energy alone is not enough.

If the outgoing ports become inaccessible, the reduced field description has
relaxed and lost its old residual. The full field-plus-port description has
not erased it. This is the same precise reduced meaning of unactualization as
FTD-0929/0951, now with a positive canonical port energy.

## 7. Why positive Routh energy is not full physical energy

For orientation `sigma=+1` or `-1`, the rotating field has

\[
 H_{\rm rot}=A_0^2\left[
 {\omega^2\over2}\|\phi\|^2
 +{1\over2}\langle\phi,K\phi\rangle
 +\Lambda\sum_x\phi_x^2(\phi_x^2-1)^2\right],             \tag{27}
\]

\[
 Q=\sigma\omega A_0^2\|\phi\|^2.                         \tag{28}
\]

Because `sigma^2=1`, equations (3), (27), and (28) give

\[
 \boxed{A_0^2\mathscr S=H_{\rm rot}-\sigma\omega Q.}       \tag{29}
\]

Combining (26) and (29),

\[
 \boxed{
 \Delta H_{\rm rot}+\Delta E_{\rm port}
 =\sigma\omega\Delta Q.}                                  \tag{30}
\]

The port closes the Routh account. The right side of (30) is still a physical
energy/charge transaction.

A co-rotating reservoir action `I>0` with

\[
 E_R=\omega I,
 \qquad Q_R=\sigma I                                      \tag{31}
\]

would close both accounts algebraically under

\[
 \Delta I=-\sigma\Delta Q.                                 \tag{32}
\]

Every declared finite region and finite number of layers has bounded field
charge, so a finite positive initial action can satisfy (32). This proves
capacity only.

## 8. Canonical obstruction for the minimum charge battery

Let the already completed field-plus-port layer be symplectic on `z`, and
give the reservoir the product form

\[
 \Omega_{\rm total}=\Omega_z+dI\wedge d\theta.              \tag{33}
\]

The minimum phase-blind debit would be

\[
 (z,I,\theta)\mapsto
 (F(z),I-\sigma\Delta Q(z),\theta).                         \tag{34}
\]

Its pullback adds

\[
 \boxed{-\sigma\,d(\Delta Q)\wedge d\theta.}               \tag{35}
\]

For a genuine state-dependent charge transfer, this is nonzero. Equation
(34) is not symplectic.

Therefore:

\[
 \boxed{
 \text{positive action capacity does not by itself constitute a canonical
 charged-body reservoir.}}                                 \tag{36}
\]

A physical completion must react on `theta`, exchange a complete co-rotating
mode through a common Hamiltonian, or export the missing conjugate
information. A signed scalar ledger or post-hoc square-root battery cannot be
promoted to that mechanism.

## 9. Epistemic accounting

Theorem-grade within the selected FTD-0949 branch:

- the Routh gradient and strong-convexity constant;
- the explicit finite-grounded error (15);
- local interior minimizers and target-blind eight-color convergence;
- the nonlinear energy coordinate and positive canonical port quarter-turn;
- exact Routh/port exchange and complete-pair history export;
- finite port and action capacity for any declared finite computation; and
- the phase-blind state-dependent action-drain obstruction.

Selected or imposed:

- the FTD-0948/0949 action, strong-nonlinearity regime, and branch intervals;
- the eight-color order and stopping decision;
- the local energy-coordinate chart and any extension outside the realized
  forward/reverse segment;
- fresh complete port pairs and their rail schedule; and
- the candidate co-rotating action/phase reservoir.

Open:

- a common Hamiltonian implementing phase-reacting charge transfer;
- native formation, orientation, and replenishment of its reservoir;
- finite three-dimensional port routing, congestion, return, compression, and
  recycling;
- autonomous scheduling and stopping;
- exact full physical finite-tick energy, charge, and reversal;
- perturbation stability/recovery, mobility, collision/backpressure, mass,
  scale, and production normalization;
- `gamma`, quartic-`G*` synchronization, Born/Bell, context, Lorentz hiding,
  and completeness; and
- every production integration.

## 10. Certificate provenance

The FTD-0952 protocol SHA-256 is
`0326481C47902DBD3EBD9442D904BD37CE014CF551135FC50D1F6CEF953246F5`.
Its immutable parent certificate SHA-256 is
`0E4C35A5C0B616A091B44906F10F1431086E88A0C1F19041DF2FA96E5496CFD5`.
The first execution halted before classification because `omega` had been
declared merely nonzero while a later reserve check required its registered
positive branch.

The FTD-0953 verifier-only repair protocol SHA-256 is
`3744105630F45E8998104FE779B5050778A42FD8E75C6D3D98B94E006C81FE92`.
The repair wrapper SHA-256 is
`092EC6B94DD6E3498A96EBDF982FAC915288FF1BADCD0DE8766A7F1C865065C8`.
Its first execution passes inherited `87/87` plus repair integrity `9/9`,
Outcome B. It performs exactly one in-memory symbol-assumption repair and
changes no source, equation, bound, outcome, or scope.

No numerical search, tolerance, fit, empirical substitution, engine source,
CMake file, type, constant, toggle, or production law changed.

## 11. Next gate

The next admissible mechanism is now specific. It is not another amplitude
relaxation or another scalar battery. It is a local common Hamiltonian that:

1. couples the body phase/charge to a complete co-rotating reservoir mode;
2. produces equation (32) together with the required phase reaction;
3. keeps total physical energy, total axial charge, and the complete inverse
   exact;
4. uses the positive Routh port for amplitude mismatch and an explicit rail
   for outgoing history;
5. remains blind to the final profile, measurement context, outcome, Born
   weights, and `G*`; and
6. has a finite local reserve/backpressure rule.

```text
NONLINEAR_ROUTH_BRANCH_STRONGLY_CONVEX=TRUE
STRONG_CONVEXITY=2172_LAMBDA_OVER_3125
FINITE_GROUNDED_TARGET_ERROR=EXPLICIT
EIGHT_COLOR_NONLINEAR_RELAXATION=TARGET_BLIND_CONVERGENT
POSITIVE_CANONICAL_ROUTH_PORT=EXACT
PORT_OUTPUT_RETAINS_COMPLETE_MISMATCH=TRUE
FINITE_PORT_BANK_HORIZON=CAPACITY
FINITE_CYCLIC_INDEFINITE_FRESHNESS=FALSE
ROUTH_IDENTITY=H_MINUS_SIGMA_OMEGA_Q
PHYSICAL_ENERGY_AND_CHARGE_SEPARATELY_CLOSED=FALSE
CO_ROTATING_ACTION_RESERVE_FINITE_FOR_FINITE_HORIZON=TRUE
PHASE_BLIND_STATE_DEPENDENT_ACTION_DRAIN_SYMPLECTIC=FALSE
COMMON_PHASE_REACTING_CHARGE_TRANSFER_HAMILTONIAN=OPEN
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
PRODUCTION_INTEGRATION=FORBIDDEN
```
