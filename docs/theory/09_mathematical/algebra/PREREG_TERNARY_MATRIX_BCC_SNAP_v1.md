# PRE-REGISTRATION — Ternary-Matrix BCC-Snap Test (v1)

**Tag:** `[PRE-REGISTRATION]`. Locks the methodology of a measurement **before** the measurement is run.
**Date:** 2026-05-23.
**LEDGER row:** to be assigned (proposed FTD-0190 series; will be confirmed when result is committed).
**Runner:** `scripts/exploration/explore_ternary_matrix_iteration.py`, SHA256 filled in at hash-lock time.
**Hash-lock status:** **PENDING** (see §7). This pre-registration is **not in force** until the git tag `preregister-ternary-matrix-bcc-snap-v1` is created over a commit containing this file and the runner. **The script must NOT be executed before that tag exists** — a pre-run measurement voids the pre-registration.

---

## §0 — Pre-registration discipline

Everything in §§2–5 below — the construction, the sweep grid, the falsifiable prediction, the outcome → tag mapping, the controls — is **fixed now, before measurement**. The git tag locks the runner's SHA256 at registration time. Any edit to §§2–5 invalidates v1 and forces a fresh v2 (a new hash-lock + a re-run); the result of a v2 wording cannot be retro-credited to v1. The result is reported as it returns — **including a result that falsifies the proposal** — with no reinterpretation.

This test is exploratory mathematics. It does not promote any FTD tag. Its purpose is to determine whether a specific structural claim made in a user-presented synthesis (see §1) survives an explicit construction + numerical test, or closes negative under that test.

## §1 — Purpose and motivation

A user-presented synthesis (2026-05-23 session) proposes a four-step extension of FTD's algebraic spine: (1) Borwein/Guillera quartic self-replication for `G*` (an existing computational route, see [`REF_GUILLERA_CORPUS_MAP.md`](../general_math/REF_GUILLERA_CORPUS_MAP.md)); (2) promotion of Guillera's scalar operator `𝒟 = a + bϑ_x` to a matrix operator `𝒟_T = A(x) + B·Θ` on a ternary carrier; (3) the claim that iterates of `𝒟_T` "snap" onto BCC lattice nodes; (4) identification of the iteration's limit set with a deterministic replacement for QM wave-function collapse.

This pre-registration tests **only step (3)**, the load-bearing structural claim. Steps (1), (2) are scaffolding, and step (4) is out-of-scope (see §4.5 of the session plan; cf. canonical-collapse-mechanism document [`DERIV_COLLAPSE_MECHANISM.md`](../06_reference_frames_and_measurement/DERIV_COLLAPSE_MECHANISM.md)).

**The problem.** A matrix promotion of a scalar identity is mathematically under-determined: many matrices reduce to a given scalar identity. The choice of `A(x)`, `B`, `Θ`, and the carrier is load-bearing. Without an explicit construction, the BCC-snap claim has no falsifiable content. This document pins all four choices explicitly, defines what "snap to BCC nodes" means operationally, and pre-commits the outcome-to-tag mapping.

**The Guillera fence.** Per [`REF_GUILLERA_CORPUS_MAP.md`](../general_math/REF_GUILLERA_CORPUS_MAP.md) §0 (2026-05-21): citation of Guillera in FTD is scholarly attribution for the mathematical tools FTD uses, never third-party validation of the framework. §4 enumerates non-bridges (e.g. Guillera  Calabi–Yau  string-theory) as warnings against pattern-matching overreach. **This test therefore claims no Guillera-validates-FTD bridge regardless of outcome.** A positive result is a `[NUMERICAL FACT]` about a specific 3×3 matrix family on the BCC sub-lattice direction set; a negative result closes the proposal on that family.

## §2 — The construction (FROZEN)

### §2.1 Carrier space — FROZEN

