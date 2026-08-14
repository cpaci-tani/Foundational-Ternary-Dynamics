# FTD-0949 — Uncontained C18 exponentially tailed recursive charge and formation boundary v1

**Date:** 2026-08-11  
**Status:** `[THEOREM — EXPONENTIALLY TAILED FINITE-ENERGY RECURSIVE RELATIVE-FIELD SOLUTION IN A DECLARED REGIME]` +
`[THEOREM — STRICT POSITIVITY AND NONCOMPACT SUPPORT]` +
`[THEOREM — COMPACT-SOURCE FINITE-LOCAL-TICK EXACT-FORMATION OBSTRUCTION]` +
`[OPEN — SHARP PARAMETER REGION, APPROXIMATE FORMATION DYNAMICS, STABILITY, EXACT FINITE-TICK ENERGY, PRODUCTION]`  
**Verdict:** `OUTCOME_A_UNCONTAINED_REFERENCE_EXISTENCE_EXACT_FINITE_TIME_COMPACT_FORMATION_CLOSED_NEGATIVE`

## 1. Result

The selected FTD-0948 relative-field action has an exact uncontained,
finite-energy, nonzero-charge rotating solution in a rigorously declared
strong-nonlinearity regime.

For

\[
 V(r)=\beta r^2(r^2-A_0^2)^2,
 \qquad \Lambda=\beta A_0^4,                                  \tag{1}
\]

and the unchanged scalar C18 stiffness, take

\[
 \boxed{\Lambda\ge10^4}.                                      \tag{2}
\]

Then there is a unique amplitude profile in the weighted ball

\[
 \left\|\phi-\sqrt{6/5}\,\delta_0\right\|_w\le10^{-3},
 \qquad
 \|u\|_w^2=\sum_x4^{|x|_1}|u_x|^2,                            \tag{3}
\]

that solves

\[
 K\phi+2\Lambda\phi(\phi^2-1)(3\phi^2-1)
 ={26\Lambda\over25}\phi.                                   \tag{4}
\]

The profile is strictly positive at every site and obeys

\[
 \boxed{|\phi_x|\le10^{-3}2^{-|x|_1}\quad(x\ne0).}           \tag{5}
\]

For any body axis `e` and supplied unit transverse direction `v`, equation
(4) gives the exact rotating relative field

\[
 q_x(t)=A_0\phi_x
 \left[\cos(\omega t)v+\sin(\omega t)J_ev\right],
 \qquad
 \omega^2={26\Lambda\over25}.                                \tag{6}
\]

It has finite Hamiltonian energy and nonzero finite axial charge

\[
 Q_e=\omega A_0^2\sum_x\phi_x^2>0.                            \tag{7}
\]

This closes the bare uncontained existence debt left by FTD-0948 in one
explicit sufficient regime. It does not identify a production normalization,
prove perturbative stability, or derive the axis/direction source.

The same proof establishes a formation boundary. Because `phi_x>0` at every
site, the exact body has an infinite exponential tail. A vacuum-preserving
radius-one tick starting from compact source/body data has finite dependency
support after every finite number of ticks. It therefore cannot form (6)
exactly in finite time. Finite-radius approximation remains compatible with
the explicit tail estimate.

## 2. Exact C18 operator and weighted bound

For one transverse polarization,

\[
 (Kf)_x={4\over3}f_x
 -{1\over9}\sum_{d\in F_6}f_{x+d}
 -{1\over18}\sum_{d\in E_{12}}f_{x+d}.                        \tag{8}
\]

Its coefficients sum to zero:

\[
 {4\over3}-6{1\over9}-12{1\over18}=0.                         \tag{9}
\]

Hence `K` is a positive graph Laplacian. Its symbol is

\[
 K(k)={4\over3}-{2\over9}
 \left(c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x\right),               \tag{10}
\]

exactly the FTD-0943 production C18 symbol.

In the norm (3), a face shift changes the weight by at most `2`, and an edge
shift by at most `4`. The conjugated operator therefore has the exact Schur
bound

\[
 \boxed{
 \|K\|_w\le {4\over3}
 +6{2\over9}+12{4\over18}={16\over3}.}                         \tag{11}
\]

No finite-volume limit or fitted decay constant is used.

## 3. The locked anti-continuum core

Write

\[
 a^2={6\over5},
 \qquad
 \Omega={\omega^2\over2\Lambda}={13\over25}.                 \tag{12}
\]

The identity

\[
 (a^2-1)(3a^2-1)={13\over25}                                  \tag{13}
\]

makes

\[
 \phi^{(0)}=a\delta_0                                        \tag{14}
\]

an exact one-site solution when intersite coupling is turned off. The vacuum
mass and frequency gap are

\[
 m^2=2\Lambda,
 \qquad
 m^2-\omega^2={24\Lambda\over25}>0.                           \tag{15}
\]

