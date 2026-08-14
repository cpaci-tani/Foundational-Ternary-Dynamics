# FTD-0941 — Preregistration: finite-capacity local reversible occupancy-carry trilemma v1

**Identifier:** `FTD-0941`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact finite-alphabet/fixed-support winding no-go; exact comparison of
cumulative face flux, transported body labels, and separately represented link
carry; finite-per-port reversible Moore-token export witness; injectivity,
reversal, collision/backpressure, capacity, logical erasure, cubic covariance,
locality, and quadratic token-energy gates; no numerical near-miss search,
fit, continuum limit, target wake, physical momentum scale, production change,
new ontology adoption, `G*`, Born, Bell, measurement context, or outcome read

## 1. Question

FTD-0940 proves that the oriented occupancy current owns one neutral-body hop,
but that the instantaneous ternary/occupancy record does not retain accumulated
winding. It requests a comparison of three possible persistent owners:

1. cumulative face flux with no body identity;
2. a locally transported body/worldline label; and
3. a separately represented link/carry state.

The present protocol asks the logically prior question: can an exact,
arbitrarily long integer winding record be retained by a fixed bounded region
whose local state alphabet is finite, while the update remains local and
reversible? If not, what is the minimum honest escape route already suggested
by the causal-history work?

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_PHASE_GATED_NEUTRAL_C4_HODGE_CHORD_AND_OCCUPANCY_CARRY_BOUNDARY_v1.md` | `13C3A820AE368CCABCF5B5DC34B2CBA869B951899B1343AAD4CFD066BCBC3299` |
| `proof_phase_gated_neutral_c4_hodge_chord_occupancy_carry_boundary_v2.py` | `412BB20D5BD14918F81892CB1EBF4495866E16E8B7544443235CC9E93AA6B5B8` |
| `THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `proof_causal_odd_pulse_history_carrier.py` | `9E1238C161851798442D75607A81E80346FFD6CBD16F9F13194FDC311FD9920D` |
| `THEOREM_RECIPROCAL_CARRY_RESERVOIR_AND_LOCAL_IMPULSE_LEDGER_BOUNDARY_v1.md` | `8696F6024CE6ED49120DF6A238F98C8C804AA7B8C441BCA83B5AFDCE111C6048` |
| `proof_reciprocal_carry_reservoir_local_impulse_ledger.py` | `A12998C3E6599BD76AA6F36615A31B1BED37EFE206CAE39ADC0E51F658A89C19` |
| `THEOREM_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md` | `4E00155889BAD84D3ED4A7B907BFBC86589DEA6873A24529519ADE310DC9CEFB` |
| `proof_quasilocal_companion_preparation_reversible_history_formation_boundary.py` | `AE6B5A068C9F1A0F0F81A73DB2EB037EF13F49F31845070B833602558B4AF0A7` |
| `THEOREM_CONFIGURATION_SPACE_CARRIER_NECESSITY.md` | `9FCD2E7AA89C8B38339D730B04AAD2A9797F40E3EDD08ACA3B5C9CFCB4996FBD` |
| `proof_configuration_space_carrier.py` | `A309DCFDD50974B3F3C7177D6365F8FBB5BF08C30C4A6CD932DC5FDB399F87CE` |

The certificate must fail closed on any source drift.

## 3. Registered capacity notion

A **fixed bounded finite-alphabet carrier** occupies a fixed finite region
`R`, with `r=|R|`, and has a finite local alphabet `A`, with `q=|A|>=2`.
Its complete distinguishable state set is

\[
 X=A^R,\qquad |X|=q^r.                                \tag{1}
\]

Exact retention of integer winding means there is an encoding
`E: Z -> X` and a readout `w: X -> Z` with `w(E(n))=n` for every integer `n`.
The certificate must prove that this is impossible by applying the pigeonhole
principle to `0,...,q^r`.

The dynamical form is also registered. A reversible autonomous update on `X`
is a permutation. Every orbit is periodic with period at most `|X|`, so no
readout can increase by one on every traversal for arbitrarily many traversals.
This is scoped to finite distinguishable capacity. It does not apply to an
exact real coordinate, an unbounded integer, or a support region that grows
with history.

## 4. Three branch classifier

### 4.1 Cumulative face flux without identity

For an oriented cut or link `e`, define

\[
 W_e(N)=\sum_{n=0}^{N-1}K_{\nu,e}(n).                 \tag{2}
\]

Integer addition gives exact composition and reversal, and the link family
transforms covariantly under signed cubic permutations. But `W_e in Z` has
unbounded local capacity. Replacing it by `W_e mod q` identifies `W_e` and
`W_e+q`; saturation is noninjective. Aggregate flux also cannot distinguish
a `(+1,-1)` pair of crossings from no crossing if individual histories are
required, although it retains the exact net winding.

### 4.2 Transported body/worldline label

On a finite periodic quotient with `L^3` positions and a finite label set `B`,
the labelled body state has at most `|B|L^3` values. Repeated circuits therefore
cannot retain an arbitrary unwrapped winding. A label can preserve identity
through a collision only if the collision law acts on the labels; the
unlabelled ternary/occupancy record does not canonically choose whether two
indistinguishable bodies pass through or exchange labels. The certificate must
classify a body label as finite bookkeeping, not a derived winding owner.

### 4.3 Separately represented link carry

