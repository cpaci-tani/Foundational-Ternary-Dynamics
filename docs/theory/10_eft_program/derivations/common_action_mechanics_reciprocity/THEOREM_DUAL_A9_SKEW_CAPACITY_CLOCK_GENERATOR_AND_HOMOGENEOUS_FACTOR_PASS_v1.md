# Dual-A9 skew capacity/clock generator and homogeneous factor pass v1

**Date:** 2026-08-24

**Status:** **[THEOREM — EXACT TOTAL REVERSIBLE DUAL-A9 SKEW
PERMUTATION]** + **[THEOREM — EVERY PHYSICAL ORBIT HAS EXACT FACTORIZED
TEMPORAL/SPATIAL PERMISSION COUNTS]** + **[THEOREM — PRIMAL/DUAL EXCHANGE AND
CHARGE-CONJUGATION COVARIANCE]** + **[THEOREM, CONDITIONAL — COMMON GATED
MAXWELL/TENSOR FIRST-MOMENT CONE]** + **[REFERENCE CONSTRUCTION — ENDOGENOUS
HOMOGENEOUS HALF-ADMISSION]** + **[OPEN — VARIATIONAL SELECTION, VARIABLE
SOURCED MARGINALS, COTANGENT/TT LIFT, STATIC RESPONSE, LENSING]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_dual_a9_skew_capacity_clock_generator.py](../../../../../scripts/proofs/proof_dual_a9_skew_capacity_clock_generator.py)
performs 5,382 exact checks. It verifies the map and inverse on all 512
primal/dual/orientation states, exhausts all 32 physical orbits, proves exact
factorization on every orbit, and checks the blocked clock/Maxwell/tensor rate
ledger rationally.

---

## 1. From a type witness to an endogenous generator

The
[A9/cotangent no-spare-scalar theorem](THEOREM_A9_COTANGENT_NO_SPARE_SCALAR_PERMISSION_AND_DUAL_COPY_PRICE_v1.md)
proved that one physical A9 clock has only one invariant ownership bit and
that the cotangent flag has no spare scalar capacity. It identified a second
independently owned A9 copy on the dual complex as the minimum
existing-alphabet type repair.

That theorem supplied only a complete product census. The present theorem
constructs one total local permutation whose **individual deterministic
orbits** realize the required factorization.

---

## 2. State and scalar permission

Let

\[
 X_P,X_D\in\mathcal X_{A9}                         \tag{1}
\]

be physical one-token A9 ownership states on primal and dual placements. Each
contains ternary endpoints, one link A9 record, and one reserve A9 record.

Let

\[
 c(X)=1-n(a_{\rm link})\in\{0,1\}                 \tag{2}
\]

be its scalar residual link capacity. Because every physical state owns one
token, $c(X)=1$ exactly when the token is reserve-owned.

Finally let

\[
 q\in\{0,1\}                                      \tag{3}
\]

be a retained structural primal/dual orientation sector. It declares which
copy is the capacity driver and which is the admitted receiver:

\[
 q=0:\ (R,C)=(P,D),
 \qquad
 q=1:\ (R,C)=(D,P).                               \tag{4}
\]

The candidate interpretation is that $q$ belongs to the global primal/dual
ownership convention rather than an independently chosen local physical
bit. That interpretation is a **[SELECTION]**, not proved by this theorem.

---

## 3. Exact triangular update

Let $F$ be the exact period-eight A9 crossing-clock permutation. Define

\[
 \boxed{
 \mathcal T(R,C,q)=\big(F^{c(C)}R,\;FC,\;q\big).}  \tag{5}
\]

Thus:

1. the controller advances on every global substrate tick;
2. its retained residual capacity $c(C)$ admits or stalls the receiver; and
3. the receiver advances by the same physical A9 manifestation/phase
   transaction already certified for the material clock.

The temporal and spatial permission readouts before the update are

\[
 g_t=c(C),
 \qquad
 g_s=c(R).                                        \tag{6}
\]

The first is endogenous: it is computed from the controller's actual finite
ownership state, not read from an external permission history.

---

## 4. Exact inverse and conservation

Given the output $(R',C',q)$, first recover

\[
 C=F^{-1}C'.                                      \tag{7}
\]

The previous permission is then known exactly as $c(C)$. Hence

\[
 R=F^{-c(C)}R'.                                   \tag{8}
\]

Equations (7)--(8) give the total inverse of equation (5). No admission bit is
stored separately or erased; it is recoverable from the retained controller
history.

Because $F$ preserves each complete A9 token, polarity, total charge, and
reserve/link ownership ledger, $\mathcal T$ preserves them independently on
both copies. The certificate verifies both inverse compositions and all
ledgers on all 512 states.

This is stronger than the earlier externally scheduled gate. It is still a
selected permutation, not a derivation from the hybrid action or a unique
stationary principle.

---

## 5. Primal/dual and charge symmetries

Define primal/dual exchange by

\[
 \mathcal E(X_P,X_D,q)=(X_D,X_P,1-q).             \tag{9}
\]

Then

\[
 \boxed{\mathcal T\mathcal E=\mathcal E\mathcal T.} \tag{10}
\]

The triangular update is therefore not physically tied to the names
``primal'' and ``dual''. Exchange also swaps the structural orientation
sector.

Let $\mathcal C$ be the A9 charge-conjugation involution applied to both
copies. Since $c(\mathcal CX)=c(X)$ and $F\mathcal C=\mathcal CF$,