Thus the tail frequency is strictly below the linear vacuum threshold, inside
the FTD-0948 charged-core window.

The rational values in (12) were selected for a clean proof, not found by a
scan and not matched to an empirical target.

## 4. Contraction proof

Define

\[
 g(z)=2\Lambda z
 \left(3z^4-4z^2+1-\Omega\right).                             \tag{16}
\]

At the anti-continuum profile,

\[
 g'(0)={24\Lambda\over25},
 \qquad
 g'(a)={384\Lambda\over25}.                                  \tag{17}
\]

If `L` is the corresponding diagonal derivative, then

\[
 \|L^{-1}\|_w={25\over24\Lambda}.                            \tag{18}
\]

Moreover,

\[
 g''(z)=24\Lambda z(5z^2-2).                                 \tag{19}
\]

On the registered ball, `|z|<6/5`, so

\[
 |g''(z)|\le{6624\over25}\Lambda.                            \tag{20}
\]

For `phi=phi^(0)+u`, use the fixed-point map

\[
 \mathcal T(u)=-L^{-1}
 \left[K(\phi^{(0)}+u)
 +g(\phi^{(0)}+u)-g(\phi^{(0)})-Lu\right].                   \tag{21}
\]

At `Lambda>=10000` and `r_*=10^-3`, equations (11), (18), and (20)
give

\[
 \operatorname{Lip}(\mathcal T)
 \le {50\over9\Lambda}+276r_*
 \le {1\over1800}+{69\over250}
 ={2489\over9000}<{1\over2}.                                \tag{22}
\]

Using `a<11/10`, the self-map bound is

\[
 \|\mathcal T(u)\|_w
 \le {55\over9\Lambda}
 +{50r_*\over9\Lambda}+138r_*^2
 \le {2249\over3000000}<10^{-3}.                             \tag{23}
\]

The closed weighted ball is complete. Banach's theorem therefore supplies a
unique fixed point in that ball.

The constant `10^4` is a coarse sufficient **[IMPOSED REFERENCE REGIME]**. It
is not claimed to be sharp, necessary, natural, or physically realized.

## 5. Strict positivity

The fixed point has

\[
 \phi_0>a-10^{-3}>0,
\]

and all exterior values have magnitude at most `10^-3`. On that exterior
interval,

\[
 3\phi^4-4\phi^2+1-\Omega
 >{12\over25}-{4\over10^6}>0.                                 \tag{24}
\]

If a negative global minimum existed, it would occur away from the positive
core. The graph-Laplacian property gives `Kphi<=0` there, while (24) gives
`g(phi)<0`. Their sum cannot vanish. Hence `phi>=0`.

If `phi_x=0` at any site, the onsite term vanishes and equation (4) requires

\[
 -{1\over9}\sum_{F_6}\phi_{x+d}
 -{1\over18}\sum_{E_{12}}\phi_{x+d}=0.                        \tag{25}
\]

All terms are nonpositive, so every neighbour must also be zero. Face
connectivity propagates that zero along a finite path to the positive core,
a contradiction. Therefore

\[
 \boxed{\phi_x>0\quad\text{for every specified site}.}        \tag{26}
\]

The solution is not compactly supported.

## 6. Finite energy and exact recursion

Weighted-square summability implies ordinary square summability. Because `K`
is bounded and finite range, the stiffness energy is finite. The profile is
also bounded, so every quadratic, quartic, and sextic onsite sum converges.
Equation (7) is therefore finite and positive.

With `p=qdot=omega J_e q`, equation (4) is exactly

\[
 Kq+\nabla V(q)=\omega^2q.                                    \tag{27}
\]

Equation (6) obeys

\[
 \ddot q=-\omega^2q=-Kq-\nabla V(q),                           \tag{28}
\]

so it is an exact relative equilibrium of the selected continuous
Hamiltonian. It returns after `2pi/omega` while retaining the sign of `Q_e`.

This is an existence and local-uniqueness theorem, not a spectral or nonlinear
perturbation-stability theorem. Attraction, recovery, and robustness remain
open.

## 7. Uncontained interpretation

The weighted completion is a proof scaffold, not an ontic claim that the
substrate is a completed infinity. Its operational content is the explicit
finite-radius estimate

\[
 \boxed{
 \sum_{|x|_1>R}|\phi_x|^2
 \le10^{-6}4^{-(R+1)}.}                                      \tag{29}
\]

For every declared epsilon, a finite radius follows directly from (29). Every
local equation concerns only finitely many neighbours, and every requested
tail accuracy is witnessed in a finite region.

## 8. Exact finite-time formation obstruction

Let a localized formation process begin from compact body/source data with
the exact vacuum elsewhere. Assume:

1. one tick reads only a radius-one Moore neighbourhood; and
2. a site whose complete dependency cone contains only vacuum remains vacuum.

