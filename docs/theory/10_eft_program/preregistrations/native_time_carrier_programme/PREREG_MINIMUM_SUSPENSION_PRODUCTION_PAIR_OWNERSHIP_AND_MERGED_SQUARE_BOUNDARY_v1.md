# Pre-registration — Minimum-suspension production pair ownership and merged-square boundary v1

**Identifier:** `FTD-0975`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

Can the FTD-0974 minimum canonical suspension be represented in the six
existing dual-field canonical pairs without double-booking the five-pair
FTD-0963 gearbox chart?

The classifier must distinguish:

1. an alternative two-pair suspension;
2. a specialization inside the five-pair gearbox chart;
3. a wholly independent simultaneous suspension; and
4. a shared-clock simultaneous suspension using the sixth unused pair.

No engine source, production phase, public type, selected scale, connection
profile, or coupling law may change. No numerical search or fit is permitted.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_SELF_DUAL_GEARBOX_v1.md` | `56711EE1A215F4418A9B8FA5E4EF6C46BD0B2767D407F70E04C7C6A0FD6345B1` |
| `THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_CHART_BOUNDARY_v1.md` | `FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C` |
| `THEOREM_C4_FIELD_COCYCLE_AND_MINIMUM_CANONICAL_SUSPENSION_v1.md` | `1729617446272A47C5A5812F88A89416E9ABC609CA672671017CFB8AEDD5D63E` |

No production file may change under this protocol.

## 3. Frozen pair inventory

Write the six conditional regional canonical pairs of FTD-0965 as

\[
 c_0,c_1,c_2,c_3,c_4,c_5.                                  \tag{1}
\]

The FTD-0963 chart owns

\[
 c_0=(\delta,\Pi),qquad c_1,c_2,c_3,c_4
 \text{ as its four exchange modes},                        \tag{2}
\]

and leaves `c_5` unused. The exact names chosen in the FTD-0965 witness are a
selected frame chart; only complete-pair ownership matters here.

The FTD-0974 suspension needs

\[
 s_0=(\theta,A),qquad s_1=(Q,P).                           \tag{3}
\]

All rank and symplectic tests use whole pairs only.

## 4. Frozen capacity cases

### Case A — alternative

Map `(s_0,s_1)` into any two complete production pairs, for example
`(c_0,c_5)`. The projection must have rank four and preserve the canonical
four-dimensional symplectic form. Four production pairs remain unowned by the
suspension.

### Case B — specialization

Identify `s_0=c_0` and `s_1=c_j` for one existing gearbox exchange pair
`j in {1,2,3,4}`. This is a two-pair subsystem of the five-pair chart. It does
not add a degree of freedom, but it replaces/specializes the FTD-0963
connection and cannot simultaneously be counted as an independent law.

### Case C — wholly independent coexistence

Five gearbox pairs plus two disjoint suspension pairs require seven complete
pairs, i.e. a fourteen-dimensional symplectic space. No rank-fourteen
symplectic injection exists into the twelve-dimensional production pair
space. This case is closed negative without a new pair.

### Case D — shared clock plus unused field pair

Identify `s_0=c_0` and `s_1=c_5`. The union owns all six production pairs and
is dimensionally representable. However, the two Hamiltonians share the same
clock momentum and may not be added as independent kinetic terms.

## 5. Clock kinetic double-booking gate

Abstract the FTD-0963 connection load by `X` and the FTD-0974 field action by
`I`. Independent addition would give

\[
 H_{\rm sum}={ (\Pi+X)^2\over2M}
             +{ (\Pi-I)^2\over2M_s}+H_{\rm rest}.           \tag{4}
\]

At `X=I=0`, the coefficient of `Pi^2` is

\[
 {1\over2M}+{1\over2M_s},                                  \tag{5}
\]

and

\[
 \dot\delta={\Pi+X\over M}+{\Pi-I\over M_s}.              \tag{6}
\]

For finite positive `M_s`, this does not preserve the original bare clock
kinetic term or rate. It is double counting, not independent coexistence.

The minimum coherent shared-clock candidate is one newly selected merged
square

\[
 H_{\rm merge}={ (\Pi+X-I)^2\over2M}+H_{\rm rest}.          \tag{7}
\]

It restores one kinetic term and gives

\[
 K=\Pi+X-I,qquad \dot\delta={K\over M}.                    \tag{8}

But equation (7) contains the new cross interaction

\[
 -{XI\over M}.                                              \tag{9}

Therefore capacity does not derive coexistence. Equation (7), its sign, mode
identity, formation, and repeated dynamics require a fresh selected-law
pre-registration before any production change.

## 6. Production firewall

FTD-0965 already closes the unchanged production connection negative. This
audit changes no such conclusion. It proves only:

- alternative/specialized suspension capacity;
- a dimension obstruction to wholly independent coexistence;
- shared-clock/full-six-pair capacity; and
- the necessity of a new merged Hamiltonian rather than summing two clock
  kinetic terms.

It does not establish regional-body formation, protection of pair ownership,
the physical identity of `c_5`, the merged interaction, switching, energy
ports, `G*`, Born/Bell, hiding, or production integration.

## 7. Frozen checks

- **G1:** hashes and all source/protocol scope markers;
- **G2:** six-pair ambient symplectic form and five-pair FTD-0963 packing;
- **G3:** alternative and specialized rank/symplectic embeddings;
- **G4:** seven-pair/fourteen-dimensional independent-coexistence
  obstruction;
- **G5:** shared-clock plus unused-pair full-rank capacity;
- **G6:** exact kinetic coefficient and clock-rate double-booking;
- **G7:** merged complete square, mechanical momentum, and new `-XI/M` cross
  term;
- **G8:** alternative/specialization/coexistence and production firewalls.

All algebra is exact. No floating comparison, numerical search, or near-miss
scan is permitted.

## 8. Frozen classifier

- **Outcome A — existing production law:** all cases close without double
  booking and the unchanged tick realizes the suspension.
- **Outcome B — conditional capacity / merged-law debt:** alternative and
  specialized capacity close; independent coexistence is dimensionally
  impossible; shared-clock coexistence fits only through a newly selected
  merged square.
- **Outcome C — no production capacity:** even the alternative suspension
  cannot be embedded symplectically in existing pairs.
- **Outcome D — invalid:** any lock, exact identity, or scope gate fails.

The expected result is Outcome B. Success does not authorize the merged law or
production integration.
