# FTD-0956/0957 — Relative-action-curvature synchronization and crossing-section energy boundary v1

**Date:** 2026-08-11  
**Status:** `[SELECTION — SAME-FIELD RELATIVE-ACTION ENERGY WITH TWO POSITIVE SCALES]` +
`[THEOREM — GLOBAL PERIODICITY, POSITIVITY, AND RECIPROCAL HAMILTON EQUATIONS]` +
`[THEOREM — EXACT AXIAL-CHARGE AND MATCHED-ENERGY CONSERVATION]` +
`[THEOREM — LYAPUNOV-STABLE PHASE LOCK AND ELLIPTIC DISCRETE FLOQUET MAP]` +
`[THEOREM — ZERO-PHASE CROSSING COMPATIBILITY WITH THE FTD-0955 COMPILER]` +
`[BOUNDARY — NON-ISOCHRONY, AUTONOMOUS ENGAGEMENT, FULL NONLINEAR STABILITY, AND ATTRACTION OPEN]`  
**Verdict:** `OUTCOME_B_MINIMUM_CONSERVATIVE_SYNCHRONIZATION_REFERENCE`

## 1. Result

The FTD-0955 missing dynamics can be supplied without adding another matter
species. The current body's phase/action and the existing reservoir's
phase/action already contain a relative canonical sector. The selected
positive energy

\[
 \boxed{
 H_{\rm sync}=
 {\bigl(L-\sigma\omega\rho^2\bigr)^2\over2M_\delta}
 +K_\delta\bigl[1-\cos(\theta-\sigma\vartheta)\bigr]}
                                                               \tag{1}
\]

turns that sector into a stable recursive system. Its two terms are the
conjugate halves: phase mismatch stores potential energy and circular-action
mismatch stores kinetic energy. Energy oscillates between them while the full
phase/action history is retained.

Equation (1) is not derived from the ternary substrate. It is the simplest
positive globally periodic reference law satisfying the nonzero-relative-
action-curvature requirement proved by FTD-0955. `M_delta` and `K_delta` are
new positive selected scales. All theorem statements below are conditional on
this explicit adoption.

## 2. Same-field canonical sector

On the regular polar branch define

\[
 \delta=\theta-\sigma\vartheta,
 \qquad
 \Pi=L-\sigma\omega\rho^2,
 \qquad
 Q=L+\sigma I,                                         \tag{2}
\]

where `sigma` is `-1` or `+1`. The matched linear term is

\[
 H_0=\sigma\omega L+\omega I=\sigma\omega Q.           \tag{3}
\]

Thus

\[
 H_{\rm match}=H_0+H_{\rm sync}.                       \tag{4}
\]

The law is single-valued and `2pi` periodic in both phases. It reads current
local amplitude, current body action, current phases, orientation, and the
maintained rate. It reads no completed profile, future target, measurement
context, outcome, probability, Born weight, Bell setting, or `G*` value.

## 3. Exact reciprocal dynamics

Hamilton's equations from (4) are

\[
 \begin{aligned}
 \dot\rho&=0,\\
 \dot p_\rho&={2\sigma\omega\rho\Pi\over M_\delta},\\
 \dot\theta&=\sigma\omega+{\Pi\over M_\delta},\\
 \dot L&=-K_\delta\sin\delta,\\
 \dot\vartheta&=\omega,\\
 \dot I&=\sigma K_\delta\sin\delta.
 \end{aligned}                                         \tag{5}
\]

The radial equation is essential. Dropping it would again remove the
reciprocal conjugate response and break the full canonical account.

Equations (2) and (5) reduce exactly to

\[
 \boxed{\dot\delta={\Pi\over M_\delta}},
 \qquad
 \boxed{\dot\Pi=-K_\delta\sin\delta}.                 \tag{6}
\]

They also give

\[
 \boxed{\dot Q=0},
 \qquad
 \boxed{\dot H_{\rm sync}=0},
 \qquad
 \boxed{\dot H_{\rm match}=0}.                        \tag{7}
\]

At `delta=0 mod 2pi` and `Pi=0`, all synchronization forces and reciprocal
kicks vanish. The maintained body/reservoir phases continue at their matched
rates, so the equilibrium does not freeze the clock.

## 4. Positive energy and Lyapunov stability

For positive `M_delta` and `K_delta`,

\[
 H_{\rm sync}\ge0,                                    \tag{8}
\]

with equality exactly at

\[
 \Pi=0,
 \qquad
 \delta=0\pmod{2\pi}.                                 \tag{9}
\]

For a level `H_sync=E` below the separatrix,

\[
 0<E<2K_\delta,                                        \tag{10}
\]