After `n` ticks, only the finite radius-`n` dependency hull of the original
support can differ from vacuum. The formed state has compact support at every
finite `n`.

The target (6) is nonzero at every site by (26). Therefore

\[
 \boxed{
 \text{no finite number of vacuum-preserving local ticks can form the exact
 exponentially tailed body from compact data}.}               \tag{30}
\]

This does not imply operational superluminal signalling. An already prepared
tail is part of the body state, and a causal process may approximate it to any
finite tolerance by growing its dependency hull. Exact preparation requires
an already present tail, unbounded formation time, or a nonlocal initializer.

The physically relevant next problem is therefore convergence and work of a
causal approximate formation law, not repeated search for exact compact
support.

## 9. Epistemic accounting

Theorem-grade in the locked reference regime:

- the exact real-space C18 operator and weighted `16/3` norm bound;
- the selected anti-continuum amplitude/frequency identity and positive vacuum
  gap;
- the diagonal inverse and nonlinear derivative bounds;
- contraction and self-map at `Lambda>=10^4`;
- existence and uniqueness in the registered weighted ball;
- nonnegativity, strict positivity, and noncompact exponential support;
- finite energy and nonzero axial charge;
- the exact rotating continuous-Hamiltonian solution;
- finite-radius epsilon tail control; and
- the compact-source finite-local-tick exact-formation obstruction.

Selected or imposed:

- the FTD-0948 action itself;
- `a^2=6/5`, `Omega=13/25`, and the sufficient `Lambda>=10^4` regime;
- body axis `e` and transverse direction `v`;
- the use of exact continuous Hamiltonian flow; and
- the localized vacuum-preserving formation class.

Open:

- the sharp existence and stability region;
- whether production normalization lies anywhere in that region;
- causal approximate formation, convergence rate, source work, and exported
  mismatch;
- an exact local finite-tick energy/charge/reversal law;
- native formation of the FTD-0948 two-frame/pseudoscalar source;
- mobility, collision/backpressure, recovery, erasure, and body identity;
- coupling to the separate exact critical-quartic `G*` clock;
- gamma, mass, physical scale, total momentum, Born/Bell, context, Lorentz
  hiding, and completeness; and
- all production integration.

## 10. Certificate provenance

The locked protocol SHA-256 is
`25667F46B981A3F0201F934F6A14856316DAF3025B54DC5C8800D31836404AC1`.
The exact certificate SHA-256 is
`A9C72A3DB5B9E5E4F814470F5DB2DBA4CEFEB3FB125DD3B3BE9E7E26BC0D9536`.
Its first immutable execution passed `71/71`, Outcome A. No repair, tolerance,
scan, engine source, production source, constant, toggle, CMake file, or
ontology changed.

## 11. Next gate

The next programme should not yet move this body. It should first close the
formation/tick layer:

1. preregister a causal finite-radius relaxation from compact data toward the
   exact profile, with the error measured against (29), not against a fitted
   target;
2. name the source reservoir and prove the local work plus exported mismatch
   ledger at every tick;
3. test whether exact charge can be injected using an existing ordered
   two-event frame and a native pseudoscalar, or book the missing source type;
4. construct or close negative an exact finite-tick local map preserving
   energy, charge, and reversal; and
5. only after those pass, test perturbation stability, mobility,
   collision/backpressure, recovery, erasure, and production integration.

```text
SELECTED_C18_SEXTIC_UNCONTAINED_RECURSIVE_SOLUTION=EXISTS_CONDITIONALLY
SUFFICIENT_REGIME=BETA_A0_FOURTH_POWER_AT_LEAST_10000
CORE_AMPLITUDE_SQUARED=6_OVER_5
FREQUENCY_SQUARED=26_LAMBDA_OVER_25
VACUUM_MASS_GAP=24_LAMBDA_OVER_25
WEIGHTED_C18_NORM_BOUND=16_OVER_3
LOCKED_BALL_RADIUS=1_OVER_1000
PROFILE_STRICTLY_POSITIVE=TRUE
PROFILE_COMPACT_SUPPORT=FALSE
PROFILE_EXPONENTIAL_TAIL=TRUE
ENERGY_FINITE=TRUE
AXIAL_CHARGE_FINITE_NONZERO=TRUE
CONTINUOUS_HAMILTONIAN_RECURSION=EXACT
PERTURBATION_STABILITY=OPEN
FINITE_LOCAL_TICKS_FROM_COMPACT_DATA_FORM_EXACT_PROFILE=FALSE
FINITE_RADIUS_EPSILON_APPROXIMATION=TRUE
EXACT_LOCAL_FINITE_TICK_ENERGY_CHARGE_RULE=OPEN
NATIVE_TWO_FRAME_PSEUDOSCALAR_SOURCE=OPEN
PRODUCTION_NORMALIZATION=OPEN
GAMMA_GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
PRODUCTION_INTEGRATION=FORBIDDEN
```
