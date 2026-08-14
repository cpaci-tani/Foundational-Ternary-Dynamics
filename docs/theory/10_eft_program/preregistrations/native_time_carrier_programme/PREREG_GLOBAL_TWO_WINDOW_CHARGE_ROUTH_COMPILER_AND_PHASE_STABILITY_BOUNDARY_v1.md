# FTD-0955 pre-registration — Global two-window charge/Routh compiler and phase-stability boundary v1

**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Scope:** proof-only continuation of FTD-0954. No engine, CMake, `Voxel`,
production field, constant, toggle, default tick phase, numerical search,
Born/Bell path, or `G*` law may change.

## 1. Question

Can one globally single-valued periodic controller compile the FTD-0954
phase-reacting charge shear and the FTD-0953 positive nonlinear Routh-port
quarter-turn in the correct order, return its controller action exactly, and
make the co-rotating phase lock dynamically restoring?

The three verdicts are separate:

1. global autonomous scheduling;
2. endpoint physical energy/charge/inverse closure on the ready section; and
3. off-lock synchronization recovery.

Scheduling success may not be counted as stability or native formation.

## 2. Frozen sources

| source | SHA-256 | role |
|---|---|---|
| `THEOREM_PHASE_LOCKED_CANONICAL_CHARGE_TRANSFER_AND_GLOBAL_PHASE_BOUNDARY_v1.md` | `203DA15FE63BC67496298C03D96A85F819142C485B18B6FC890B14E6A989BAA5` | local shear, energy/charge ledger, periodic boundary |
| `proof_phase_locked_canonical_charge_transfer_global_phase_boundary.py` | `C08625778490F7559311CB8A24A6E04BA150D8009302DC6BD5F8405507FE0257` | `83/83` proof of record |
| `THEOREM_AUTONOMOUS_PHASE_PARITY_AND_SOURCE_REACTION_BOUNDARY_v1.md` | `0FEEF83C38BE9A4929644A229EAEA1B22424A54161BE8E2F3F8B882194DFDF39` | periodic disjoint-window controller pattern |
| `proof_autonomous_phase_parity_source_reaction_splitter_v2.py` | `4C19F1A8197ED7C2198B59E56F288A707C3BC784CA4DE586B99A601C762AFC17` | repaired window-controller certificate |
| `THEOREM_NONLINEAR_C18_ROUTH_PORT_RELAXATION_AND_CHARGE_RESERVOIR_BOUNDARY_v1.md` | `A207C274B176EE784B1E4846414B3C3DB5E4D20EF26948BD07153AAA1121CB05` | positive nonlinear Routh chart/quarter-turn |
| `proof_nonlinear_c18_routh_port_relaxation_charge_reservoir_boundary_v2.py` | `092EC6B94DD6E3498A96EBDF982FAC915288FF1BADCD0DE8766A7F1C865065C8` | repaired Routh-port certificate |

## 3. Controller circle and disjoint windows

Introduce one controller pair `(varphi,J)` with `J>0`, `Omega>0`, and

\[
 \Omega_C=d\varphi\wedge dJ.                             \tag{1}
\]

On `varphi in [0,2pi)` define the periodic `C^1` windows

\[
 w_0(\varphi)=
 \begin{cases}
 \sin^2\varphi,&0\le\varphi\le\pi,\\
 0,&\pi<\varphi<2\pi,
 \end{cases}                                             \tag{2}
\]

\[
 w_1(\varphi)=
 \begin{cases}
 0,&0\le\varphi<\pi,\\
 \sin^2\varphi,&\pi\le\varphi\le2\pi.
 \end{cases}                                             \tag{3}
\]

Both windows and their first derivatives vanish at every join, their
interiors are disjoint, and

\[
 \int_0^\pi w_0d\varphi
 =\int_\pi^{2\pi}w_1d\varphi={\pi\over2}.                \tag{4}
\]

## 4. Frozen generators and autonomous compiler

Retain

\[
 \delta=\theta-\sigma\vartheta,
 \qquad
 D=\sigma\omega(\rho_*^2-\rho^2),                       \tag{5}
\]