the connected component containing the lock is compact and obeys

\[
 |\Pi|\le\sqrt{2M_\delta E},
 \qquad
 1-\cos\delta\le {E\over K_\delta}<2.                \tag{11}
\]

Therefore the lock is Lyapunov stable. Every nonzero regular orbit in this
well is a libration and crosses `delta=0` twice per period. This is
conservative restoration: an error is converted recursively between relative
phase and relative action rather than erased.

It is not attraction. A nonzero orbit retains its positive energy and cannot
converge to the zero-energy fixed point.

## 5. Exact discrete Floquet law

Let

\[
 \kappa=\sqrt{K_\delta/M_\delta},
 \qquad
 \nu=\kappa T.                                         \tag{12}
\]

Sampling the exact flow every duration `T` gives the lock linearization

\[
 R_T=
 \begin{pmatrix}
 \cos\nu & \sin\nu/\sqrt{M_\delta K_\delta}\\
 -\sqrt{M_\delta K_\delta}\sin\nu & \cos\nu
 \end{pmatrix}.                                        \tag{13}
\]

It satisfies

\[
 \det R_T=1,                                           \tag{14}
\]

is symplectic, and exactly preserves the quadratic lock energy

\[
 H_{\rm lin}={1\over2}K_\delta\delta^2
             +{\Pi^2\over2M_\delta}.                  \tag{15}
\]

Its characteristic polynomial is

\[
 \lambda^2-2\cos\nu\,\lambda+1.                     \tag{16}
\]

For `nu` outside `pi Z`, the multipliers are the nondegenerate elliptic pair

\[
 \lambda_\pm=e^{\pm i\nu},
 \qquad |\lambda_\pm|=1.                              \tag{17}
\]

This is the exact discrete answer to the FTD-0955 unit-multiplier boundary:
the single neutral direction becomes a bounded conjugate rotation. It still
does not contract phase-space volume.

## 6. Stable is not isochronous

For `m=E/(2K_delta)`, the nonlinear libration period is

\[
 \boxed{
 T(E)=4\sqrt{M_\delta/K_\delta}\,\mathbf K(m),}        \tag{18}
\]

where `bold K` is the complete elliptic integral of the first kind. Hence

\[
 T(0)=2\pi\sqrt{M_\delta/K_\delta}.                   \tag{19}
\]

The exact expansion begins

\[
 \mathbf K(m)={\pi\over2}
 \left(1+{m\over4}+{9m^2\over64}+\cdots\right),       \tag{20}
\]

and the period diverges at the separatrix. The positive divergence follows
without trusting a complex-infinity CAS value: with `m=1-epsilon` the
terminal integral is bounded below by

\[
 \int_0^1{dy\over\sqrt{\epsilon+y^2}}
 =\operatorname{asinh}(1/\sqrt\epsilon)\to+\infty.     \tag{21}
\]

The law is therefore stable but amplitude-detuned. It does not identify a
unique physical cadence and does not derive the critical-quartic `G*` period.

## 7. Exact FTD-0955 crossing-section ledger

At any zero-phase crossing, not only at zero relative momentum, apply the
FTD-0955 locked compiler endpoint

\[
 D=\sigma\omega(\rho_*^2-\rho^2),                     \tag{22}
\]

\[
 L'=L+D,
 \qquad I'=I-\sigma D,
 \qquad \rho'=\rho_*.
                                                               \tag{23}
\]

Then

\[
 Q'=Q,                                                  \tag{24}
\]

and, crucially,

\[
 \Pi'=L'-\sigma\omega\rho'^2
 =L-\sigma\omega\rho^2=\Pi.                          \tag{25}
\]

Because `delta'=delta=0`,

\[
 H_{\rm sync}'=H_{\rm sync}.                          \tag{26}
\]

Appending this exact identity to the FTD-0955 endpoint ledger gives

\[
 \boxed{
 \Delta\left(H_{\rm rot}+E_{\rm port}+\omega I
             +H_{\rm sync}\right)=0.}                 \tag{27}
\]

The global compiler action returns at its boundaries and cannot hide a term
in (27). Reverse compiler flow followed by reverse synchronization flow
recovers every retained variable.

This crossing-section identity has a useful interpretation: phase error need
not be destroyed before an event. The conservative dynamics converts it into
relative-action energy, the event occurs at the zero-phase crossing, and the
stored distinction remains available to the inverse.

## 8. Coupled nonlinear boundary

After a general off-crossing charge/Routh endpoint, the mismatch obeys

\[
 \Pi'=\Pi+D(\cos\delta-1).                             \tag{28}
\]

