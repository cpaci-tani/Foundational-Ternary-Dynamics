# FTD-0777 — Finite Dyadic Monodromy Clock–Memory Boundary v1

**Date locked/run:** 2026-08-02  
**Status:** `[EXACT — FINITE COVER] + [SELECTED SYNTHETIC MODEL] + [EXACT NO-GO] + [CONDITIONAL] + [OPEN — NATIVE BRIDGE]`  
**Synthetic verdict:** `DYADIC_MONODROMY_CLOCK_MEMORY_EXACT_PASS`  
**Native verdict:** `NATIVE_DYADIC_CLOCK_MEMORY_NOT_TESTED`

Pre-registration:
[`PREREG_DYADIC_MONODROMY_CLOCK_MEMORY_v1.md`](../preregistrations/PREREG_DYADIC_MONODROMY_CLOCK_MEMORY_v1.md)

## 1. Claim tested

The run tested whether the following finite construction is internally exact:

\[
z_0=e^{i\Theta},
\qquad
z_{j+1}^{2}=z_j,
\qquad
z_K(t;m)=
\exp\!\left(\frac{i(\Theta(t)+2\pi m)}{2^K}\right).
\]

It also tested the boundary among:

- forward harmonics `z_0^(2^j)`, which add resolution but no independent
  state;
- compatible root lifts, which record recurrence epoch modulo `2^K`;
- independent ternary coefficients, which form a different event register;
- a single-tower payload claim, which is obstructed exactly; and
- a selected two-tower relational payload, which is stable under common
  monodromy.

This was an exact/synthetic construction audit. It was not a native FTD engine
campaign.

## 2. Configuration and provenance

| Field | Run-of-record value |
|---|---|
| Repository base commit | `dc3bcf965fb45c6d8684500dcf6e1622eff10583` |
| Protocol SHA-256 | `D1AA721CCB4B6A3D6C6AD657477DC3EE62B865E2821581F18BC4727D641AF32A` |
| Verifier | `scripts/proofs/proof_dyadic_monodromy_clock_memory.py` |
| Verifier SHA-256 | `8907670CAF40D165AFB077639FBAF482FD985AD62C6F3145E92D6B9753DD9685` |
| Runtime | Python `3.13.12`; SymPy `1.14.0` |
| Command | `python scripts/proofs/proof_dyadic_monodromy_clock_memory.py --output-dir engine/results/dyadic_monodromy_clock_memory_20260802` |
| Result | exit `0`; `16/16 PASS` |
| Production engine changes | none |
| Engine execution | none |

The repository worktree already contained unrelated uncommitted temporal-clock
work. The protocol and verifier hashes isolate this execution; no engine state,
trajectory, `q_active` artifact, or mutable production source was consumed.

**Locked-protocol and verifier errata.** The immutable pre-registration has
two stray commas in displayed mathematics: `z_{j+1}^{,2}` in section 2.2 means
`z_{j+1}^{2}`, and `\nu,2^k d_4` in section 2.6 means
`\nu\,2^k d_4`. The surrounding prose, gates G1--G5/G15, and hashed verifier
all use these intended forms. In addition, the verifier's G11 artifact label is
`[EXACT | SELECTED MODEL]`, whereas the locked protocol correctly specifies
`[CONDITIONAL EXACT COMPOSITION]`: the ratio `T_K/T_4=2^K` is exact, but the
quartic formula substituted for `T_4` remains selected and conditional. The
protocol/analysis status controls the interpretation. Finally,
`branch_words.csv` prints `(b_0,b_1,...)` in least-significant-bit-first order;
for example, depth-4 `0001` maps to sheet `8`. None of these points changes a
Boolean gate result. The pre-registration and verifier remain byte-for-byte
unchanged so their recorded hashes remain valid; a future v2 should correct
the labels and lock each gate's ordered status/name/detail tuple in regression
tests.

## 3. Native configuration and imposed terms

### Native configuration

`NOT APPLICABLE / NOT RUN.` There was no lattice size, seed, boundary,
backend, toggle profile, manifested body, or native observable.

### Selected or imposed content

- the compatible square-root tower;
- the one-winding carrier hypothesis;
- maximum certified depth `K=10`;
- the selected two-tower reference repair;
- the prior selected quartic carrier used only in the conditional composition;
- the conditional FTD-0771 edge/clock fraction; and
- the exact synthetic causal control `d_4=1/16`, `nu=2`.

None is relabeled as native or emergent.

### Preselected observable

The sole result object was the locked exact gate table G0–G15. No scalar time
series, fitted period, Fourier peak, or post-result replacement observable was
permitted.

## 4. Raw artifacts

Artifact root:
`engine/results/dyadic_monodromy_clock_memory_20260802/`