where `rho_*` is the FTD-0953 target-blind local minimizer. Define

\[
 G_Q=-\sin\delta\,D                                      \tag{6}
\]

and, in the FTD-0953 nonlinear Routh chart,

\[
 G_R=a\pi_u-u\pi_a.                                     \tag{7}
\]

The single autonomous periodic controller is frozen as

\[
 \boxed{
 H_C=\Omega J+{2\Omega\over\pi}w_0(\varphi)G_Q
              +\Omega w_1(\varphi)G_R.}                 \tag{8}
\]

Because `H_C` is independent of `J` outside `Omega J`,

\[
 \dot\varphi=\Omega.                                    \tag{9}
\]

During the first window `G_Q` is constant along its own flow; during the
second, `G_R` is constant along its own flow. Equation (4) must therefore
produce exactly:

\[
 \Phi_{G_Q}^{1}quad\text{then}\quad
 \Phi_{G_R}^{\pi/2}.                                    \tag{10}
\]

The second map must equal

\[
 (u,a,\pi_u,\pi_a)\mapsto(a,-u,\pi_a,-\pi_u).           \tag{11}
\]

## 5. Controller action and reserve

At every window boundary the interaction vanishes. If the entering action is
`J_0`, conservation of (8) gives

\[
 J=J_0-{2\over\pi}w_0G_Q                                \tag{12}
\]

during the first window and

\[
 J=J_0-w_1G_R                                           \tag{13}
\]

during the second. Hence `J` returns exactly at `varphi=pi` and `2pi`.

Using

\[
 |G_Q|\le|D|,
 \qquad
 |G_R|\le N_R:={1\over2}(u^2+a^2+\pi_u^2+\pi_a^2),     \tag{14}
\]

freeze the sufficient reserve

\[
 \boxed{J_0>\max\left({2|D|\over\pi},N_R\right).}       \tag{15}
\]

If (15) fails, the cycle must fail closed before the first window. No
clipping, saturation, reset, or erasure is allowed.

## 6. Ready-section endpoint

On the registered ready section

\[
 \delta=0,
 \quad p_\rho=0,
 \quad L=\sigma\omega\rho^2,
 \quad a=\pi_a=\pi_u=0,                                 \tag{16}
\]

the first window must give

\[
 L'=L+D,
 \qquad I'=I-\sigma D,
 \qquad p_\rho'=0,                                      \tag{17}
\]

and the second must move `rho` to `rho_*` while storing the complete Routh
mismatch in the outgoing port. The full cycle must conserve

\[
 L+\sigma I                                             \tag{18}
\]

and the endpoint physical account

\[
 \boxed{
 \Delta(H_{\rm rot}+E_{\rm port}+\omega I)=0.}           \tag{19}
\]

The controller action must return, so no controller-energy term may be used
to hide a failure of (19). Backward autonomous flow must apply the inverse
Routh quarter-turn before the inverse charge shear and recover every phase,
action, radial variable, and port variable.

## 7. Off-lock map

For general `delta`, the first window must give

\[
 \Delta L=D\cos\delta,
 \qquad
 \Delta I=-\sigma D\cos\delta,
 \qquad
 \Delta p_\rho=(\sin\delta)\partial_\rho D.             \tag{20}
\]

Both generators are independent of `L` and `I`, and the Routh generator is
independent of `theta` and `vartheta`. Therefore the certificate must test

\[
 \boxed{\delta_{n+1}=\delta_n}                           \tag{21}
\]

for the complete controller cycle. The phase-lock Floquet multiplier is
exactly `+1`. Equation (20) reacts to phase error but equation (21) does not
restore it.

## 8. Minimum Hamiltonian stability boundary

Use the canonical change

\[
 \delta=\theta-\sigma\vartheta,
 \qquad \chi=\vartheta,
 \qquad P_\delta=L,
 \qquad P_\chi=\sigma L+I.                              \tag{22}
\]

The one-form must satisfy

