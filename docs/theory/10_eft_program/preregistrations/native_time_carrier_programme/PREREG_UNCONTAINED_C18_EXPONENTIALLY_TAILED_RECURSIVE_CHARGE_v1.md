# FTD-0949 — Preregistration: uncontained C18 exponentially tailed recursive charge v1

**Identifier:** `FTD-0949`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact weighted-space contraction proof for the FTD-0948 selected
relative-field sextic with the unchanged scalar C18 stiffness; explicit
one-site anti-continuum core, frequency, strong-nonlinearity sufficiency bound,
strict positivity, exponential tail, finite energy/charge, rotating exact-flow
solution, compact-source finite-tick formation obstruction, epsilon-tail
approximation; no numerical search, fit, engine change, production
integration, physical parameter identification, mobility, Born, Bell, context,
outcome, `G*`, or gamma read

## 1. Question

FTD-0948 proves a finite-region fixed-charge recursive minimizer for the
selected same-field action, but deliberately leaves uncontained localization
open. The next question is:

> Does the exact C18-coupled radial sextic admit at least one nonzero,
> finite-energy, exponentially localized recursive rotating solution on the
> uncontained substrate, and can such an everywhere-tailed solution form
> exactly in finitely many local ticks from compact body data?

The proof must be constructive and inequality based. No parameter scan or
near-miss search is permitted. A coarse sufficient regime is acceptable if it
is explicitly selected and not represented as physically derived.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_SOURCE_FRAME_BOUNDARY_v1.md` | `BD5B9DB5C9543F76241E6525B0CCD44787D16FE933D24E742C3982F9E6898981` |
| `proof_minimum_nonlinear_relative_field_recursive_charge_and_source_frame_v3.py` | `D801DE377BA6C34F1A6D882F9420091CB7165D2E094F7B9200D2D6F46A99FFC0` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md` | `C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329` |

The certificate must fail closed on source drift.

## 3. Exact scalar C18 operator

For one transverse polarization, use

\[
 (Kf)_x={4\over3}f_x
 -{1\over9}\sum_{d\in F_6}f_{x+d}
 -{1\over18}\sum_{d\in E_{12}}f_{x+d}.                         \tag{1}
\]

The coefficients sum to zero, so (1) is a positive finite-range graph
Laplacian. Its Fourier symbol is the frozen FTD-0943 symbol

\[
 K(k)={4\over3}-{2\over9}
 \bigl(c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x\bigr).                \tag{2}
\]

Define the exponentially weighted norm

\[
 \|u\|_w^2=\sum_x4^{|x|_1}|u_x|^2.                            \tag{3}
\]

Conjugating a face shift costs at most `2` and an edge shift at most `4`.
The certificate must establish the exact Schur bound

\[
 \boxed{\|K\|_w\le{4\over3}
 +6{2\over9}+12{4\over18}={16\over3}.}                        \tag{4}
\]

This weighted sequence space is an analysis scaffold. The ontic statement
must be restated through the explicit decay and finite-radius epsilon bounds
below, not as adoption of a completed-infinity substrate totality.

## 4. Locked rotating core

Use the selected potential

\[
 V(r)=\beta r^2(r^2-A_0^2)^2,
 \qquad \Lambda=\beta A_0^4.                                  \tag{5}
\]

For a body axis `e`, choose one unit transverse vector `v` and seek

\[
 q_x(t)=A_0\phi_x
 \bigl(\cos(\omega t)v+\sin(\omega t)J_ev\bigr).               \tag{6}
\]

The stationary amplitude equation is

\[
 K\phi+2\Lambda\phi(\phi^2-1)(3\phi^2-1)
 =\omega^2\phi.                                                \tag{7}
\]

Lock, without search,

\[
 a^2={6\over5},
 \qquad \Omega={\omega^2\over2\Lambda}={13\over25},
 \qquad \omega^2={26\Lambda\over25}.                          \tag{8}
\]

Then `(a^2-1)(3a^2-1)=13/25`, so the anti-continuum profile

\[
 \phi^{(0)}=a\delta_0                                         \tag{9}
\]

solves the onsite equation with `K` turned off. The vacuum mass is

\[
 m^2=2\Lambda,
 \qquad m^2-\omega^2={24\Lambda\over25}>0,                    \tag{10}
\]

so the frequency lies strictly inside the FTD-0948 localization window.

## 5. Quantitative contraction gate

Write (7) as

\[
 K\phi+g(\phi)=0,
\]

where

\[
 g(z)=2\Lambda z
 \left(3z^4-4z^2+1-\Omega\right).                             \tag{11}
\]

Let `L` be the diagonal derivative of `g` at (9). The exact entries are

\[
 g'(0)={24\Lambda\over25},
 \qquad g'(a)={384\Lambda\over25},
\]

so

\[
 \boxed{\|L^{-1}\|_w={25\over24\Lambda}.}                     \tag{12}
\]

Set

\[
 r_*=10^{-3},
 \qquad \Lambda\ge10^4.                                      \tag{13}
\]

On the ball `||u||_w <= r_*`, `|a+u_0|<6/5` and every other
coordinate has magnitude at most `r_*`. Since

\[
 g''(z)=24\Lambda z(5z^2-2),
\]

the exact uniform bound

\[
 |g''(z)|\le {6624\over25}\Lambda                             \tag{14}
\]

holds on the registered interval.

For `phi=phi^(0)+u`, define the Newton fixed-point map

\[
 \mathcal T(u)=-L^{-1}
 \left[K(\phi^{(0)}+u)
 +g(\phi^{(0)}+u)-g(\phi^{(0)})-Lu\right].                    \tag{15}
\]

The certificate must verify the exact bounds

