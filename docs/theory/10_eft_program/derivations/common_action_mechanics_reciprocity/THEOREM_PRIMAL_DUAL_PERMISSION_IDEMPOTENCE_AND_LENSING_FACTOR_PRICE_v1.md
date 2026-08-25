# Primal/dual permission idempotence and lensing-factor price v1

**Date:** 2026-08-24

**Status:** **[THEOREM — EXACT SINGLE-BINARY-PERMISSION IDEMPOTENCE
OBSTRUCTION]** + **[THEOREM — EXACT REVERSIBLE TWO-PERMISSION PRODUCT
ENUMERATOR]** + **[THEOREM, CONDITIONAL — EQUAL PRIMAL/DUAL MARGINALS GIVE
THE CLASS-2 WEAK OPTICAL COEFFICIENT]** + **[SELECTION CANDIDATE — DUAL
CAPACITY OWNERSHIP]** + **[OPEN — ACTION-GENERATED PERMISSIONS, FINITE
COTANGENT/HODGE LIFT, STATIC POLE, INHOMOGENEOUS LENSING]**

**Production status:** unchanged; the production Maxwell stencil remains
unread of latency or bond capacity

**Ledger status:** no row minted

**Exact certificate:**
[proof_primal_dual_capacity_permission_lensing_type_price.py](../../../../../scripts/proofs/proof_primal_dual_capacity_permission_lensing_type_price.py)
performs 1,527,446 exact checks. It exhausts every one-bit history through ten
ticks, every two-bit history through six ticks on a finite clock/wave product,
and deterministic product enumerators through $24\times24$ permission states.
All identities are integer, rational, or symbolic; no target constant or
numerical search is used.

---

## 1. Question inherited from the common-admission theorem

The
[common-admission clock/Maxwell theorem](THEOREM_COMMON_ADMISSION_CLOCK_MAXWELL_AND_SPATIAL_LENSING_PRICE_v1.md)
proved that one retained local admission history gives

\[
 {d\tau_{\rm clock}\over dn}=\nu_t,
 \qquad
 c_{\rm EM}^{\rm global}={\nu_t\over6},
 \qquad
 a_0=a_t.                                           \tag{1}
\]

Under clock/fall coherence $a_t=a_m$, equation (1) reaches the temporal-only
class

\[
 \mathscr D=\mathscr S=1.                           \tag{2}
\]

The open question was whether the **same binary permission** could simply be
read a second time as a spatial/Hodge capacity and thereby generate the
missing coefficient $a_s$.

It cannot.

---

## 2. Exact one-bit idempotence obstruction

Let

\[
 g_n\in\{0,1\}                                      \tag{3}
\]

be the retained permission at global tick $n$. If both temporal advance and
spatial transit read this same bit, their joint gate is

\[
 g_n^{\rm temporal}g_n^{\rm spatial}=g_n^2=g_n.     \tag{4}
\]

For every finite history,

\[
 \sum_n g_n^2=\sum_n g_n.                           \tag{5}
\]

Consequently a second verbal interpretation of the same binary event does
not create a second response factor. Conditional on a temporally admitted
tick, the duplicated spatial gate is open with frequency exactly one:

\[
 {\sum_n g_n^2\over\sum_n g_n}=1                   \tag{6}
\]

whenever the denominator is nonzero.

Therefore the blocked wave speed remains $c_*\nu_t$, not $c_*\nu_t^2$. In the
weak response notation, rereading one bit supplies $a_0=a_t$ and $a_s=0$.

> **Scoped no-go.** A single retained binary permission cannot generate the
> temporal and spatial halves of weak lensing merely by being named twice.

This does not exclude a non-binary capacity weight, a history-dependent
non-idempotent gate, or two separately retained permissions. It closes only
the minimal duplicated-binary route.

---

## 3. Minimal retained primal/dual lift

Introduce two separately retained local permissions,

\[
 (g_{t,n},g_{s,n})\in\{0,1\}^2,                    \tag{7}
\]

with the intended ownership:

- $g_t$: primal/site admission of the material recurrence and complete local
  Floquet tick;
- $g_s$: dual/face admission of the spatial edge--face incidence transfer.

The material clock advances by

\[
 N_t=\sum_n g_{t,n},                                \tag{8}
\]

