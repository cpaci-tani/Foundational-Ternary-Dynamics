# Audit — P4-Preserving Period-Two Lorentz Prototype

**Registry:** FTD-0408  
**Status:** `[SELECTED IMPLEMENTATION PROTOTYPE]` + `[THEOREM — free linear Floquet pole]`  
**Verdict:** `LR-1-TREE-POLE-PASS; COMMON-CONE-FAILS-PENDING-RECALIBRATION`  
**Exact verifier:** [`proof_lorentz_p4_period2.py`](../../../scripts/proofs/proof_lorentz_p4_period2.py) (27/27)  
**Native gate:** `lorentz_period2_floquet`

---

## 0. Result

The FTD-0407 obstruction applies to a **constant-coefficient**, nearest-time
update. It does not apply to a local update with a two-tick temporal cell.
There is an exact P4-preserving escape:

\[
\kappa_{2n}=\frac3{13},\qquad
\kappa_{2n+1}=-\frac1{13},
\]

used in the otherwise unchanged kick-drift recurrence

\[
J^{n+1}-2J^n+J^{n-1}=-\kappa_n M_{18}(q)J^n.
\]

Every microscopic tick still reads only the current site and its nearest Moore
shell. Composition over two ticks generates the required `M18²` term without a
radius-two one-tick dependency, an elliptic solve, or a new propagating field.

For a two-tick Floquet eigenmode with phase `theta` per microscopic tick, the
exact pole is

\[
\boxed{\sin^2\theta=\frac{M_{18}}{13}+\frac{3M_{18}^2}{676}}.
\]

Since `0 <= M18 <= 16/3`, the right-hand side is monotone and obeys

\[
0\le X(M_{18})\le X(16/3)=\frac{272}{507}<1.
\]

Every nonzero Fourier mode therefore has unit-modulus Floquet multipliers.
The zero mode has the usual free-wave uniform-velocity drift.

The low-momentum pole is

\[
\boxed{\theta^2=\frac1{13}S_2+0\cdot S_2^2+O(|q|^6)}.
\]

The complete dimension-six pole correction vanishes. Dimension-eight cubic
breaking remains. This passes LR-1 for the bare free-flux prototype only; it is
not physical Lorentz recovery.

---

## 1. Independent derivation

For one scalar Fourier component, define the one-tick transfer matrix

\[
T(\kappa)=
\begin{pmatrix}
2-\kappa M&-1\\
1&0
\end{pmatrix},\qquad \det T=1.
\]

The two-tick monodromy is `P=T(kappa_1)T(kappa_0)`. Write

\[
A=\frac{\kappa_0+\kappa_1}{2},\qquad
B=\frac{\kappa_0\kappa_1}{4}.
\]

Direct multiplication gives

\[
\frac12\operatorname{tr}P=1-2(AM-BM^2)=\cos(2\theta),
\]

hence

\[
\sin^2\theta=AM-BM^2.
\]

Using

\[
M_{18}=S_2-\frac1{12}S_2^2+O(|q|^6),\qquad
\arcsin^2\sqrt X=X+\frac13X^2+O(X^3),
\]

the quartic coefficient in `theta²` is

\[
-\frac{A}{12}-B+\frac{A^2}{3}.
\]

It vanishes exactly when

\[
B=\frac{A(4A-1)}{12}.
\]

Real kick coefficients require

\[
(2A)^2-16B=\frac{4A(1-A)}3\ge0.
\]

A rational parametrization is

\[
A=\frac1{1+3t^2},\qquad
\kappa_{0,1}=A\pm\frac{t}{1+3t^2}.
\]

The selected `t=2` member gives `A=1/13` and
`(kappa_0,kappa_1)=(3/13,-1/13)`. The use of `3` and `13` is a
reverse-engineered catalog-compatible selection, not a derivation of these
coefficients from P1–P5.

---

## 2. Why the anti-kick is not optional in this scalar class

If both kicks are nonnegative, then `B>=0`, so a nontrivial cancellation has
`A>=1/4`. Real cancellation pairs also require `A<=1`.

At the production endpoint `M=16/3`,

\[
X(16/3)-1=-\frac{256A^2-208A+27}{27}.
\]

This is positive from `A=1/4` through
`A=(13+sqrt(61))/32`. Above the interior-vertex threshold `A=17/32`, the
maximum occurs at `M*=6/(4A-1)` and equals

\[
X(M_*)=\frac{3A}{4A-1}=1+\frac{1-A}{4A-1}>1
\]

for every `A<1`. At `A=1`, `X(16/3)<0`. Thus no two-nonnegative-kick member is
stable over the full production band. The alternating anti-kick is the price of
this minimal scalar construction.

