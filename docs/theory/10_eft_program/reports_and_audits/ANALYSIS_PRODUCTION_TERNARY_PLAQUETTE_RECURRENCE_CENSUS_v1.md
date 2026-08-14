# Analysis — Production ternary-plaquette recurrence census v1

**Identifiers:** `FTD-0915`, `FTD-0916`, `FTD-0917`  
**Date:** 2026-08-11  
**Status:** `[MEASURED — IDENTITY-BEARING PLAQUETTE EXPOSURES]` +
`[CLOSED NEGATIVE — DIRECTED QUARTER-TURN TRANSPORT AND FOUR-STEP
RECURRENCE IN THE LOCKED MATRIX]` +
`[OPEN — NATIVE ORIENTED CIRCULATION/CONJUGATE-MOMENTUM LAW]`

## 1. Result

The repaired FTD-0915 campaign returns the preregistered exact result

```text
FTD0915_OUTCOME=D_EXPOSURE_WITHOUT_DIRECTED_TRANSPORT
```

The unchanged production engine forms the exact neutral word on elementary
plaquettes, often with persistent signed particle identities. It never
advances that word by one forward or reverse quarter-turn in the complete
held-out matrix.

This separates two claims that had previously been adjacent but untested:

1. **[THEOREM, FTD-0914]** the substrate contains the minimum spatial hardware
   capable of realizing `J^2=-I`; and
2. **[CLOSED NEGATIVE, FTD-0915 scope]** the current production dynamics do
   not run that hardware as an oriented recursive clock.

## 2. Execution integrity

The parent protocol was locked before runner implementation:

- FTD-0915 protocol SHA-256:
  `C302319900BAC4920277FACCC3A9164F0AE64DCAC8FBD256A4F36B48E7CC970C`.

The first FTD-0916 execution is preserved but execution-invalid because its
processed corpus omitted the four raw per-site identities and fields needed
for independent reconstruction. No A--E result is issued from it.

FTD-0917 froze a telemetry-only repair before rerun:

- repair SHA-256:
  `26D4488E2BB8EB6783C1C7F6B4D413D79D487D78A9A43A98D793F2B02D55DF44`;
- repaired runner SHA-256:
  `D24970F34346167197D53681F1E6231A68C5E81F0515E6CA85B7335FBED83F21`;
- repaired executable SHA-256:
  `E02B56E25F8FD38C0E12815A30D342378E7E9CC072DD0A7011CB71A80548249D`;
- v3 preflight: `46/46`;
- independent result certificate SHA-256:
  `3AC6BEB23D0867A86BB72CB69DFF7336E187EF254AB4870473B8FA4406068DE2`;
- independent result certificate: `33/33`.

The repair changed no source of production dynamics, support, arm, seed,
tick count, threshold, or outcome. It added raw telemetry only. The repaired
run completed all `64` arms and `8,192` tick records. Every observer
nonmutation hash, enumeration count, finite-telemetry gate, exact algebraic
control, and reconstruction check passed.

Run-of-record corpus hashes are:

| Artifact | SHA-256 |
|---|---|
| tick census | `F006ADACDABFEF970F4DE4914ADDBE3DCE2B812E49993596CAB23ED1AA80AA47` |
| exposure census | `E68705751A9126AC857DF5702BD66714C60589174C053AD50EA0A5485AFD5EBA` |
| transition census | `956815D69ED08DF3CD47AFCD5AE1889B9BAFAC163FE22FCC8E629733694CF381` |
| summary | `53CC7D0C78BB5EB050B1D0F45F1CAD0F6118C48C1092CA6CAACFC3A6915D204E` |

## 3. Census

Across the `48` injected arms:

| Quantity | Result |
|---|---:|
| Identity-bearing orbit exposures | `2,860` |
| Arms with at least one exposure | `21 / 48` |
| Retained consecutive comparisons | `2,806` |
| `STATIONARY` | `2,800` |
| `SUPPORT_LOSS` | `6` |
| `FORWARD` | `0` |
| `REVERSE` | `0` |
| `HALF_TURN` | `0` |
| `ADJACENT_DEFECT` | `0` |
| Full four-transition cycles | `0` |

The cell ledger is:

