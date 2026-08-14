# Theorem — Local canonical Hamiltonian parity rail and scalar locality boundary v1

**Identifier:** `FTD-0875`  
**Status:** `[THEOREM — POSITIVE LOCAL CANONICAL HAMILTONIAN LIFT]` +
`[CONDITIONAL THEOREM — TWO REAL CARRIER COORDINATES PER SITE MINIMUM IN THE REGISTERED LOCAL CLASS]` +
`[THEOREM — EXACT LOCAL ANTISYMMETRIC ENERGY CURRENT]` +
`[THEOREM — UNDOUBLED SCALAR COMMON FORM IS BOUNDARY-GLOBAL]` +
`[IMPOSED — COMMON HARMONIC CLOCK, SCALE, AND BOND LAW]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[OPEN — NATIVE DOUBLET/CLOCK FORMATION, ROUTING, COLLISIONS, PRODUCTION, G*]`  
**Date:** 2026-08-11  
**Certificate:** locked first execution `56/56`; no repair

## 1. Result

The FTD-0874 alternating ternary rail has an exact positive Hamiltonian lift
whose spatial couplings are nearest-neighbour local. Give each rail site one
canonical pair

\[
 z_j=(q_j,p_j),\qquad\{q_j,p_k\}=\delta_{jk}.                  \tag{1}
\]

For the disjoint tick-`n` matching define

\[
 N=\frac12\sum_j(q_j^2+p_j^2),                                \tag{2}
\]

\[
 L_n=\sum_{(j,k)\in\mathcal M_n}(q_jp_k-q_kp_j).              \tag{3}
\]

With one selected common reference-clock pair `(theta,I)`, the imposed law

\[
 H_{n,\sigma}=\Omega I+\Omega N
 +\sigma\frac{\Omega}{4}(1-\cos\theta)L_n                    \tag{4}
\]

generates, after one complete clock cycle, the FTD-0874 forward layer for
`sigma=+1` and its exact inverse for `sigma=-1` on both `q` and `p`.

On the actual gate section

\[
 q_j=ax_j,\qquad p_j=0,qquad x_j\in\{-1,0,+1\},              \tag{5}
\]

the endpoint returns to `p=0`, and `q/a` is exactly the discrete ternary rail.

## 2. Why the scalar rail does not give local canonical hardware

The undoubled scalar rail is not abstractly non-symplectic. For every even
open length `L=2m`, define

\[
 J^{(L)}_{i,L-1-i}=(-1)^{m+i+1}\quad(0\le i<m),                \tag{6}
\]

and complete it antisymmetrically. Direct block reflection gives

\[
 (J^{(L)})^2=-I,
 \qquad U_0^TJ^{(L)}U_0=J^{(L)},
 \qquad U_1^TJ^{(L)}U_1=J^{(L)}.                              \tag{7}
\]

Thus both alternating scalar layers preserve one common symplectic form. But
equation (6) pairs site zero with site `L-1`, site one with `L-2`, and so on.
The form depends on the retained boundary length and is globally mirror-
paired. It is not onsite substrate hardware.

In the registered local class, the symplectic form is a direct sum of
identical nondegenerate onsite fibers. A one-dimensional real skew form is
zero. Therefore one scalar per site cannot work in that class. A
two-dimensional canonical pair is sufficient, so two real carrier coordinates
per site are minimum **within this declared local class**. No stronger
universal dimension theorem is claimed.

This is the precise sense in which the framework needs a self-dual local
doublet: not as metaphor, but as the minimum local phase-space cell capable of
carrying orientation and its conjugate response.

## 3. Exact stroboscopic flow

Each active bond contributes

\[
 L_{jk}=q_jp_k-q_kp_j.                                        \tag{8}
\]

Disjoint `L_jk` commute, and

\[
 \{N,L_n\}=0.                                                   \tag{9}
\]

The `Omega N` term gives one complete onsite oscillator winding during

\[
 T=\frac{2\pi}{\Omega}.                                       \tag{10}
\]

The coupling angle is independently

\[
 \beta_\sigma(T)
 =\sigma\frac{\Omega}{4}
 \int_0^T(1-\cos\Omega t)dt
 =\sigma\frac{\pi}{2}.                                       \tag{11}
\]

Because the two generators commute, their flows factor exactly. The onsite
winding is identity at `T`, while the spatial flow gives

\[
 \begin{array}{lll}
 \sigma=+1:&(q_j,q_k)\mapsto(-q_k,q_j),
 &(p_j,p_k)\mapsto(-p_k,p_j),\\
 \sigma=-1:&(q_j,q_k)\mapsto(q_k,-q_j),
 &(p_j,p_k)\mapsto(p_k,-p_j).
 \end{array}                                                    \tag{12}
\]

This is exactly `R` or `R^-1` on every disjoint bond.

## 4. Positivity

For each bond, the determinant inequality gives

\[
 |q_jp_k-q_kp_j|
 \le\frac12(q_j^2+p_j^2+q_k^2+p_k^2).                         \tag{13}
\]

Summing over disjoint bonds yields `|L_n|<=N`. Since
`0<=1-cos(theta)<=2`,