\[
 \operatorname{Lip}(\mathcal T)
 \le {50\over9\Lambda}+276r_*
 \le {1\over1800}+{69\over250}< {1\over2},                  \tag{16}
\]

and, using `a<11/10`,

\[
 \|\mathcal T(u)\|_w
 \le {55\over9\Lambda}
 +{50r_*\over9\Lambda}+138r_*^2
 <r_*.                                                         \tag{17}
\]

Banach's fixed-point theorem then gives a unique solution of (7) in the
registered weighted ball.

The bound `Lambda>=10^4` is a coarse sufficient **[IMPOSED REFERENCE REGIME]**.
It is not a fitted physical value, a necessary threshold, or a derivation of
the production coupling.

## 6. Strict positivity and noncompact tail

The fixed point obeys

\[
 \phi_0>a-r_*>0,
 \qquad |\phi_x|\le r_*2^{-|x|_1}\quad(x\ne0).                 \tag{18}
\]

The certificate must prove strict positivity, not merely an absolute tail
bound. If a negative global minimum occurred outside the core, then
`K phi <= 0` there, while for `|phi|<=r_*`

\[
 3\phi^4-4\phi^2+1-\Omega>0,
\]

so `g(phi)<0`; equation (7) would be impossible. Hence `phi>=0`.
If `phi_x=0` at any site, equation (7) and the positive graph weights force
all C18 neighbours to vanish; face connectivity would propagate the zero to
the positive core. Therefore

\[
 \boxed{\phi_x>0\quad\hbox{at every site}.}                    \tag{19}
\]

The solution is exponentially localized but not compactly supported.

## 7. Finite energy, charge, and recursive exact flow

Equation (18) gives `phi in ell^2_w`, hence all stiffness, onsite, kinetic,
and axial-charge sums converge absolutely. With

\[
 p_x(t)=\dot q_x(t)=\omega J_eq_x(t),                           \tag{20}
\]

the exact conserved charge is

\[
 Q_e=\omega A_0^2\sum_x\phi_x^2>0.                            \tag{21}
\]

Equations (6)--(7) imply `qddot=-omega^2 q=-Kq-grad V`, so (6) is an exact
recursive relative equilibrium of the selected continuous Hamiltonian.

For every `R>=0`, the explicit tail estimate is

\[
 \sum_{|x|_1>R}|\phi_x|^2
 \le r_*^2,4^{-(R+1)}.                                       \tag{22}
\]

Equation (22) is the finite-radius/epsilon content appropriate to the
uncontained ontology.

## 8. Causal formation boundary

Consider a deterministic radius-one tick with a fixed vacuum, initialized by
compact body/source data and exact vacuum outside it. After `n` ticks, only
the radius-`n` dependency hull can differ from vacuum. The result therefore
has compact support at every finite `n`.

Equation (19) is nonzero at every site. Consequently:

\[
 \boxed{\text{no finite number of such local ticks can form (6) exactly
 from compact data}.}                                         \tag{23}
\]

This is not a superluminal-signalling claim. The tail is part of the prepared
global body state, and (22) permits finite-radius approximation to any declared
epsilon. Exact formation requires either an already present tail, unbounded
formation time, or a nonlocal initializer. Which approximate formation law is
physical remains open.

## 9. Frozen outcomes

| Outcome | Exact condition | Verdict |
|---|---|---|
| A | The weighted contraction, positivity, exponential tail, finite energy/charge, exact rotating solution, and finite-tick exact-formation obstruction all pass | uncontained exponentially tailed recursive reference body exists in a declared parameter regime; exact compact-source finite-time formation is closed negative |
| B | Existence and finite energy pass but positivity/noncompactness or causal-formation classification cannot be proved | localized reference solution only; formation boundary remains open |
| C | The contraction ball or stationary equation fails | FTD-0948 remains finite-region only in this regime |
| D | source drift or exact gate failure | execution invalid; no theorem |

No tolerance, fit, numerical near-miss, parameter scan, target tail, target
history, Born weight, context, outcome, `G*`, or post-hoc outcome change is
permitted.

## 10. Acceptance and stop conditions

The certificate must report separately:

1. all frozen hashes and protocol firewalls;
2. exact real-space C18 coefficients, zero row sum, symbol, positivity, and
   weighted `16/3` bound;
3. the `a^2=6/5`, `Omega=13/25` onsite identity and vacuum gap;
4. exact diagonal derivative and inverse norm;
5. nonlinear second-derivative bound;
6. contraction and self-map rational inequalities at `Lambda>=10^4`;
7. unique weighted fixed point;
8. maximum-principle nonnegativity and strong positivity;
9. exponential pointwise and finite-radius tail bounds;
10. finite energy, nonzero charge, and exact rotating Hamiltonian equation;
11. finite local dependency hull versus everywhere-positive tail; and
12. all selection, production, scale, Born/Bell/G*/gamma, and ontology
    firewalls.

Stop on source drift. Do not modify production engine sources, tests, CMake,
toggles, constants, or ontology.

## 11. Promotion boundary

Outcome A would close the bare existence question only. The next physical
programme would have to:

1. replace the coarse `Lambda>=10^4` sufficiency bound by a rigorous interval
   classification or a preregistered non-targeted continuation at production-
   relevant normalization;
2. construct a causal approximate formation/relaxation law and book its source
   work, exported mismatch, and convergence rate using (22);
3. close an exact finite-tick energy/charge/reversal rule;
4. attach the native ordered two-frame plus pseudoscalar source identified by
   FTD-0948; and
5. only then test motion, collision/backpressure, recovery, erasure, and
   production integration.

No result here derives matter, mass, gamma, `G*`, Born's rule, Hilbert space,
Lorentz recovery, or completeness.
