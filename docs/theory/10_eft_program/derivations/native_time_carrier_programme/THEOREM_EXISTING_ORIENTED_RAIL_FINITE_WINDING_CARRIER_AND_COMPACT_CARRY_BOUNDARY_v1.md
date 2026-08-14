# FTD-0960/0961 — Existing oriented-rail finite winding carrier and compact-carry boundary v1

**Date:** 2026-08-11  
**Status:** `[THEOREM, CONDITIONAL — EXISTING EXPANDING ORIENTED RAIL RETAINS EVERY FINITE CROSSING WORD AND NET WINDING]` +
`[THEOREM — EXACT INVERSE/LOCALITY/ORIENTATION/ENERGY ACCOUNTING]` +
`[CLOSED NEGATIVE — REVERSIBLE FAIL-CLOSED INCREMENT ON A FIXED REGISTER WITHOUT OVERFLOW STATE]` +
`[BOUNDARY — EXISTING PARITY TRANSPORT IS NOT A COMPACT CARRY TRANSACTION]` +
`[OPEN — NATIVE ACQUISITION, COMPACT CARRY, ACTIVE GEARBOX, G*]`  
**Verdict:** `OUTCOME_B_EXISTING_EXPANDING_HISTORY_CARRIER_EXACT_COMPACT_NATIVE_GEARBOX_OPEN`

## 1. Result

FTD does not need another public memory type to retain a declared finite
history of lifted-clock crossings. Conditional on the already-selected
oriented history-channel interface, choose one existing Moore ray `nu_0` and
route the signed crossing current

\[
 s_t=\operatorname{sign}\Pi_t\in\{-1,0,+1\}             \tag{1}
\]

into the opposite channels `+nu_0` and `-nu_0`. Stream the retained tokens
outward by one causal depth per registered rail update. After `N` updates,

\[
 c_\sigma^{(N)}(j)=
 \begin{cases}
 1,&s_{N-1-j}=\sigma,\\
 0,&\text{otherwise},
 \end{cases}
 \qquad \sigma\in\{-1,+1\}.                           \tag{2}
\]

The two channel bits at each known depth reconstruct the complete ternary
crossing word. Their count difference gives

\[
 \boxed{
 w_N=w_0+\sum_{j=0}^{N-1}
       \left(c_+^{(N)}(j)-c_-^{(N)}(j)\right)
     =w_0+\sum_{t=0}^{N-1}s_t.}                       \tag{3}
\]

Equation (3) is exactly the iterated oriented atlas rule `w'=w+s` from
FTD-0959. The substrate can therefore distinguish clockwise from
counterclockwise at the memory interface: the distinction is the choice of
opposite oriented channel. The even clutch `s^2` still cannot make that
choice.

This closes a conditional finite-horizon carrier, not native acquisition or a
compact counter. The carrier retains each nonzero crossing as a separate
token, so canceling `(+1,-1)` history remains present even though its net
winding is zero.

## 2. Exact injection and inverse

For fixed `N`, every depth `j` has exactly one of three valid states:

\[
 (c_+(j),c_-(j))\in\{(1,0),(0,0),(0,1)\}.             \tag{4}
\]

Reading depth `N-1-t` maps these states back to `+1,0,-1`, respectively.
Thus (2) is an injection

\[
 \{-1,0,+1\}^N\hookrightarrow
 \bigl(\{0,1\}^2\bigr)^N                              \tag{5}
\]

onto the valid subspace (4), and the depth readout is its exact inverse.

The update has the recursive form

\[
 c_+'=(\mathbf1_{s=+1},c_+(0),c_+(1),\ldots),
 \qquad
 c_-'=(\mathbf1_{s=-1},c_-(0),c_-(1),\ldots).          \tag{6}
\]

Reverse streaming returns the depth-zero pair to the source port and recovers
`s`. This is the existing FTD-0941 swap/stream inverse specialized to two
opposite channels. Every output depth depends on only the previous adjacent
depth; the update is radius-one causal.

No ontic completed infinite rail is used. For any preregistered finite
horizon, only the finite causal prefix reached by (6) is required. Continuing
indefinitely requires an advancing blank front, tail export, or exact
backpressure.

## 3. Winding and orientation

Summing (2) gives

\[
 \sum_j c_+^{(N)}(j)=\#\{t:s_t=+1\},
 \qquad
 \sum_j c_-^{(N)}(j)=\#\{t:s_t=-1\}.                  \tag{7}
\]

Subtracting proves (3). Exchanging `nu_0<->-nu_0` swaps the two channel
families and sends

\[
 s_t\mapsto-s_t,
 \qquad
 w_N-w_0\mapsto-(w_N-w_0).                            \tag{8}
\]

The words `(+1,-1)` and `(-1,+1)` both have zero net winding, but occupy
different channel-depth states. Thus the carrier preserves orientation and
order, not merely the aggregate integer.

This is stronger memory than the lift strictly needs. The harmonic lift reads
only the count difference, while exact reversibility keeps the complete word.
That surplus is the cost of avoiding hidden erasure.

## 4. Energy and support price

For the already-selected positive token scale `epsilon_*`, the specialization
inherits