This is spectral stability, not positivity of a one-tick instantaneous energy.
The conserved quadratic form is a two-tick Floquet invariant and is
parity-dependent under micromotion. Any interacting implementation must prove
that the anti-kick does not create nonlinear parametric instabilities.

---

## 3. Surviving cutoff term

The exact sixth-order term is

\[
-\frac1{395460}\left[
216\sum_i q_i^6
+479\sum_{i\ne j}q_i^4q_j^2
+1803q_x^2q_y^2q_z^2
\right].
\]

It is nonzero and cubically anisotropic. The prototype therefore moves the
first bare-pole violation from dimension six to dimension eight; it does not
eliminate lattice Lorentz violation.

---

## 4. P4 locality and the new assumption bill

The live implementation changes only the coefficient multiplying the existing
18-point Laplacian in `phase_read`:

- even tick: `delta_j=(3/13) Delta18 J`;
- odd tick: `delta_j=-(1/13) Delta18 J`.

There is one nearest-Moore read per tick. The effective `M18²` appears only
after two legal local steps, so no influence crosses more than one Moore shell
in one microscopic tick.

The construction nevertheless adds two commitments:

1. a distinguished even/odd temporal phase, equivalently a global `Z2` clock
   bit if the dynamics is written as one autonomous map;
2. a negative spatial-stiffness kick on alternate ticks.

Neither is forced by the five postulates. Both are `[SELECTED]`. The global
clock phase also breaks one-tick time-translation symmetry to a two-tick
subgroup at the substrate scale.

---

## 5. Common-cone defect exposed immediately

The repaired leading speed is

\[
c_{\rm flux,prototype}=\sqrt A=\frac1{\sqrt{13}}
\]

voxel per tick, not the production `C_WAVE=C_SPEED=1/sqrt(3)`. Consequently the
prototype does **not** pass LR-2. It creates an explicit leading-speed mismatch
with the manifested-particle causal budget, the existing clock calibration,
and the standalone Wilson matter module.

A physical-clock reinterpretation could change the tick calibration to
`t_phys=a_phys/(sqrt(13)c_phys)`, but that is a global recalibration, not a free
coordinate relabeling once matter, gravity, decay, and detector clocks share the
same tick. Every sector must be rederived or measured under that calibration.

The fastest stable cancellation member has a larger `A` than `1/13`, but no
selection principle currently fixes it. The `3/13,-1/13` point was chosen for
exact rational closure on existing catalog integers, not for maximal signal
speed or a common cone.

---

## 6. Engine scope

The CPU-only toggle `lorentz_period2_floquet` is default OFF and conflicts with
the alternate Verlet and explicit-`dt` leapfrog owners. The default engine and
all existing golden trajectories are unchanged.

The implementation is intentionally a prototype:

- exact proof: 27/27 symbolic/source-contract gates;
- native test: coefficients, stability endpoint, validation conflicts, live
  even/odd accelerations, and exact two-tick recurrence;
- GPU/WASM: not implemented;
- interacting, damping, Gauss, genesis, and matter-sector stability: not
  claimed by the free proof.

---

## 7. Recovery status after FTD-0408

| Gate | Status |
|---|---|
| LR-0 full bare flux symbol | `[PASS — exact two-tick Floquet pole]` |
| LR-1 stable local tree improvement | `[PASS — CPU prototype, free linear sector]` |
| LR-2 common cone | `[FAIL/OPEN]`; prototype flux speed is `1/sqrt(13)` while the live matter budget remains `1/sqrt(3)` |
| LR-3 interacting/radiative closure | `[OPEN]`; anti-kick nonlinear stability and dimension-3/4 mixing uncomputed |
| LR-4 Ward/unitarity compatibility | `[OPEN]`; determinant-one free transfer is not interacting unitarity |
| LR-5 SME phenomenology | `[OPEN]` |
| LR-6 operational boosts | `[OPEN]` |

FTD-0409 executes the first fixed-cone gate. Scalar period-two and period-three
updates at `c²=1/3` are exact no-go classes, as is the minimal positive-gap
Hermitian one-auxiliary stiffness pencil. A stable degree-four target trace
exists, but its natural `c3=0` witness has no four-real-kick factorization. The
FTD-0410 subsequently shows that the Gauss AGM does not derive the live
`1/sqrt(3)` cone and isolates a conditional unit-cone branch. For the original
fixed-cone compatibility problem, the next load-bearing calculation remains
the general period-four semialgebraic realizability problem or a branchwise
paraunitary multi-state construction,
followed by interacting operator mixing. See
[`AUDIT_LORENTZ_FIXED_CONE_GATE.md`](AUDIT_LORENTZ_FIXED_CONE_GATE.md).
