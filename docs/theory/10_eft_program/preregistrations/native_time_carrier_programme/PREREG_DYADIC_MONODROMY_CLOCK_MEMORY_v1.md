# PRE-REGISTRATION — Finite Dyadic Monodromy Clock–Memory Boundary v1

**Date frozen:** 2026-08-02  
**Identifier:** `FTD-0777`  
**Status:** `[PRE-REGISTRATION — LOCKED FOR EXACT/SYNTHETIC EXECUTION]`  
**Scope:** `[EXACT — FINITE COVER] + [SELECTED MODEL] + [CONDITIONAL] + [OPEN — NATIVE BRIDGE]`  
**Repository base commit:** `dc3bcf965fb45c6d8684500dcf6e1622eff10583`  
**Locked verifier:** `scripts/proofs/proof_dyadic_monodromy_clock_memory.py`  
**Verifier SHA-256:** `8907670CAF40D165AFB077639FBAF482FD985AD62C6F3145E92D6B9753DD9685`  
**Protocol SHA-256:** recorded in the companion analysis after this file is frozen; this file is not modified after execution.

## 1. Locked question

For a finite carrier phase and a selected compatible square-root tower, establish
the exact boundary among three different constructions:

1. forward dyadic harmonics, which refine a carrier but add no independent
   state;
2. compatible dyadic root lifts, which form a finite recurrence hierarchy and
   record epoch modulo a power of two; and
3. independently writable modal coefficients, which can carry event content
   but are not one coherent root cover.

The protocol must also test the proposed memory interpretation adversarially.
In particular, it must determine whether one transitive root tower can preserve
an arbitrary cycle-invariant payload, and whether a separately transported
reference lift repairs that obstruction.

No engine trajectory, FTD-0772 artifact, or FTD-0776 `q_active` artifact is an
input to this protocol.

## 2. Frozen construction

Let the carrier have a faithful unwrapped phase satisfying

\[
\Theta(t+T)=\Theta(t)+2\pi,
\]

where `T` is its fundamental period and the winding number is one.

### 2.1 Forward harmonic tower

\[
h_j=z_0^{2^j},\qquad z_0=e^{i\Theta}.
\]

Every `h_j` is a deterministic function of `z_0`. Adding these powers may add
readout resolution, but it supplies no new branch state.

### 2.2 Compatible root tower

At finite depth `K`, choose

\[
z_{j+1}^{,2}=z_j,
\qquad
z_K(t;m)=
\exp\!\left(\frac{i(\Theta(t)+2\pi m)}{2^K}\right),
\qquad
m\in\mathbb Z/2^K\mathbb Z.
\]

At a fixed carrier phase, the fiber has `2^K` compatible lifts. One positive
carrier winding acts by

\[
M_K(m)=m+1\pmod{2^K}.
\]

Consequently the selected depth-`K` lift first returns after `2^K` carrier
cycles, conditional on the fundamental-period and one-winding hypotheses.

### 2.3 Branch word and prefix activation

Sequential branch choices `b_j in {0,1}` address the sheet

\[
m_K=\sum_{j=0}^{K-1}b_j2^j.
\]

At fixed depth this gives `2^K` sheet addresses. If active depth itself is part
of the state and may range from `0` through `M`, the disjoint prefix count is

\[
\sum_{K=0}^{M}2^K=2^{M+1}-1.
\]

This is not the `3^M` state count of `M` independent ternary coefficients.
Those coefficients define a different register.

### 2.4 Single-tower payload no-go

At the base return section the observed sheet after `n` cycles is

\[
m_{\rm observed}=m_{\rm written}+n\pmod{2^K}.
\]

Since `M_K` is one transitive cycle, every observable satisfying
`R(m+1)=R(m)` is constant. The single tower can record epoch modulo `2^K`, but
it cannot simultaneously carry a nonconstant cycle-invariant payload.

### 2.5 Selected relational repair

Introduce a separately co-transported reference sheet `r` and payload sheet
`u`. Under one carrier cycle both advance by one, so

\[
(u+1)-(r+1)=u-r\pmod{2^K}.
\]

The relative address is stable. This is exact inside a **selected two-tower
model**. It does not derive a native reference carrier, writer, or decoder.

### 2.6 Quartic and causal boundary

For the previously selected quartic clock normalization,

\[
T_4(E)=\frac{\sqrt\pi G^*}{\rho(2E)^{1/4}},
\]

