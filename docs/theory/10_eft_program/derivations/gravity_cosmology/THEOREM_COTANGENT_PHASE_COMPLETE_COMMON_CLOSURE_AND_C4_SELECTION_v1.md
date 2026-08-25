# Cotangent phase-complete common closure and C4 selection v1

**Date:** 2026-08-24

**Status:** **[THEOREM — PHASE-COMPLETE COMMON CLOSURE CENSUS]** +
**[THEOREM — C4 LINEAR SELECTION RULE]** + **[SCOPE CORRECTION — RANK THIRTY
IS A FIXED-QUADRATURE SLICE; PHASE-COMPLETE TARGET PRICE IS FIFTY]** +
**[CONDITIONAL DIRAC PRICE]** + **[OPEN — NATIVE PHASE REALITY OR NONLINEAR
SHARED VERTEX]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_phase_complete_common_closure_and_c4_selection.py](../../../../../scripts/proofs/proof_cotangent_phase_complete_common_closure_and_c4_selection.py)
performs 231 exact checks. It exhausts all 48 right collisions on all three
cotangent layers through the exact C4 tensor-product lift, verifies one full
representative of every closure class on every layer, and certifies the
selected phase-complete generator and selection rule directly.

---

## 1. Why the phase type must be restored

The
[rank-thirty common closure theorem](THEOREM_COTANGENT_COMMON_MAXWELL_TENSOR_COLLISION_CLOSURE_PRICE_v1.md)
uses the 48 cotangent flags at a fixed C4 phase. That is a valid Poincaré
section and a useful collision diagnostic. It is not yet the complete
one-record carrier of the phase-complete action.

The native microscopic alphabet includes four phases. Their two real
quadratures are

\[
 P=
 \begin{pmatrix}
 1&0&-1&0\\
 0&1&0&-1
 \end{pmatrix},                              \tag{1}
\]

while a phase-independent observable uses

\[
 u_0=(1,1,1,1).                              \tag{2}
\]

Exactly,

\[
 \operatorname{rank}P=2,\qquad
 \operatorname{rank}u_0=1,\qquad
 Pu_0^{\mathsf T}=0.                         \tag{3}
\]

The tensor readout carries the nontrivial C4 quadratures \(P\). The registered
Maxwell readout is phase independent and carries \(u_0\). They are therefore
linearly independent types before any spatial dynamics is considered.

---

## 2. C4 action

Let \(K\) be the four-cycle on phase. Then

\[
 u_0K=u_0,\qquad PK=JP,                      \tag{4}
\]

with

\[
 J=
 \begin{pmatrix}
 0&-1\\
 1&0
 \end{pmatrix},
 \qquad J^2=-I_2.                            \tag{5}
\]

Thus Maxwell belongs to the trivial real C4 representation, while the tensor
quadratures belong to the two-dimensional rotation representation.

---

## 3. Complete closure census

For a phase-blind right collision \(C\), let \(d_T\) and \(d_M\) be the
fixed-quadrature tensor and phase-independent Maxwell closure dimensions.
The phase-complete tensor closure is

\[
 \mathcal V_T^{\rm C4}=P\otimes\mathcal V_T,
 \qquad \dim\mathcal V_T^{\rm C4}=2d_T,       \tag{6}
\]

while

\[
 \mathcal V_M^{\rm C4}=u_0\otimes\mathcal V_M,
 \qquad \dim\mathcal V_M^{\rm C4}=d_M.        \tag{7}
\]

Orthogonality in (3) makes their sum direct. The exact census is identical on
all three cotangent layers:

| Collisions | Phase-complete tensor | Maxwell | Common |
|---:|---:|---:|---:|
| 8 | 20 | 7 | 27 |
| 8 | 20 | 13 | 33 |
| 16 | 40 | 10 | 50 |
| 16 | 52 | 19 | 71 |

The earlier curl-closure theorem proved that the fixed-slice target first
appears in the \((20,10,30)\) class. Its C4 lift contains both quadratures of
that target and therefore first appears in

\[
 \boxed{\dim\mathcal V_{\rm common}^{\rm C4}=40+10=50.} \tag{8}
\]

Hence rank thirty is not the phase-complete common carrier price. It is the
price after fixing one quadrature slice.

---

## 4. Exact C4 linear-selection rule

For the selected witness, the tensor generator on the forty-dimensional
phase-complete closure is exactly

\[
 A^{\rm C4}_{T,a}=I_2\otimes A_{T,a},
 \qquad a\in\{x,y,z\}.                       \tag{9}
\]

The common generator has the block form

\[
 A^{\rm C4}_a=
 \begin{pmatrix}
 I_2\otimes A_{T,a}&0\\
 0&A_{M,a}
 \end{pmatrix}.                              \tag{10}
\]

All \(40\times10\) and \(10\times40\) cross blocks vanish exactly.