The carrier space is `ℝ³`, with the standard orthonormal basis `{e₁, e₂, e₃}`. The BCC primitive offsets are the FTD-canonical `(±1, ±1, ±1)` per [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](../08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md) line 27. Implementation uses `mpmath.mpf` at 50-digit precision.

### §2.2 The matrix family `𝒟_T = A + B · Θ` — FROZEN

**Θ (the discrete theta / Euler operator):** `Θ = diag(1, 2, 3)`. This is the natural ℝ³ projection of Guillera's `ϑ_x = x · d/dx` to a finite-dimensional carrier: on the standard basis it weights the i-th coordinate by `i`, exactly as `ϑ_x` weights the n-th coefficient of a power series by `n`. **FROZEN.**

**A (diagonal):** two candidate forms are pre-committed.

- **A1** — master-quadratic-coefficient ladder: `A1 = diag(G*, G*², G*³)` (the same `G*` powers that appear as coefficients of the master quadratic `x² − 16G*²x + 16G*³`).
- **A2** — canonical three-constant set: `A2 = diag(G*, ϖ, π)`, using `G* = 2ϖ/√π` to keep the three constants in their natural relation. (`ϖ` is the lemniscate constant, distinct from `G*` per FTD-0117.)

Both are tested. **FROZEN.**

**B (off-diagonal):** four candidate sign patterns are pre-committed. All have zero diagonal and entries in `{−1, 0, +1}`, encoding BCC-like coupling between the three coordinates.

- **B1 — all-positive symmetric:** `B1 = [[0,1,1],[1,0,1],[1,1,0]]`. Sum of off-diagonal entries: +6.
- **B2 — sign-pattern from (1,1,1)·(1,−1,1):** `B2 = [[0,1,1],[−1,0,1],[1,−1,0]]`. Tests an asymmetric BCC sign.
- **B3 — cyclic antisymmetric:** `B3 = [[0,1,−1],[−1,0,1],[1,−1,0]]`. (Matrix of the cross-product `v ↦ (1,1,1)/√3 × v`; its kernel is the BCC primitive direction `(1,1,1)/√3` itself — an a priori non-trivial structural feature to test.)
- **B4 — diagonal-Toeplitz `sign(i−j)`:** `B4 = [[0,1,1],[−1,0,1],[−1,−1,0]]`. Tests the upper-triangular sign convention.

**FROZEN.**

### §2.3 The iteration rule — FROZEN

Power iteration on the unit sphere `S²`:

```
v_{k+1} = (A + B · Θ) · v_k / || (A + B · Θ) · v_k ||
```

This converges (generically) to the right-dominant eigenvector of `A + B · Θ` (the eigenvector associated with the largest-modulus eigenvalue). The normalization is critical — without it the iteration either blows up or contracts to zero, neither of which can "snap to a lattice node."

**Termination:** when `|| v_{k+1} − v_k ||₂ < 10⁻¹²` or `k = 500`, whichever comes first.

**Seeds:** five seed states generated deterministically from `numpy.random.default_rng(seed=42)`, drawn from `Normal(0, 1)³` and then normalized. These five seeds are committed to the runner and frozen.

## §3 — Sweep design (FROZEN)

**Primary sweep:**

- 2 `A` candidates × 4 `B` candidates × 5 seeds = **40 iterations**.

**Control 1 (B = 0):**

- 2 `A` candidates × 5 seeds = **10 iterations**. Tests whether the BCC-snap (if any) requires the off-diagonal coupling.

**Control 2 (randomized B):**

- For each of 10 random instances per `A` candidate: `B` entries drawn uniformly from `{−1, 0, +1}` with the diagonal forced to zero (deterministic `numpy.random.default_rng(seed=12345)`). 2 `A` × 10 random `B` × 5 seeds = **100 iterations**. Tests whether *any* off-diagonal pattern produces BCC-snap, or only the four pre-committed natural patterns.

**Total: 150 iterations.**

## §4 — The falsifiable prediction (FROZEN)

### §4.1 Definition of "snap to a BCC direction"