while the first-order propagating phase advances by

\[
 N_{11}=\sum_n g_{t,n}g_{s,n}.                     \tag{9}
\]

The certificate realizes this on a finite product of an eight-state clock and
a seventeen-state test wave phase:

\[
 (c,w)\mapsto
 \big(c+g_t\pmod8,\;w+g_tg_s\pmod{17}\big).        \tag{10}
\]

Because both permission bits are retained, the inverse subtracts the same
increments in reverse history order. Equation (10) is a total permutation for
every fixed permission pair. No bit is erased.

The period seventeen is only an exhaustive finite reversibility witness; it
is not a proposed FTD physical constant.

---

## 4. First-order cotangent interpretation

Let the normalized complete cotangent return map on its vacuum slow subspace
be

\[
 M(k)=I+i kA+O(k^2),\qquad M(0)=I.                 \tag{11}
\]

Here $A$ has the already-proved two transverse Maxwell pairs with
$c_*=1/6$. A temporally stalled cycle applies identity. A temporally admitted
cycle with a closed dual transit gate retains its internal state but has no
first-order Bloch displacement. Therefore the gated cycle has expansion

\[
 M_n(k)=I+i k\,g_{t,n}g_{s,n}A+O(k^2).             \tag{12}
\]

Multiplication through $N$ blocked cycles gives

\[
 \prod_{n=0}^{N-1}M_n(k)
 =I+i kN_{11}A+O(k^2).                             \tag{13}
\]

Define

\[
 \nu_t={N_t\over N},
 \qquad
 \sigma={N_{11}\over N_t}.                        \tag{14}
\]

Then

\[
 \boxed{
 {d\tau_{\rm clock}\over dn}=\nu_t,
 \qquad
 c_{\rm EM}^{\rm global}=c_*\nu_t\sigma.}         \tag{15}
\]

Equation (15) separates temporal admission from spatial incidence without
altering the global tick. It is a first-order homogeneous-block theorem. A
finite inhomogeneous cotangent operator, interface scattering, exact Gauss
closure, and a static source solution are not supplied here.

---

## 5. Deterministic exact factorization

Factorization does not require stochastic permissions. For any positive
integer $L$, let a retained pointer traverse the reversible cycle
$\mathbb Z_{L^2}$. Read it in mixed radix as

\[
 r=n\bmod L,
 \qquad
 s=\left\lfloor{n\over L}\right\rfloor.           \tag{16}
\]

One complete orbit visits every ordered pair $(r,s)\in\mathbb Z_L^2$ exactly
once. Choose fixed permission subsets of sizes $A_t$ and $A_s$. Then exactly

\[
 N_t=A_tL,
 \qquad
 N_s=A_sL,
 \qquad
 N_{11}=A_tA_s.                                    \tag{17}
\]

Hence

\[
 \boxed{
 {N_{11}\over L^2}
 ={N_t\over L^2}{N_s\over L^2}.}                  \tag{18}
\]

This is a finite deterministic product census, not statistical independence
assumed over repeated experiments. The pointer successor and predecessor are
exact inverses.

The subsets and their sizes are not generated by the present action. Equation
(18) is an existence witness for target-free factorized blocking, not a
physical derivation of a capacity profile.

---

## 6. Equal marginal response and the class-2 condition

Write the blocked marginal admissions as

\[
 \nu_t(U)=1-a_tU+O(U^2),
 \qquad
 \nu_s(U)=1-a_sU+O(U^2).                           \tag{19}
\]

Under the exact factorized census,

\[
 c_{\rm ray}(U)
 ={1\over6}\nu_t(U)\nu_s(U).                      \tag{20}
\]

Therefore

\[
 {c_*/c_{\rm ray}(U)}
 =1+(a_t+a_s)U+O(U^2),                             \tag{21}
\]

and the blind response is

\[
 \mathscr D=\mathscr S={a_t+a_s\over a_m}.        \tag{22}
\]

If a primal/dual exchange symmetry of the native action derives

\[
 a_s=a_t                                             \tag{23}
\]

and the same sourced capacity also derives clock/fall coherence

\[
 a_t=a_m,                                           \tag{24}
\]

then

\[
 \boxed{
 \mathscr D=\mathscr S=2.}                         \tag{25}
\]

