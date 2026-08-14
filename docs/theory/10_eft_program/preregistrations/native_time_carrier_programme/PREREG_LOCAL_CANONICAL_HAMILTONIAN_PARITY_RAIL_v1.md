# FTD-0875 — Local canonical Hamiltonian parity rail v1

**Identifier:** `FTD-0875`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Parents:** `FTD-0872`, `FTD-0873`, `FTD-0874`  
**Production status:** unchanged; isolated imposed continuous reference lift

## 1. Registered question

FTD-0874 supplies the exact alternating discrete rail

\[
 R(a,b)=(-b,a)
\]

but leaves intersite Hamiltonian formation and the physical energy current
open. Can every rail site be given the minimum local canonical phase-space
fiber so that one common reference clock generates the complete disjoint bond
layer, transfers the imposed record energy exactly, and closes the clock-
action ledger without replacing the separate quartic `G*` calendar?

The protocol must also distinguish a genuinely local canonical lift from a
boundary-global symplectic form on the undoubled scalar rail. It does not claim
that the common harmonic clock, amplitude/frequency scale, bond coupling,
axis choice, or production realization follows from P1--P5.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_ORIENTED_TERNARY_QUARTER_TURN_GEARBOX_v1.md` | `898A9130DFBAAE23B76D3FB5339851D026B50E5B7EFFB8B4B8DC66513F5A9317` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_HAMILTONIAN_TERNARY_QUARTER_TURN_ACTUATOR_v1.md` | `73214057949BC5BE115AF7E273DE2CECE1F87D63237E94ADADB83F64442C7B98` |
| `docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_v1.md` | `92C090ED43306249B963F757AD205F8C2B948944759A75CA46436606DDDC9BBB` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_AND_ONE_SHOT_BOUNDARY_v1.md` | `E70F2AD61BFA1C8BBFF4EA03DCF0312B8F96224ECF2453FDF4B81B0FEA845CA1` |
| `engine/include/ftd/eft/oriented_ternary_quarter_turn.h` | `46CD15943F5EB8EDBBCE4676CDE558A7C2B08556E1AC64E7C9720D30FFEB68E1` |
| `engine/include/ftd/eft/hamiltonian_ternary_quarter_turn_actuator.h` | `10BB9BFF5CC98E6CD72EC77F46E67766D458214E474296A7F3023AA27E2F8A94` |
| `engine/include/ftd/eft/alternating_oriented_ternary_parity_rail.h` | `E62026FA4228CFB8FB798EBF2E0C68011E6ABA6328050F80F9FD0573275604DD` |

Any mismatch invalidates the run. Exact symbolic algebra and exhaustive finite
ternary checks are permitted. Numerical near-miss search, parameter fitting,
post-run coefficient changes, and production promotion are forbidden.

## 3. Frozen scalar-rail locality boundary

For an even open scalar rail of length `L=2m`, define the signed
anti-diagonal matrix

\[
 J^{(L)}_{i,L-1-i}=(-1)^{m+i+1}\quad(0\le i<m),                \tag{1}
\]

with antisymmetry and all other entries zero. It obeys

\[
 (J^{(L)})^2=-I,
 \qquad U_0^T J^{(L)}U_0=J^{(L)},
 \qquad U_1^T J^{(L)}U_1=J^{(L)},                              \tag{2}
\]

where `U_0,U_1` are the FTD-0874 scalar layers. Thus the undoubled even rail
does admit a common symplectic structure. But (1) pairs site zero with site
`L-1` and generally pairs every site with its boundary mirror. It is
length-dependent and nonlocal.

The registered local class instead requires a direct sum of identical onsite
nondegenerate symplectic fibers. A real skew form on a one-dimensional fiber
is zero. Hence one scalar per site is impossible in this class, while one
canonical pair `(q_j,p_j)` per site is sufficient and minimum. This is a
minimum only in the declared onsite-direct-sum class.

## 4. Frozen canonical lift

Give every rail site the canonical bracket

\[
 \{q_j,p_k\}=\delta_{jk}.                                     \tag{3}
\]

For the tick-`n` disjoint matching `M_n`, define

\[
 N=\frac12\sum_j(q_j^2+p_j^2),                                \tag{4}
\]

\[
 L_n=\sum_{(j,k)\in\mathcal M_n}
 (q_jp_k-q_kp_j).                                              \tag{5}
\]

Let `(theta,I)` be one independent common reference-clock pair. With
orientation `sigma in {-1,+1}`, freeze one matching during one complete clock
cycle and register

\[
 H_{n,\sigma}
 =\Omega I+\Omega N
 +\sigma\kappa(1-\cos\theta)L_n,
 \qquad \kappa=\frac{\Omega}{4},\quad\Omega>0.                \tag{6}
\]

The actual ternary record is the gate-zero section

\[
 q_j=a x_j,qquad p_j=0,qquad a>0.                            \tag{7}
\]

The continuous subcycle is a selected EFT scaffold inside one ontic global
tick; it is not asserted as microscopic sub-tick ontology.

## 5. Frozen exact solution

The disjoint bond generators commute and

\[
 \{N,L_n\}=0.                                                   \tag{8}
\]

The onsite part of (6) makes one identity winding in

\[
 T=\frac{2\pi}{\Omega},                                        \tag{9}
\]

while the bond rotation angle is

\[
 \beta_{\sigma}(T)
 =\sigma\int_0^T\kappa(1-\cos\Omega t)dt
 =\sigma\frac{\pi}{2}.                                       \tag{10}
\]

Therefore every active bond obeys

\[
 \begin{aligned}
 \sigma=+1:&\quad(q_j,q_k)\mapsto(-q_k,q_j),
 & (p_j,p_k)\mapsto(-p_k,p_j),\\
 \sigma=-1:&\quad(q_j,q_k)\mapsto(q_k,-q_j),
 & (p_j,p_k)\mapsto(p_k,-p_j).
 \end{aligned}                                                  \tag{11}
\]

On (7), the endpoint again has `p=0`, and the `q/a` labels are exactly the
FTD-0874 forward or inverse ternary layer.

## 6. Frozen positivity, clock, and current ledger

For every disjoint matching,

\[
 |L_n|\le N.                                                    \tag{12}
\]

Since `0<=1-cos(theta)<=2`, the carrier plus interaction part satisfies

\[
 \Omega N+\sigma\frac{\Omega}{4}(1-\cos\theta)L_n
 \ge\frac{\Omega}{2}N\ge0.                                   \tag{13}
\]

Both `N` and `L_n` are constants of motion. The clock action is

\[
 I(\theta)=I_0
 -\sigma\frac{\kappa}{\Omega}(1-\cos\theta)L_n.               \tag{14}
\]

Hence

\[
 |\Delta I|_{\max}=\frac{|L_n|}{2},
 \qquad
 |\Delta E_{\rm ref}|_{\max}
 =|E_{\rm int}|_{\max}
 =\frac{\Omega|L_n|}{2},                                      \tag{15}
\]

and the endpoint energy residual is exactly zero. A sufficient positive-
action reserve is `I_0>|L_n|/2`.

Define the onsite carrier energy

\[
 E_j=\frac{\Omega}{2}(q_j^2+p_j^2).                            \tag{16}
\]

On an active bond `(j,k)`, with

\[
 c(t)=\sigma\kappa(1-\cos\theta(t)),
\]

the exact current is

\[
 \dot E_j=-\mathcal J_{j\to k},
 \qquad
 \dot E_k=+\mathcal J_{j\to k},
 \qquad
 \mathcal J_{j\to k}
 =\Omega c(t)(q_jq_k+p_jp_k).                                 \tag{17}
\]

For a ready record `(q_j,p_j)=(as,0)` and an empty neighbour, (11) transfers
the imposed energy

\[
 \epsilon_{\rm rec}=\frac{\Omega a^2}{2}                      \tag{18}
\]

completely from `j` to `k`.

On the actual section (7), `L_n=0` initially and remains zero. Consequently
the interaction value and clock-action exchange vanish along that special
orbit even though the Hamiltonian vector field is nonzero and transports the
record. This is an exact zero-backreaction reference submanifold, not a proof
that physical switching hardware is cost-free. Generic phase-space states
have the nonzero ledger (15).

## 7. Registered certificate gates

The frozen certificate must report exactly fifty-six checks.

### Provenance

- **C1--C7:** the seven source hashes match section 2.
- **C8:** the protocol hash matches the pre-run lock embedded in the
  certificate before first execution.

### Scalar locality boundary

- **C9:** the scalar bond map is the exact FTD-0874 quarter-turn.
- **C10:** (1) is antisymmetric on every registered even length.
- **C11:** `(J^(L))^2=-I`, so (1) is nondegenerate.
- **C12:** both parity layers preserve (1).
- **C13:** (1) pairs the two endpoints for every registered `L>=4`.
- **C14:** every real one-dimensional skew onsite block is zero.
- **C15:** one scalar per site cannot supply a nondegenerate onsite-direct-sum
  symplectic form.
- **C16:** a two-dimensional canonical onsite fiber is nondegenerate.
- **C17:** two real coordinates per site are minimum in the registered local
  canonical class.

### Hamiltonian lift

- **C18:** (4) is positive definite.
- **C19:** (5) is the sum of oriented bond determinants.
- **C20:** `{N,L_n}=0` exactly.
- **C21:** disjoint bond generators Poisson commute.
- **C22:** the `q` Hamilton equations carry the registered spatial generator.
- **C23:** the `p` Hamilton equations carry the same spatial generator.
- **C24:** `N` is conserved.
- **C25:** `L_n` is conserved.
- **C26:** `theta=Omega t` is exact.
- **C27:** (14) solves the clock-action equation.
- **C28:** substitution of (14) makes total `H` phase independent.
- **C29:** inequality (12) holds.
- **C30:** inequality (13) follows at maximum coupling.
- **C31:** the onsite oscillator completes one identity winding.
- **C32:** the forward pulse angle is `+pi/2`.
- **C33:** the reverse pulse angle is `-pi/2`.
- **C34:** commuting onsite and spatial flows factor exactly.
- **C35:** the forward endpoint applies `R` to both `q` and `p` bond pairs.
- **C36:** the reverse endpoint applies `R^-1` to both pairs.
- **C37:** the actual section `p=0` returns to `p=0`.
- **C38:** the endpoint `q/a` labels match every finite FTD-0874 layer state
  in the registered exhaustive domain.

### Energy, current, and clock ledger

- **C39:** onsite energies (16) are nonnegative.
- **C40:** the base onsite winding changes no onsite energy.
- **C41:** (17) is the exact upstream energy derivative.
- **C42:** the downstream derivative is its negative.
- **C43:** each active bond conserves total carrier energy.
- **C44:** the complete disjoint layer conserves total carrier energy.
- **C45:** a ready ternary record carries the imposed energy (18).
- **C46:** the ready forward endpoint empties the upstream energy.
- **C47:** the ready forward endpoint receives all record energy downstream.
- **C48:** the integrated current equals the endpoint energy transfer.
- **C49:** the maximum clock-action excursion is `|L_n|/2`.
- **C50:** the maximum reference-energy exchange is `Omega |L_n|/2`.
- **C51:** interaction energy carries the opposite transient amount.
- **C52:** clock action and interaction return exactly at the endpoint.
- **C53:** the endpoint total-energy residual is zero.
- **C54:** the actual section has `L_n=0` but nontrivial prepared transport.
- **C55:** all frozen scope markers below are present.
- **C56:** the terminal verdict is emitted only if C1--C55 all pass.

## 8. Frozen interpretation

If all fifty-six checks pass, the permitted result is:

- **[THEOREM]** the FTD-0874 parity layer has an exact positive local
  Hamiltonian lift on one canonical pair per rail site plus one common
  reference-clock pair;
- **[THEOREM, CONDITIONAL MINIMUM]** two real carrier coordinates per site are
  minimum in the registered onsite-direct-sum symplectic class;
- **[THEOREM]** the imposed record energy obeys an exact local antisymmetric
  bond-current and complete-transfer ledger;
- **[THEOREM]** the undoubled even scalar rail admits a boundary-global mirror
  symplectic form, so the obstruction is locality rather than abstract
  symplectic existence;
- **[IMPOSED]** the harmonic common clock, amplitude `a`, frequency `Omega`,
  bond generator, matching freeze, and one-cycle-per-global-tick
  correspondence remain reference inputs;
- **[OPEN]** native doublet formation, local clock synchronization,
  axis/routing choice, nonlinear backpressure/collisions, finite boundaries,
  production coupling, robustness, and synchronization to the separate
  quartic `G*` calendar.

The result refines the existing `SEL-CA-PHASE-RAIL`; it adds no selected type.

## 9. Frozen outcome rule

- **Outcome A:** `56/56`; book the scoped theorem and isolated `ftd::eft`
  witness.
- **Outcome B:** provenance passes but any mathematical gate fails; book the
  counterexample and no theorem.
- **Execution invalid:** a hash mismatch, exception, wrong check count, or
  scope-marker failure; preserve the run and preregister any repair.

## 10. Scope markers

```text
CANONICAL_SITE_DOUBLET=IMPOSED_REFERENCE_MINIMUM_IN_REGISTERED_CLASS
SCALAR_COMMON_SYMPLECTIC_FORM=BOUNDARY_GLOBAL_NOT_LOCAL
COMMON_HARMONIC_CLOCK=SELECTED_REFERENCE
ACTUAL_SECTION_CLOCK_BACKREACTION=ZERO_SPECIAL_ORBIT_NOT_COST_FREE_HARDWARE
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR_NOT_INTERSITE_ACTUATOR
BORN_BELL_STATUS=UNTOUCHED
```

## 11. Pre-run lock

The exact SHA-256 of this byte-frozen protocol must be embedded in the
certificate and recorded in the preregistration manifest before first
execution. Later outcome prose must not alter this evidence hash.
