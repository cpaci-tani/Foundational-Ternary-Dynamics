# AUDIT — Newton-postulates reconciliation: SPEC_FTD_LAGRANGIAN.md vs DERIV_NEWTON_FROM_SUBSTRATE.md

**Tag:** `[AUDIT]` — cross-doc tag-reconciliation verdict. Records which `[POSTULATE]` flags in `DERIV_NEWTON_FROM_SUBSTRATE.md` (2026-05-03) are subsumed by `[THEOREM]`-tagged content in `SPEC_FTD_LAGRANGIAN.md` (v3.2, 2026-03-16) + cognate docs. **Does not** promote tags in any other doc; recommends downstream housekeeping edits that should be executed as separate commits.
**Date:** 2026-05-24
**LEDGER row reservation:** to be confirmed against `../07_assessment/core_ledgers/LEDGER.md` (provisional placeholder; next-free below FTD-0203).
**Plan:** `~/.claude/plans/let-s-plan-that-as-twinkling-volcano.md` v2 (Wilsonian reframe) — Arc B P0 → P1 deliverable per `SCOPE_NEWTON_POSTULATES_RECONCILIATION.md` §5.
**Sources read in full or in load-bearing sections:**
- [`DERIV_NEWTON_FROM_SUBSTRATE.md`](../../03_derivations/gravity_and_cosmology/DERIV_NEWTON_FROM_SUBSTRATE.md) (2026-05-03, FTD-0131) — §0 through §8
- [`../01_reference/SPEC_FTD_LAGRANGIAN.md`](../01_reference/SPEC_FTD_LAGRANGIAN.md) (v3.2, 2026-03-16) — §1, §2, §3.1–§3.7, §4.1–§4.3, §5.1–§5.5, §6.1–§6.3, §7, §8 claims table L-1..L-11
- [`../02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md`](../02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md) — §14.1–§14.9 (gravity-as-tick-rate-variation)
- [`DERIV_EINSTEIN_FIELD_EQUATIONS.md`](../../03_derivations/gravity_and_cosmology/DERIV_EINSTEIN_FIELD_EQUATIONS.md) (2026-02-25) — Step 1 through Step 5 + Verification Checks + Claims Table EFE-1..EFE-13
- [`../../../scripts/proofs/proof_newton_from_substrate.py`](../../../scripts/proofs/proof_newton_from_substrate.py) (239 lines)
- [`../../../engine/include/ftd/ontic.h`](../../../engine/include/ftd/ontic.h) (umbrella header)
- [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) — FTD-0131, FTD-0189, FTD-0026 rows (sampled)

---

## §0 — Summary verdict

| Postulate (DERIV_NEWTON_FROM_SUBSTRATE.md) | Reading | Substantive closure source | Net status |
|---|---|---|---|
| **P1** §1.2 `ρ_g(x) = K_B^grav · 𝟙_manifested(x)` | **READING A** (tag mismatch; DERIV_NEWTON `[POSTULATE 1]` is residue) | SPEC §4.2 [THEOREM] (variation of S w.r.t. ℒ → `∇²ℒ = 4πGρ_mass` with `ρ_mass = K_B · n`, K_B = m_e per §3.4) under identification `K_B^grav = K_B` | **Substantively closed**; reduces to LEDGER + DERIV_NEWTON §4-table housekeeping (~1 day) |
| **P2** §1.4 `dτ/dT_U = 1 + 2φ_g/c²` | **READING A under convention reconciliation** (DERIV_NEWTON's "tick_rate" = `g_00`; SPEC's "tick rate" = `dτ/dt` = √f) | SPEC §4.3 [THEOREM] full form `dτ/dt = √(f-v²/f)`, modulo the **clock hypothesis** which is the genuine remaining interpretive step | **Substantively closed modulo clock hypothesis**; reduces to (a) housekeeping + (b) clock-hypothesis treatment (~1–2 weeks) |

