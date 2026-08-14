# Theorem — Finite port rail, positive source battery, and recycling boundary v1

**Identifier:** `FTD-0883` / repaired execution `FTD-0884`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — FINITE EXPLICIT PORT-BANK FRESHNESS CAPACITY]` +
`[THEOREM — EXACT FULL-STATE FINITE-HORIZON REVERSIBILITY]` +
`[CONDITIONAL THEOREM — UNIQUE SIGN-PRESERVING POSITIVE QUADRATIC BATTERY LAW]` +
`[CLOSED NEGATIVE — INDEFINITE FRESHNESS OF A FINITE CYCLIC EXPLICIT PORT BANK]` +
`[IMPOSED — BATTERY LAW AND RESERVE SCALE]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[OPEN — CANONICAL HAMILTONIAN RESERVOIR, UNBOUNDED/OPEN HISTORY,
3D ROUTING, MOVING SOURCES, PRODUCTION, G*]`

## 1. Verdict

FTD-0882's local preparation mechanism needs two resources: a fresh signed
environment port at every active cell and work from the fixed source. FTD-0884
separates those obligations exactly.

1. An explicit cyclic bank of `C` initially zero signed-port vectors supplies
   exactly the first `C` fresh checkerboard layers. Because each outgoing
   residual is stored in the consumed coordinate, the complete field, port
   bank, cursor, and battery state is reversible. When the cursor returns, a
   generic nonzero history returns with it, so layer `C+1` is not fresh.
2. If a nonzero battery amplitude `b_x` carries positive energy `b_x^2/2`,
   sign preservation and exact energy conservation uniquely force

   \[
   b'_x=\operatorname{sgn}(b_x)\sqrt{b_x^2-2w_x},
   \qquad w_x=\frac{q_x}{6}(e_x-r_x),
   \]

   whenever the strict reserve condition `b_x^2-2w_x>0` holds. The inverse is
   exact and the battery loses precisely the work gained by field plus port.

Thus a finite piece of reference hardware can prepare a record reversibly for
a declared finite horizon and can pay for it from a positive energy account.
It cannot both keep every distinction and return the same finite explicit
port bank to perpetual readiness. Indefinite exact operation requires growing
or outgoing signed history, or a separately declared compression mechanism.

This is not a universal finite-dimensional memory no-go. Exact-real natural
extensions can encode indefinitely many distinctions in progressively finer
coordinates; they are deliberately outside the registered explicit one-
vector-per-port bank.

## 2. Finite cyclic port bank

Let the bank be

\[
 \mathcal H_C=(h_0,\ldots,h_{C-1}),\qquad h_j\in\mathbb R^V,
 \qquad h_j^{(0)}=0,
\]

with cursor `k in Z_C`. At half-layer `n`, parity is `n mod 2`, the incoming
environment is `e=h_k`, and the FTD-0882 layer produces a field `J'` and the
complete outgoing signed vector `e'`. The bank update is

\[
 h'_k=e',\qquad h'_j=h_j\ (j\ne k),\qquad k'=k+1\pmod C.     \tag{1}
\]

For an initially zero bank, the selected coordinate is fresh for the first
`C` applications. Each fresh application is therefore the affine orthogonal
checkerboard projection proved in FTD-0882.

The inverse decrements the cursor, reads the stored `e'`, applies the exact
residual/environment inverse, and restores the recovered incoming `e` to the
same bank coordinate. Hence (1) is not an erasure. The field subsystem can
contract only because the bank keeps the missing signed distinction.

For the locked neutral `L=4` dipole, the first outgoing vector is nonzero.
After `C` layers the cursor returns to `h_0`, so

\[
 h_0=e'_0\ne0.                                                  \tag{2}
\]

Equation (2) fails the fresh-port gate on layer `C+1`. More generally, a
finite cyclic explicit bank supplies indefinitely many fresh inputs only on
special histories whose returning coordinates happen to be zero. It does not
guarantee indefinite fresh preparation.

The exact escapes are already visible in the causal-history theorem:

- increase capacity with elapsed history;
- use a bilateral/unbounded rail with a prepared blank future;
- export the complete signed tail to an open environment; or
- adopt and independently justify a reversible exact-real compression law.

Energy-only tail export is insufficient because it loses the sign.

## 3. Positive quadratic source battery

For one active cell, FTD-0882 gives

\[
 \Delta(E_{\rm field}+E_{\rm port})=w_x,
 \qquad w_x=\frac{q_x}{6}(e_x-r_x).                           \tag{3}
\]

Introduce a nonzero signed amplitude `b_x` with

\[
 E_{b,x}=\frac12b_x^2.                                        \tag{4}
\]

Demand three properties in this registered reference class:

1. exact conservation of (3) plus (4);
2. retention of the sign of `b_x`; and
3. deterministic continuity on each nonzero sign branch.

Conservation requires