| Artifact | SHA-256 | Role |
|---|---|---|
| `gate_table.csv` | `B648F116860A585A4EAEA29FBBA6DD7B6AB924FB1D2D0E3708EC18DB119D077E` | Complete gate table |
| `monodromy_strobe.csv` | `5F0AE6BE55A1E91AEBC41B548A471868C8BE79E4EEC5950F43352A3CCC9B924D` | Depth-4 sheet sequence through first return |
| `branch_words.csv` | `9E8D387FE243AF7FB2260C4AF766AED0BE96760E0095794DCF9C40B11535CC82` | Depth-4 branch-word/sheet bijection |
| `summary.json` | `0B533885633E2E620C51F25CC807D0B4D50616F3246A89D3C28BBDE830027D65` | Scoped verdict and firewalls |

These deterministic artifacts are ignored under the repository's general
`engine/results/` policy. Their paths and hashes are recorded here.

## 5. Complete gate table

| Gate | Result | Epistemic status | Exact result |
|---|---|---|---|
| G0 | PASS | `[EXACT]` | `h_j=z_0^(2^j)` leaves joint capacity equal to carrier capacity |
| G1 | PASS | `[EXACT — SELECTED COVER]` | Depth `K` has exactly `2^K` compatible lifts |
| G2 | PASS | `[EXACT — SELECTED COVER]` | One carrier loop maps `m -> m+1 mod 2^K` |
| G3 | PASS | `[EXACT — SELECTED COVER]` | First lifted return occurs after exactly `2^K` carrier loops |
| G4 | PASS | `[EXACT — SELECTED COVER]` | Every old sheet has two children; horizon doubles |
| G5 | PASS | `[EXACT — SELECTED COVER]` | `K` branch bits biject with the `2^K` sheet addresses |
| G6 | PASS | `[EXACT — SELECTED COVER]` | Variable-depth prefix count is `2^(M+1)-1` |
| G7 | PASS | `[EXACT]` | `M` independent ternary coefficients have `3^M` words |
| G8 | PASS | `[EXACT NO-GO — SELECTED MODEL]` | Permanent append writes obstruct full-state recurrence through a write |
| G9 | PASS | `[EXACT NO-GO — SELECTED MODEL]` | One tower confounds written payload with elapsed epoch |
| G10 | PASS | `[EXACT — SELECTED TWO-TOWER MODEL]` | `(u-r) mod 2^K` is invariant under common transport |
| G11 | PASS | `[CONDITIONAL EXACT COMPOSITION]` | Selected quartic carrier gives `T_(4,K)=2^K T_4` |
| G12 | PASS | `[EXACT BOUNDARY]` | Uniform root coordinate has fourth moment `3/8`, not quartic `1/3` |
| G13 | PASS | `[EXACT — SELECTED COVER]` | `mu_j=exp(2*pi*i/2^j)` has exact order `2^j` |
| G14 | PASS | `[CONDITIONAL]` | Integer-tick carrier period `P_0` gives `P_K=2^K P_0` |
| G15 | PASS | `[CONDITIONAL]` | Forward fast-mode causal ceiling and locked rational control agree |

The depth-4 stroboscopic control visited sheets `0,1,...,15` once and first
returned to sheet `0` after carrier cycle `16`.

## 6. Exact equations and what they establish

### 6.1 Powers versus roots

The forward tower

\[
h_j=z_0^{2^j}
\]

is a deterministic function of `z_0`. Adding powers does not increase the
state count once the carrier is retained.

For the root tower, `z_K^(2^K)=z_0`, so a fixed carrier phase has exactly
`2^K` terminal roots. Each terminal root uniquely determines its compatible
prefix. Thus

\[
|\mathcal F_K(z_0)|=2^K.
\]

### 6.2 Monodromy and recurrence hierarchy

After `n` carrier cycles,

\[
z_j(t+nT)=e^{2\pi i n/2^j}z_j(t).
\]

The full depth-`K` prefix returns iff `2^K` divides `n`. Therefore

\[
T_K=2^K T,
\]

conditional on `T` being the fundamental carrier period with winding number
one. This demonstrates a finite recurrence hierarchy, not autonomous time.

### 6.3 Capacity distinctions

At fixed depth, sequential branch choices give

\[
m_K=\sum_{j=0}^{K-1}b_j2^j,
\]

so there are exactly `2^K` sheet addresses. Across explicitly labeled depths
`0..M`, the disjoint count is `2^(M+1)-1`. Independent ternary modes instead
give `3^M` words and are not the same construction.

### 6.4 Main negative result: one tower is not stable event memory

At a carrier return section,

\[
m_{\rm observed}=m_{\rm written}+n\pmod{2^K}.
\]

The monodromy is one transitive `2^K`-cycle. Hence any payload invariant under
each carrier cycle must be constant. A single tower records epoch modulo
`2^K`; payload and elapsed cycles are confounded.

This corrects the stronger initial intuition that branch capacity alone
supplies stable event memory.

### 6.5 Selected relational repair

If reference and payload towers are co-transported,

\[
r\mapsto r+1,
\qquad
u\mapsto u+1,
\]

then

\[
(u-r)\bmod 2^K
\]

