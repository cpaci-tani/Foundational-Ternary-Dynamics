# A9/cotangent no-spare scalar permission and dual-copy price v1

**Date:** 2026-08-24

**Status:** **[THEOREM — EXACT A9 INVARIANT-BINARY-READOUT
CLASSIFICATION]** + **[THEOREM — PERIOD-EIGHT SELF-ADMISSION PERMUTATION
NO-GO]** + **[THEOREM — COTANGENT $O_h\times C_4$ TRANSITIVITY/NO-SPARE-SCALAR
OBSTRUCTION]** + **[THEOREM — SECOND A9 OWNERSHIP COPY IS TYPE-SUFFICIENT FOR
EQUAL FACTORIZED BINARY MARGINALS]** + **[OPEN — DUAL-COMPLEX OWNERSHIP,
NATIVE GENERATOR, SOURCED RESPONSE, MAXWELL/TENSOR LIFT, LENSING]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_a9_cotangent_no_spare_scalar_permission.py](../../../../../scripts/proofs/proof_a9_cotangent_no_spare_scalar_permission.py)
performs 6,006 exact checks. It classifies the finite A9 symmetry orbits,
exhausts every binary self-admission mask on both period-eight clock cycles,
proves transitivity of all 192 cotangent flag/phase states under
$O_h\times C_4$, and checks the complete $16\times16$ product of two
independently owned A9 clock carriers.

---

## 1. Question inherited from the primal/dual price

The
[primal/dual permission theorem](THEOREM_PRIMAL_DUAL_PERMISSION_IDEMPOTENCE_AND_LENSING_FACTOR_PRICE_v1.md)
proved that one retained binary permission cannot provide separate temporal
and spatial weak responses. A factorized binary route requires two separately
retained permissions,

\[
 (g_t,g_s)\in\{0,1\}^2.                            \tag{1}
\]

The phase-complete action candidate already contains:

1. one A9 ternary-square token split between link and reserve ownership; and
2. one 192-state cotangent flag/phase carrier with edge, face, handedness, and
   C4 data.

This theorem asks whether equation (1) can be read from those existing
degrees of freedom without introducing another ownership coordinate.

The answer is no.

---

## 2. The physical A9 orbit has one ownership bit

For one A9 state $a=(u,v)$, let

\[
 n(a)=u^2+v^2-u^2v^2,
 \qquad
 c(a)=1-n(a).                                      \tag{2}
\]

The autonomous material-clock carrier has local state

\[
 X=(s_L,s_R,a_{\rm link},a_{\rm reserve})          \tag{3}
\]

and its physical one-token domain obeys

\[
 n(a_{\rm link})+n(a_{\rm reserve})=1.             \tag{4}
\]

Manifestation is exactly link ownership:

\[
 \mathbf1_{(s_L,s_R)\ne(0,0)}
 =n(a_{\rm link})=c(a_{\rm reserve}).              \tag{5}
\]

Likewise,

\[
 n(a_{\rm reserve})=c(a_{\rm link}).               \tag{6}
\]

Thus the apparent link and reserve capacities are not two bits. They are two
names for one binary ownership coordinate and its complement.

---

## 3. Complete invariant-readout classification

The C4 phase rotation and charge-conjugation involution preserve ownership.
On the sixteen physical owned states, their generated action has exactly two
orbits:

\[
 \mathcal O_R:	ext{ reserve owned},\qquad
 \mathcal O_L:	ext{ link owned},                  \tag{7}
\]

each of cardinality eight.

Every symmetry-invariant binary scalar is constant on each orbit. Therefore
there are exactly four such readouts:

\[
 (0,0),\quad(1,1),\quad(0,1),\quad(1,0).           \tag{8}
\]

The first two are constant. The only nonconstant readouts are link ownership
and reserve ownership, which are complements.

Every autonomous period-eight clock orbit spends four states in each
ownership sector. Hence either:

1. the same readout is used twice, giving joint count four by idempotence; or
2. the complementary readouts are used, giving joint count zero.

For two nontrivial half-density permissions, exact factorization would require
joint count two:

\[
 8N_{11}=N_tN_s=16.                                \tag{9}
\]

Neither four nor zero satisfies equation (9). Therefore the current A9
ownership orbit cannot supply a factorized primal/dual pair.

---

## 4. A clock cannot generate its own stalls without another state

Let $F$ be either physical period-eight clock cycle. Attempt to define a
self-admitted map

\[
 G_p(x)=
 \begin{cases}
 F(x),&p(x)=1,\\
 x,&p(x)=0,
 \end{cases}                                       \tag{10}
\]

for a binary predicate $p$ on the same cycle.

If $p(x)=1$ and $p(Fx)=0$, then both $x$ and $Fx$ map to $Fx$, violating
injectivity. Avoiding this collision requires

\[
 p(x)\le p(Fx)                                     \tag{11}
\]

around the entire finite cycle. Cyclic closure forces equality everywhere.
Therefore

\[
 \boxed{p\equiv0\quad\text{or}\quad p\equiv1.}    \tag{12}
\]

The certificate exhausts all $2^8$ masks on both polarity cycles and finds
only these two permutations.

> A reversible clock cannot obtain a variable local admission rate by
> conditionally applying its own successor map from a predicate stored only
> in that same transitive cycle.

A retained environmental controller, delay state, or enlarged cycle is
necessary. This gives a state-space reason why the earlier permission history
could not simply be declared endogenous to the eight-state clock.

---

## 5. The cotangent carrier has no spare scalar permission