\[
 \boxed{\mathcal T\mathcal C=\mathcal C\mathcal T.} \tag{11}
\]

The permission generator does not select a charge sign.

---

## 6. Every deterministic orbit factorizes

The 512 states decompose into exactly 32 cycles, each of period sixteen. On
every cycle separately,

\[
 N=16,
 \qquad
 N_t=\sum g_t=8,
 \qquad
 N_s=\sum g_s=8,
 \qquad
 N_{11}=\sum g_tg_s=4.                             \tag{12}
\]

Therefore

\[
 \boxed{
 {N_{11}\over N}
 ={N_t\over N}{N_s\over N}
 ={1\over4}.}                                     \tag{13}
\]

Equation (13) is not an ensemble average over unknown initial phases. Every
allowed deterministic history has the same counts. The receiver completes
exactly one eight-admitted-step physical A9 cycle in sixteen global ticks,
while the controller completes two cycles.

Thus the local material-clock relation is

\[
 {d\tau_R\over dn}={1\over2}.                     \tag{14}
\]

This is the first exact endogenous finite example of a global substrate tick
coexisting with a slower local recurrence.

It is a homogeneous reference fixture at one rate, not a derivation of a
gravitational profile.

---

## 7. Conditional common field cone

If—and only if—the complete first-order cotangent Maxwell advance is admitted
by the joint gate $g_tg_s$, equation (12) gives

\[
 c_{\rm EM}^{\rm global}
 ={1\over6}{N_{11}\over N}
 ={1\over24}.                                     \tag{15}
\]

If the parity-staggered STF symmetric-curl lift reads the same joint gate,
then conditionally

\[
 \boxed{c_T=c_{\rm EM}={1\over24}}                \tag{16}
\]

at this homogeneous half-admission fixture.

The certificate checks the rational rate ledger, not the full 192-state
gated cotangent collision or a finite tensor lift. Equations (15)--(16) are
therefore common-first-moment pass criteria, not production dispersion
relations.

---

## 8. What has become endogenous

One finite map now joins:

\[
 \text{dual A9 ownership cycle}
 \longrightarrow
 \begin{cases}
 \text{retained temporal permission},\\
 \text{primal recurrent actualization/clock advance},\\
 \text{separate spatial-capacity readout},\\
 \text{factorized joint field permission}.
 \end{cases}                                      \tag{17}
\]

The receiver's admitted $F$ step is the same reversible transaction that
creates and removes ternary endpoint manifestation while retaining the C4
phase/polarity token. Accordingly, the generator connects manifestation,
proto-matter recurrence, local clock rate, and the two field permissions more
tightly than an externally supplied word.

It does not make the recurrent pair a stable spatially bound material body,
derive its energy/mass, or couple an implemented field operator.

---

## 9. Exact epistemic boundary

### Proved

1. Equation (5) is a total finite permutation with exact inverse.
2. It preserves both complete A9 token, charge, phase/polarity, and ownership
   ledgers.
3. Its permission is generated from a retained local capacity state rather
   than an external schedule.
4. It is covariant under primal/dual exchange with $q\mapsto1-q$ and under
   charge conjugation.
5. Every physical orbit has the exact factorized counts in equation (12).
6. The receiver realizes one local period-eight clock in sixteen global
   ticks.

### Not proved

1. that equation (5) is selected by the native variational action;
2. that $q$ is generated by the actual primal/dual lattice ownership rather
   than imposed as a structural sector;
3. that a source changes the fixed half-admission rate into a weak field
   $\nu(U)$;
4. that the cotangent Maxwell collision/streaming map accepts $g_tg_s$ while
   retaining Gauss closure and its inverse at interfaces;
5. that the odd STF carrier and TT constraints use the same gate;
6. that the recurrent receiver is stable matter with inertial mass;
7. that a static inverse-distance capacity solution exists; or
8. that FTD lenses light.

Production remains unchanged and class 0.

---

## 10. Next locked gate

Extend the exact homogeneous reference generator without inserting a desired
weak coefficient:

1. derive equation (5), or a uniquely equivalent permutation, from one local
   discrete action/stationarity rule;
2. identify $q$ with explicit primal/dual lattice ownership;
3. add a conservative source interaction that changes orbit residence counts
   and derive $\nu_t(U)$ and $\nu_s(U)$ blindly;
4. prove equality of their first derivatives from exchange covariance;
5. lift the joint gate into the complete finite cotangent Maxwell map and
   preserve its vacuum/Gauss slow sector at inhomogeneous boundaries;
6. use the same dual record for the odd STF symmetric-curl carrier; and
7. only then solve a static source and evaluate deflection/delay.

Passing items 1--5 would turn the present fixed-rate endogenous clock into the
first native spatial-Hodge response operator. Items 6--7 would connect that
operator to spin-2-equivalent transport and lensing.

**Successor response audit.** The
[dual-capacity cyclic-mixing theorem](THEOREM_DUAL_CAPACITY_CORRELATION_OBSTRUCTION_AND_CYCLIC_MIXING_RESPONSE_v1.md)
shows that simply mixing these half-admission cells with vacuum creates a
positive common-cause covariance and a $3/2$, not 2, weak coefficient. A
reversible one-hop translation of the dual layer restores exact factorization
for arbitrary finite deficit counts. It also proves that simultaneous
independent primal/dual occupancy costs two retained tokens or an explicit
time-sharing equivalent.
