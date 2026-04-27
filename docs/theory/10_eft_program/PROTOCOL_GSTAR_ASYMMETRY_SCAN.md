# PROTOCOL — G*/π Asymmetry Scan: Pre-Registered Candidate Matrix and Falsifier Criteria

**Tag:** [PROTOCOL · pre-registration]
**Date:** 2026-04-27
**LEDGER row:** FTD-0106
**Companion:** [`AUDIT_GSTAR_ASYMMETRY_SCAN.md`](AUDIT_GSTAR_ASYMMETRY_SCAN.md)
**Plan:** `~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md`

This protocol is **pre-registered before any engine measurement** per CLAUDE.md epistemic discipline (Constraints 8–11). The git tag `preregister-gstar-asymmetry-v1` will be applied at the commit containing this document, BEFORE any per-domain engine campaign extension.

---

## 1 · Scope locked

**Three Tier-1 domains** (per AUDIT §0):

- **Domain A** — Time-direction / dissipation / Rayleigh damping (anchor: `DERIV_HEAT_EQUATION_FROM_RATIO.md` [THEOREM])
- **Domain B** — Coulomb scattering phase shift (anchor: standard Γ-ratio at complex-conjugate args)
- **Domain C** — Hawking evaporation timescale and BH thermodynamic prefactors (anchor: `DERIV_BLACK_HOLE_PHYSICS.md` §5 [THEOREM])

This protocol pre-registers the **shared candidate value list** and the **falsifier criteria** that any per-domain follow-up engine campaign (FTD-0107 Domain A, FTD-0108 Domain B, FTD-0109 Domain C) MUST adhere to.

Per-domain engine campaigns will have their own protocols specifying observable, lattice configuration, and discretisation convention. **Those domain-specific protocols are NOT pre-registered here** — they will be written ahead of each follow-up campaign and tagged separately (e.g., `preregister-domain-a-v1`).

What IS pre-registered here:

1. The numerical candidate values (§2 below)
2. The structural rule for adding new candidates (§3 below)
3. The falsifier criteria per row (§4 below)
4. The discretisation-convention pre-registration requirement for any follow-up engine campaign (§5 below — FTD-0105 lesson)
5. The look-elsewhere expected-hit-count thresholds (§6 below)

---

## 2 · Pre-registered candidate value list (LOCKED)

These values are computed from `scripts/constants.py` and verified to ≥4 decimal places. **No post-hoc adjustment** is permitted; if an additional candidate needs to be added in a follow-up, it MUST be added through the structural rule (§3) and reported as such.

| Constant | Value | Provenance |
|---|---|---|
| π | 3.14159265 | universal |
| 2π | 6.28318531 | universal |
| 4π | 12.56637061 | universal |
| 8π | 25.13274123 | universal |
| π/2 | 1.57079633 | universal |
| π² | 9.86960440 | universal |
| ϖ (varpi) | 2.62205755 | Γ(1/4)²/(2√(2π)), `scripts/constants.py:VARPI_CLASSICAL` |
| 2ϖ | 5.24411511 | derived |
| 4ϖ | 10.48823022 | derived |
| 8ϖ | 20.97646043 | derived |
| ϖ² | 6.87519602 | derived |
| G\* | 2.95867512 | Γ(1/4)²/(√2·π), `scripts/constants.py:G_STAR` |
| 2G\* | 5.91735024 | derived |
| 4G\* | 11.83470048 | derived |
| 8G\* | 23.66940096 | derived |
| G\*² | 8.75375846 | derived |
| G\*²·π/2 | 13.75035927 | derived (FTD-0105 Candidate C) |
| 5120π | 16085.04 | C1 standard (Hawking evap) |
| 5120G\* | 15148.41 | C1 candidate I |
| 5120ϖ | 13424.93 | C1 candidate II |
| 15360π | 48255.13 | C2 standard (Hawking lum) |
| 15360G\* | 45445.22 | C2 candidate I |
| 15360ϖ | 40274.80 | C2 candidate II |

---

## 3 · Structural rule for adding new candidates

Any G\*-native or ϖ-native candidate added in a follow-up campaign MUST be derived through ONE of:

**(a) Heat Equation operator route** — the candidate is the eigenvalue of D^(α) for some pre-specified α determined by the formula's dimensional structure. Example: A1's $\sqrt{k\rho c}$ on $\partial_t^{1/2}$ is structurally G\* via D^(−1/2).

**(b) CM L-function route** — the candidate is L(E, n) for some integer n where E is the lemniscatic CM elliptic curve (per `DERIV_LFUNCTION_GSTAR_CONNECTION.md`). Pre-specified n MUST be locked in the per-domain protocol.

**(c) Reflection-ratio direct identification** — the candidate is Γ(z)/Γ(1−z) at a pre-specified z determined by the formula's structure. z = 1/4 gives G\*; other rational z values give related Γ-ratios that may map onto specific physics (e.g., complex-conjugate ratios at half-integer for spin-1/2).

**No naive π → G\* substitution promoted to a primary candidate.** Naive substitutions are listed as Candidate I in the AUDIT for completeness only; they don't carry derivation status and are NOT used as PASS criteria in falsifier definitions.

---

## 4 · Pre-registered falsifier criteria

For each row in the AUDIT Phase 2 prediction matrix that has a **derivation-anchored** candidate (per §3 above), the per-domain follow-up engine campaign MUST report:

- **Standard prediction value** (locked in §2 above)
- **Derivation-anchored candidate value** (locked at the per-domain protocol commit)
- **Naive Candidate I/II values** (reported for completeness; do NOT determine PASS)

**Falsifier verdict per row (locked):**

- **PASS-Standard** — measured value within ±5% of standard prediction across all configurations tested. Conclusion: **for this observable, lattice agrees with standard π-laden form**; G\*-native replacement closes negative for this row.
- **PASS-Derivation-Anchored** — measured value within ±5% of derivation-anchored G\*-native value AND >5σ separated from standard. Conclusion: **structural finding for this row**; promotes the row's tag to [PARTIAL] or [SELECTION] depending on derivation rigor.
- **PASS-NONE / INCONCLUSIVE** — measured value lands outside both windows. Per FTD-0105 lesson, this typically indicates a **discretisation-convention overhead** that wasn't pre-registered; report measured value with stderr; structural reading deferred.
- **Cross-row consistency** — at least 2 derivation-anchored rows must produce consistent verdicts (PASS-Standard or PASS-Derivation-Anchored) for the per-domain campaign to count as evidence.

**Pre-registered ±5% window** matches FTD-0105 PROTOCOL §4 for consistency.

---

## 5 · Discretisation-convention pre-registration (FTD-0105 lesson — MANDATORY)

The strongest lesson from FTD-0105 (`AUDIT_FTD0105_MATH_CHECK.md` §3.2): the engine's actual measurement of an observable depends on the **discretisation convention** chosen. FTD-0105's Moore-boundary isosurface count had a digital-geometry overhead factor of ~1.5× that wasn't anticipated, leading to PASS-NONE for all four candidates even though the lattice horizon was sphere-symmetric within the corrected reading.

**Mandatory requirement for ALL follow-up engine campaigns under this investigation:**

Every per-domain protocol (FTD-0107/0108/0109) MUST specify, BEFORE measurement:

1. **Exact engine observable** — function name, output units, lattice convention used (e.g., 26-Moore-neighbor-boundary count vs. 1-voxel-thick shell vs. central-difference gradient)
2. **Expected discretisation overhead factor** — for the observable AND for a known reference scale (e.g., for FTD-0105 should have been "1-thick shell scales as 4π·r²; 26-Moore-boundary scales as 1.5·4π·r²"). The overhead must be derivable from digital-geometry literature OR computed from a reference geometry on the lattice BEFORE measurement.
3. **Per-cluster statistical replication strategy** — how the multi-seed bootstrap is constructed AND verified to produce truly independent samples (the broken seed-shift in FTD-0105 produced 5× fake replication; effective n=4 not 20). Effective sample size MUST be reported separately from raw count.
4. **Multi-configuration sweep** — at least 4 configurations (e.g., 4 cluster_radii, 4 mass values, 4 scattering energies) for the headline result, to provide a sample variance not just per-config replication.

Failure to pre-register any of (1)–(4) means the campaign's results cannot count as evidence under this investigation. The discipline is to **lock the bookkeeping before running the engine**.

---