One cotangent state is

\[
 f=(d,n,h,p),                                      \tag{13}
\]

where $d$ is a polar edge tangent, $n$ an axial face normal, $h$ a
pseudoscalar handedness, and $p$ a C4 phase. There are 192 states.

The certificate proves that the full signed-cubic and phase action

\[
 O_h\times C_4                                     \tag{14}
\]

is transitive on all 192 states. Therefore every invariant scalar binary
readout is constant.

Handedness is not a counterexample. Every improper cubic transformation sends

\[
 h\mapsto-h.                                       \tag{15}
\]

Using $(1+h)/2$ as a capacity permission would choose an orientation under
reflection. It is a pseudoscalar selector, not the scalar isotropic capacity
required by weak lensing. Phase likewise rotates under C4 and is already used
by the electromagnetic/tensor quadratures.

Consequently the cotangent payload contains the correct edge--face geometry
but no unused invariant occupancy bit that can become $g_s$.

---

## 6. Minimum existing-alphabet repair

Take two **independently owned copies** of the same physical A9 carrier:

\[
 X_P\in\mathcal X_{A9},
 \qquad
 X_D\in\mathcal X_{A9}.                            \tag{16}
\]

Interpret their ownership bits as

\[
 g_t=n(a_{P,\rm link}),
 \qquad
 g_s=n(a_{D,\rm link}).                            \tag{17}
\]

Across the complete $16\times16$ product census,

\[
 N=256,qquad N_t=N_s=128,qquad N_{11}=64,        \tag{18}
\]

so

\[
 \boxed{
 {N_{11}\over N}
 ={N_t\over N}{N_s\over N}
 ={1\over4}.}                                     \tag{19}
\]

Both copies independently retain one complete phase/polarity token and its
inverse ownership transfer. Thus equation (19) requires no new *alphabet*:
each record remains exactly A9.

It does require a second independently owned **placement** of that record. In
the live geometric candidate, the natural interpretation is:

- $X_P$: primal bond/site recurrence ownership;
- $X_D$: dual face/incidence ownership.

The present action has an A9 record on each unoriented C18 bond but has not
declared an independent A9 ownership site on the cotangent dual face. The
repair therefore extends the ownership complex even though it reuses the same
finite local alphabet.

Equation (19) is a type/census witness only. It does not show that one local
permutation dynamically traverses the product with the required sourced
marginals.

---

## 7. Convergence with spin-2 and lensing

Three independent results now converge:

1. Tensor parity requires an even/odd primal--dual stagger for the first-order
   symmetric curl.
2. Weak class-2 lensing requires distinct temporal and spatial permissions.
3. The current A9 plus cotangent payload has no spare invariant scalar bit;
   a second A9 ownership copy on the dual complex is the smallest
   existing-alphabet repair.

This suggests one sharply defined candidate architecture:

\[
 \boxed{
 \text{primal A9 recurrence capacity}
 \quad\leftrightarrow\quad
 \text{dual A9 Hodge/tensor capacity}.}            \tag{20}
\]

An exchange symmetry between the two placements could in principle enforce
equal blocked marginals. The theorem does not derive that symmetry or its
dynamics.

---

## 8. Exact epistemic boundary

### Proved

1. The physical A9 clock contains exactly one invariant nonconstant ownership
   bit up to complementation.
2. Link and reserve capacities cannot furnish factorized primal/dual
   permissions.
3. A partial self-gate of the same period-eight successor is reversible only
   for the constant all/none masks.
4. The cotangent flag/phase carrier contains no nonconstant
   $O_h\times C_4$-invariant scalar permission.
5. A second independently owned A9 copy is sufficient for equal factorized
   binary marginals at the complete-census level.

### Not proved

1. that a dual A9 face record is part of the native ontology;
2. that one action creates and transports its tokens conservatively;
3. that a local primal/dual exchange symmetry produces equal sourced
   marginals;
4. that the dual occupation gates the actual cotangent Hodge incidence;
5. that the same carrier realizes the odd STF tensor partner and TT
   constraints;
6. that a static source produces an inverse-distance capacity field; or
7. that FTD lenses light.

No production or coupling claim is promoted.

---

## 9. Next locked gate

Construct one Moore-local reversible permutation on primal and dual A9
ownership copies that:

1. conserves the complete token, phase, polarity, and reserve ledger;
2. generates a variable local $g_t$ without an external permission word;
3. generates $g_s$ from dual face occupancy;
4. is equivariant under primal/dual exchange and the signed cubic group;
5. preserves the cotangent Maxwell slow space and gives the required spatial
   first-moment factor;
6. simultaneously transports the even/odd STF pair; and
7. yields sourced weak response coefficients before any lensing comparison.

A pass would turn the abstract two-permission price into a concrete finite
ownership action. A no-go would prove that even a duplicated A9 placement is
insufficient and force a larger local controller.

**Successor pass.** The
[dual-A9 skew generator theorem](THEOREM_DUAL_A9_SKEW_CAPACITY_CLOCK_GENERATOR_AND_HOMOGENEOUS_FACTOR_PASS_v1.md)
constructs such a total local permutation at the homogeneous half-admission
fixture. One A9 copy advances as a retained controller; its capacity admits
the other copy's physical clock. All 32 deterministic orbits have
$(N,N_t,N_s,N_{11})=(16,8,8,4)$, and primal/dual exchange is exact after
exchanging the structural orientation sector. Variable sourced marginals and
the physical cotangent/Hodge lift remain open.