The 8 BCC primitive directions are the unit vectors `b_{(s₁,s₂,s₃)} = (s₁, s₂, s₃) / √3` for `(s₁, s₂, s₃) ∈ {−1,+1}³`. They form 4 antipodal pairs, so on the projective sphere there are **4 distinct BCC directions** modulo sign.

A converged iterate `v_∞` "snaps to a BCC direction" if there exists `(s₁, s₂, s₃) ∈ {−1,+1}³` such that

```
d_BCC(v_∞) := min_s || v_∞ − s · b_{(s₁,s₂,s₃)} ||₂ ≤ 10⁻⁶,
```

where the inner min runs over the choice of sign `s ∈ {−1,+1}` (antipodal identification).

The 6 "axis directions" `±e_i` (for `i ∈ {1,2,3}`) are the alternative natural attractors of a diagonal-dominant matrix. Define `d_axis(v_∞)` analogously.

### §4.2 The pre-registered prediction

**P1 (positive — the proposal's claim):** For at least one of the 8 (A, B) pairs from the primary sweep, the converged dominant eigenvector satisfies `d_BCC < 10⁻⁶` *and* `d_BCC < d_axis` (i.e. the limit lands on BCC, not on an axis), uniformly over the 5 seeds.

**P2 (negative — falsification of the BCC interpretation):** Either (a) no (A, B) pair satisfies P1, or (b) the same fraction of randomized-B controls also satisfy `d_BCC < 10⁻⁶` (the BCC-snap is generic, not BCC-specific).

### §4.3 Outcome → tag mapping (FROZEN)

| Outcome | Operational signature | Tag for the proposal | Disposition |
|---|---|---|---|
| **A. BCC-snap on at least one (A, B), absent in B=0 and rare in random-B controls** | P1 holds for some (A, B); `d_BCC > 10⁻³` for all B=0 controls; random-B BCC-snap rate `< 10%` | `[NUMERICAL FACT]` + `[OBSERVATION]`; the structural claim survives this test | Write `EXPLR_TERNARY_MATRIX_BCC_SNAP.md` documenting result; open LEDGER row at honest tag; **no theorem upgrade**, no promotion of x₊ = 1/α status |
| **B. Convergence but to a different attractor structure** | iterates converge (`||v_{k+1} − v_k|| → 0`) but `d_BCC > 10⁻³` for all (A, B); some other structure (e.g. dominant eigenvector ≈ `e_3`, the largest-A_ii axis) | `[CLOSED NEGATIVE]` on BCC-snap; `[NUMERICAL FACT]` for the alternative attractor | Write a closed-negative summary; document the actual attractor honestly |
| **C. Random-B controls match the BCC-snap rate of natural B** | P1 holds for some natural (A, B), but random-B controls show BCC-snap at `≥ 10%` rate | `[CLOSED NEGATIVE]` on BCC-specificity (the snap is not structurally tied to BCC) | Closed-negative writeup; explain that the snap is generic |
| **D. No convergence under any natural (A, B)** | iteration fails to reach `||v_{k+1} − v_k|| < 10⁻¹²` within 500 steps for all natural (A, B) | `[CLOSED NEGATIVE]` on construction as stated | Closed-negative writeup; proposal would need a different iteration rule to survive |

### §4.4 Items explicitly **out of scope**

This test does NOT bear on:

- The identification `x₊ = 1/α` (FTD-0013, `[STRONGLY MOTIVATED CONJECTURE]` per LEDGER) — unchanged regardless of outcome.
- The collapse-mechanism question (canonical proposal in `DERIV_COLLAPSE_MECHANISM.md` is a Softplus/Lindblad / Type III₁→I phenomenon, not a BCC-snap; the present test does not engage it).
- The claim "gravity is the quartic folding of mathematical tension" (overclaim per LEDGER; gravity is `[PARTIAL]` per FTD-0131).
- The Weierstrass-class fractal limit (adjacent and testable in a separate experiment via basin-of-attraction analysis; not in this run).

## §5 — Methodological guards

**F1 (pattern-matching overreach).** The four B sign-patterns are pinned BEFORE any execution. The random-B control of 10 instances per A is the look-elsewhere test: if BCC-snap appears across random patterns at rate ≥ 10%, the natural-pattern result is not load-bearing.

**F3 (aesthetic capture).** The 4 pre-committed B patterns include the "elegant" cyclic-antisymmetric B3 (whose kernel is `(1,1,1)/√3`) but it is one of four on equal footing; a positive result must satisfy P1 for *at least one* pattern, and the script reports all four.

**F9 (collusion bias).** The script is deterministic — same seeds, same precision, same construction. Anyone can `git checkout preregister-ternary-matrix-bcc-snap-v1` and reproduce. The construction is the smallest viable matrix that captures the four ingredients of the synthesis (G*, BCC, Θ, ternary carrier).

**F10 (tag-as-resolution).** A positive Outcome A does not upgrade `x₊ = 1/α` to `[DERIVED]`; it produces a `[NUMERICAL FACT]` about a specific eigenvector. The fence around step (4) of the original synthesis (collapse mechanism, gravity) holds regardless.

**Guillera-fence guard.** The runner does not call any Guillera function. `G*` is taken from `scripts/constants.py` (canonical FTD constant; computed by `mpmath` to 50-digit precision). `ϖ` and `π` likewise. The proposal is tested as a matrix-eigenvector question on FTD-canonical constants, *not* as an extension of Guillera's mathematics into physics.

## §6 — Runner specification

**File:** `scripts/exploration/explore_ternary_matrix_iteration.py`
**SHA256 (hash-lock):** `77c2fc6965b83d9392fbd0c8fa30fb39210cf1663346e49aa2e2ec42ef256c1d`
**Dependencies:** `mpmath` (≥1.3), `numpy` (≥1.24). No scipy.
**Output:**
- `scripts/exploration/results/ternary_matrix_iteration_2026-05-23.csv` — full sweep table: one row per iteration, columns `(label, A_form, B_form, seed_idx, converged, n_steps, d_BCC, d_axis, dominant_eigvec, dominant_eigval)`.
- `scripts/exploration/results/ternary_matrix_iteration_2026-05-23.md` — outcome interpretation following §4.3 mapping.

**Reproducibility:** running the script at the pre-registered git tag must produce identical CSV byte-for-byte (modulo timestamp comment line).

## §7 — Hash-lock and execution authorization

This pre-registration becomes hash-locked when:

1. This file is committed to `main`.
2. The runner file is committed to the same commit (or an earlier reachable commit) with its real SHA256 inserted in §6.
3. The git tag `preregister-ternary-matrix-bcc-snap-v1` is created over that commit.

Until all three are done, the runner must NOT be executed.

After hash-lock: the runner is executed exactly once at the locked SHA256, the results are committed under `scripts/exploration/results/`, the outcome is mapped to a tag via §4.3, and the EXPLR_ writeup is produced. No re-runs at the same tag; if the construction is later refined, the change goes to v2 with a fresh hash-lock and a fresh runner SHA256.

## §8 — Cross-references

- [`REF_GUILLERA_CORPUS_MAP.md`](../general_math/REF_GUILLERA_CORPUS_MAP.md) — Guillera fence (§0, §4).
- [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](../08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md) — canonical BCC formalism; (±1, ±1, ±1) primitive offsets.
- [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md) — Theorems 1–9 (G*, master quadratic) at their honest tier per §0.
- [`DERIV_COLLAPSE_MECHANISM.md`](../06_reference_frames_and_measurement/DERIV_COLLAPSE_MECHANISM.md) — canonical collapse proposal (out-of-scope for this test).
- [`LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) — per-claim tags.
- `scripts/proofs/proof_quartic_quarter_constants.py` — Guillera's quartic instance (not invoked by this runner).
