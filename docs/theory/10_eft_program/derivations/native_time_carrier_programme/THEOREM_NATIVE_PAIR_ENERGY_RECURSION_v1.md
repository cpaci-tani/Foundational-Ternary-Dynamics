# Theorem — Native Pair-Energy Recursion and Cadence Boundary v1

**Identifier:** `FTD-0840`  
**Date:** 2026-08-10  
**Status:** `[THEOREM — EXACT CONDITIONAL DISCRETE RECURSION]` +
`[THEOREM — SELF-DUAL PAIR ENERGY AND ORIENTATION]` +
`[THEOREM — CONTINUUM G* SHAPE FACTOR]` +
`[BOUNDARY — NOT AN EXACT FINITE-TICK G* CADENCE]` +
`[SELECTION/OPEN — NATIVE LOCAL PAIR COUPLING AND MAINTENANCE]`  
**Certificate:**
[`proof_native_pair_energy_recursion.py`](../../../../../scripts/proofs/proof_native_pair_energy_recursion.py)
(`24/24` exact checks)  
**Pre-registration:**
[`PREREG_NATIVE_PAIR_ENERGY_RECURSION_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_NATIVE_PAIR_ENERGY_RECURSION_v1.md)

## 1. Result

There is a simple exact answer to the request for a natural self-dual energy
or stable recursive system, but it is a **candidate extension**, not a hidden
production result.

Retain an oriented real canonical lift `(q,p)`, define the signed self-pair

\[
u=q|q|,
\]

and assign an ordinary positive quadratic energy to that pair. With

\[
y=\frac{p}{\sqrt{2m\lambda}},
\]

the Hamiltonian becomes

\[
\boxed{
H(q,p)=\frac{p^2}{2m}+\lambda q^4
=\lambda(u^2+y^2).}                            \tag{1}
\]

This is the minimal algebraic bridge between the two layers isolated by
FTD-0839:

- the unsquared lift `(q,p)` retains orientation and sheet information;
- the signed square `u=q|q|` supplies the quartic coordinate;
- the energy is quadratic and exchange-symmetric in `(u,y)`; and
- the nonuniform rotation of `(u,y)` supplies the lemniscatic continuum
  period factor.

An exact symmetric discrete-gradient rule then gives a globally unique,
deterministic, time-reversible, energy-conserving, oriented, bounded recursion
for every `m>0`, `lambda>0`, and `h>0`.

The result closes the **conditional mathematical recursion**. It does not
close the native physical gearbox. The current production energy contains no
pair-energy term; its target-blind `(q,p)` chart is a free-field modal chart,
not yet a localized maintained clock. The finite-step recursion is also not
the exact quartic Hamiltonian flow, so its integer ticks do not automatically
inherit the continuum `G*` period.

## 2. Why the signed square is the correct minimum

The unsigned square `q^2` supplies quarticity but forgets the sign of `q`.
The complex square in FTD-0839 similarly sends both quarter orientations to
one half-twist. The smallest repair is not a second independent square. It is
to retain the unsquared lift and use the signed self-pair

\[
u=q|q|.
\]

Then

\[
u^2=q^4,                                        \tag{2}
\]

while `sign(q)` remains available from the lift. This is precisely the
information architecture required by the earlier `(U,chi)` result: an energy
quotient plus a sheet/orientation witness. Here the witness is not an extra
bit added after the fact; it is the retained coordinate `q` from which `u` is
derived.

At a primitive ternary record site, `s|s|=s`, so this map creates no new
degree. Its nontrivial use is on a continuous flux/modal coordinate. A future
claim that (2) is a local constituent-pair observable still requires an
explicit context and closure law.

## 3. Self-dual pair flow

Hamilton's equations for (1) are

\[
\dot q=\frac pm,
\qquad
\dot p=-4\lambda q^3.                           \tag{3}
\]

Let `r=|q|`. Away from the sheet crossing `q=0`, (3) implies

\[
\frac d{dt}\begin{pmatrix}u\\y\end{pmatrix}
=2r\sqrt{\frac{2\lambda}{m}}
J\begin{pmatrix}u\\y\end{pmatrix},
\qquad
J=\begin{pmatrix}0&1\\-1&0\end{pmatrix}.      \tag{4}
\]

Therefore

\[
\frac d{dt}(u^2+y^2)=0.                         \tag{5}
\]

The oriented swept-area current is

\[
u\dot y-y\dot u
=-2r\sqrt{\frac{2\lambda}{m}}(u^2+y^2)<0       \tag{6}
\]

for every nonzero point with `q != 0`. The rate reaches zero at `q=0`, but the
underlying `(q,p)` orbit crosses the sheet continuously. The square alone
would make this crossing ambiguous; the retained lift resolves it.

Equation (4) gives a precise left/right interpretation without importing a
biological claim: two complementary registers rotate into one another, while
their quadratic radius is the conserved energy. The asymmetry is in the
oriented symplectic form, not in unequal energy assigned to the two sides.

## 4. Exact continuum `G*` law

At amplitude `A>0`, the energy is `E=lambda A^4`. On one quarter cycle,

\[
\frac T4
=\sqrt{\frac{m}{2\lambda}}\frac1A
\int_0^1\frac{dx}{\sqrt{1-x^4}}.               \tag{7}
\]

With `t=x^4`,

\[
\int_0^1\frac{dx}{\sqrt{1-x^4}}
=\frac14B\!\left(\frac14,\frac12\right)
=\frac{\sqrt\pi}{4}
\frac{\Gamma(1/4)}{\Gamma(3/4)}.               \tag{8}
\]

Hence

\[
\boxed{
TA=\sqrt\pi G^*\sqrt{\frac{m}{2\lambda}}.}     \tag{9}
\]

No occurrence of `G*` was placed in (1), (3), or (4). The factor follows from
the traversal integral of the quartic shape. The constants `m` and `lambda`
set the dimensional clock scale; `G*` is the dimensionless waveform factor.

This is a theorem of the adopted Hamiltonian. It is not a derivation of that
Hamiltonian from P1--P5.

## 5. Exact discrete recursion

For step `h>0`, define the next state by

\[
\frac{q_1-q_0}{h}=\frac{p_1+p_0}{2m},          \tag{10}
\]

\[
\frac{p_1-p_0}{h}
=-\lambda(q_1^3+q_1^2q_0+q_1q_0^2+q_0^3).     \tag{11}
\]

This is a discrete-gradient rule: the polynomial in (11) is the exact divided
difference

\[
\frac{q_1^4-q_0^4}{q_1-q_0}.                  \tag{12}
\]

It remains well defined on the diagonal by its polynomial form.

### 5.1 Determinism

Eliminating `p_1` produces

\[
f(q_1)=\frac{2m}{h}(q_1-q_0)-2p_0
+h\lambda(q_1^3+q_1^2q_0+q_1q_0^2+q_0^3)=0.  \tag{13}
\]

Its derivative is

\[
f'(q_1)=\frac{2m}{h}
+h\lambda\left[2q_1^2+(q_1+q_0)^2\right]>0.   \tag{14}
\]

Since the cubic has positive leading coefficient and opposite limits at
`+/- infinity`, (13) has exactly one real root. The implicit definition does
not introduce a branch-selection variable.

The two-equation Jacobian also has

\[
\det D_{(q_1,p_1)}F
=1+\frac{h^2\lambda}{2m}
\left[2q_1^2+(q_1+q_0)^2\right]>0.             \tag{15}
\]

### 5.2 Exact energy closure

Using (10)--(12),

\[
\begin{aligned}
H_1-H_0
&=\frac{(p_1-p_0)(p_1+p_0)}{2m}
  +\lambda(q_1-q_0)
  (q_1^3+q_1^2q_0+q_1q_0^2+q_0^3)\\
&=0.                                            \tag{16}
\end{aligned}
\]

No diagnostic bath is needed for this isolated recursion. A bath becomes
necessary only if damping is added or if the framework demands attraction
and recovery to one selected positive-energy shell.

### 5.3 Reversibility

Exchanging endpoints and sending `h -> -h` changes both residual equations by
an overall minus sign. Thus

\[
\Phi_{-h}=\Phi_h^{-1}.                          \tag{17}
\]

Because the Hamiltonian is even in `p`, the physical reversal
`R(q,p)=(q,-p)` obeys

\[
R\Phi_hR=\Phi_h^{-1}.                           \tag{18}
\]

### 5.4 Exact discrete orientation

Let

\[
\bar q=\frac{q_1+q_0}{2},
\qquad
\bar p=\frac{p_1+p_0}{2}.
\]

The discrete swept-area witness is

\[
\begin{aligned}
\chi_h
&=\bar q(p_1-p_0)-\bar p(q_1-q_0)\\
&=-h\left[
\frac\lambda2(q_1+q_0)^2(q_1^2+q_0^2)
+\frac{(p_1+p_0)^2}{4m}
\right].                                       \tag{19}
\end{aligned}
\]

It is nonpositive. Equality would require `q_1=-q_0` and `p_1=-p_0`.
Substitution into (10)--(11) then forces `q_0=p_0=0`. Therefore

\[
\boxed{\chi_h<0}
\]

on every nonzero step. The recursion itself distinguishes clockwise from
counterclockwise; this information is not reconstructed from the square.

### 5.5 Stability

The origin is the only fixed point. For every initial energy `E>0`, (16)
implies

\[
|q_n|\leq(E/\lambda)^{1/4},
\qquad
|p_n|\leq\sqrt{2mE}                            \tag{20}
\]

for all integer `n`. Every orbit stays on a compact energy shell. This proves
bounded recurrence, Lyapunov stability of the origin, and invariance/stability
of energy sublevel sets. It does **not** prove pointwise Lyapunov stability of
every positive-energy phase point; phase shear on an invariant shell remains
possible. It is not asymptotic stability or critical feedback maintenance.

## 6. Why the finite-tick cadence remains open

The discrete rule has the correct continuum generator. But it is not the
exact Hamiltonian time-`h` map. From a turning point `(A,0)`, it gives

\[
q_1=A-\frac{2\lambda A^3}{m}h^2
+\frac{6\lambda^2A^5}{m^2}h^4+O(h^6),          \tag{21}
\]

\[
p_1=-4\lambda A^3h
+\frac{12\lambda^2A^5}{m}h^3+O(h^5).           \tag{22}
\]

The exact flow has `2`, not `6`, as the `q` coefficient at order `h^4`, and
`8`, not `12`, as the `p` coefficient at order `h^3`. Thus the finite-step
orbit has its own discrete rotation number.

The correct conclusion is

\[
\boxed{
\text{exact stable recursion}
+\text{ exact continuum }G^*\text{ shape}
\not\Rightarrow
\text{ exact integer-tick }G^*\text{ cadence}.}
\]

An exact-flow update would build the elliptic solution into each step and
would not count as a substrate derivation. A native cadence must instead be
shown from a source-blind local rule, with convergence and operational gate
tests preregistered.

## 7. Production boundary and revised minimum

The frozen production audit establishes:

1. [`native_modal_phase_action.h`](../../../../../engine/include/ftd/eft/native_modal_phase_action.h)
   contains a target-blind canonical `(q,p)` chart for one source-free
   eigenmode;
2. [`energy_ledger_compute.cpp`](../../../../../engine/src/energy_ledger_compute.cpp)
   accounts the registered quadratic field/wave energy and contains no
   pair-energy channel; and
3. the smooth frozen `phase_read` core has no quartic restorer, as already
   proved by FTD-0838.

The revised interface debt is therefore:

| Interface | Exact progress | Remaining physical debt |
|---|---|---|
| `OrientedPhasePair` | source-free modal `(q,p)` exists; recursion fixes orientation | local, interacting, persistent realization |
| `PairClosureMap` | `u=q|q|` is an exact signed self-pair | constituent/context interpretation and native coupling |
| `PairEnergyCoupling` | `lambda u^2` gives exact quartic/self-dual energy | absent from production; `lambda` and support must be selected or derived |
| `EnergyBath` | unnecessary for isolated conservative stability | required for damping, work, recovery, or selected-shell maintenance |
| `CadenceMap` | continuum traversal gives `sqrt(pi)G*` exactly | finite global-tick rotation/gate law remains open |

This is genuinely more than matter in the narrow mathematical sense: the
recursive core is an oriented potentiality/phase pair with a conserved
relational energy. It does not justify identifying that pair with brain
hemispheres, consciousness, or an actualization selector.

## 8. Bell/Born separation

FTD-0840 is deterministic and contains no probability measure, measurement
context, effect, selector, or target frequency. It can serve as candidate
clock hardware for a later actualization model, but it cannot derive Born
weights. In particular, using the oscillator phase as a quantile seed would
be a reference selector only unless the equilibrium measure and its physical
pushforward were independently recovered from substrate histories.

Keeping this separation is essential: `G*` may control eligibility cadence;
it must not be allowed to read or encode the outcome probabilities it gates.

## 9. Epistemic disposition

| Claim | Status |
|---|---|
| `u=q|q|` gives `u^2=q^4` | `[THEOREM]` |
| `H=lambda(u^2+y^2)` | `[THEOREM — CONDITIONAL ON ADOPTED PAIR ENERGY]` |
| the weighted pair flow is oriented and radius-preserving | `[THEOREM]` |
| the continuum period contains `sqrt(pi)G*` | `[THEOREM — CONDITIONAL HAMILTONIAN]` |
| the registered discrete next state exists uniquely | `[THEOREM]` |
| the discrete rule is exactly energy-conserving and reversible | `[THEOREM]` |
| every nonzero discrete step has one orientation | `[THEOREM]` |
| positive-energy discrete orbits are bounded | `[THEOREM]` |
| the production substrate already contains `lambda u^2` | `[REJECTED BY SOURCE AUDIT]` |
| the discrete rule is the exact finite-time quartic flow | `[REJECTED BY SERIES CONTROL]` |
| `G*` is now an exact integer gate cadence | `[OPEN]` |
| the recursion is a Born mechanism | `[NOT ESTABLISHED / OUT OF SCOPE]` |

## 10. Certificate outcome

The first locked execution returned:

```text
FTD-0840 native pair-energy recursion: 24/24 PASS
SIGNED_SELF_PAIR_GIVES_QUADRATIC_SELF_DUAL_ENERGY
DISCRETE_RECURSION_UNIQUE_REVERSIBLE_ENERGY_CLOSED_AND_ORIENTED
CONTINUUM_GSTAR_SHAPE_FACTOR_EXACT
PRODUCTION_PAIR_COUPLING_AND_FINITE_TICK_GSTAR_CADENCE_OPEN
```

Registered Outcome B passes. No production code was changed.

## 11. Isolated reference implementation

The theorem was subsequently implemented without changing its scope in:

- [`native_pair_energy_recursion.h`](../../../../../engine/include/ftd/eft/native_pair_energy_recursion.h),
  SHA-256 `81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A`;
- [`test_native_pair_energy_recursion.cpp`](../../../../../engine/tests/test_native_pair_energy_recursion.cpp),
  SHA-256 `F0D2BFD7E8222C60C8807A10BC835413E044A00294A6F46B31A7DB0D481142F7`.

The header is an isolated `ftd::eft` selected reference. It uses the monotone
cubic proved in (13)--(14), reports equation/energy/orientation diagnostics,
and fails closed on invalid data or an unresolved root. It contains no `G*`,
target period, `Voxel`, `RenderBridge`, production toggle, or tick-phase
consumer.

The focused Release CTest passes `1/1`. Direct execution reports:

```text
maximum_equation_residual=1.0921819004749977e-14
maximum_energy_residual=2.6645352591003757e-15
maximum_reverse_residual=7.4384942649885488e-15
maximum_long_energy_drift=4.0550895974433843e-13
scope=SELECTED_EFT_REFERENCE_NOT_PRODUCTION_PAIR_COUPLING
cadence=CONTINUUM_GSTAR_FACTOR_FINITE_TICK_OPEN
```

The long-run check covers 20,000 steps. This validates the numerical
realization of the conditional recursion; it is not substrate evidence. No
production dynamics changed.
