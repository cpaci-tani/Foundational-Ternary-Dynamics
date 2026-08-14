# Pre-registration — Existing oriented-rail finite winding carrier boundary v1

**Identifier:** `FTD-0960`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

Can the signed crossing current and lifted winding required by FTD-0958/0959
be carried by already-booked ternary/history rails without adopting another
public state type?

The test must distinguish:

1. exact finite-horizon retention of the crossing word and net winding;
2. indefinite winding on expanding causal support;
3. a compact fixed-rail counter over `|w|<=W`; and
4. native acquisition/loading from the physical crossing.

Success in one category may not be reported as success in another.

## 2. Frozen sources

The certificate must byte-check these sources:

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_AND_ONE_SHOT_BOUNDARY_v1.md` | `E70F2AD61BFA1C8BBFF4EA03DCF0312B8F96224ECF2453FDF4B81B0FEA845CA1` |
| `THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `THEOREM_FINITE_CAPACITY_LOCAL_REVERSIBLE_OCCUPANCY_CARRY_TRILEMMA_v1.md` | `A89DE2964B7D48100EC850547D00BB540D05F1166CF18CABE654EB9D26917548` |
| `THEOREM_GLOBAL_ISOCHRONY_LIFT_AND_ORIENTED_CROSSING_LATCH_BOUNDARY_v1.md` | `746F855A432D7E662236315066115174493554285CD3FC25071B892A05AEA68E` |

No engine source, production tick, constant, toggle, or ontology type may be
changed by this protocol.

## 3. Frozen specialization of the existing rail

Choose one already-priced oriented Moore ray `nu_0` and one existing binary
lane. Use the opposite channel labels `+nu_0` and `-nu_0` for the two signs of
the crossing current

\[
 s_t=\operatorname{sign}\Pi_t\in\{-1,0,+1\}.
\]

At rail update `t`, stream every retained token outward by one causal depth and
insert:

- one token in the `+nu_0` channel when `s_t=+1`;
- one token in the `-nu_0` channel when `s_t=-1`; and
- no token when `s_t=0`.

For a declared horizon `N`, the frozen candidate state is

\[
 c_\sigma^{(N)}(j)=
 \begin{cases}
 1,&s_{N-1-j}=\sigma,\\
 0,&\text{otherwise},
 \end{cases}
 \quad \sigma\in\{-1,+1\},\quad 0\le j<N.             \tag{1}
\]

The global update count supplies the positions of zero entries. No completed
infinite rail is assumed: a horizon `N` requires only the finite causal prefix
used by (1).

## 4. Frozen mathematical gates

### G1 — Source and marker integrity

The four hashes in section 2 and the scope markers in this protocol must pass.

### G2 — Exact word injection and inverse

For every fixed `N`, equation (1) must be injective on
`{-1,0,+1}^N`. Reading the two channel bits at depths `N-1-t` must reconstruct
every `s_t`. Reverse streaming followed by the inherited source swap must be
the exact inverse.

The certificate may exhaust finite words only as an exact implementation
check; no numerical fitting, tolerance, or near-miss search is permitted.

### G3 — Lifted winding readout

With retained initial winding `w_0`, define

\[
 \widehat w_N=w_0+
 \sum_{j=0}^{N-1}\left(c_+^{(N)}(j)-c_-^{(N)}(j)\right). \tag{2}
\]

The certificate must prove

\[
 \widehat w_N=w_0+\sum_{t=0}^{N-1}s_t,                 \tag{3}
\]

which is exactly the iterated FTD-0959 atlas update `w'=w+s`. Opposite
crossing words with equal event count must remain distinct even if their net
winding agrees.

### G4 — Locality, covariance, and energy inheritance

The shift/swap must inherit radius-one causal locality and exact inverse from
FTD-0941. Exchanging `nu_0<->-nu_0` and `s<->-s` must reverse the winding
readout. For positive selected token scale `epsilon_*`, the retained carrier
energy must be

\[
 H_{\rm cross}={\epsilon_*\over2}
 \sum_j(c_+(j)+c_-(j)),                               \tag{4}
\]

and must be preserved by transport. Loading remains a separate transaction
that must be paid by the source or a reservoir.

### G5 — Finite-horizon capacity and expanding-support price

For a word with `m` nonzero crossings, equation (1) must contain exactly `m`
tokens and energy `m epsilon_*/2`. A finite prefix of length `N` retains at
most `N` rail-update slots. Continued exact history requires a fresh causal
front, tail export, or backpressure.

This is a direct-history carrier, not the information-theoretically compact
ternary register allowed by `3^n>=2W+1`.

### G6 — Fixed compact-counter boundary

Let `E(-W),...,E(W)` be distinct encodings in a fixed finite register. A
claimed reversible positive increment may not both satisfy

\[
 U_+E(w)=E(w+1),\quad -W\le w<W,                       \tag{5}
\]

and hold the boundary fixed, `U_+E(W)=E(W)`, because `E(W-1)` and `E(W)`
would be two preimages of `E(W)`. Cyclic wrap retains only a residue and is not
integer winding.

Therefore a compact finite counter requires an explicit overflow/backpressure
state or port. Any carry or cancellation compression must retain its inverse
history in that register, an output, or an environment.

The FTD-0874 bond law `R(a,b)=(-b,a)` and its compositions are signed
coordinate permutations preserving `sum_j x_j^2`. They transport prepared
labels and backpressure exactly but do not by themselves supply a nonlinear
compact carry/overflow transaction.

### G7 — Acquisition and production firewall

The specialization may read only the signed local crossing label and the
selected rail/channel labels. It may not read target winding, Born weights,
future crossings, or a completed history.

The following remain open unless independently proved:

- physical acquisition of `sign(Pi)` into the event port;
- payment of equation (4) by a native source/reservoir;
- moving-hub attachment, routing, collisions, recycling, and finite boundary;
- compact balanced-ternary carry hardware;
- the active controller gearbox, its work, reserve, reaction, and inverse;
- any identification with `G*`; and
- production, Born/Bell, Lorentz hiding, or completeness.

## 5. Frozen outcomes

- **Outcome A:** existing fixed hardware realizes an exact compact finite
  winding counter with explicit reversible overflow/backpressure and no new
  public type, while the expanding carrier also passes.
- **Outcome B:** the existing expanding oriented rail exactly retains every
  finite crossing word and net winding, but compact carry and/or native loading
  remain priced/open.
- **Outcome C:** even the expanding existing-rail specialization fails word,
  inverse, winding, locality, or energy gates.
- **Outcome D:** certificate or source-integrity failure prevents
  classification.

The expected result is Outcome B. The result may not promote the selected ray,
token energy, latch acquisition, or active gearbox into derived substrate
dynamics.