\[
 Ld\theta+Id\vartheta
 =P_\delta d\delta+P_\chi d\chi.                       \tag{23}
\]

The matched linear body/reservoir energy is

\[
 H_0=\sigma\omega L+\omega I=\omega P_\chi.            \tag{24}
\]

It has no `P_delta` curvature. For every periodic phase potential `V(delta)`,

\[
 H=H_0+V(\delta)
 \quad\Longrightarrow\quad
 \dot\delta={\partial H\over\partial P_\delta}=0.      \tag{25}
\]

Thus a phase potential alone cannot restore lock in the matched linear-action
reservoir. The minimum conservative restoring extension must contain nonzero
relative-action curvature. The reference witness is

\[
 H_{\rm sync}={\Pi^2\over2M_\delta}
              +K_\delta(1-\cos\delta),
 \qquad M_\delta,K_\delta>0,                            \tag{26}
\]

with small-perturbation equation

\[
 \ddot\delta+{K_\delta\over M_\delta}\delta=0.          \tag{27}
\]

Equation (26) is a selected future energy law with two unnormalized positive
scales. It is not part of the FTD-0955 compiler and cannot be used to promote
the outcome. It gives elliptic bounded restoration, not asymptotic attraction.
Hamiltonian volume preservation forbids an attracting fixed point without an
open/export channel or coarse-graining.

## 9. Frozen outcomes

| outcome | required result | interpretation |
|---|---|---|
| A | The periodic compiler closes exact scheduling/ledger/inverse and the frozen matched-energy dynamics restores off-lock phase errors without a new energy law or open channel | global autonomous and self-synchronizing reference closure |
| B | The periodic compiler closes exact scheduling/ledger/inverse, but `delta` has unit multiplier; relative-action curvature is required for conservative restoration and an open channel for attraction | global controller closure / synchronization boundary |
| C | The windows fail to reproduce the ordered shear and Routh map, controller action fails to return, or endpoint energy/charge/inverse fails | reject compiler |
| D | Hash, algebra, source, scope, or classifier fails | no theorem |

The frozen expected classifier is Outcome B.

## 10. Acceptance gates

The exact certificate must check:

1. every frozen hash and scope marker;
2. periodic `C^1` joins, disjoint interiors, and exact window integrals;
3. `varphi_dot=Omega` and exact ordered pulse areas;
4. constancy of each active generator under its own window flow;
5. exact charge-shear and Routh-port endpoint maps;
6. full symplecticity, determinant one, and inverse order;
7. exact controller-action excursion and boundary return;
8. inequalities (14) and sufficient positive reserve (15);
9. fail-closed backpressure;
10. ready-section circular compatibility, total charge, and endpoint physical
    energy (19);
11. off-lock equations (20) and exact neutral phase map (21);
12. canonical transformation (22)--(23);
13. zero relative-action curvature of (24);
14. phase-potential-only obstruction (25);
15. positive restoring witness and linearization (26)--(27);
16. no asymptotic-attraction claim for a closed Hamiltonian map;
17. no target/profile/context/outcome/probability/Born/Bell/`G*` read;
18. non-promotion to native reservoir/source, 3D recycling, production, mass,
    scale, `gamma`, Lorentz hiding, or completeness; and
19. the frozen outcome classifier.

No numerical parameter search, floating tolerance, empirical substitution,
or completed-infinity limit is permitted.

## 11. Promotion boundary

Outcome B would close global autonomous scheduling and controller-work return
for one charge-plus-Routh cycle. It would prove that phase reaction and phase
synchronization are different mechanisms.

Still open would be:

- adoption or derivation of a relative-action curvature and its physical
  scales;
- an exact energy/charge ledger after adding that synchronization energy;
- attraction/recovery through explicit exported history rather than free
  damping;
- eight-color multi-site compilation, finite 3D controller/port routing,
  congestion, return, and recycling;
- native reservoir/source identity, formation, replenishment, and erasure;
- stability of the complete nonlinear body, mobility, collision, mass,
  scale, `gamma`, production, `G*`, Born/Bell, Lorentz hiding, and
  completeness.

