# Theorem — Finite-capacity local reversible occupancy-carry trilemma v1

**Identifier:** `FTD-0941`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — FIXED-BOUNDED FINITE-ALPHABET UNBOUNDED-WINDING NO-GO]` +
`[CLASSIFICATION — INTEGER / LABEL / FINITE-LINK CARRY]` +
`[REFERENCE CONSTRUCTION — FINITE-PER-PORT REVERSIBLE MOORE-TOKEN EXPORT]` +
`[THEOREM — FIXED-HUB ORDERED C4 HISTORY AND ORIENTATION RETENTION]` +
`[BOUNDARY — EXPANDING SUPPORT / FREE ENERGY SCALE / BODY-LOCAL CARRY OPEN]`  
**Production status:** unchanged

## 1. Result

An exact, arbitrarily long integer winding record cannot live in a fixed
bounded region with a finite distinguishable local alphabet. If the region
has `r` sites and the alphabet has `q` symbols, it has only `q^r` complete
states. The `q^r+1` winding values

\[
 0,1,\ldots,q^r                                      \tag{1}
\]

cannot be encoded injectively. A reversible update on the same state set is a
permutation, so every orbit is periodic and can retain only a finite residue,
not an unbounded traversal count.

Therefore exact persistent winding has a strict trilemma:

\[
 \boxed{
 \text{unbounded local state}
 \quad\text{or}\quad
 \text{expanding/exported support}
 \quad\text{or}\quad
 \text{identified/lost histories}.}                  \tag{2}
\]

The three FTD-0940 candidate owners instantiate this boundary differently.
An integer cumulative flux is exact but unbounded. A finite body label can
track identity but not unwrapped winding and is not canonically supplied by
the unlabelled occupancy record. A finite link register is modular, saturates
noninjectively, or must apply backpressure.

There is an exact local reversible escape through the second branch. A
finite-alphabet Moore-token rail can export every registered neutral-body hop
into expanding causal support. Each port is binary, the update is a local
permutation, the two occupancy lanes carry the exact FTD-0940 aggregate `2d`,
and the complete direction word remains recoverable at a fixed hub. Opposite
C4 orientations remain distinct even though each has zero net displacement.

This is a reference construction, not a production law. It establishes the
minimum information architecture and prices it: finite capacity per port is
compatible with exact history only because occupied support and token energy
grow with the retained history.

## 2. Fixed-support no-go

Let a carrier occupy a fixed finite region `R`, with

\[
 X=A^R,\qquad |A|=q\ge2,qquad |R|=r<\infty.           \tag{3}
\]

Suppose exact winding retention supplies maps

\[
 E:\mathbb Z\to X,
 \qquad w:X\to\mathbb Z,
 \qquad w(E(n))=n.                                    \tag{4}
\]

Equation (4) makes `E` injective. But equation (1) already contains more
inputs than `X` has states. Hence no such `E` exists.

The dynamical statement is the same obstruction. Every reversible map
`U:X->X` is a permutation. For every `x in X`, there is an `m<=|X|` such that

\[
 U^m x=x.                                             \tag{5}
\]

A readout satisfying `w(U^n x)=w(x)+n` would give both
`w(U^m x)=w(x)+m` and `w(U^m x)=w(x)`, a contradiction.

This is deliberately scoped to finite distinguishable capacity. A real
coordinate with exact infinite precision, an integer counter, a growing
support region, or an open environment lies outside the theorem. FTD-0570's
exact-real natural extensions are therefore not contradicted.

## 3. Classification of the three proposed owners

### 3.1 Cumulative occupancy flux

For an oriented link or cut,

\[
 W_e(N)=\sum_{n=0}^{N-1}K_{\nu,e}(n)                 \tag{6}
\]

has exact composition and reversal. It is local per update and transforms
with the oriented link family under the signed cubic group. Its state space is
`Z`, however, so it fails finite local capacity.

Replacing it by `W_e mod q` preserves only a residue. Saturation maps two
inputs to the full state and is noninjective. Aggregate flux also forgets a
canceling `(+1,-1)` pair if individual histories matter, although it correctly
retains the net winding.

### 3.2 Transported body label

On a periodic computational quotient of side `L`, a finite label set `B`
gives at most

\[
 |B|L^3                                                \tag{7}
\]

labelled position states. Repeated circuits again exceed the capacity. An
unwrapped coordinate repairs this only by reintroducing an integer.

There is a second boundary. If two indistinguishable bodies meet, “pass
through” and “exchange labels” have the same unlabelled `s^2` endpoint. A
labelled collision law can choose between them, but that label and law are
additional bookkeeping. The actual ternary/occupancy algebra does not select
them canonically.

### 3.3 Finite link carry

A finite link alphabet under a reversible update has the periodicity (5). It
can retain a residue class or a bounded token occupancy, not every integer.
Once a local port reaches its multiplicity limit, another identical incoming
token requires one of four honest outcomes:

1. reject or delay it;
2. return backpressure;
3. export it to another port; or
4. lose injectivity.

An exact integer link counter remains a valid reference ledger, as in
FTD-0897, but is not a finite-capacity realization.

## 4. Reversible Moore-token export

Let

\[
 \mathcal M=\{-1,0,+1\}^3\setminus\{0\}             \tag{8}
\]

be the 26 oriented Moore directions. For each direction `nu`, each of two
occupancy lanes `b`, and causal depth `j in Z`, use binary carrier ports
`c_{nu,b}(j)` and a binary event port `a_{nu,b}`.

At depth zero first swap

\[
 (a_{\nu,b},c_{\nu,b}(0))
 \mapsto(c_{\nu,b}(0),a_{\nu,b}),                    \tag{9}
\]

then stream

\[
 c'_{\nu,b}(j+1)=\widetilde c_{\nu,b}(j).            \tag{10}
\]

Both operations are permutations. The exact inverse is the negative-depth
stream followed by the same swap. Equation (10) advances one Moore edge when
depth `j` is embedded on the ray `j nu`; thus the map is radius-one causal.

Here `j in Z` is coordinate notation for locally extendable directed
adjacency, not an ontic completed bi-infinite rail. Every state used by the
certificate has finite support; after any finite event history only finitely
many depths are occupied. The construction therefore needs the next adjacent
site as the causal front advances, not a pre-existing completed totality.

A blank incoming rail clears the source port and sends the event outward. If
the depth-zero carrier port is occupied, equation (9) returns its token to the
event port rather than overwriting it. On a finite periodic rail, a returning
token therefore becomes exact backpressure.

This is the finite-alphabet version of the architecture isolated in FTD-0852.
It is more narrowly registered here: two binary lanes and direction words,
not arbitrary real event amplitudes.

## 5. Exact occupancy carry and orientation memory

For a registered neutral hop `d`, load one token in each of the two `(d,b)`
event lanes. After a successful blank-port transfer,

\[
 C=\sum_{\nu,b,j}c_{\nu,b}(j)\nu=2d.                \tag{11}
\]

This equals the integrated FTD-0940 occupancy current. Loading `-d` gives
`-2d`. The certificate verifies equation (11) for all twelve Moore-edge
directions, including the four live FTD-0936 directions.

For a fixed hub with blank incoming rails, a token emitted at tick `t` lies at
depth `N-t` after `N` ticks. Reading channel and depth therefore reconstructs
the complete ordered direction word. In particular, the registered words

\[
 \begin{aligned}
 w_+={}&((-1,1,0),(-1,-1,0),(1,-1,0),(1,1,0)),\\
 w_-={}&-\operatorname{rev}(w_+)
 \end{aligned}                                        \tag{12}
\]

both have vector sum zero but occupy different channel-depth states. The
carrier distinguishes clockwise from counterclockwise through ordered
transport, not through the symmetric square or net displacement.

The fixed-hub condition matters. A moving or colliding body does not yet have
a derived hub attachment, routing rule, or identity. Equation (12) proves a
reference memory, not body-local production carry.

## 6. Cubic covariance, collisions, and energy

Every signed permutation of the spatial axes permutes the 26 channel labels.
Equations (9)--(10) do not otherwise depend on `nu`, so the update commutes
with all 48 signed cubic transformations. Different directions compose in
different ports. The two neutral-occupancy lanes stream without merging.

A third simultaneous identical occupancy token exceeds the registered two
lanes. The construction must serialize, add a lane, or reject that event; it
does not silently superpose multiplicity. Consequently collision closure is
exact only inside the declared capacity and backpressure rules.

For any positive normalization `epsilon_*`, define

\[
 H_{\rm tok}={\epsilon_*\over2}
 \sum_{\nu,b}\left(a_{\nu,b}^2+
 \sum_jc_{\nu,b}(j)^2\right).                         \tag{13}
\]

Swap and streaming preserve equation (13) exactly. Loading an event port is a
separate transaction: it must debit the source or another reservoir. The
carrier algebra leaves `epsilon_*` free and does not identify it with the
FTD-0940 Hodge work or the FTD-0933 wake.

Logical deletion `0->0, 1->0` is noninjective. This proves only that deletion
is not a reversible update of the registered carrier. No thermodynamic
`kT ln 2` cost or temperature law follows.

## 7. The capacity price

After `N` accepted neutral hops with no cancellation or return, the rail holds
`2N` tokens and reaches causal depth `N`. Local capacity is finite, but total
occupied support and equation (13)'s energy grow linearly:

\[
 \#\text{tokens}=2N,
 \qquad H_{\rm tok}=N\epsilon_*                       \tag{14}
\]

for blank event ports after each transfer. Thus the construction does not
hide an unbounded integer in a finite box. It spatializes history in the
uncontained substrate and pays for every retained token.

This also separates two physical roles:

- **winding/history** remembers how the system arrived;
- **instantaneous momentum or clock phase** describes what it can do now.

They need not be the same variable. Treating accumulated winding as mandatory
body-local momentum would force unnecessary infinite local memory.

## 8. Global calendar and local recurrence corollary

A finite local recursive clock can run indefinitely only by revisiting its
phase states. It cannot also encode its absolute cycle number in the same
fixed finite state. FTD already has the correct type separation:

1. the ontic tick `n` supplies global update order;
2. a local clock phase supplies recurrent eligibility; and
3. an outward history channel may retain selected event details at an
   increasing support/energy cost.

Accordingly `G*` may set a local critical period factor without becoming the
global time coordinate. Nothing in this theorem derives a finite-tick `G*`
clock or the gearbox between `n` and that phase; it removes the false
requirement that the local clock itself remember all elapsed cycles.

This is also the exact place where reduced “unactualization” can occur. A
local record may discard obsolete path detail while the full reversible
ontology exports it. Which details are exported, thermalized, or genuinely
identified remains dynamical, not semantic.

## 9. Closed and open

### Closed

1. `[THEOREM]` fixed bounded finite-alphabet state cannot retain arbitrary
   integer winding injectively.
2. `[THEOREM]` a reversible finite-state carrier is periodic and retains at
   most a finite quotient of traversal count.
3. `[CLASSIFICATION]` cumulative integer flux is exact but unbounded; finite
   body labels do not unwrap winding; finite links are modular or
   backpressured.
4. `[REFERENCE CONSTRUCTION]` finite-per-port Moore-token export is exactly
   local, reversible, signed-cubic covariant, and energy preserving.
5. `[THEOREM]` two occupancy lanes reproduce `2d` for all twelve edge
   directions and reverse exactly.
6. `[THEOREM]` the fixed-hub carrier reconstructs ordered direction words and
   distinguishes opposite zero-net C4 orientations.
7. `[THEOREM]` exact history retention in this construction consumes expanding
   support and token energy.

### Open

1. a production-native deposit from the FTD-0940 occupancy chord into an
   existing L/R field channel;
2. a common reversible action paying event-port loading from the source/
   Hodge transaction without reading the wake;
3. body attachment, moving-hub routing, collisions beyond two local lanes,
   recovery, and port recycling;
4. whether native real-field pulses remain distinguishable under
   superposition or require a nonlinear protected carrier/new velocity-port
   type;
5. exact FTD-0933 wake identification and equal/opposite field--matter
   impulse;
6. `epsilon_*`, `p_*`, inertia, mass, `gamma`, and integer-hop normalization;
7. finite-tick `G*` synchronization, preferred-order hiding, Born/Bell
   recovery, Lorentz recovery, and completeness.

No production law, ontology type, selected physical momentum, energy scale,
or `G*` identification is added.

## 10. Certificate

The frozen preregistration has SHA-256
`46F9F124C5324CDB35F34E7F228D451630460C586FC2FE62F30563EBE218AB45`.
The proof certificate
`scripts/proofs/proof_finite_capacity_local_reversible_occupancy_carry_trilemma.py`
has SHA-256
`0256BF01710F8D6B9FFCE717FA8CB6A0E0E0B0715F2BC2F004380B9A5374FBC7`
and passed `681/681` exact gates on its first execution. The registered verdict
is Outcome B.

## 11. Next acceptance gate

Pre-register an existing-hardware carrier-realization classifier before any
production change. It must test whether the current L/R real fields can
implement the exact source swap, outward transport, collision/backpressure,
and separate energy ledger of equations (9)--(13) while being driven only by
the pre-hop occupancy/Hodge chord. Linear superposition that cancels distinct
tokens is a failure, not a history carrier. If the current fields fail, the
result must price the minimum additional velocity-port or protected-pulse type
rather than silently adding it.

Only a surviving realization may be placed under one temporal ordering with
the quartic source and FTD-0933 companion wake.

```text
FIXED_BOUNDED_FINITE_ALPHABET_RETAINS_UNBOUNDED_WINDING=FALSE
CUMULATIVE_INTEGER_FLUX=EXACT_BUT_UNBOUNDED_LOCAL_CAPACITY
FINITE_BODY_LABEL=IDENTITY_BOOKKEEPING_NOT_UNWRAPPED_WINDING
FINITE_LINK_REGISTER=MODULAR_OR_BACKPRESSURED
REVERSIBLE_MOORE_TOKEN_EXPORT=FINITE_PER_PORT_EXPANDING_SUPPORT
OCCUPANCY_CARRY_PER_HOP=2*d
CLOCKWISE_COUNTERCLOCKWISE_FIXED_HUB_HISTORY=DISTINGUISHED
TOKEN_ENERGY=EXACT_WITH_FREE_NORMALIZATION
GLOBAL_TICK_AND_LOCAL_RECURRENT_PHASE=SEPARATE_TYPES
PRODUCTION_BODY_LOCAL_CARRY=OPEN
EXACT_FTD0933_WAKE_IDENTIFICATION=OPEN
PHYSICAL_MOMENTUM_MASS_GAMMA=OPEN
NEW_ONTOLOGY_TYPE_ADOPTED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```