the selected root lift has

\[
T_{4,K}=2^K T_4.
\]

This is a composition theorem conditional on the selected quartic carrier. It
does not derive the carrier or `G*`. A uniform root phase retains arcsine
coordinate occupancy, with fourth moment `3/8`, rather than the quartic moment
`1/3`.

The minimum-interval ceiling applies in the opposite, forward-harmonic
direction. With the FTD-0771 conditional edge/clock fraction

\[
d_4=\frac{\rho(2E)^{1/4}}{u\sqrt\pi G^*},
\]

requiring `nu` causal intervals per fastest cycle gives

\[
\nu,2^k d_4\le1.
\]

This is `[CONDITIONAL]`; it does not derive a minimum physical `dt`.

## 3. Epistemic firewalls

The following are binding:

- The root tower is `[SELECTED MODEL]`, not an FTD-native structure.
- A Fourier/root coordinate is mathematical bookkeeping, not voxel ontology.
- The construction may be called a finite cover, lift, sheet hierarchy, or
  odometer. Its algebraically dependent levels are not independent physical
  modes.
- One tower stores epoch modulo `2^K`, not stable arbitrary event content.
- The two-tower relational repair adds a selected reference structure.
- Permanent append writes obstruct recurrence of the complete state through a
  write. Only an explicitly named projection or a fixed-depth state may recur.
- Root levels lengthen return times. They do not produce smaller `dt`.
- Root lifting does not produce quartic occupancy or recover `G*`.
- FTD-0772 and FTD-0776 remain unchanged. This protocol does not replace their
  preregistered observables or reclassify their negative results.
- No native, emergent, physical-memory, proper-time, or arrow-of-time claim is
  licensed by a synthetic pass.

Violation of any firewall yields `DYADIC_MONODROMY_EXECUTION_INVALID`.

## 4. Frozen implementation domain

| Field | Frozen value |
|---|---|
| Exact backend | Python `3.13.12`; SymPy `1.14.0`; integer enumeration |
| Maximum root/Floquet depth | `K=10` |
| Finite carrier shadows for harmonics | periods `P=1..64`, all carrier states |
| Root fibers and returns | all sheets for `K=0..10` |
| Branch-word enumeration | all binary words for `K=0..10` |
| Prefix-capacity check | `M=0..10` |
| Independent ternary check | `M=0..7` |
| Append-only histories | start depth and write count `0..5` |
| Relational repair enumeration | `K=0..7`, all reference/payload sheets, locked wraparound loop set |
| Quartic checks | symbolic positive `E`, `rho`, and `G_star` |
| Integer-tick commensurability | base periods `P_0=1..16`, `K=0..10` |
| Conditional causal control | selected exact rationals `d_4=1/16`, `nu=2`; expected accepted `k={0,1,2,3}` |
| Synthetic artifact depth | `K=4`; expected first full stroboscopic return after `16` carrier cycles |
| Artifact directory | `engine/results/dyadic_monodromy_clock_memory_20260802/` |
| Engine/FTD data | none |

No field may be tuned after execution. Any implementation change requires a
new verifier hash and protocol version.

## 5. Locked gate table

| Gate | Status | Requirement |
|---|---|---|
| G0 | `[EXACT]` | Forward harmonic joint capacity equals carrier capacity |
| G1 | `[EXACT — SELECTED COVER]` | Depth `K` has exactly `2^K` compatible sheets |
| G2 | `[EXACT — SELECTED COVER]` | One carrier loop gives `m -> m+1 mod 2^K` |
| G3 | `[EXACT — SELECTED COVER]` | No positive return before `2^K`; exact return at `2^K` |
| G4 | `[EXACT — SELECTED COVER]` | Each old sheet has exactly two children and the horizon doubles |
| G5 | `[EXACT — SELECTED COVER]` | Binary branch words biject with sheet addresses |
| G6 | `[EXACT — SELECTED COVER]` | Variable-depth prefix count is `2^(M+1)-1` |
| G7 | `[EXACT]` | `M` independent ternary coefficients have `3^M` words |
| G8 | `[EXACT NO-GO — SELECTED MODEL]` | A permanent append prevents complete-state recurrence through the write |
| G9 | `[EXACT NO-GO — SELECTED MODEL]` | One transitive tower confounds written payload with elapsed epoch |
| G10 | `[EXACT — SELECTED TWO-TOWER MODEL]` | Relative payload `(u-r) mod 2^K` is invariant under common transport |
| G11 | `[CONDITIONAL EXACT COMPOSITION]` | `T_(4,K)=2^K T_4` for the selected quartic carrier |
| G12 | `[EXACT BOUNDARY]` | Uniform root coordinate has fourth moment `3/8`, not quartic `1/3` |
| G13 | `[EXACT — SELECTED COVER]` | Root Floquet multiplier at level `j` has exact order `2^j` |
| G14 | `[CONDITIONAL]` | Integer base period `P_0` gives sampled return `2^K P_0` |
| G15 | `[CONDITIONAL]` | Forward fast-mode causal ceiling and locked rational control agree |