This is forced by C4 covariance, not by a convenient basis. The tensor phase
action is \(Q_T=J\otimes I_{20}\), whereas Maxwell has \(Q_M=I_{10}\). A
linear intertwiner \(X:\mathcal V_T^{\rm C4}\to\mathcal V_M\) would have to
satisfy

\[
 XQ_T=X.                                     \tag{11}
\]

But

\[
 \operatorname{rank}(Q_T-I_{40})=40,         \tag{12}
\]

so \(X=0\). The reverse intertwiner vanishes by the same argument.

Therefore:

\[
 \boxed{\text{a C4-equivariant vacuum linear action cannot mix the
phase-independent Maxwell and tensor-quadrature sectors.}} \tag{13}
\]

---

## 5. Meaning for “one action”

Equation (13) is not a failure of unification. Standard unified actions also
contain distinct linearized vacuum sectors. The physically relevant demand is
that one microscopic rule:

1. evolves both sectors;
2. supplies their constraints and work ledger;
3. gives them a shared matter/actualization source; and
4. fixes their nonlinear reciprocal coupling.

C4 says where that coupling may live. It must be:

1. a phase-neutral nonlinear product;
2. a matter-mediated vertex carrying the compensating phase history;
3. an action-derived phase-reality relation; or
4. an explicitly declared breaking of C4 covariance.

A bare linear Maxwell/tensor cross term is unavailable.

This clarifies the role of the
[actualization shared-moment source vertex](../common_action_mechanics_reciprocity/THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md):
it is a candidate common source, but it must now be lifted to a phase-neutral
reciprocal vertex rather than treated as evidence of vacuum linear mixing.

The
[phase-neutral shared charge/stress successor](../common_action_mechanics_reciprocity/THEOREM_C18_PHASE_NEUTRAL_SHARED_CHARGE_STRESS_VERTEX_v1.md)
closes the source half of that lift. Contracting the vector and tensor
doublets with the same token phase gives
\(j_{\rm evt}=\epsilon d/9\) and
\(t_{\rm evt}=dd^{\mathsf T}/18=-\Delta K\). The current is charge odd; the
tensor/capacity source is charge even. Reciprocal work and propagating
response remain open.

---

## 6. Conditional physical reduction price

If the complete fifty-dimensional carrier is treated as phase space, two
Maxwell polarizations plus two tensor polarizations and their partners require
eight physical dimensions. Conditional Dirac counting gives

\[
 50-2F-S=8,
\]

so

\[
 \boxed{2F+S=42.}                             \tag{14}
\]

There is a logically distinct route. A native phase-reality or global
quadrature-synchronization rule could first remove twenty tensor-copy
dimensions:

\[
 50-20=30,\qquad 42-20=22.                   \tag{15}
\]

That would recover the previous rank-thirty and \(2F+S=22\) accounting. No
such rule has been derived. It may not be assumed merely because the substrate
has a global clock: global tick order does not by itself prove equal internal
phase offsets for records with different genesis histories.

---

## 7. Exact scope

### Theorem-grade content

This theorem proves:

1. the complete \(27,33,50,71\) closure census for phase-blind right
   collisions;
2. the minimum phase-complete lift of the selected target-containing carrier
   is rank fifty;
3. the selected tensor-40 generator is two exact C4-related copies of the
   fixed-slice tensor-20 generator; and
4. C4 covariance forbids every linear tensor/Maxwell intertwiner.

### Not claimed

It does not prove:

1. that all admissible native collisions must be phase blind;
2. a phase-reality or synchronization constraint;
3. a nonlinear common coupling;
4. an isolated Maxwell or spin-2 pole;
5. static gravity, universal coupling, or lensing;
6. a physical Born preparation/pushforward from the same action; or
7. a native electromagnetic action normalization or alpha measurement.

Production remains unchanged and class 0.

---

## 8. Next locked gate

The phase-complete action must take one of two honest routes:

### Route A — derive a reality/synchronization quotient

Construct a reversible, source-compatible rule that reduces tensor-40 to
tensor-20 without inserting a preferred quadrature. It must explain phase
offsets of independently created records and preserve the global inverse.

### Route B — retain rank fifty

Keep both C4 quadratures, derive the full \(2F+S=42\) reduction, retain the
proved phase-neutral actualization charge/stress source, and complete it into
a reciprocal nonlinear matter vertex coupling Maxwell, tensor, clock, and
capacity sectors.

Both routes must then:

1. produce a layer-covariant direction-stable Maxwell and tensor symbol;
2. isolate two physical polarizations in each sector;
3. generate charged Gauss continuity and a tensor constraint algebra;
4. yield a reciprocal static response for the blind lensing observable; and
5. expose the electromagnetic action curvature for the blind native-alpha
   readout.

Until one route passes, rank fifty is the honest phase-complete capacity price,
not a completed unified action.
