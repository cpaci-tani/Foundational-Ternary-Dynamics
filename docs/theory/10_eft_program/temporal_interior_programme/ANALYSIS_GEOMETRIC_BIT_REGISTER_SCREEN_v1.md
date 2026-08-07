# ANALYSIS — The Geometric Bit: T2 First Screen v1

**Status:** `[STATICS-LEVEL SCREEN — SELECTED SCAFFOLD]`; scores one pool
candidate against `SPEC_REGISTER_CRITERIA_v1.md` at statics level; no
LEDGER row minted
**Date:** 2026-08-07 · **Artifact:**
`scripts/experiments/register_geometric_bit_screen.py`
**Parents:** `SPEC_REGISTER_CRITERIA_v1.md` (R1–R9), the registered compact
law (`DERIV_MINIMAL_MANY_BODY_MATTER_NETWORK_v1.md` eq. 1–3), FTD-0777

## 1. The candidate

One `+` body held by three `−` anchors on an equilateral triangle (side
`s`, pinned scaffold). For `s < √3` the body has **two mirror equilibria**
`C_± = (0, 0, ±h)`, `h = √(1 − s²/3)`, all three bonds at exactly `r = 1`
— a barrier-separated two-state system at zero tension: a configurational
bit native to the registered law's statics.

## 2. Measured (exact statics, watershed over the full configuration space)

| s | separation 2h | E₀ | Hessian eigs (×96ε) | true barrier | hinge path |
|---|---|---|---|---|---|
| 0.90 | 1.709 | −3ε | 0.41/0.41/2.19 | **1.021 ε** | 1.000 ε |
| 1.00 | 1.633 | −3ε | 0.50/0.50/2.00 | **1.021 ε** | 1.000 ε |
| 1.10 | 1.545 | −3ε | 0.61/0.61/1.79 | **1.020 ε** | 1.000 ε |
| 1.20 | 1.442 | −3ε | 0.72/0.72/1.56 | **1.016 ε** | 1.000 ε |
| 1.30 | 1.322 | −3ε | 0.85/0.85/1.31 | **1.017 ε** | 1.000 ε |

Three findings. (i) Both states are zero-tension, first-order-rigid
minima — a register is rigid exactly where a clock must not be; the two
purchased structures occupy opposite corners of the same rigidity
criterion. (ii) **The true barrier is `ε` — one bond's dissociation
depth — essentially independent of geometry**: the cheapest flip breaks
one anchor bond, swings on the remaining two-bond hinge, and re-forms
below; the through-plane route costs ~30ε on the repulsive core. The
watershed (union-find over the energy-sorted 3D grid — the statics
analogue of the G5 coupled-escape lesson) confirms no cheaper channel
exists. (iii) The excess of watershed over hinge (≤ 2%) is grid
resolution plus the third-bond tail, not a new channel.

**Consequence:** the register's retention scale is set by the *same* `ε`
that prices the entire matter sector (and the carrier programme's C2
wall). One constant governs how strongly matter binds, how fast a
purchased clock can tick, and how long a purchased memory holds:
`τ_flip ∼ ν₀⁻¹ exp(ε/T_noise)` under any declared noise ensemble
(`[IMPOSED]` — P5 supplies none).

## 3. R-matrix for the geometric bit

| Criterion | Verdict at this scope |
|---|---|
| R1 distinct carrier | **OPEN** — anchors are pinned scaffold here; the self-holding composite is the registered next construction |
| R2 state space | **PASS (exact)** — two states, separation 1.3–1.7, barrier ε, positive-definite Hessians |
| R3 retention | **DEFERRED** — Arrhenius in `ε/T` under a declared ensemble; needs the composite preregistration |
| R4 write | **SKETCHED** — an over-barrier push (≥ ε delivered to the bit) flips it; channel unpriced |
| R5 read | **OPEN** — readout observable undeclared; FTD-0394 budget applies if routed through manifestation |
| R6 drain-freedom | **DEFERRED** — no field coupling in the statics toy |
| R7 co-transport | **OPEN** — requires the composite (bit + frame + clock) |
| R8 licensing | **PASS at scope** — registered law only; pinned anchors declared `[SELECTED]` |
| R9 preregistration | **HONORED in form** — exact instrument, full-space watershed; the verdict-producing dynamics run will carry its own lock |

## 4. Pool matrix (qualitative, at criteria level)

| Candidate | R2 | R3 outlook | Blockers |
|---|---|---|---|
| geometric bit (this screen) | exact ✓ | Arrhenius(ε/T) | composite construction (R1/R7) |
| dressed-composite internal modes | oscillatory states fail fast (`Γ_E = 0.0065`/tick, FTD-0676); configurational sub-class unscored | unknown | engine campaign required |
| history fiber (FTD-0494/0495) | n/a | n/a | baseline only, per its own registered scope |
| relational kernel state (FTD-0669) | plausible | unknown | cleared-region energy-ledger discipline; engine campaign |

## 5. Registered next constructions (in order)

1. **The self-holding bit**: anchors bonded into a frame (MVC-grade
   pieces) so R1 closes — a statics + rigidity verification like today's,
   including the coupled-escape gate.
2. **The clock–register composite**: the bit riding with an MVC 4-chain;
   retention measured in clock cycles under a declared ensemble — the
   first full R1–R9 candidate, preregistered.
3. Only then rung b of the tracker ladder (`SPEC_DISPOSITION_TRACKER_
   LADDER_v1.md`) unblocks.
