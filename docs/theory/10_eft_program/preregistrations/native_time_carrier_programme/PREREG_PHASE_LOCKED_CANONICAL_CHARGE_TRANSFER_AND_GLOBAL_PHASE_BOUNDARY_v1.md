# FTD-0954 pre-registration — Phase-locked canonical charge transfer and global-phase boundary v1

**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Scope:** proof-only continuation of FTD-0952/0953; no engine, CMake,
`Voxel`, production field, constant, toggle, default phase, Born, Bell, or
`G*` mutation.

## 1. Question

Can the FTD-0953 nonlinear Routh-port update be completed by a local,
phase-reacting canonical transfer that conserves physical axial charge and
endpoint physical energy without reading the completed body profile?

The test must distinguish three claims:

1. local canonical completion on a preregistered co-rotating phase section;
2. a globally periodic, phase-independent action debit; and
3. an autonomous native reservoir/formation mechanism.

Success on the first is not success on the other two.

## 2. Frozen sources

| source | SHA-256 | frozen role |
|---|---|---|
| `THEOREM_NONLINEAR_C18_ROUTH_PORT_RELAXATION_AND_CHARGE_RESERVOIR_BOUNDARY_v1.md` | `A207C274B176EE784B1E4846414B3C3DB5E4D20EF26948BD07153AAA1121CB05` | positive Routh port, physical `H/Q` separation, phase-blind obstruction |
| `proof_nonlinear_c18_routh_port_relaxation_charge_reservoir_boundary_v2.py` | `092EC6B94DD6E3498A96EBDF982FAC915288FF1BADCD0DE8766A7F1C865065C8` | repaired `87/87 + 9/9` certificate |
| `THEOREM_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_SOURCE_FRAME_BOUNDARY_v1.md` | `BD5B9DB5C9543F76241E6525B0CCD44787D16FE933D24E742C3982F9E6898981` | body-axis transverse complex plane and conserved axial charge |
| `THEOREM_COMMON_RELATIVE_CONNECTION_AND_MOMENTUM_GEARBOX_BOUNDARY_v1.md` | `3E2895157741C19DC8603E92E31A71933BFDAAF5B35062DFCE2F92404F8B9542` | existing common/relative canonical representation |
| `THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md` | `FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1` | complete-mode Hamiltonian exchange and phase-controller cost |
| `THEOREM_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md` | `A7DC30C90C491976F58CDEAF71FB5ABFCE04952ECE971CA7FF72C65A7B9B90BF` | phase-complete reservoir lower bound and prepared-pair transfer witness |

No source outside this table may be used to promote the result.

## 3. Polar canonical chart

At one active transverse site use physical radial amplitude `rho>0`, body
phase `theta`, radial momentum `p_rho`, and axial charge `L`. The full
transverse vectors are

\[
 q=\rho e_r(\theta),
 \qquad
 p=p_\rho e_r(\theta)+{L\over\rho}e_\theta(\theta).       \tag{1}
\]

The certificate must prove

\[
 p\cdot dq=p_\rho\,d\rho+L\,d\theta,
 \qquad q\wedge p=L.                                    \tag{2}
\]

Thus the body chart carries

\[
 \Omega_B=d\rho\wedge dp_\rho+d\theta\wedge dL.        \tag{3}
\]

On the registered circular branch,

\[
 p_\rho=0,
 \qquad L=\sigma\omega\rho^2,
 \qquad \sigma\in\{-1,+1\},\quad\omega>0.              \tag{4}
\]

Here `rho=A_0 phi`; no new amplitude scale is introduced.

## 4. Local target-blind charge increment

For one FTD-0953 color update let `rho_*` be the unique local minimizer
computed from the current inactive C18 neighbours, core flag, and already
selected action parameters. It may not read the completed uncontained
profile, measurement context, outcome, probability, or `G*`.

Freeze

\[
 D=\sigma\omega(\rho_*^2-\rho^2).                        \tag{5}
\]

On the ready circular section, `D` is exactly the body's axial-charge
change required to remain on the same frequency/orientation branch.