\[
 H_{\rm cross}={\epsilon_*\over2}
 \sum_j(c_+(j)+c_-(j)).                               \tag{9}
\]

Streaming and source-port swap preserve (9). Loading a nonzero crossing adds
one token and therefore requires a separate debit of `epsilon_*/2` from the
source or a named reservoir. A zero crossing adds no token.

After a word containing `m` nonzero crossings,

\[
 \#\text{tokens}=m,
 \qquad H_{\rm cross}=m\epsilon_*/2.                  \tag{10}
\]

Consequently a long canceling history can have `w_N=w_0` while retaining
arbitrarily many tokens. The construction spatializes history; it does not
compress it into the current winding.

The scale `epsilon_*`, the loading transaction, native reservoir, routing,
recycling, and finite-tail completion remain selected/open. Equation (10) is
an accounting identity inside the chosen carrier, not a derived physical
normalization.

## 5. Why the existing parity rail is transport, not compact carry

The FTD-0874 bond update is

\[
 R(a,b)=(-b,a).                                       \tag{11}
\]

Every bond layer and every composition of such layers is a signed coordinate
permutation. It preserves

\[
 Q(x)=\sum_jx_j^2.                                    \tag{12}
\]

It therefore moves, reverses, and backpressures prepared labels exactly. It
does not merge several input trits into a balanced-ternary carry, erase a
canceling pair, or manufacture the missing overflow record. Those operations
need another reversible transaction whose output/history is explicit.

This does not prove that compact ternary carry is impossible. It proves that
the currently registered parity-rotation layers are not, by themselves, that
transaction.

## 6. Fixed-register fail-closed no-go

Let a finite register contain distinct encodings

\[
 E(-W),E(-W+1),\ldots,E(W).                           \tag{13}
\]

Suppose a total reversible positive-increment map obeys

\[
 U_+E(w)=E(w+1),\qquad -W\le w<W.                    \tag{14}
\]

If it also “fails closed” by holding the boundary,

\[
 U_+E(W)=E(W),                                        \tag{15}
\]

then both `E(W-1)` and `E(W)` map to `E(W)`, contradicting injectivity. A
cyclic update `E(W)->E(-W)` is reversible but stores only a residue; it is not
the lifted integer.

Therefore a compact fixed-register counter must expose at least one of:

1. an overflow/backpressure state or output port;
2. a larger retained carry/history register;
3. a reversible export environment; or
4. a declared modular identification of winding values.

The information bound

\[
 3^n\ge2W+1                                           \tag{16}
\]

remains necessary for `n` ternary cells, but it is not a dynamics. FTD-0960
does not construct balanced-ternary increment/decrement hardware merely from
the fact that enough configurations exist.

## 7. What is now closed

Theorem-grade or conditional theorem-grade:

- the existing opposite-channel specialization injectively stores every
  fixed finite crossing word;
- reverse streaming reconstructs the word exactly;
- the oriented token-count difference equals the lifted winding update;
- opposite zero-net words remain distinct;
- transport is radius-one causal and sign-covariant;
- token count and selected carrier-energy accounting are exact;
- fixed boundary hold after an increment is incompatible with reversibility
  unless overflow/backpressure is retained; and
- the existing parity layers are signed permutations, not compact carry
  hardware.

No new public ontology type is required for this conditional finite-horizon
memory. The selected ray/channel specialization remains an architectural
choice, not a consequence of P1--P5.

## 8. What remains open

- native acquisition of `sign(Pi)` into the existing event port;
- source/reservoir payment for token loading;
- compact reversible balanced-ternary carry and its overflow protocol;
- moving-body/hub attachment, routing, collisions, finite boundary, and
  recycling;
- whether full history should remain, thermalize, or be identified under a
  physical unactualization law;
- the active no-reset controller gearbox with exact work, reserve, reciprocal
  reaction, and inverse;
- any relation between that gearbox and `G*`;
- full nonlinear repeated-map stability and positive attraction export;
- production integration, Born/Bell recovery, Lorentz hiding, and
  completeness.

The next cheapest test is no longer “does a winding-memory type exist?” It is:

> Can the signed crossing latch load the existing oriented channel through a
> positive local canonical transaction, and can the same transaction engage
> the controller without reset while closing work, reserve, reaction, and
> inverse?

## 9. Certificate record

The locked FTD-0960 protocol has SHA-256
`B8BDCCCDEB5ECFE4FE2B9CAAD1C00AAF69C5E5F6CD0E4266866FBDF79A6ADDBA`.
The immutable parent certificate has SHA-256
`EAF1890622606B584EB3473FB6D5444C52CAB79B38AA8E70A808AE28CA6A28C8`
and first reported `55/60`, Outcome D, on five source-marker normalization
defects after every mathematical gate passed.

The FTD-0961 repair protocol has SHA-256
`2A5D0CE0857C5EB218D979071A313A02C66EFD3681F0679F57EA25BBBF9CE336`.
The verifier-only wrapper has SHA-256
`E87D4D4BB24BD5C2E237B2ECAAE77D6BC5733D36A7DD197DDD0279FB98E9C0B4`
and passes inherited `60/60` plus repair integrity `19/19`, Outcome B.

No engine, CMake, production type, default tick, constant, toggle, Born law,
or `G*` identification changed.