## 6 · Look-elsewhere control thresholds

Per AUDIT §4, the per-row prior under null is ≈0.2 (range 0.1–0.4). Across the 7 active candidate rows in the catalog, expected hits under null = 1.4.

**Pre-registered evidence threshold:** the investigation contributes evidence (and the LEDGER row promotes from [HYPOTHESIS] toward [PARTIAL]) only if:

(i) Observed match count from per-domain engine campaigns is **≥ 2× expected = ≥ 3 rows match within ±5%**
(ii) AND the matches are **concentrated in derivation-anchored rows** (per §3) — at least 50% of matches must come from rows with Heat Equation, CM L-function, or reflection-ratio derivations
(iii) AND **standard predictions FAIL** in the same rows — i.e., the engine produces values consistent with G\*-native rather than standard

If any of (i)–(iii) fails, the investigation closes negative or stays [HYPOTHESIS].

**Special case**: if Phase 1 catalog already shows that direct-substitution candidates are systematically off (as observed in AUDIT §4: 0/7 rows match within ±5% even before engine measurement), the investigation enters the engine phase with a low prior — the engine is being asked whether its specific lattice realisation produces G\*-aligned values that the naive candidates didn't. This is a structural test, not a fishing expedition.

---

## 7 · Anti-targets (locked)

This protocol **WILL NOT**:

- Permit post-hoc adjustment of candidate values, falsifier criteria, or look-elsewhere thresholds
- Promote any row above [HYPOTHESIS] without engine measurement landing within ±5% AND >5σ separation from standard AND derivation-route alignment per §3
- Bundle multiple domains into a single campaign — each domain gets its own protocol and tag (`preregister-domain-a-v1`, etc.)
- Treat "the engine landed close to one of the candidates" as PASS without the cross-row consistency check (§4)
- Skip the discretisation-convention pre-registration (§5)
- Skip the look-elsewhere threshold check (§6) even if results "look promising" mid-campaign
- Treat the existing [THEOREM]-level Heat Equation derivation as evidence FOR a particular numerical match in a specific physics formula — that theorem is an algebraic identity about operator eigenvalues, not a numerical claim about Kramers prefactors or evaporation timescales

This protocol **WILL**:

- Lock the candidate values at this commit (§2)
- Tag `preregister-gstar-asymmetry-v1` BEFORE any engine code extension
- Require per-domain protocols for engine campaigns
- Require discretisation-convention pre-registration for each per-domain campaign
- Apply the look-elsewhere threshold strictly
- Tag results honestly per CLAUDE.md epistemic ladder

---

## 8 · Implementation checklist

1. [x] AUDIT_GSTAR_ASYMMETRY_SCAN.md committed (this commit)
2. [x] PROTOCOL_GSTAR_ASYMMETRY_SCAN.md committed (this commit)
3. [x] LEDGER row FTD-0106 [HYPOTHESIS] added (this commit)
4. [ ] `git tag preregister-gstar-asymmetry-v1` applied at this commit — **MANDATORY GATE**
5. [ ] Tag pushed to origin
6. [ ] Per-domain follow-up tickets (FTD-0107 / 0108 / 0109) opened separately, each with its own pre-registration before any engine code

Steps 1–3 complete in this commit. Step 4–5 follow immediately. Step 6 is deferred to per-domain follow-up sessions.

---

## 9 · Single-line summary

**Pre-registers the shared candidate value list, falsifier criteria, discretisation-convention requirements, and look-elsewhere thresholds for the FTD-0106 G\*/π asymmetry investigation. Three Tier-1 domains (time-direction/dissipation, Coulomb scattering, Hawking evaporation timescale) each get their own follow-up ticket (FTD-0107/0108/0109) with separate per-domain pre-registration. ±5% match window matches FTD-0105 convention. Discretisation-convention pre-reg is MANDATORY (FTD-0105 lesson: the digital-geometry overhead factor matters). Look-elsewhere threshold: ≥3 of 7 active rows must match AND be derivation-anchored AND make standard fail in the same rows. Tag `preregister-gstar-asymmetry-v1` applied at this commit; per-domain engine campaigns deferred to separately-pre-registered follow-ups. No promotion above [HYPOTHESIS] without measurement + derivation alignment.**