The added term has zero value and zero first derivative at lock, so the
FTD-0955 compiler has identity linearization on `(delta,Pi)`. Its composition
with synchronization therefore has the exact Floquet matrix (13).

But equation (28) begins at quadratic order. Linear Floquet stability does
not prove nonlinear stability under indefinite repeated compilation. That
question remains separately preregisterable.

The theorem also does not derive how a zero-phase crossing autonomously
releases or aligns the already-global two-window controller without resetting
its phase. A Poincare-section composition is not yet native engagement
hardware.

## 9. Epistemic and ontology accounting

Theorem-grade within the selected law (1):

- global phase periodicity and positive synchronization energy;
- the full six Hamilton equations including radial reciprocal reaction;
- exact total axial-charge and matched-energy conservation;
- Lyapunov stability and recurrent sub-separatrix crossings;
- the exact elliptic discrete Floquet matrix and energy metric;
- the nonlinear period and non-isochrony boundary;
- exact crossing-section `Q`, `Pi`, synchronization-energy, physical-energy,
  and inverse closure with FTD-0955; and
- the quadratic-order off-crossing coupling boundary.

Selected or imposed:

- the functional law (1), including the cosine phase potential;
- positive scales `M_delta` and `K_delta`;
- the synchronization sampling duration and phase origin; and
- engagement of the compiler on a zero-phase crossing.

Open:

- a substrate-native origin or reduction of the selected law and scales;
- autonomous crossing detection and phase-aligned controller engagement;
- an isochronous or feedback-stabilized cadence;
- full nonlinear stability of the repeated charge/Routh/sync map;
- attraction through complete positive phase-error export;
- native source/reservoir formation, replenishment, finite 3D routing and
  recycling, mobility, collision, erasure, mass, scale, `gamma`, and
  production;
- quartic-`G*` synchronization, Born/Bell recovery, Lorentz hiding, and
  completeness; and
- every engine integration.

## 10. Certificate and repair provenance

Parent protocol SHA-256:
`EB22D8BC597A22E676D9B38BD38C9E1DB8B9C9D703D68A856A9B3525CE2D4D28`.
The immutable parent certificate SHA-256 is
`04BAE420DFC7C49CA5A5DCAA4D6E2F547DF4F1EF91C7A8ADE2EC4D79F8613FE3`.
Its first execution reached `108/111`, Outcome D, on two equality-
normalization defects and one real one-sided-limit serialization defect.

Repair protocol SHA-256:
`FA260358D7830E056780A158FF47AD710C8D612C8112B3998C8F68156EC64471`.
The in-memory repair wrapper SHA-256 is
`28E1CB38FCC5653D984D2555BFB0D94B916DCD7C952E3A03661D6F531127323D`.
Its first execution passes inherited `111/111` plus repair integrity `12/12`,
Outcome B. Both parent files remain byte-preserved.

No numerical search, parameter scan, floating tolerance, empirical
substitution, engine source, CMake file, `Voxel` type, production field,
constant, toggle, or default tick phase changed.

## 11. Next gate

The next mechanism should attack cadence and engagement before attraction:

1. preregister whether a globally periodic positive same-field law can be
   isochronous on a finite basin without introducing another selected
   function or amplitude feedback;
2. construct or close negative a canonical crossing latch that releases the
   FTD-0955 controller without resetting phase or erasing history;
3. certify the full nonlinear repeated map including equation (28);
4. only then, if attraction is required, export complete phase-error history
   through a finite positive canonical port; and
5. forbid target/profile/context/outcome/probability/Born/Bell/`G*` reads.

```text
RELATIVE_ACTION_CURVATURE=ADOPTED_SELECTED_REFERENCE
SYNCHRONIZATION_ENERGY=POSITIVE
TOTAL_AXIAL_CHARGE=CONSERVED
MATCHED_SYNCHRONIZATION_ENERGY=CONSERVED
RADIAL_RECIPROCAL_REACTION=EXACT
PHASE_LOCK=LYAPUNOV_STABLE
DISCRETE_FLOQUET_PAIR=ELLIPTIC_UNIT_MODULUS
ZERO_PHASE_CROSSINGS=RECURRENT_BELOW_SEPARATRIX
CROSSING_SECTION_COMPILER_ENERGY_CHARGE=EXACT
NONLINEAR_PENDULUM_CADENCE=AMPLITUDE_DEPENDENT
AUTONOMOUS_ENGAGEMENT=OPEN
FULL_NONLINEAR_COUPLED_STABILITY=OPEN
ATTRACTION=OPEN_POSITIVE_EXPORT_REQUIRED
NATIVE_RESERVOIR_PRODUCTION_GSTAR_BORN_BELL_LORENTZ=OPEN
```