## 5. Co-rotating reservoir and common transfer pulse

Give the active site one complete positive reservoir pair `(vartheta,I)`:

\[
 \Omega_R=d\vartheta\wedge dI,
 \qquad E_R=\omega I,
 \qquad Q_R=\sigma I,
 \qquad I>0.                                             \tag{6}
\]

On a local lift of the two phase circles, freeze the common pulse

\[
 \boxed{H_{\rm tr}=(\sigma\vartheta-\theta)D.}           \tag{7}
\]

During the unit pulse, `rho`, `theta`, and `vartheta` are fixed, so `D` is
fixed. The exact flow to be tested is

\[
 \begin{aligned}
 \rho'&=\rho,\\
 p_\rho'&=p_\rho+(\theta-\sigma\vartheta)\partial_\rho D,\\
 \theta'&=\theta,\\
 L'&=L+D,\\
 \vartheta'&=\vartheta,\\
 I'&=I-\sigma D.
 \end{aligned}                                           \tag{8}
\]

The certificate must check the full Jacobian against

\[
 \Omega=d\rho\wedge dp_\rho+d\theta\wedge dL
       +d\vartheta\wedge dI,                             \tag{9}
\]

not only the phase-locked slice.

It must also check

\[
 L'+\sigma I'=L+\sigma I.                                \tag{10}
\]

The selected gate section is

\[
 \boxed{\theta=\sigma\vartheta}                          \tag{11}
\]

inside one declared phase chart. On (11), the radial reaction in (8)
vanishes exactly while the charge/action transfer remains nonzero. Away from
(11), the radial conjugate receives the reciprocal phase-error kick that the
FTD-0953 phase-blind debit omitted.

## 6. Composition with the nonlinear Routh port

Apply (8) immediately before the already-certified FTD-0953 positive
Routh-port quarter-turn for that site. On the ready circular/zero-port/
phase-locked section, equation (5) makes

\[
 L'=\sigma\omega\rho_*^2.                                \tag{12}
\]

The Routh identity and positive-port exchange are frozen as

\[
 \Delta H_{\rm rot}+\Delta E_{\rm port}
 =\sigma\omega D.                                        \tag{13}
\]

Equation (8) gives

\[
 \Delta E_R=\omega\Delta I=-\sigma\omega D.             \tag{14}
\]

Therefore the certificate must check the endpoint identity

\[
 \boxed{
 \Delta(H_{\rm rot}+E_{\rm port}+E_R)=0}                 \tag{15}
\]

together with exact total axial-charge conservation (10), symplecticity,
and the reverse composition. This is a stroboscopic section theorem. It is
not permission to claim one globally autonomous positive physical
Hamiltonian away from the gate section.

## 7. Capacity, locality, and backpressure

For a debit with `sigma D>0`, positivity requires

\[
 I>\sigma D.                                             \tag{16}
\]

The stronger local rule `I>|D|` is a sign-independent sufficient reserve.
If it fails, the gate must fail closed; clipping, saturation, reset, or
erasure is forbidden.

One reservoir pair is assigned per simultaneously active site. Same-color
updates have disjoint active coordinates, so their transfer Hamiltonians
must Poisson commute while inactive neighbours are held fixed. A finite
region and finite number of layers therefore require finite reserve and
finite pair count. Indefinite reuse, return routing, and replenishment remain
open.

## 8. Global phase-circle boundary

Equation (7) uses a local lift of the relative phase

\[
 \delta=\theta-\sigma\vartheta.                           \tag{17}
\]

For a globally single-valued `2pi`-periodic pulse of the frozen separable
form

\[
 H_{\rm per}=g(\delta)D,                                  \tag{18}
\]

a phase-independent nonzero shear would require `g'(delta)` to be one fixed
nonzero constant. Periodicity instead forces

\[
 \int_0^{2\pi}g'(\delta)d\delta=0.                       \tag{19}
\]

The certificate must therefore close negative only this class:

```text
GLOBAL_PERIODIC_PHASE_INDEPENDENT_SEPARABLE_SHEAR=IMPOSSIBLE_FOR_D_NONZERO
```

This does not exclude phase-window controllers, phase-dependent transfers,
multiple strokes, or complete-mode exchange.

## 9. Complete-mode global control

To demonstrate the scope of section 8, use two canonical action modes
`B=(x_b,y_b)` and `R=(x_R,y_R)` with

\[
 N={1\over2}(x_b^2+y_b^2+x_R^2+y_R^2),
 \qquad
 G=x_b y_R-x_R y_b.                                      \tag{20}
\]

For `kappa>0`, freeze

\[
 H_{\rm ex}=4\kappa N+\kappa G.                          \tag{21}
\]

The certificate must prove `|G|<=N`, strict positivity away from zero,
symplecticity, total-action conservation, and at

\[
 T={\pi\over2\kappa}                                     \tag{22}
\]

the exact complete-mode map

\[
 (B,R)\mapsto(-R,B)                                      \tag{23}
\]

up to the frozen sign convention.

Equation (23) is a global periodic-phase escape, but it is a prepared-mode
swap. To place a context-dependent nonlinear target `B_*` into the body, the
incoming reservoir must already contain `-B_*`; an empty fixed reservoir
cannot do so. It is therefore a compatibility control, not target-blind
nonlinear formation.

## 10. Frozen outcomes

| outcome | required result | interpretation |
|---|---|---|
| A | The phase-locked local shear, endpoint energy/charge ledger, global periodic realization, and native existing-field reservoir all close without prepared target data or a new law | physical charged-body formation reservoir closes conditionally |
| B | The phase-locked shear gives an exact local canonical and endpoint energy/charge completion; global phase-independent shear fails in the frozen periodic class; complete-mode exchange works only with a prepared mode | local stroboscopic reservoir closure; global autonomous/native formation remains open and sharply priced |
| C | The shear is not symplectic, does not conserve charge/endpoint energy, or cannot compose with the Routh port | reject the proposed completion |
| D | Hash, source, algebra, scope, or classifier gate fails | no theorem |

The frozen expected outcome is B.

## 11. Acceptance gates

The exact certificate must check:

1. every frozen hash and required source/scope marker;
2. the polar canonical one-form and axial-charge identity;
3. the circular branch and exact increment (5);
4. Hamilton equations and exact unit flow (8);
5. full six-dimensional symplecticity and determinant one;
6. exact inverse and total charge (10);
7. vanishing radial kick at (11) and nonzero reciprocal response away from it;
8. circular endpoint compatibility (12);
9. the exact physical endpoint energy ledger (13)--(15);
10. positive finite reserve and fail-closed backpressure;
11. same-color locality/commutation;
12. the scoped periodic derivative obstruction (19);
13. the positive complete-mode exchange control (20)--(23);
14. explicit detection of prepared-target leakage in that control;
15. non-promotion to a global autonomous physical Hamiltonian, native
    reservoir/source, finite 3D recycling, stability, production, `gamma`,
    `G*`, Born/Bell, Lorentz hiding, or completeness; and
16. the frozen outcome classifier.

No numerical search, parameter scan, floating tolerance, empirical
substitution, or completed-infinity limit is permitted.

## 12. Promotion boundary

Outcome B would retire the FTD-0953 phase-blind canonicality debt on one
declared co-rotating stroboscopic section. It would not derive the reservoir
or its synchronization.

Still open would be:

- a globally periodic autonomous multi-stroke implementation with its clock
  work and phase-reset ledger;
- native identification, formation, orientation, and replenishment of the
  reservoir mode;
- finite three-dimensional complete-pair routing, return, congestion, and
  recycling;
- phase-lock stability, detuning recovery, autonomous scheduling/stopping,
  and perturbation recovery;
- mobility, collision/backpressure, erasure, mass, scale, and production;
- the ordered two-frame/pseudoscalar source and `gamma`;
- quartic-`G*` synchronization, Born/Bell recovery, Lorentz hiding, and
  completeness.