**Net for Arc B horizon (Wilsonian-reframe plan v2):** confirmed at **2–4 weeks** (vs v2's pre-reconciliation estimate of 6–10 weeks). Critical-path Arc B becomes housekeeping + clock-hypothesis attempt, not a multi-week independent substrate-physics derivation.

**Genuine remaining open piece for Arc B:** substrate-derivation (or honest-axiom tier) of **the clock hypothesis** — the interpretive identification "Born-Infeld action measure IS proper time".

**Bonus finding (separate housekeeping item):** `DERIV_EINSTEIN_FIELD_EQUATIONS.md` (2026-02-25) tags EFE-6, EFE-8, EFE-9 as `[THEOREM]` citing Theorem 14.1 of `DERIV_RELATIVITY_DERIVATION.md`, but FTD-0189 (2026-05-21) retagged Theorem 14.1 to `[SELECTION/CONDITIONAL]`. `DERIV_EINSTEIN_FIELD_EQUATIONS.md` is **STALE** relative to FTD-0189 and should be re-audited. This is a load-bearing input for Arc C2 boundary-theorem framing.

---

## §1 — Postulate 1 reconciliation (source coupling)

### §1.1 — Verbatim side-by-side

`DERIV_NEWTON_FROM_SUBSTRATE.md` §1.2 (line 47–51):
> "For the gravitational source density, postulate that each manifested voxel acts as a gravitational source of strength `K_B_grav` (a coupling constant to be determined):
> `ρ_g(x) = K_B^grav · 𝟙_manifested(x)`
> **[POSTULATE 1, flagged]**: The gravitational source is proportional to manifestation indicator. Substrate-derivation of this coupling form (e.g., from the Born-Infeld action of `SPEC_FTD_LAGRANGIAN.md` extended to gravity) is [OPEN]."

`SPEC_FTD_LAGRANGIAN.md` §3.4 (line 117): `K_B = M_P√(2π)·(16/3)·α¹¹ = 0.510 MeV (= m_e)`, tag `[THEOREM]` for the formula form (depends on FTD-0015 which is `[SMC]` per LEDGER).

`SPEC_FTD_LAGRANGIAN.md` §4.2 (lines 205–227) [THEOREM]:
> "The latency field ℒ appears in both the matter and gravitational sectors. ...
> **From the matter term:** Using `f = 1 - ℒ²` and `∂f/∂ℒ = -2ℒ`: `∂ℒ_matter/∂ℒ = K_B · ℒ(f²+v²) / (f^{3/2}√(f²-v²))`
> In the static, weak-field limit (v = 0, ℒ << 1): this reduces to `K_B · ℒ ≈ ρ_mass · ℒ`, where `ρ_mass = K_B · n` is the mass density (n = number density of manifested sites).
> ...
> **The field equation:** Setting the total variation to zero and taking the weak-field limit:
> `∇_L² ℒ = 4π G ρ_mass`
> **This is Poisson's equation**, derived from the action — not postulated. ☐"

`SPEC_FTD_LAGRANGIAN.md` §8 claims table line 367: **L-7 `[THEOREM]`** — "Poisson equation ∇²ℒ = 4πGρ follows from the action".

### §1.2 — The reconciliation

SPEC §4.2 derives the matter-source coupling form as `ρ_mass = K_B · n` where `n` is the number density of manifested voxels and `K_B` is the manifestation threshold defined in §3.4 = m_e = 0.511 MeV. For a single-cluster source, `n_voxel = 𝟙_manifested(x)`, so:
- SPEC §4.2 form: `ρ_mass(x) = K_B · 𝟙_manifested(x)` with `K_B = m_e`
- DERIV_NEWTON P1 form: `ρ_g(x) = K_B^grav · 𝟙_manifested(x)` with `K_B^grav` "to be determined"

**These are the same equation under `K_B^grav = K_B = m_e`.**

The `proof_newton_from_substrate.py` script (line 104) then INVERTS to find what K_B_grav must be to match the measured G_N. But this inversion is unnecessary under the SPEC §4.2 reading: the source coupling form IS the derivation; K_B^grav = K_B = m_e directly.

The dimensionless prediction `α_G(e,e) = (m_e/m_P)²` (FTD-0131 §2.1, 0.38% match) is the cleanest check: it uses K_B = m_e throughout, never invokes a separate `K_B^grav` constant, and matches measurement. If K_B^grav were genuinely distinct, the 0.38% numerical match would be coincidental.

### §1.3 — Verdict and recommended housekeeping

**Verdict: READING A.** SPEC §4.2 [THEOREM] substantively closes P1 under `K_B^grav = K_B = m_e`.

The `K_B^grav` superscript in DERIV_NEWTON §1.2 is bookkeeping ("the coupling constant entering the gravity term"), not a claim that the value differs from K_B. The `[POSTULATE 1, flagged]` tag is residue from the older 2026-05-03 derivation that did not cross-reference SPEC §4.2.

**Recommended housekeeping (deferred to separate commits per ledger-discipline):**
1. `DERIV_NEWTON_FROM_SUBSTRATE.md` §1.2: footnote `[POSTULATE 1]` line with "Subsumed by SPEC_FTD_LAGRANGIAN.md §4.2 [THEOREM] under K_B^grav = K_B = m_e identification (per AUDIT_NEWTON_POSTULATES_RECONCILIATION.md §1)."
2. `DERIV_NEWTON_FROM_SUBSTRATE.md` §4 honest-tagging table: change row "1.2 | ρ_g = K_B_grav · 1_manifested | **[POSTULATE 1]**" to "**[DERIVED via SPEC §4.2 [THEOREM]]**".
3. `LEDGER.md` FTD-0131 row: update "2 postulates flagged" to "1 postulate flagged (clock hypothesis, see §2 below)".
4. `proof_newton_from_substrate.py` lines 11-12 + line 78: update comments so STEP 2 K_B_grav explicit equals K_B; STEP 3 keep POSTULATE flag pending §2 clock-hypothesis closure.

These are documentation reconciliations; no tag promotion in this audit itself. (Each housekeeping item must land as a separate commit reviewed against LEDGER discipline; promotion happens at the commit that lands the change, not in this audit.)

---

## §2 — Postulate 2 reconciliation (linearized tick-rate response)

### §2.1 — Verbatim side-by-side

`DERIV_NEWTON_FROM_SUBSTRATE.md` §1.4 (lines 65–73):
> "Per `FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md` §3.2: local tick rate responds linearly to local gravitational potential:
> `dτ/dT_U(x) = 1 + 2·φ_g(x)/c²`
> In lattice units (c_lat² = 1/3): tick_rate = 1 + 6 · φ_g.
> **[POSTULATE 2, flagged]**: Linearized tick-rate response with coefficient 2/c². This matches GR's standard linearization of `g_00 = -(1 + 2Φ/c²)`. Substrate-dynamics derivation that produces *exactly* this coefficient (rather than α/c² or β/c² for some other constant) is [OPEN]. The coefficient 2 is what GR requires; FTD postulates it to match."

`SPEC_FTD_LAGRANGIAN.md` §4.3 (lines 229–239) [THEOREM]:
> "The proper time per coordinate tick follows from the Born-Infeld core. The action of a free particle (s = 0, no constraint term) is `S = -K_B Σ_t √((f²-v²)/f)`. **By the clock hypothesis**, `dτ ∝ √((f²-v²)/f) dt`, giving:
> `dτ/dt = 1/γ_FTD = √(f²-v²)/√f = √(f - v²/f)`
> With `f = 1 - r_s/r` (Schwarzschild identification), this is **exactly** the proper time of the Schwarzschild metric ... The agreement is exact for all `f ∈ (0, 1]` and all `v ∈ [0, f)`. ☐"

`SPEC_FTD_LAGRANGIAN.md` §5.2 [THEOREM]: `dτ/dt = √f = √(1 - ℒ²)` (v=0 special case → gravitational time dilation).

`FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md` §14.2 [THEOREM] (line 629): `dτ/dT_U = √f(r) = √(1 - r_s/r)`.

`SPEC_FTD_LAGRANGIAN.md` §8 claims table line 360: **L-1 `[THEOREM]`** — "Born-Infeld core exactly reproduces Schwarzschild proper time for all f".

### §2.2 — The convention/factor-of-2 issue

DERIV_NEWTON §1.4 writes `dτ/dT_U = 1 + 2·φ_g/c²` (coefficient 2). Linearizing SPEC §4.3's `dτ/dt = √f = √(1 - r_s/r) ≈ 1 - r_s/(2r) = 1 - GM/(rc²) = 1 + φ_g/c²` gives coefficient 1, not 2.

**Resolution**: DERIV_NEWTON §1.4's "tick_rate" is implicitly the metric component `g_00`, not the proper-time ratio `dτ/dt = √g_00`. The note "matches GR's standard linearization of `g_00 = -(1 + 2Φ/c²)`" confirms this (it's quoting the `g_00` linearization). The factor of 2 disappears when going from `g_00` to `dτ/dt` via the square root.

Both DERIV_NEWTON §1.4 and SPEC §4.3 are correct under their own conventions. They compute the same physical content:
- DERIV_NEWTON: `g_00 = 1 + 2φ_g/c²` (linearized metric component)
- SPEC §4.3: `dτ/dt = √f = √g_00 ≈ 1 + φ_g/c²` (linearized proper-time ratio)
- Square root translates: `√(1 + 2φ_g/c²) ≈ 1 + φ_g/c²` for small φ_g/c²

### §2.3 — The genuine open piece: the clock hypothesis

SPEC §4.3's [THEOREM] derivation says: "**By the clock hypothesis**, `dτ ∝ √((f²-v²)/f) dt`". The clock hypothesis is the interpretive step that identifies the Born-Infeld action measure with proper time.

A grep across `docs/` for "clock hypothesis" returns **2 files**: this AUDIT doc and `SPEC_FTD_LAGRANGIAN.md` itself. The clock hypothesis is **NOT formally tagged anywhere** in the FTD corpus. It is treated as a definitional / interpretive step in SPEC §4.3 without explicit `[AXIOM]` or `[SELECTION]` flag.

This is the genuine remaining substantive question for Arc B P2:

1. **Can the clock hypothesis be substrate-derived?** If substrate manifestation rate scales with the Born-Infeld action measure (which has independent physical content via the bandwidth constraint `v < f`), this could be a substrate-level theorem. The chain would be: (a) substrate dynamics → tick rate scales with Born-Infeld action → (b) action measure IS proper time by independent derivation, not by hypothesis. The bandwidth-constraint argument (§3.7 of SPEC: "v < f", "equivalence principle made manifest: velocity and gravity draw from same bandwidth budget") is a candidate route.
2. **Or is it irreducibly interpretive?** In standard relativistic-particle theory the clock hypothesis is an empirical input (ideal clocks measure proper time along their worldline). If FTD's clock hypothesis is similarly interpretive, it should be tagged `[AXIOM]` or `[SELECTION]` explicitly in SPEC §4.3.

### §2.4 — Verdict and recommended action

**Verdict: READING A under convention reconciliation, modulo clock hypothesis.** SPEC §4.3 [THEOREM] substantively closes DERIV_NEWTON's `[POSTULATE 2]` for the full form `dτ/dt = √(f-v²/f)` for all `f ∈ (0,1]`. The linearized version is a corollary. The factor-of-2 discrepancy is a `g_00`-vs-`dτ/dt` convention difference, not a substantive disagreement.

**The actual `[OPEN]` piece narrows to the clock hypothesis.** This is a much sharper, smaller target than "derive coefficient 2/c² from substrate dynamics" (which DERIV_NEWTON §1.4 framed as the obstacle).

**Recommended action (Arc B P1 P1, ~1–2 weeks):**
1. Bandwidth-constraint attempt: derive the clock hypothesis from §3.7 SPEC v<f constraint + substrate manifestation-rate scaling. If successful → SPEC §4.3 fully [THEOREM] without conditional flag.
2. If attempt fails → tag clock hypothesis as `[AXIOM]` in SPEC §4.3 explicitly + propagate to LEDGER FTD-0131 row + DERIV_NEWTON §4 table.

Either outcome is honest progress on Arc B and serves the Wilsonian-reframe success criterion.

**Recommended housekeeping for the convention difference (deferred):**
- `DERIV_NEWTON_FROM_SUBSTRATE.md` §1.4: footnote that "tick_rate" refers to `g_00`, not `dτ/dt`, and that the linearized statement is consistent with SPEC §4.3's full form `dτ/dt = √f` after square-root.
- Update the `2/c²` vs `1/c²` framing in DERIV_NEWTON §1.4 to remove ambiguity.

---

## §3 — FTD-0189 ripple analysis (load-bearing for Arc C2)

`SPEC_DOCTRINE_LEDGER.md` cross-references and FTD-0189 LEDGER row (sampled): the 2026-05-21 audit retagged:
- Theorem 14.1 (`DERIV_RELATIVITY_DERIVATION.md`): "linearized Einstein [THEOREM]" → `[SELECTION/CONDITIONAL]` (rests on h ↔ J correspondence)
- Theorem 15.3 (`DERIV_RELATIVITY_DERIVATION.md`): "graviton = transverse flux modes [THEOREM]" → `[CONJECTURE]` + spin-count flag (2 transverse J-modes = helicity-±1, not ±2)

**Does FTD-0189 audit propagate to SPEC §4.2, §4.3, §4.1?**

| SPEC content | Uses h_μν correspondence? | FTD-0189 impact |
|---|---|---|
| §4.1 [THEOREM] — Newton's 2nd law from EL equation on Born-Infeld | No (Lagrangian variation; no metric tensor decomposition) | **Survives unchanged** |
| §4.2 [THEOREM] — Poisson from ℒ variation | No (scalar latency ℒ; no h_μν) | **Survives unchanged** |
| §4.3 [THEOREM] — Born-Infeld → Schwarzschild proper time | No (uses scalar f only; clock hypothesis) | **Survives unchanged modulo clock hypothesis** |
| §6.2 row "Linearized Einstein eqs (DERIV_EINSTEIN_FIELD_EQUATIONS.md / DERIV_QFT_GRT_BRIDGE.md) [THEOREM]" | YES (cites the Theorem 14.1 chain) | **Should be retagged** `[SELECTION/CONDITIONAL]` per FTD-0189 |
| §6.2 row "Nonlinear completion via Lovelock [THEOREM]" | YES (depends on linearized form as input premise) | **Inherited retag** `[SELECTION/CONDITIONAL]` per FTD-0189 |
| §6.2 row "Exact Schwarzschild via Born-Infeld §4.3 [THEOREM]" | No (cites §4.3 chain) | **Survives unchanged modulo clock hypothesis** |

So SPEC §4.1 + §4.2 + §4.3 + §6.2 Schwarzschild row survive. SPEC §6.2 linearized-Einstein and Lovelock-completion rows should be retagged per FTD-0189 (separate housekeeping item; not blocking Arc B P2).

**For Arc C2 (boundary theorem)**: the load-bearing finding is that `DERIV_EINSTEIN_FIELD_EQUATIONS.md` (2026-02-25) tags EFE-6, EFE-8, EFE-9 as `[THEOREM]` but is **STALE** relative to FTD-0189 (2026-05-21). Specifically:
- EFE-6 [THEOREM] "Linearized Einstein: □ h̄_μν = -(16πG/c⁴) T_μν" cites DERIV_RELATIVITY Thm 14.1 → retag `[SELECTION/CONDITIONAL]`
- EFE-8 [THEOREM] "Nonlinear completion via Lovelock" depends on EFE-6 → inherited retag
- EFE-9 [THEOREM] "Full Einstein equations recovered" inherits from EFE-6 + EFE-8 → retag

Arc C2 boundary theorem statement should reflect this: "FTD substrate-derives `g_00 = 1 - r_s/r` via SPEC §4.3 [THEOREM] modulo clock hypothesis; the full Einstein equations are recovered conditional on Conjecture 10.1 (h_μν posited per DERIV_RELATIVITY) which is `[CLOSED NEGATIVE per FTD-0193]` for substrate emergence in the probed regime". This is consistent with the plan v2 Arc C2 framing.

**Recommended downstream housekeeping** (separate from Arc B, for Arc C2 P0 or independently):
- Audit `DERIV_EINSTEIN_FIELD_EQUATIONS.md` against FTD-0189; retag EFE-6, EFE-8, EFE-9 as appropriate
- Audit `SPEC_FTD_LAGRANGIAN.md` §6.2 against FTD-0189; retag rows that cite linearized-Einstein / Lovelock-completion
- Add cross-reference from `DERIV_EINSTEIN_FIELD_EQUATIONS.md` header to FTD-0189 audit

---

## §3.5 — Arc D gap (iv) verification: ENGINE ALIGNED (added 2026-05-24)

**Verification executed in same session as this AUDIT.** The plan v2 / AUDIT §5 priority 3 item ("audit engine Poisson implementation against SPEC §4.2 [THEOREM]") was run.

**Note**: AUDIT §5 originally cited `engine/src/render_bridge.cpp:709`, but the actual Poisson implementation was moved to `engine/src/poisson_solvers.cpp` during the 2026-04-18 refactor sweep (R1). `render_bridge.cpp:280` is now a thin wrapper: `void RenderBridge::solve_latency_poisson() { solve_latency_poisson_cpu(...); }`.

**The substantive verification (`poisson_solvers.cpp:190-228` `solve_latency_poisson_cpu`):**

```cpp
constexpr double FOUR_PI_G = 4.0 * PI * G_N;          // line 197
// ...
const double rho_mass = K_B * std::abs(voxels[i].state);   // line 206
sor_source[i] = FOUR_PI_G * (rho_mass - mean_mass);        // line 207
// ...
sor_sweep_18pt(phi_latency, sor_source, lattice, OMEGA);   // line 211
```

This is **exactly** SPEC §4.2 [THEOREM] form: `∇_L² ℒ = 4πG · ρ_mass` with `ρ_mass = K_B · n` (number density of manifested sites) and `∇_L²` = 18-point isotropic Laplacian per SPEC §3.1. The K_B coupling is sourced from `using ontic::K_B` (constants.h:143) which pulls from the canonical ontic Layer 6 (= m_e per `ontic.h` umbrella header). The mean subtraction is gauge-fixing for periodic BC compatibility, not a substantive deviation.

**Verdict: gap (iv) CLOSED POSITIVE.** The engine implementation IS the SPEC §4.2 [THEOREM] derivation. No re-engineering needed. Plan v2 Arc D gap (iv) row removed from outstanding work.

**One observation worth noting (not a gap):** the engine's `G_N = 1/(b_3+N_c)² = 0.01` (per `engine/include/ftd/cosmic_engine.h:22` documentation) is the engine-internal coupling constant used by the SOR solver. Per FTD-0131, this engine-internal value is operationally meaningful but is **not** identifiable with physical G_N (which is the falsified identification). The physical interpretation runs through the dimensionless prediction `α_G(e,e) = (m_e/m_P)²` per FTD-0131 §2.1, not through the engine's `G_N = 0.01`. This is consistent with FTD-0131's honest framing; no further action needed.

---

## §4 — Implications for Arc B plan v2

| Plan v2 item | Pre-reconciliation estimate | Post-reconciliation reality | Status |
|---|---|---|---|
| Arc B P0 (scoping) | 1 week | DONE (SCOPE memo + this AUDIT) | **CLOSED** |
| Arc B P1 (theory attempt) | 3-4 weeks per postulate (6-8 wk total) | 1-2 weeks (clock-hypothesis attempt) + 1 day housekeeping | **REDUCED** |
| Arc B P2 (engine instrumentation) | 1 week (Newton-from-substrate test gap (iv) + G_N(M) scan gap (ii)) | Newton-from-substrate test (gap iv) probably already passes — `render_bridge.cpp:709` IS the SPEC §4.2 derivation; reduces to verification of correspondence (~3 days). G_N(M) scan still needed (~1 week) | **REDUCED** |
| Arc B P3 (pre-reg lock) | 1 week per postulate | 1 pre-reg for clock-hypothesis attempt (P2 only; P1 reduces to housekeeping that does not need pre-reg) | **REDUCED** |
| Arc B P4 (closure attempt) | 1 week per postulate | 1 attempt (clock hypothesis) | **REDUCED** |
| Arc B P5 (result document) | Per postulate | `REPORT_CLOCK_HYPOTHESIS_DERIVATION.md` (single) + housekeeping reconciliation memo | **REDUCED** |
| **Arc B total horizon** | **6–10 weeks** | **2–4 weeks** (confirmed per SCOPE §4 prediction) | **REDUCED** |

**Critical-path update for plan v2:** Arc B now runs in ~3 weeks parallel with Arc D gaps (ii) + (v); Arc C2 can start P0/P1 earlier. Total program horizon (Wilsonian reframe) tightens from 8-12 weeks to potentially **6-10 weeks** if Arc B closure (or honest-axiom verdict on clock hypothesis) lands quickly.

---

## §5 — Recommended next concrete actions

In priority order:

1. **Arc B P1 (clock hypothesis substrate-derivation attempt) — 1–2 weeks.**
   - Pre-register `preregister-clock-hypothesis-derivation-v1` per 9-section template
   - Section §4 frozen catalog: SPEC §3.7 bandwidth constraint `v < f`; substrate manifestation rate; Born-Infeld action measure
   - Section §7 falsifiers: F-a no insertion of GR clock-hypothesis; F-j no reverse-engineering from proper-time formula
   - Outcomes: FOUND (clock hypothesis substrate-derived, SPEC §4.3 fully [THEOREM]) / CLOSED-NEGATIVE (clock hypothesis is irreducibly interpretive, tag as `[AXIOM]` in SPEC §4.3) / UNDERDETERMINED
2. **Reconciliation commits (parallel housekeeping) — ~1 day total across 4 commits.**
   - Commit 1: DERIV_NEWTON_FROM_SUBSTRATE.md §1.2 footnote + §4 table update for P1 (Reading A)
   - Commit 2: DERIV_NEWTON_FROM_SUBSTRATE.md §1.4 convention clarification (g_00 vs dτ/dt) + cross-ref SPEC §4.3
   - Commit 3: LEDGER FTD-0131 update ("1 postulate flagged" pending clock-hypothesis verdict)
   - Commit 4: proof_newton_from_substrate.py STEP 2 + STEP 3 comment updates
3. **Arc D gap (iv) verification (concurrent with #1) — ~3 days.**
   - Audit `engine/src/render_bridge.cpp:709` Poisson implementation against SPEC §4.2 derivation
   - Confirm the implementation IS the substrate-derived equation, not a parallel boundary-condition imposition
   - If aligned: gap (iv) closes without new test
   - If misaligned: need to re-engineer Poisson source from substrate manifestation count
4. **Arc D gap (ii) G_N(M) scan implementation — ~1 week.** Independent of #1-#3. Build `benchmark_g_n_mass_spectrum.cpp` per plan v2 Arc D.
5. **FTD-0189 ripple housekeeping (separate from Arc B; supports Arc C2) — ~1 day.** Audit DERIV_EINSTEIN_FIELD_EQUATIONS.md tag freshness; retag EFE-6, EFE-8, EFE-9 as appropriate; add FTD-0189 cross-reference to header.

---

## §6 — What this audit does NOT claim

- **NOT a closure of Arc B.** The clock-hypothesis attempt is the remaining substantive work for Arc B P2; this audit identifies that it's narrower than DERIV_NEWTON §1.4's framing implied but does not execute it.
- **NOT a tag promotion in any doc.** Tag changes (DERIV_NEWTON §4 table, LEDGER FTD-0131, SPEC §6.2 linearized-Einstein/Lovelock rows) require separate housekeeping commits with explicit review.
- **NOT a claim that the substrate derivation of full Einstein equations is complete.** The Lovelock-completion chain in `DERIV_EINSTEIN_FIELD_EQUATIONS.md` is STALE per FTD-0189 ripple; what's actually [THEOREM]-grade is SPEC §4.2 + §4.3 (Poisson + Schwarzschild proper time scalar form). The full Einstein equations remain `[SELECTION/CONDITIONAL]` per FTD-0189 audit.
- **NOT a finding that K_B^grav has been measured to equal K_B independently.** The identification follows from SPEC §4.2's derivation (where the matter coupling IS K_B, no separate constant); the 0.38% α_G(e,e) numerical match in FTD-0131 §2.1 is consistent with this identification but does not independently prove K_B^grav = K_B (since the proof script INVERTS to find K_B^grav rather than computing α_G forward from K_B = m_e). A separate forward-computation test of α_G(e,e) using K_B = m_e directly (no inversion) would be cleaner — recommended as a small follow-up to proof_newton_from_substrate.py.

---

## §7 — Honest limits of this audit

- **Reading B remains possible if SPEC §4.2 / §4.3 has subtler conditions** I missed in section reads. The full v3.2 SPEC doc was sampled in sections (§1, §2, §3.1–§3.7, §4.1–§4.3, §5.1–§5.5, §6.1–§6.3, §7, §8) but not every line; a careful side-by-side full re-read might surface additional caveats.
- **The clock hypothesis treatment in §4.3** is stated as a single phrase ("By the clock hypothesis, dτ ∝ √((f²-v²)/f) dt") without elaboration; whether SPEC's author intended it as derivable or definitional is not explicit. The §3.7 bandwidth-constraint framing ("equivalence principle made manifest: v and ℒ draw from same bandwidth budget") is suggestive of a substrate-physics derivation but not a proof.
- **FTD-0189 audit's exact scope** was sampled from LEDGER position + Doctrine §13.5 cross-refs; the full FTD-0189 ledger row was not read line-by-line. A direct read may surface additional retag implications for SPEC §6.2 or DERIV_EINSTEIN_FIELD_EQUATIONS.md.
- **DERIV_LATTICE_SCHWARZSCHILD.md** is cited multiple times in SPEC §6.2 + DERIV_EINSTEIN_FIELD_EQUATIONS.md but `find` returned no such file. Either the file was renamed/archived, or the content was inlined into SPEC §4.3. This should be verified.
- **DERIV_RELATIVITY_DERIVATION.md** (the source of Theorem 11.1 g_00 = 1 - r_s/r and Theorem 14.1 linearized Einstein) was NOT read. FTD-0189 audited specific theorems within it but the full doc state is not assessed here.

---

## §8 — Single-line summary

**`SPEC_FTD_LAGRANGIAN.md` §4.2 + §4.3 [THEOREM]s substantively subsume DERIV_NEWTON_FROM_SUBSTRATE.md's `[POSTULATE 1]` (under `K_B^grav = K_B = m_e` identification) and `[POSTULATE 2]` (under `g_00`-vs-`dτ/dt` convention reconciliation), modulo a single genuine remaining open piece — the substrate-derivation (or honest-axiom tier) of the clock hypothesis used implicitly in §4.3 — reducing Arc B horizon under the Wilsonian-reframe plan v2 from 6–10 weeks to 2–4 weeks; downstream FTD-0189 audit additionally implies `DERIV_EINSTEIN_FIELD_EQUATIONS.md` EFE-6/EFE-8/EFE-9 tags are STALE and should be retagged `[SELECTION/CONDITIONAL]` per separate housekeeping, with load-bearing input to Arc C2 boundary theorem framing.**