Equation (25) is **conditional** on equations (18), (23), and (24). It is not
a native lensing derivation because the action has not generated the two
permission fields, their equality, a radial static solution, or the
inhomogeneous ray operator.

---

## 7. Convergence with the spin-2 type price

The
[STF parity/curl theorem](../gravity_cosmology/THEOREM_COTANGENT_STF_PARITY_PRICE_AND_SPIN2_CURL_TARGET_v1.md)
independently proved that first-order tensor propagation cannot couple two
inversion-even on-site STF quadratures. It requires either:

1. an even/odd primal--dual stagger, or
2. a rank-twenty on-site phase/parity carrier.

The present theorem reaches a compatible requirement from lensing:

1. one primal temporal permission is insufficient by idempotence;
2. a second dual spatial permission is the minimum binary repair; and
3. equal primal/dual marginal response is exactly the class-2 condition.

Thus native spin-2 transport and the missing spatial half of lensing now point
to the same **candidate ownership architecture**: primal even capacity paired
with dual odd capacity. This agreement reduces the type search but does not
prove that one finite transaction realizes both roles.

---

## 8. Exact epistemic boundary

### Proved

1. Reusing one binary permission gives $g^2=g$ and cannot double the weak
   optical response.
2. Two retained permissions admit an exact reversible local controlled map.
3. A finite deterministic product orbit realizes exact factorized marginal
   and joint counts.
4. Under that factorization, equal primal/dual marginals give
   $c_{\rm ray}=\nu^2/6$ and the weak coefficient $a_t+a_s=2a_t$.
5. Within the retained-binary-permission class, two coordinates are necessary
   and sufficient at the census/first-moment level.

### Not proved

1. that the microscopic action generates either permission;
2. that the two permissions are physical primal/dual Hodge ownership rather
   than abstract controls;
3. that a native symmetry forces $a_s=a_t$;
4. that the gated cotangent map is reversible across inhomogeneous interfaces;
5. that matter sources a static $1/r$ capacity profile;
6. that rays and material clocks jointly realize equation (25);
7. that the same dual carrier closes the finite spin-2 lift; or
8. that production FTD lenses light.

The current engine remains in the measured FTD-1020 class 0. The earlier
common-admission candidate would reach class 1 if implemented. The present
primal/dual construction identifies the exact additional finite type and
factorization required for class 2; it does not authorize a lensing fixture.

---

## 9. Next locked gate

Construct one Moore-local reversible primal/dual transaction in which:

1. the existing ternary-square capacity debit generates $g_t$ rather than an
   external schedule;
2. the cotangent edge/face ownership generates $g_s$ on the dual incidence;
3. a local exchange symmetry proves equal blocked linear responses without a
   fitted coefficient;
4. the full inhomogeneous map retains an inverse and preserves the transverse
   Maxwell and Gauss sectors;
5. the same dual ownership supplies the odd STF tensor partner; and
6. a blind static-source calculation produces deflection and Shapiro delay
   before comparison with a gravity target.

Pass closes the finite spatial-Hodge type gate. Failure of every such
primal/dual lift would force a non-binary capacity weight or a larger local
ontology.

**Successor type audit.** The
[A9/cotangent no-spare-scalar theorem](THEOREM_A9_COTANGENT_NO_SPARE_SCALAR_PERMISSION_AND_DUAL_COPY_PRICE_v1.md)
proves that the existing one-token link/reserve clock cannot supply the pair:
its two capacity readings are exact complements, and a partial self-gate of
the same period-eight cycle is a permutation only for all/none admission. The
192-state cotangent carrier is transitive under $O_h\times C_4$ and has no
spare invariant scalar bit. The minimum existing-alphabet repair is therefore
a second independently owned A9 copy on the dual complex, not a relabeling of
an existing flag component.

The next
[dual-A9 skew generator](THEOREM_DUAL_A9_SKEW_CAPACITY_CLOCK_GENERATOR_AND_HOMOGENEOUS_FACTOR_PASS_v1.md)
then supplies one exact endogenous reference permutation on those two copies.
Every orbit factorizes at $\nu_t=\nu_s=1/2$, closing external scheduling at
that homogeneous point. It does not yet derive a variable weak response or
implement the cotangent Hodge gate.