is invariant. The repair is exact, but it requires a second selected physical
structure and a relational decoder. Their native origin remains `[OPEN]`.

### 6.6 Append-only obstruction

For nondecreasing active depth, a strict write gives

\[
K_{\rm end}=K_{\rm start}+W>K_{\rm start}.
\]

Therefore the complete state cannot recur across a write. Exact recurrence is
available only for a fixed-depth state, a named quotient, or after writing
stops.

### 6.7 Quartic and minimum-interval boundaries

The selected quartic composition is

\[
T_{4,K}
=2^K\frac{\sqrt\pi G^*}{\rho(2E)^{1/4}}.
\]

It multiplies an already selected period; it neither derives `G*` nor produces
quartic occupancy. With uniform phase, `Re(z_K)` has the arcsine fourth moment
`3/8`, while the quartic coordinate has `1/3`.

Root levels lengthen the horizon. The shorter interval belongs to forward
harmonics. Under the separately conditional causal matching,

\[
\nu\,2^k
\frac{\rho(2E)^{1/4}}{u\sqrt\pi G^*}
\le1.
\]

This is a ceiling on selected fast modes, not a native derivation of minimum
dimensionless `dt`.

## 7. What the result proves

1. **[EXACT]** Forward dyadic powers do not add independent state.
2. **[EXACT within the selected finite cover]** Depth `K` has `2^K` sheets,
   monodromy is `m -> m+1`, and the first lifted return is after `2^K`
   fundamental carrier cycles.
3. **[EXACT]** Prefix and independent-ternary capacities are different.
4. **[EXACT NO-GO]** A permanent append prevents complete-state recurrence
   through the write.
5. **[EXACT NO-GO]** One transitive tower cannot preserve a nonconstant
   cycle-invariant event payload.
6. **[EXACT within a selected two-tower model]** A co-transported relative
   sheet can preserve a payload.
7. **[CONDITIONAL]** Quartic-period multiplication, integer-tick recurrence,
   and the fast-mode causal ceiling follow under their explicit hypotheses.

## 8. What the result does not prove

- that native FTD generates any carrier, root lift, mode birth, reference lift,
  writer, or decoder;
- that the constructed sheets are voxel degrees of freedom;
- that a synthetic append rule is a physical arrow of time;
- that a global Fourier or Floquet coordinate is locally readable;
- that a mode can be populated without accounted source work;
- that the current engine contains a stable recurrent body;
- that quartic occupancy or `G*` emerges from dyadicity;
- that a minimum physical or dimensionless `dt` has been derived; or
- that FTD-0772 or FTD-0776 is superseded.

## 9. Native bridge gate table

| Native gate | Status | Required future evidence |
|---|---|---|
| Local carrier | `[OPEN] / NOT RUN` | Bounded voxel-local state; no global registry |
| Autonomous occupation | `[OPEN] / NOT RUN` | Local event, no tick/depth oracle |
| Moore causality | `[OPEN] / NOT RUN` | Dependency inside the prior-tick 26-neighbor hull |
| Source/work closure | `[OPEN] / NOT RUN` | Engine energy plus explicit source work closes |
| Persistence | `[OPEN] / NOT RUN` | Source-off lifetime and perturbation robustness |
| Local decoder | `[OPEN] / NOT RUN` | Bounded readout without FFT or elapsed-time oracle |
| Reference carrier | `[OPEN] / NOT RUN` | Native co-transported reference for stable payload |
| Recurrence separation | `[OPEN] / NOT RUN` | Carrier, quotient, memory, and complete state reported separately |
| Robustness | `[OPEN] / NOT RUN` | Volume, boundary, seed, backend, and alias controls |
| Arrow | `[OPEN] / NOT RUN` | Forward/reverse asymmetry plus export ledger |

Existing imposed phase, color, `SU2Link`, and `SU3Link` structures are not
admissible as emergent carriers.

## 10. Retired interpretations

The run retires the following statements **for this construction**:

- “Adding forward harmonics adds independent memory.”
- “One root tower stores a stable arbitrary event payload without a reference.”
- “Every sheet is a separate invariant memory sector.”
- “Adding root levels produces a smaller time step.”
- “Dyadic lifting produces quartic occupancy or derives `G*`.”
- “A permanent append-only full state remains exactly recurrent.”

The narrower surviving construction is:

> A finite compatible root tower is an exact recurrence/epoch hierarchy. A
> stable payload requires an additional relational reference structure.

## 11. Next admissible step

Do **not** run a native mode search from this result alone. The next admissible
step is a fresh, observation-only preregistration that first identifies a
stable localized native recurrence and its intrinsic return map. Only if that
passes may it ask whether the monodromy contains preselected unit-modulus
dyadic roots and whether a local perturbation can populate and decode a pair of
co-transported modes with source/work closure.

That future campaign must not replace `q_active` inside FTD-0776, reuse its
negative output as training data, or treat existing imposed phase/gauge fields
as emergent.