| Volume/family | Seeds with exposure | Exposures | Stationary | Support loss | Directed |
|---|---:|---:|---:|---:|---:|
| `21 / axial_live` | `3/8` | `410` | `402` | `0` | `0` |
| `21 / diagonal_live` | `4/8` | `389` | `380` | `0` | `0` |
| `21 / axial_no_bath` | `3/8` | `425` | `413` | `3` | `0` |
| `27 / axial_live` | `2/8` | `133` | `130` | `0` | `0` |
| `27 / diagonal_live` | `4/8` | `694` | `682` | `0` | `0` |
| `27 / axial_no_bath` | `5/8` | `809` | `793` | `3` | `0` |

All `16` empty-control arms have zero identity-bearing exposure.

The difference between exposure totals and consecutive comparisons comes
from beginnings/endings of observed histories; it is not an unclassified
transition.

## 4. What the result means

The exact FTD-0914 word is not rare enough to be absent, and it is not merely
an identity-free state coincidence: signed production particle IDs occupy the
opposite vertices. Yet when the support persists, the word is stationary.

Therefore the current bottleneck is not:

- the ternary alphabet;
- the existence or minimum size of a closed loop;
- the representation of multiplication by `i`;
- clockwise/counterclockwise distinguishability in an ordered history; or
- an available identity-bearing support.

The bottleneck is a **native antisymmetric generator** that turns one member
of the two-dimensional plaquette representation into the other while
accounting for energy and reaction. In physical language, the clock face
exists, but there is no winding current.

This is why `G*` cannot repair the result by itself. A period factor can set
the elapsed cadence of an already-running orbit. Multiplying the duration of
a stationary configuration does not create the missing angular momentum or
choose its sign.

## 5. Simplest mathematically natural successor

Let a scalar or vector-valued local field `f_j` live on the four ordered
vertices. Its exact `C_4` first-harmonic coordinates are

\[
q_f=\frac{f_0-f_2}{2},\qquad
r_f=\frac{f_1-f_3}{2}.
\]

The cyclic shift acts by

\[
(q_f,r_f)\longmapsto(-r_f,q_f).
\]

Thus the same real two-plane that realizes `i` for the actual ternary word is
already available for the continuous flux/wave variables. The missing datum
is the antisymmetric phase-space charge

\[
\mathcal L_P
=q_J\!\cdot r_W-r_J\!\cdot q_W,
\]

or a rigorously derived equivalent from the canonical variables of the
production action. It changes sign under time reversal and distinguishes the
two chiral circulations.

For the reference isotropic ring Hamiltonian

\[
H_P=\frac12\left(
\|p_q\|^2+\|p_r\|^2
+\omega^2\|q\|^2+\omega^2\|r\|^2
\right),
\]

the `O(2)` rotation of `(q,r)` conserves

\[
L_P=q\!\cdot p_r-r\!\cdot p_q.
\]

A circular branch has equal kinetic/potential channels and nonzero `L_P`; a
standing or balanced counter-rotating branch has `L_P=0`. This explains the
special status of the **inert** configurations without declaring the
reference Hamiltonian native: they possess the clock face but no net chiral
charge.

This Hamiltonian is presently a **[SELECTED reference model]**, not a
production derivation. The next theorem must determine whether the existing
flux/wave action induces this `C_4` doublet and conserved charge without a new
term. Only then may a held-out campaign ask whether a nonzero continuous
plaquette circulation precedes actual ternary quarter-turns.

## 6. Ledgerable boundary

The result is ledgerable because every noun in the claim has a fixed finite
definition:

- the support is one enumerated elementary plaquette;
- the carrier word is one four-element orbit;
- identity is the signed production ID pair;
- direction is an exact successor relation;
- recurrence is four consecutive same-direction transitions with direct
  closure;
- defects exhaust all other same-support/next-tick possibilities; and
- the observer's nonmutation and reconstruction are certified from raw data.

The closed-negative statement is limited to the locked production matrix. It
does not prove that no modified or newly derived dynamics can circulate the
mode.

## 7. Scope firewalls

This campaign does not derive or establish:

- a native plaquette Hamiltonian or circulation charge;
- an invariant barrier or topological protection;
- maintenance work, dissipation, or erasure cost;
- `G*` coupling, absolute period, or `gamma` from `i`;
- Born weights, Bell correlations, selector dynamics, or measurement context;
- production integration of a new term; or
- whole-framework completeness.

The proper next gate is exact and theory-first: decompose the existing
flux/wave production action into the plaquette `C_4` irreducible sectors,
derive—or fail to derive—the antisymmetric conserved charge, and only then
pre-register new held-out data.