All 16 gates are conjunctive. No near-miss or fitted verdict exists.

## 6. Required artifacts

The locked execution command is

```text
python scripts/proofs/proof_dyadic_monodromy_clock_memory.py \
  --output-dir engine/results/dyadic_monodromy_clock_memory_20260802
```

Required outputs:

- `gate_table.csv` — complete G0–G15 result table;
- `monodromy_strobe.csv` — depth-4 sheet sequence through the first return;
- `branch_words.csv` — depth-4 branch-word/sheet bijection; and
- `summary.json` — scoped verdict and epistemic firewall summary.

The companion analysis records hashes for the frozen protocol, verifier, and
all outputs.

## 7. Allowed verdicts

| Verdict | Meaning |
|---|---|
| `DYADIC_MONODROMY_CLOCK_MEMORY_EXACT_PASS` | G0–G15 pass; finite mathematical construction and boundaries demonstrated |
| `DYADIC_MONODROMY_CLOCK_MEMORY_EXACT_FAIL` | Valid execution, at least one exact/conditional locked gate fails |
| `DYADIC_MONODROMY_EXECUTION_INVALID` | Lock or epistemic firewall violated |
| `NATIVE_DYADIC_CLOCK_MEMORY_NOT_TESTED` | Mandatory native companion verdict for this protocol |

## 8. Native bridge gates — all `[OPEN] / NOT RUN`

A future native campaign requires a new preregistration and must demonstrate:

1. a bounded local carrier rather than a global Fourier/sheet sidecar;
2. autonomous level occupation triggered by local state, not tick number;
3. Moore-neighborhood causal propagation of any write;
4. engine energy plus explicit source-work closure;
5. source-off persistence and perturbation robustness;
6. bounded local decoding without an FFT or external elapsed-time oracle;
7. a native reference structure if stable relational payload is claimed;
8. recurrence reported separately for carrier, quotient, memory, and complete
   state;
9. alias, volume, boundary, seed, and backend controls; and
10. forward/reverse evidence before any physical-arrow interpretation.

Existing imposed phase, color, `SU2Link`, or `SU3Link` fields are inadmissible
as emergent carriers.

## 9. Boundary references

- [`ANALYSIS_NATIVE_TEMPORAL_OCCUPANCY_v1.md`](../../derivations/native_time_carrier_programme/ANALYSIS_NATIVE_TEMPORAL_OCCUPANCY_v1.md) — FTD-0772 recurrence-unqualified native candidate.
- [`ANALYSIS_NATIVE_QACTIVE_TEMPORAL_PILOT_v1.md`](../../derivations/native_time_carrier_programme/ANALYSIS_NATIVE_QACTIVE_TEMPORAL_PILOT_v1.md) — FTD-0776 scoped negative and retired exact-profile candidate.
- [`ANALYSIS_QUARTIC_WAVEFORM_NONLINEAR_EDGE_SIGNATURE_v1.md`](../../derivations/native_time_carrier_programme/ANALYSIS_QUARTIC_WAVEFORM_NONLINEAR_EDGE_SIGNATURE_v1.md) — conditional quartic identities and native boundary.
- [`EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md`](../../../09_mathematical/general_math/EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md) — documented forward-power curve, not the root tower defined here.
- [`EXPLR_DYADIC_GEOMETRIC_BYTE.md`](../../../09_mathematical/general_math/EXPLR_DYADIC_GEOMETRIC_BYTE.md) — independent support/ternary-word counts; native map open.
- [`EXPLR_DYADIC_CUBIC_MASTER_QUADRATIC_BOUNDARY.md`](../../../09_mathematical/general_math/EXPLR_DYADIC_CUBIC_MASTER_QUADRATIC_BOUNDARY.md) — no dyadic/master-quadratic promotion.
