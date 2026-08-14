# Pre-registration — Krylov-degeneracy ternary latch and oriented C4 transition v1

**Identifier:** `FTD-0971`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

Can the exact `kappa=0` degeneracy of the FTD-0969 neutral body, together
with one blank/positive/negative ternary latch, form the minimum
self-delimiting reversible transition that distinguishes both crossing
directions? Does its oriented mode supply a natural real representation of
`i`, and does that already constitute the active FTD-0963 field gearbox?

The candidate must be derived from exact branch limits and finite-state
injectivity. It may not read `G*`, a Born weight, a target outcome, a selected
quarter-turn angle, or a future state. No numerical search, fitted tolerance,
engine mutation, or production promotion is permitted.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_GLOBAL_ISOCHRONY_LIFT_AND_ORIENTED_CROSSING_LATCH_BOUNDARY_v1.md` | `746F855A432D7E662236315066115174493554285CD3FC25071B892A05AEA68E` |
| `THEOREM_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_SELF_DUAL_GEARBOX_v1.md` | `56711EE1A215F4418A9B8FA5E4EF6C46BD0B2767D407F70E04C7C6A0FD6345B1` |
| `THEOREM_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md` | `100A5539A1116FD6BEC5ABF2B7CE7BA2C32DDA557564EC7C964CDF5877512739` |
| `THEOREM_MOVING_REGIONAL_FRAME_COTANGENT_CONNECTION_AND_PURE_GAUGE_BOUNDARY_v1.md` | `C5C28405CA439BF2341D545F99E9BDFC985BF65155B1CD49075541CD5C258462` |

No ontology type, clock law, connection profile, selector, engine source,
tick phase, or production action may change under this protocol.

## 3. Frozen exact degeneracy family

Use the FTD-0969 one-cube family

\[
\begin{array}{c|cccc}
x&(0,0,0)&(1,0,0)&(0,1,0)&(1,1,u)\\ \hline
s_x&+1&+1&-1&-1.
\end{array}                                                   \tag{1}
\]

The certificate must recover

\[
 d=(0,-2,-u),qquad
 \kappa(u)=-{u^5\over256},                                  \tag{2}
\]

and, with the FTD-0969 frame convention, the exact one-sided limits

\[
 e_1^+=e_1^-=(0,-1,0),qquad
 e_2^+=e_2^-=(-1,0,0),                                      \tag{3}
\]

\[
 \chi^+=-1,quad e_3^+=(0,0,1),qquad
 \chi^-=+1,quad e_3^-=(0,0,-1).                            \tag{4}
\]

Here `+/-` denotes `u -> 0+/-`, not time orientation. Therefore

\[
 S=(F^+)^TF^- = \operatorname{diag}(1,1,-1),qquad
 S^2=I,quad\det S=-1.                                      \tag{5}
\]

The crossing flips the handed complex structure by improper conjugation. It
is a reflection/conjugation transition, not multiplication by `i` and not the
active quarter-turn by itself.

## 4. Minimum self-delimiting transition memory

At the degenerate body snapshot the spatial record alone is the same for the
two incoming branches. A fail-closed self-delimiting latch must distinguish

\[
 \{\text{blank},\text{incoming from }+,\text{incoming from }-\}. \tag{6}
\]

By finite-set injectivity its alphabet has cardinality at least three. One
ternary latch `ell in {-1,0,+1}` is therefore sufficient and minimum **within
this self-delimiting one-cell interface**. This is not a universal one-bit
claim: a separately retained phase/type flag could change the accounting and
must be priced if used.

The modular load/clear maps

\[
 L_s:\ell\mapsto\ell+s\pmod3,qquad L_s^{-1}=L_{-s},          \tag{7}
\]

must be verified as permutations. Loading a blank latch with the incoming
sign retains the branch before the spatial record reaches degeneracy.

## 5. Frozen oriented four-state transition

Define four admissible composite states

\[
 A=(R_+,0),\quad B=(D,+1),\quad
 C=(R_-,0),\quad D=(D,-1),                                  \tag{8}
\]

where `R_+` and `R_-` are the regular `chi=+1` and `chi=-1` branches and `D`
is the common degenerate spatial body. Freeze the forward update

\[
 T:A\to B\to C\to D\to A.                                  \tag{9}
\]

It must obey

\[
 T^4=I,qquad T^{-1}=T^3.                                   \tag{10}
\]

Time reversal is the involution

\[
 \Theta:A\mapsto A,quad C\mapsto C,quad B\leftrightarrow D, \tag{11}
\]

and must satisfy

\[
 \Theta T\Theta=T^{-1}.                                    \tag{12}
\]

Thus the ordered update, not either regular snapshot, distinguishes the two
directions through the same degenerate body.

## 6. Natural real `i` mode and symmetric-square loss

In the real one-hot state space let

\[
 c=A-C,qquad s=B-D.                                        \tag{13}
\]

The two-plane `span{c,s}` must be invariant and the forward transition must
act as

\[
 T|_{\{c,s\}}=
 J=\begin{pmatrix}0&-1\\1&0\end{pmatrix},qquad J^2=-I.     \tag{14}
\]

Reverse traversal acts as `-J`, while

\[
 (+J)^2=(-J)^2=-I.                                         \tag{15}
\]

This is an exact real representation of multiplication by `i` on the
oriented transition mode. It is a property of the retained C4 cycle, not of
the static ternary alphabet and not a derivation of Hilbert space.

## 7. Energy and active-coupling firewall

The four-cycle permutation preserves the equal-weight one-hot norm and every
quadratic form invariant under the cycle. That proves losslessness of the
finite-state carrier only. It does not prove that the actual regular and
degenerate substrate bodies have equal physical energy or that production
dynamics forms the cycle without work.

No field variable is changed by (9). Coupling the `J` mode actively to a
field pair requires a separately specified local generator or exact
symplectic transaction with:

- physical source and reciprocal body reaction;
- work/energy ledger;
- finite reserve and fail-closed backpressure;
- complete inverse and retained routing history; and
- a proof that the interaction is not merely passive chart relabelling.

One update per global tick would give a four-tick reference cadence. Nothing
in the transition identifies that cadence with the critical quartic period or
derives `G*`; dwell times, feedback, and the substrate/CM gearbox remain open.

## 8. Frozen checks

- **G1:** hashes and all source/protocol scope markers;
- **G2:** exact `d`, covariance, `kappa`, normalized one-sided frame limits;
- **G3:** reflection/involution/determinant and complex-conjugation boundary;
- **G4:** three-symbol self-delimiting lower bound and ternary modular
  load/clear permutations;
- **G5:** exact four-state permutation, inverse, and latch occupancy;
- **G6:** time-reversal conjugacy;
- **G7:** invariant oriented plane, `J^2=-I`, reverse `-J`, and identical
  symmetric squares;
- **G8:** permutation norm/invariant-quadratic preservation;
- **G9:** physical-energy, active-coupling, formation, `G*`, Born, and
  production firewalls.

All calculations are exact. No floating comparison or search is permitted.

## 9. Frozen classifier

- **Outcome A — active native C4 gearbox:** G1--G9 pass and the transition is
  formed by production dynamics with physical energy/reaction closure and an
  active field exchange.
- **Outcome B — exact minimum retained C4 carrier / active coupling open:**
  the ternary latch makes the singular passage reversible and its oriented
  mode realizes `J^2=-I`, but formation, physical energy, cadence, and active
  field coupling remain open.
- **Outcome C — memory or orientation obstruction:** the latch cannot make the
  two passages injective or the retained cycle has no oriented `C4` mode.
- **Outcome D — invalid:** any lock, exact identity, or scope gate fails.

The expected result is Outcome B. Success would identify a minimal recursive
orientation carrier, not a production particle, Born mechanism, `G*` gearbox,
or complete dynamics.