\[
 \Omega N+\sigma\frac{\Omega}{4}(1-\cos\theta)L_n
 \ge\frac{\Omega}{2}N\ge0.                                  \tag{14}
\]

The carrier plus interaction Hamiltonian is therefore bounded below. The
reference clock action still needs a positive reserve; it is not made free by
this inequality.

## 5. Clock-action ledger

Both `N` and `L_n` are conserved. Hamilton's equation for the clock action
integrates exactly to

\[
 I(\theta)=I_0
 -\sigma\frac14(1-\cos\theta)L_n.                             \tag{15}
\]

Therefore

\[
 |\Delta I|_{\max}=\frac{|L_n|}{2},
 \qquad
 |\Delta E_{\rm ref}|_{\max}
 =|E_{\rm int}|_{\max}
 =\frac{\Omega|L_n|}{2}.                                      \tag{16}
\]

At the cycle endpoint, `I=I_0`, the interaction vanishes, and total energy has
zero residual. A sufficient bidirectional reserve is `I_0>|L_n|/2`.

On the actual section (5), every bond determinant is initially zero. Since
`L_n` is conserved,

\[
 L_n(t)=0                                                       \tag{17}
\]

along the complete actual-section orbit. The interaction **value** and clock
backreaction are consequently zero there, even though the gradient of `L_n`
is nonzero and its Hamiltonian vector field transports the record. This is a
special zero-backreaction reference submanifold, not evidence that formation,
synchronization, or switching hardware costs no energy.

## 6. Local energy current

Assign onsite carrier energy

\[
 E_j=\frac{\Omega}{2}(q_j^2+p_j^2).                            \tag{18}
\]

The base onsite oscillator changes no `E_j`. On active bond `(j,k)`, let

\[
 c(t)=\sigma\frac{\Omega}{4}(1-\cos\theta(t)).
\]

Hamilton's equations give

\[
 \dot E_j=-\mathcal J_{j\to k},
 \qquad
 \dot E_k=+\mathcal J_{j\to k},                              \tag{19}
\]

with

\[
 \mathcal J_{j\to k}
 =\Omega c(t)(q_jq_k+p_jp_k).                                 \tag{20}
\]

Thus every bond has an exact antisymmetric current and conserves its two-site
energy. For a ready ternary record at `j` and an empty `k`,

\[
 E_j(0)=\frac{\Omega a^2}{2},\qquad E_k(0)=0,
\]

and after the forward cycle,

\[
 E_j(T)=0,
 \qquad E_k(T)=\frac{\Omega a^2}{2}.                           \tag{21}
\]

The imposed scale `Omega a^2/2` is transported exactly, not derived from the
substrate.

## 7. Clock hierarchy

Three roles remain distinct:

1. integer `n` selects the even or odd bond matching;
2. the imposed common harmonic phase `theta` generates one exact bond layer;
3. the separate quartic `G*` calendar may determine when a physical event is
   eligible.

FTD-0875 derives no gearbox identifying roles 2 and 3. Nor does a continuous
subcycle become microscopic ontology merely because it is a useful
Hamiltonian interpolation of one discrete global tick.

## 8. Certificate and implementation

The byte-frozen preregistration has SHA-256
`659CAA27079D08BE620E6DF0DBCF0828B0923D242636EA34B7A7A454C2B75CB0`.
The frozen certificate
`scripts/proofs/proof_local_canonical_hamiltonian_parity_rail.py` has SHA-256
`B971DDA9A79AD53C340B00A4268EF9DA5BF089AF62DC37DE3D04757FAE03E326`
and passed `56/56` on its first execution without repair.

The isolated implementation is:

- `engine/include/ftd/eft/local_canonical_hamiltonian_parity_rail.h`;
- `engine/src/eft/local_canonical_hamiltonian_parity_rail.cpp`; and
- `engine/tests/test_local_canonical_hamiltonian_parity_rail.cpp`.

It exhaustively checks both orientations, both parity layers, and all ternary
rails through length six; generic continuous-state inverse recovery; positive
carrier bounds; local bond-energy/current ledgers; the boundary-global scalar
form; actual-section zero backreaction; and fail-closed inputs. It changes no
production `Voxel`, field, toggle, boundary, renderer, or tick phase.

## 9. Boundary statement

FTD-0875 closes the **reference intersite Hamiltonian trajectory and local
energy-current ledger** for the selected parity rail. It does not close:

- native formation or persistence of the onsite canonical doublet;
- derivation of `a`, `Omega`, or the record-energy scale;
- physical generation and local synchronization of the common harmonic pulse;
- selection of rail axis, orientation, branching, or multidimensional route;
- nonlinear collision and sustained-backpressure resolution;
- reciprocal finite-boundary completion;
- production event/energy-current coupling;
- robustness under disorder, missed pulses, overlap, or noise;
- synchronization to the separate quartic `G*` calendar;
- Born recovery, Bell correlations, operational Lorentz hiding; or
- whole-framework completeness.

No new selected type is added. The construction refines
`SEL-CA-PHASE-RAIL` and leaves the production falsifier active.