A finite link alphabet and reversible local update again form a finite
permutation orbit and retain at most a residue class. An exact integer link
counter is sufficient but not finite-capacity. A bounded multiplicity port
cannot accept one more identical simultaneous token after it is full without
one of: rejection/backpressure, export to another port, a larger alphabet, or
noninjective loss.

## 5. Finite-per-site reversible Moore-token export witness

The certificate must test a reference escape through **expanding causal
support**, not claim it as production dynamics.

Let

\[
 \mathcal M=\{\nu\in\{-1,0,+1\}^3:\nu\ne0\}         \tag{3}
\]

be the 26 oriented Moore directions. For each `nu`, occupancy lane `b in
{1,2}`, and integer causal depth `j`, use binary event and carrier ports
`a_{nu,b}` and `c_{nu,b}(j)`. The two lanes represent the two occupied sites
of the registered neutral dipole; they do not generalize arbitrary local
multiplicity.

At the registered source depth, apply a local swap

\[
 (a_{\nu,b},c_{\nu,b}(0))
 \longmapsto
 (c_{\nu,b}(0),a_{\nu,b}),                            \tag{4}
\]

then stream every carrier port one causal cell,

\[
 c'_{\nu,b}(j+1)=\widetilde c_{\nu,b}(j).             \tag{5}
\]

Equations (4)--(5) are a composition of permutations. Their inverse is the
opposite stream followed by the same swap. A blank incoming half-rail clears
the event port and exports the token. A returning or occupied source port is
not overwritten: it appears at the event port as exact backpressure.

For a neutral hop in direction `d`, load one token in each of the two
`(d,b)` event lanes. After successful export define

\[
 C=\sum_{\nu,b,j}c_{\nu,b}(j)\,\nu.                  \tag{6}
\]

One hop must add `2d`, matching the FTD-0940 integrated occupancy current;
the reverse hop must add `-2d`. For a fixed source hub and a blank incoming
rail, depth records token age, so the full ordered direction sequence is
recoverable. A clockwise and counterclockwise C4 word must therefore remain
distinct even when both have zero net displacement.

The update must be equivariant under all 48 signed permutation matrices,
which merely permute the Moore channels, and local because equation (5)
advances by one Moore edge. With

\[
 H_{\rm tok}={\epsilon_*over2}
 \sum_{\nu,b}\left(a_{\nu,b}^2+\sum_jc_{\nu,b}(j)^2\right),\qquad
 \epsilon_*>0,                                      \tag{7}
\]

the carrier update must conserve `H_tok` exactly. Loading an event port is a
separate source transaction and is not free; neither `epsilon_*` nor its
identification with the Hodge debit is fixed here.

After `N` accepted same-direction hops, the construction uses `2N` occupied
ports and causal depth `N`. It therefore has finite capacity **per site/port**
but not fixed total support or fixed total energy. On a finite periodic rail,
return produces backpressure rather than silent overwrite.

## 6. Collision and erasure gates

The certificate must verify:

1. different Moore directions compose in independent channels;
2. same-direction tokens in distinct lanes never merge under streaming;
3. an occupied injection port returns a token to the event port rather than
   deleting it;
4. a third simultaneous identical occupancy token exceeds the registered
   two-lane capacity and must be rejected, serialized, or exported elsewhere;
5. the map `0 -> 0`, `1 -> 0` is noninjective, so logical erasure is not a
   reversible carrier update; and
6. no thermodynamic `kT ln 2` or other physical erasure-energy bound is to be
   inferred from this combinatorial fact.

## 7. Frozen outcome table

| Outcome | Exact condition | Verdict |
|---|---|---|
| A | A fixed bounded finite-alphabet branch injectively retains every integer winding and passes reversal, collision, covariance, locality, and energy gates | finite local carry closes in the registered class |
| B | The fixed-support no-go holds, and the finite-per-site reversible Moore-token export passes all gates by using expanding support/backpressure | carry trilemma closes; distributed export survives as a reference class |
| C | The no-go holds and the token-export witness fails an exact gate | no surviving registered finite-capacity reference carrier |
| D | A frozen source drifts or the certificate cannot evaluate a gate | execution invalid; no theorem |

No tolerance or post-hoc branch change is permitted.

## 8. Acceptance and stop conditions

The certificate must report separately:

- finite-state injectivity and reversible-orbit periodicity;
- cumulative-flux exactness versus capacity;
- body-label capacity and collision ambiguity;
- finite-link modularity/saturation;
- exact forward/inverse token update;
- sequence recovery and C4 orientation discrimination;
- `2d` occupancy-carry matching for all 12 Moore-edge directions;
- signed-cubic covariance under all 48 group elements;
- collision/backpressure behavior;
- token-energy preservation; and
- expanding-support and free-energy-scale boundaries.

Stop immediately if any source hash drifts. Do not modify production `Voxel`,
the engine tick, CMake, the contextual-actualization interfaces, or any
physical parameter. Do not identify the abstract causal depth with global
time, a body proper time, or a physical extra dimension.

## 9. Promotion boundary

Even Outcome B will not derive physical momentum, inertia, mass, `gamma`, the
FTD-0933 abrupt wake, or a production carrier. It will prove only that the
missing history can be retained with finite local alphabet by paying in
expanding support, backpressure, and token energy. Joining that carrier to the
Hodge chord requires one common reversible source--field action and a derived
identification with existing L/R hardware, or a separately priced new type.