\[
 (b'_x)^2=b_x^2-2w_x.                                         \tag{5}
\]

When the right side is strictly positive, sign retention selects exactly one
root:

\[
 \boxed{b'_x=\operatorname{sgn}(b_x)
              \sqrt{b_x^2-2w_x}.}                            \tag{6}
\]

The inverse Gauss gate first recovers `e_x` and `r_x`, hence `w_x`; then

\[
 \boxed{b_x=\operatorname{sgn}(b'_x)
             \sqrt{(b'_x)^2+2w_x}.}                          \tag{7}
\]

The strict reserve test is performed before the field mutation. If it fails,
the complete step fails closed. Negative `w_x` recharges the battery; positive
`w_x` drains it. The fixed ternary charge `q_x` supplies the coupling sign and
acts catalytically, while `b_x` is the degree that actually changes energy.

Equation (6) is unique only after the quadratic battery energy and sign-
preserving one-amplitude class are chosen. Those choices and the initial
reserve scale are **[IMPOSED reference structure]**. No canonical conjugate,
symplectic form, Hamiltonian flow, or natural amplitude scale has been derived.

## 4. Complete finite-horizon ledger

For a port bank and cell batteries define

\[
 E_{\rm tot}
 =\frac12\lVert J\rVert^2
  +\sum_{j=0}^{C-1}\sum_x\frac{h_{j,x}^2}{12}
  +\sum_x\frac{b_x^2}{2}.                                    \tag{8}
\]

Same-color gates have disjoint face support. Equations (3) and (6) therefore
give, for every accepted layer,

\[
 E_{\rm tot}'=E_{\rm tot},\qquad
 \Delta E_{\rm battery}=-\sum_xw_x.                          \tag{9}
\]

After `N<=C` fresh layers,

\[
 E_{\rm battery}^{(0)}-E_{\rm battery}^{(N)}
 =W_{\rm source}^{(N)}
 =E_{\rm field}^{(N)}+E_{\rm history}^{(N)}.                 \tag{10}
\]

Reversing all `N` layers restores the empty field, all-zero bank, every signed
battery amplitude, and the original cursor exactly.

If an unbounded fresh rail supported the FTD-0882 asymptotic limit, then

\[
 E_{\rm battery}^{(0)}-E_{\rm battery}^{(\infty)}
 =\lVert J_s\rVert^2,                                        \tag{11}
\]

with half of that energy in the static record and half in signed history. A
sufficient reserve is therefore finite in energy, even though exact history
capacity is unbounded in this explicit representation. Energy capacity and
information capacity are different constraints.

## 5. What is natural, imposed, and still absent

### Forced within the registered class

- one fresh layer per initially ready bank coordinate;
- exact cursor/bank inverse when the signed output is retained;
- failure of generic indefinite freshness when a nonzero coordinate returns;
- the squared-amplitude change `(b'_x)^2=b_x^2-2w_x`;
- the sign-preserving root once the battery sign branch is retained; and
- the total-energy and reversal identities (9)--(10).

### Imposed

- the explicit cyclic one-vector-per-port bank;
- the quadratic one-amplitude battery energy;
- sign preservation as the battery branch convention;
- the initial battery reserve scale; and
- the cursor schedule, reusing the existing tick/phase-rail selection.

### Still open

- a canonical Hamiltonian or symplectic battery/reservoir lift;
- a substrate-native formation and recharge mechanism for the battery;
- an unbounded, open, or justified compressed history environment;
- local 3D routing of one bank lane per active cell;
- finite-boundary signed-tail export and backpressure;
- moving-source continuity and source recoil;
- production migration to the matched face complex;
- physical time and amplitude scales;
- synchronization to the separate quartic `G*` calendar; and
- Born recovery, Bell laboratory recovery, Lorentz hiding, and completeness.

No sixth selected v2 type is added: the witness uses existing continuous
carrier amplitudes and the selected phase/history rail. The battery update is
a new imposed functional law on those types and is booked as such.

## 6. Verification and provenance

The frozen FTD-0883 protocol SHA-256 is
`0B6ACD3C1E41B4D1EE60CCA9A5E04E91E84FC96F06A3725B1F41DDDFD79E8C0B`.
The frozen parent certificate SHA-256 is
`9596738C5FA23964CDEE234BD73E1A48B658516D931B5E92CC085118D90DD02B`.
Its first locked execution reported `54/56`: all substantive gates passed;
C3 used an unnormalized line-wrapped Markdown marker and C56 failed
dependently. No theorem is booked from that parent run.

FTD-0884 froze a one-substitution whitespace repair. Its protocol SHA-256 is
`13B9456E1DCF188DB26BDA7D6816FB88CEDADF1F59653841CE5FAA289BD4BDE8`;
its wrapper SHA-256 is
`E2129A5284AB5C664C5A257B0D861D2A5C4329776CC0E684365845B120379D87`.
The inherited certificate passes `56/56` with terminal markers:

```text
FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_THEOREM
FINITE_CYCLIC_FRESH_LAYERS=CAPACITY
FINITE_CYCLIC_INDEFINITE_FRESHNESS=NO
EXACT_REAL_MEMORY_NO_GO=NOT_CLAIMED
POSITIVE_QUADRATIC_BATTERY=UNIQUE_SIGN_PRESERVING_LAW
BATTERY_LAW_STATUS=IMPOSED_REFERENCE
FULL_FINITE_STATE_REVERSIBILITY=EXACT
CANONICAL_HAMILTONIAN_RESERVOIR=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR
BORN_BELL_STATUS=UNTOUCHED
```

The isolated `ftd::eft::FinitePortGaussBattery` implementation passes its
focused Release CTest `1/1`; the coupled contextual-actualization chain passes
`18/18`. Production `Voxel`, toggles, defaults, and tick phases are unchanged.

