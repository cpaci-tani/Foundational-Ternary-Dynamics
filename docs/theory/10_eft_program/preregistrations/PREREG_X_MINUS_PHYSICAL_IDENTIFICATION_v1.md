# Pre-Registration — x_- Physical Identification: Harmonic-Sum-Rule-Constrained Adversarial Search (v1)

**Tag:** [PRE-REGISTRATION] — this document locks the *design* of a search for an FTD-internal physical identification of `x_-`, the smaller root of the master quadratic. It contains **no result**. All three pre-blessed outcomes — FOUND / UNDERDETERMINED / CLOSED-NEGATIVE — are admissible; the search's verdict is genuinely open, and the prior-favoured outcome is CLOSED-NEGATIVE (Candidate C5: pure mathematical artifact).

**Date:** 2026-05-27
**Hash-lock target tag:** `preregister-x-minus-physical-identification-v1`
**LEDGER row reservation:** FTD-0210 (provisional; confirm next-free identifier against `../07_assessment/core_ledgers/LEDGER.md` at hash-lock).
**Supersedes:** none — first pre-registration of an x_- physical-identification search after the 2026-05-2X retirement of FTD-0014 (the original `x_- ↔ N_c` identification, removed in commit `ca7eb61` per FTD/FQCR Cleanup Taxonomy v1.4 §5).
**Companion docs:**
- `../01_reference/SPEC_ALGEBRAIC_SPINE.md` §§2, 5 (master quadratic; coefficient 16; nine-theorem spine context)
- `../01_reference/SPEC_DOCTRINE_LEDGER.md` §§13.5, 14 (closed-negative reminders; hardening targets; "earn the map" framing)
- `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §§3, 4 (anti-laundering rules adapted to the x_- case)
- `../07_assessment/core_ledgers/LEDGER.md` rows FTD-0001, FTD-0013, FTD-0014 (the master quadratic, the x_+ conjecture, the retired x_- identification)
- `../09_mathematical/MATH_LOG_GSTAR_IDENTITY.md` (G* algebraic provenance; the J-chain link ② of the spine)
- `../09_mathematical/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md` (the algebraic identity catalog this search uses to derive constraints)
- `../10_eft_program/PREREG_ADVERSARIAL_LOOK_ELSEWHERE_v1.md` (FTD-0189; the methodological template for the dual-match uniqueness scan)
- `../02_foundations/FOUND_FORCE_STRUCTURE.md` (the dual substrate `(J_L, J_R)` and L/R-asymmetry framing)
- `../10_eft_program/PREREG_FINITE_NEUTRAL_LOCK_v1.md`, `../08_structural/PREREG_COLOUR_SINGLET_RANK_v1.md` (format templates)
- `../../../scripts/exploration/check_zeta_hessian_gstar2.py` (Path A first-pass numerical check, 2026-05-27 — CLOSED-NEGATIVE for the naive Hessian-identity hypothesis; cited in §3)

> **Pre-registration discipline.** Sections §§2–9 are committed before the search is run. After commit: SHA256 → `REF_PREREGISTER_MANIFEST.md`, git tag applied. Any post-hoc edit to §§2–9 invalidates v1; a v2 is required before the search is run or re-run. The search's result lands in a separate doc (`FOUND_*`, `AUDIT_*`, or `AUDIT_*_CLOSED_NEGATIVE.md`), never as edits to this file.

**Purpose.** Lock, *before* any numerical match-search runs, (a) what would count as an admissible FTD-internal physical identification of `x_-`, (b) what would **falsify** any candidate identification, and (c) the banned-moves list that prevents this search from devolving into a re-litigation of the retired FTD-0014 `x_- ↔ N_c` reading or into a generic numerical-coincidence fishing expedition.

---

## §1 — Context and doctrine

**Algebraic provenance of x_- (LOCKED).** The master quadratic
`x² − 16 G*² x + 16 G*³ = 0` (FTD-0001 / `SPEC_ALGEBRAIC_SPINE.md` §2; theorem-grade) has two roots:

- `x_+ = 8G*²(1 + δ) ≈ 137.036` — conjecturally identified with `1/α` (FTD-0013, `[STRONGLY MOTIVATED CONJECTURE]`, 1.26 ppm match)
- `x_- = 8G*²(1 − δ) = G*/(1 − αG*) ≈ 3.024` — currently **algebraically derived but physically unidentified** (the original `x_- ↔ N_c` reading was retired in commit `ca7eb61` per FTD/FQCR Cleanup Taxonomy v1.4 §5)

where `δ = √(1 − P_*/(S²/4)) = √((4G* − 1)/(4G*))` and `αG* = (1 − δ)/2`. The two roots satisfy the harmonic sum rule `1/x_+ + 1/x_- = S/P_* = 1/G*`. **All algebraic facts above are theorem-grade (link ② of the spine, with the recent x_- derivation as a direct consequence of FTD-0001 + FTD-0013).**

**FTD-0014 retirement summary.** The original identification `x_- ↔ N_c` (number of QCD colors) was retired because:
(i) N_c = 3 has independent first-principles structural derivations from topology (`DERIV_NC_FROM_TOPOLOGY.md`, four routes), the Moore Layer Theorem (`THEOREM_MOORE_LAYER_DECOMPOSITION.md`), and master-quadratic dual-prediction was redundant with these.
(ii) `x_- ≈ 3.024` is an L/R-symmetric algebraic number; N_c is a strong-coupling parameter on an L/R-symmetric gauge sector — the physical match was structurally well-formed but the alignment was 0.80% (poor relative to other FTD identifications), and the cleaner independent N_c derivations made the master-quadratic identification surplus to requirements.
(iii) The FTD/FQCR Cleanup Taxonomy v1.4 §5 standard for retiring a load-bearing identification was met: structurally redundant + cleaner alternative exists + retaining the identification costs F10 tag-drift risk.

**What x_- still is.** Algebraically: the smaller root of the master quadratic, completely determined by G* and the polynomial's structure. **Physically: an unidentified algebraic number with the precise form `G*/(1 − αG*)` whose physical correspondent — if any — is unknown.** Three candidate stances exist (per the 2026-05-27 brainstorm):

- **Candidate C1 (Algebraic ghost):** x_- is a pure mathematical artifact of the quadratic structure; no physical reading exists.
- **Candidate C2 (L/R-asymmetric reading):** x_- corresponds to an L/R-asymmetric SM quantity (electroweak / neutrino / CP-violating sector), reflecting the master quadratic's intrinsic chirality split via δ.
- **Candidate C3 (Effective coupling reading):** x_- corresponds to some effective EM-like coupling at a non-physical-α reference scale, with `αG* = (1 − δ)/2` providing the scale-translation identity.

This pre-registration tests whether C2 (the most physically motivated candidate) admits a load-bearing identification, with FOUND/UNDERDETERMINED/CLOSED-NEGATIVE pre-blessed; CLOSED-NEGATIVE collapses the choice between C1 and C3 to a subsequent decision, both of which are also acceptable framework outcomes.

**Doctrine clause this serves.** CLAUDE.md goal-clause 2: "Derive everything we can from a discrete ontology — **and rigorously establish what we cannot.**" A clean CLOSED-NEGATIVE here would discharge the unfinished business left by FTD-0014's retirement and explicitly map x_- as a *boundary* of what the master quadratic determines. A FOUND verdict would identify a second physically-meaningful root of FTD's load-bearing polynomial, sharpening (but not promoting) FTD-0013.

**No tag promotions.** This pre-registration neither promotes nor demotes any LEDGER claim. A FOUND verdict opens a separate ratification pass (analogous to the ARC-3 pathway of `SPEC_ALPHA_READOUT_CONTRACT.md` §7); a CLOSED-NEGATIVE verdict establishes a new ledger row for the closed-negative provenance.

---

## §2 — The question (LOCKED)

**Q-X-MINUS.** Does there exist a dimensionless SM observable `Q_phys` such that:

1. **Algebraic constraint.** `Q_phys` satisfies the harmonic sum rule with `1/α`:
   $$\frac{1}{Q_{\text{phys}}} + \frac{1}{\alpha^{-1}_{\text{CODATA}}} = \frac{1}{G^{*}}$$
   to within a pre-specified algebraic-error tolerance of `|residual| < 10⁻⁴` (relative); equivalently, `Q_phys ≈ G*/(1 − α G*) ≈ 3.024` to ≤ 100 ppm;

2. **Structural constraint.** `Q_phys` is natively L/R-asymmetric in the SM — i.e. its definition explicitly distinguishes left-chirality from right-chirality, parity, or CP, and **is not** an L/R-symmetric quantity (which would re-litigate the FTD-0014 N_c retirement);

3. **Methodological constraint.** A FTD-0189-style adversarial dual-match scan over an extended observable basket (per §4) finds the master quadratic to be the **unique** dual-matcher for `(1/α, Q_phys)` at the same precision floor at which it is the unique dual-matcher for `(1/α, x_-)` in the pure-numerical FTD-0189 scan;

and is the identification `[DERIVED]` from the algebraic structure plus an independent structural argument, rather than constructed by post-hoc selection?

The verdict is genuinely open. All three §5 outcomes are pre-blessed; CLOSED-NEGATIVE is the prior-favoured outcome.

---

## §3 — Background and prior work

**The master quadratic and its two roots** — theorem-grade per FTD-0001 / `SPEC_ALGEBRAIC_SPINE.md` §2. The five equivalent closed forms for `x_-` derived 2026-05-27:

- `x_- = 16G*² − x_+` (Vieta sum)
- `x_- = 16G*³/x_+` (Vieta product)
- `x_- = G*/(1 − αG*)` (harmonic form, given `x_+ = 1/α`)
- `x_- = 8G*² − 4G*^(3/2)·√(4G* − 1)` (quadratic formula)
- `x_- = 8G*²·(1 − δ)` (chirality form — cleanest)

A new identity surfaced in the derivation:
$$\alpha\, G^* \;=\; \frac{1 - \delta}{2} \qquad\Longleftrightarrow\qquad \delta = 1 - 2\alpha G^*$$
This restates the master quadratic in chirality coordinates.

**FTD-0189 dual-match uniqueness** — `PREREG_ADVERSARIAL_LOOK_ELSEWHERE_v1.md` established that across 2.65M degree-2 polynomials over an 18-constant basket FTD did not design, the master quadratic is the unique dual-matcher to `(1/α, N_c)` at FTD's empirical precision. This is the methodological template for §2(3) of the present pre-registration: any candidate `(1/α, Q_phys)` pair must remain uniquely matched by the master quadratic under the same adversarial scan, otherwise the structural-uniqueness evidence dilutes.

**FTD-0014 retirement** — commit `ca7eb61` per FTD/FQCR Cleanup Taxonomy v1.4 §5. The identification `x_- ↔ N_c` was structurally redundant with `DERIV_NC_FROM_TOPOLOGY.md` (four independent routes) and Moore Layer Theorem (`THEOREM_MOORE_LAYER_DECOMPOSITION.md`); the alignment was 0.80% (poor relative to other FTD predictions); F10 tag-drift risk made retention costly. The retirement is the canonical record; this pre-registration's banned-move B-2 specifically forbids re-litigating it.

**Path A status (2026-05-27 numerical check) — CLOSED-NEGATIVE for the naive Hessian-identity hypothesis.** The companion math-side route — deriving `(S, P_*) = (16G*², 16G*³)` from higher-order ζ-determinant invariants of the J-chain — was tested at 80-digit precision via `scripts/exploration/check_zeta_hessian_gstar2.py`. Specifically, the candidate identity
$$\zeta''(0,1/4) - \zeta''(0,3/4) \;\stackrel{?}{=}\; \pm\log(16\,G^{*2})$$
**fails**: the LHS computes to `+1.81380334124874848569…`, the RHS to `+4.94207186877334540944…`. The values differ at order 1, with no closed-form combination from a sweep of natural targets (involving `log G*`, `log(2π)`, `log 2`, `G_Catalan`) bringing them within `10⁻¹⁰` precision. The first-order J-chain identity verifies cleanly (`ζ'(0,1/4) − ζ'(0,3/4) = log G*` to numerical-differentiation noise), so the framework is correctly set up; what fails is the naive second-order extension.

**Implication.** The K_2-regulator-of-E machinery (Beilinson conjectures, Bloch–Wigner dilogarithm at CM points, Brunault–Zudilin's monograph *Many Variations of Mahler Measures*) remains a possible math-side route, but it has become a multi-month research project rather than a session-tractable extension. The Path A closed-negative status **increases the relative value of this pre-registration**: the physical-identification question for `x_-` is no longer paired with a near-term math-side win, so disciplining the physics-side question matters more.

**Structural symmetry argument supporting C2 (L/R-asymmetric reading).** Per `AUDIT_DUAL_SUBSTRATE_PROVENANCE.md` §3, the chirality decomposition `E_L = x_+`, `E_R = x_-` ties `x_-` to the L/R-asymmetric component of the dual substrate. Per `FOUND_FORCE_STRUCTURE.md`, the dual substrate `(J_L, J_R)` is the FTD analog of left/right gauge structure. The retired FTD-0014 identification's structural awkwardness — N_c is L/R-symmetric, but `x_-` lives on the chirality-asymmetric side of the master quadratic — supports C2 over C1 as the more physically natural candidate. This is the argument the present pre-registration tests under disciplined search.

---

## §4 — Admissible search space (LOCKED)

**A. The Q_phys observable basket (frozen, FINITE).**

The search enumerates exactly the following dimensionless SM observables drawn from CODATA 2022 / PDG 2024 per `../reference/REF_EXTERNAL_CONSTANTS.md`:

**A.1 — Electroweak sector (L/R-asymmetric by design):**
1. `sin²θ_W(M_Z)` (on-shell, MSbar) — Weinberg angle
2. `g_R/g_L` at M_W — left/right coupling ratio (sector-defined)
3. `Γ(W → eν_e) / Γ(Z → e⁺e⁻)` — W/Z decay-width ratio
4. `M_W² / (M_W² + M_Z²)` — electroweak mass-mixing ratio
5. `ρ_param = M_W²/(M_Z² cos²θ_W)` — ρ-parameter (=1 at tree level, ≠1 at loop)
6. `s²_eff` (effective leptonic) — measured at LEP/SLC
7. Z partial-width ratio `R_l = Γ_had/Γ_l`

**A.2 — Neutrino / lepton sector (intrinsically L/R-asymmetric):**
8. `m_ν_2² − m_ν_1²` / `m_ν_3² − m_ν_2²` (Δm²_21 / Δm²_32) — neutrino-mass-squared splitting ratio
9. `sin²(2θ_12)` (solar) — neutrino mixing
10. `sin²(2θ_13)` (reactor) — neutrino mixing
11. `sin²(2θ_23)` (atmospheric) — neutrino mixing
12. `δ_CP / π` — leptonic CP-violating phase normalised to π
13. `m_μ / m_τ` × parity factor — lepton mass ratio with chirality bookkeeping

**A.3 — CKM / quark sector (intrinsically L/R-asymmetric via V_CKM):**
14. `|V_us|/|V_ud|` — Cabibbo angle
15. `|V_cb|/|V_tb|` — third-row mixing
16. `|V_ub|/|V_cb|` — Wolfenstein ratio
17. `J/η` — Jarlskog invariant normalised
18. `arg(V_td V*_ts V_cs V*_cd)` / π — unitarity-triangle angle
19. `m_t / m_b × (V_tb/V_cb)²` — chirality-weighted mass ratio

**A.4 — Strong-CP / instanton sector:**
20. `θ_QCD` (experimental upper bound) — strong-CP angle
21. `m_u / m_d` — quark-mass ratio (light, with chirality bookkeeping)

**A.5 — Composite L/R-asymmetric ratios (defined explicitly, not free-form):**
22. `α_W(M_Z) / α_EM(M_Z)` — gauge-coupling ratio at M_Z
23. `(g - 2)_μ - (g - 2)_e` / α — anomalous moment difference
24. `[Γ(K_L → π⁺π⁻) / Γ(K_S → π⁺π⁻)]` — ε_K-related ratio
25. `[B(B_s → μμ) / B(B_d → μμ)]` × |V_td/V_ts|⁻² — flavor SU(3) test

**Basket constraints (LOCKED):**
- Basket is FROZEN — exactly the 25 entries above. No additions after hash-lock.
- Each candidate evaluation uses CODATA 2022 / PDG 2024 values per `../reference/REF_EXTERNAL_CONSTANTS.md` (no value updates before result reporting).
- "Composite" candidates beyond the 25 (e.g. arbitrary rational functions of basket entries) are inadmissible; the basket is the search space, not the seed for an extended scan.

**B. The FTD-0189 constant basket extension for §2(3) (frozen).**

The FTD-0189 baseline is 18 fundamental constants spanning math + physics. For this pre-registration's §2(3) adversarial dual-match scan, the basket is **extended to include the defining parameters of whichever Q_phys candidate is currently under test**, capped at +5 added constants per candidate. The extension procedure is mechanical (no curated additions): the candidate's defining CODATA parameters become elements of the scan basket. **If a candidate's definition requires > 5 added constants, the candidate is automatically declared inadmissible (insufficient algebraic locality).**

**C. The Hessian-route adjunct (REFERENCE ONLY).**

The companion ζ-Hessian-derivation work (Path A of the 2026-05-27 plan; numerical check in `scripts/exploration/check_zeta_hessian_gstar2.py`) is **not** an input to this pre-registration's search and **does not** enter the §4 observable basket. It is recorded here so that future readers know the algebraic-side derivation effort exists independently; the Path A naive Hessian-identity hypothesis is **CLOSED-NEGATIVE** as of 2026-05-27 (see §3), with K_2-regulator-of-E machinery remaining as a multi-month research direction. Neither positive nor negative results from any future Path A iteration close or open this pre-registration's verdict.

---

## §5 — The three pre-registered outcomes (LOCKED)

**FOUND.** A Q_phys candidate from the §4.A basket is exhibited satisfying:
- §2(1) algebraic constraint to `ε_alg = 10⁻⁴` (D2);
- §2(2) L/R-asymmetry per D3;
- §2(3) dual-match uniqueness per D5;
- An explicit structural argument linking Q_phys to the `(1 − δ)` chirality of x_- (e.g. δ corresponds to an L/R-asymmetry parameter in the SM sector containing Q_phys; the harmonic sum rule `1/x_+ + 1/x_- = 1/G*` has a substrate-physical interpretation tying EM strength to the L/R-asymmetric quantity);
- No §7 falsifier fires; no §8 banned move is invoked.

Tag consequences:
- The identification is tagged `[DERIVED]` only if the structural argument is itself `[THEOREM]`/`[DERIVED]`-grade from existing FTD theorems; otherwise `[SELECTION]`.
- The retired FTD-0014 row remains retired; a new ledger row is opened (FTD-0210 or next-free) for the FOUND identification.
- FTD-0013 (x_+ ↔ 1/α) gains structural support from the now-explained dual-match property but is *not* promoted by this verdict alone; promotion requires its own separate ratification (per `SPEC_ALPHA_READOUT_CONTRACT.md` §7).
- No spine theorem is created or modified.

**UNDERDETERMINED.** Exactly two of §2(1)+(2)+(3) are satisfied by a candidate, with the third borderline. Specifically:
- Subcase A: §2(1) satisfied at ε_alg; §2(2) satisfied; §2(3) finds 1 competitor at the threshold (uniqueness margin is < 2×).
- Subcase B: §2(1) satisfied at ε_alg; §2(3) satisfied (zero competitors); §2(2) is "weakly L/R-asymmetric" (suppression bookkeeping is incomplete or judgment-dependent).
- Subcase C: §2(2) and §2(3) satisfied; §2(1) holds at the boundary 1×10⁻⁴ ≤ |residual| < 5×10⁻⁴.

Tag: candidate noted as `[CANDIDATE — UNDERDETERMINED]` with exactly which constraint is borderline. A v2 pre-registration with tightened scope is the only admissible follow-up.

**CLOSED-NEGATIVE.** No Q_phys candidate in §4.A basket satisfies all three §2 constraints. Tag: `[CLOSED NEGATIVE]`. A new ledger row is opened for the closed-negative provenance. **Per CLAUDE.md goal-clause 2, a clean CLOSED-NEGATIVE here is a load-bearing deliverable**: it would establish that x_- is *not* identified by any pre-specified L/R-asymmetric SM observable, sharpening the algebraic-vs-physical boundary the framework can rigorously claim about its central polynomial's two roots. The remaining candidates (C1: pure algebraic artifact; C3: effective coupling at unphysical scale) become the surviving options, both of which are acceptable framework states.

---

## §6 — Measurement procedure (LOCKED)

The search executes exactly these steps in order. Each step reports its result before the next runs.

1. **Hash-lock.** Compute `sha256sum docs/theory/10_eft_program/PREREG_X_MINUS_PHYSICAL_IDENTIFICATION_v1.md`; record in `REF_PREREGISTER_MANIFEST.md`; add LEDGER row (FTD-0209 or next-free) tagged `[PRE-REGISTRATION]`; create git tag `preregister-x-minus-physical-identification-v1`.

2. **Compute the target value.** Using CODATA 2022 / PDG 2024 values per `REF_EXTERNAL_CONSTANTS.md`, compute the target: `Q_target = G*/(1 − α_CODATA · G*)`. Record to 15 significant figures. This is the value any Q_phys candidate must match per D2.

3. **Enumerate basket-evaluated values.** For each of the 25 §4.A basket entries, compute the numerical value from CODATA / PDG. Tabulate.

4. **Apply the §2(1) algebraic filter.** Identify all basket entries with `|value − Q_target| / Q_target ≤ ε_alg = 10⁻⁴`. **Report this list (possibly empty) before proceeding.**

5. **Apply the §2(2) structural filter.** For each survivor of step 4, audit the L/R-asymmetry per D3. Document the bookkeeping. **Report the surviving list before proceeding.**

6. **Apply the §7 falsifier checklist.** For each survivor of step 5, mechanically check F-a through F-j; record any firings.

7. **Apply the §8 banned-moves checklist.** For each survivor of step 6, mechanically verify no banned move is invoked in the candidate's construction.

8. **For each surviving candidate, run the §2(3) adversarial dual-match scan.** Construct the extended basket (per §4.B), run the FTD-0189-template scan, count dual-matchers, compute uniqueness margin (= 2nd-best fit residual / master-quadratic residual). Report.

9. **Verdict assignment per §5.** Apply the three-outcome scheme mechanically; produce the verdict.

10. **Result document.** Land the result in a NEW document — `FOUND_X_MINUS_PHYSICAL_IDENTIFICATION.md` (if FOUND), `AUDIT_X_MINUS_UNDERDETERMINED.md` (if UNDERDETERMINED), or `AUDIT_X_MINUS_CLOSED_NEGATIVE.md` (if CLOSED-NEGATIVE). Update LEDGER (new row); update `META_INDEX.md`; update `TRACKER_OPEN_ITEMS.md`.

**Step-by-step transparency rule.** The results of steps 2, 3, 4, and 5 are reported BEFORE steps 6–8 are run, so that any "tuning the filter to admit a desired candidate" move would be visible in the commit history. Specifically, the basket evaluation (step 3) is committed to a results file before the algebraic filter (step 4) is applied.

---

## §7 — Falsifier rules (LOCKED)

A candidate fails the search if any of the following fire. Each is mechanically checkable.

- **F-a — Algebraic miss.** `|Q_phys − Q_target| / Q_target > ε_alg = 10⁻⁴`.

- **F-b — L/R-symmetry violation.** The candidate is L/R-symmetric by D3 (e.g. an N_c-style total color count, generation count, sum-over-chiralities quantity). **Re-litigation of the FTD-0014 N_c retirement is automatically falsified by this rule.**

- **F-c — Dual-match non-uniqueness.** The §2(3) FTD-0189-style scan finds ≥ 1 non-master-quadratic dual-matcher in the extended basket at threshold ≤ 10 ppm for 1/α AND ≤ ε_alg for Q_phys. **The master quadratic's uniqueness margin must exceed 2× (its residual is < 1/2 the second-best polynomial's residual on the joint match).**

- **F-d — Post-hoc basket extension.** Any addition to the §4.A basket after hash-lock and before result reporting fires this rule irrevocably and invalidates v1.

- **F-e — Post-hoc tolerance loosening.** Any relaxation of ε_alg (D2) or the FTD-0189 uniqueness threshold (D5) after hash-lock and before result reporting fires this rule irrevocably and invalidates v1.

- **F-f — Hidden numerical fit.** The Q_phys candidate's definition contains a free integer, exponent, sign choice, or normalization factor whose value is tuned to make the algebraic constraint hold. Tested by: would the candidate still match if all integers in its definition were perturbed by ±1? If no, the integer is fit; F-f fires.

- **F-g — Sector confusion.** The Q_phys candidate's structural argument linking it to (1 − δ) reduces (under reformulation) to a previously closed-negative route (the 11 closed alpha-derivation routes per `SPEC_DOCTRINE_LEDGER.md` §13.5). For example: if the structural argument reads "δ corresponds to electroweak symmetry breaking via the Higgs mechanism, and Q_phys is sin²θ_W" — but this would require the master quadratic to function as a Higgs sector readout, fundamentally incompatible with FTD-0125 (Phase II matter-sector vertex closed-negative).

- **F-h — Sector mismatch in scale.** Q_phys is defined at a renormalisation scale fundamentally incompatible with α's defining scale (M_Z for α_EM running). For example: a low-energy QCD observable (chiral perturbation theory regime) paired with the high-energy α_EM(M_Z) cannot satisfy the harmonic sum rule unless an unjustified RG-running is silently inserted (which would also fire F-f or F-i).

- **F-i — Look-elsewhere violation.** The candidate was selected by examining the basket numerically before the §2(1)+(2)+(3) constraints were applied, rather than by enumerating the basket and applying the filters mechanically. Tested by: the commit history shows whether the basket evaluation (step 3) or the structural classification (step 5 of §6) was completed before the surviving candidate(s) were identified. If a candidate appears in the verdict that was not among the §6-step-4 survivors, F-i fires.

- **F-j — Identification by analogy.** The structural argument linking Q_phys to (1 − δ) appeals to "x_- looks like ~3 and N_c is 3" or any other surface numerical resemblance to the retired FTD-0014 reading, without an independent structural derivation. F-j is the FTD-0014-retirement guard rail.

Any single firing → verdict is at best UNDERDETERMINED for that candidate; if all candidates fire ≥ 1, the verdict is CLOSED-NEGATIVE.

---

## §8 — Banned moves / anti-target discipline (LOCKED)

- **B-1 — No numerical search before structural filtering.** A candidate is admissible only if it appears among the §4.A basket entries that satisfy the §2 constraints in the mechanical order of §6. Searching for "any number ≈ 3.024 in SM phenomenology" without first establishing the harmonic-sum-rule constraint is an anti-target move (the search-for-near-misses pattern explicitly proscribed by CLAUDE.md).

- **B-2 — No L/R-symmetric candidates.** Re-litigating FTD-0014 (N_c, generation count, color factors, total fermion content, etc.) is automatically forbidden. The retirement provenance in commit `ca7eb61` is the canonical record; any move to re-open the N_c reading requires its OWN separate pre-registration with explicit justification for revisiting the retired decision, NOT this one.

- **B-3 — No FTD-0189 threshold relaxation.** The dual-match uniqueness criterion (D5) inherits FTD-0189's standard; lowering this threshold would silently weaken the structural-uniqueness evidence that x_+ ↔ 1/α currently rests on.

- **B-4 — No post-hoc basket adjustment.** The §4.A basket is frozen at hash-lock. Adding observables after looking at numerical results is fishing; removing observables is selection bias. F-d catches both.

- **B-5 — No CODATA value updates mid-search.** The CODATA 2022 / PDG 2024 values per `REF_EXTERNAL_CONSTANTS.md` are frozen for the duration of the search. Updates to CODATA 2026 (when it lands) are out of scope for v1.

- **B-6 — No substitution-identity laundering.** A candidate of the form "Q_phys = (some standard SM formula) with FTD constants plugged in" is a parametric insertion, not a derivation (CLAUDE.md anti-target). The candidate's value must come from CODATA / PDG measurement, not from an FTD-side formula.

- **B-7 — No appeal to "G* and α make x_- algebraically forced, so this must mean something."** The algebraic forcing is FTD-0001's content (theorem-grade). Whether x_- has a physical correspondent is exactly the question this pre-registration tests; the algebraic fact does not pre-decide the physical answer.

- **B-8 — No tag promotion or demotion as a result of this search.** Any FOUND/UNDERDETERMINED/CLOSED-NEGATIVE verdict opens a SEPARATE ratification or demotion process. This pre-registration neither moves LEDGER tags nor authorises others to do so.

- **B-9 — No deferral of the falsifier checks.** All F-a through F-j must be applied mechanically before the verdict is assigned. A "we'll come back to F-c later" deferral is an automatic UNDERDETERMINED at minimum, CLOSED-NEGATIVE at strict reading.

- **B-10 — No conflation with the Hessian-route Path A.** The companion ζ-Hessian-derivation work (Path A of the 2026-05-27 plan; CLOSED-NEGATIVE for the naive identity per §3) is a *separate* attack on the algebraic side; its progress or failure does not enter this pre-registration's verdict.

---

## §9 — Sign-off and hash-lock

**Author:** lead-physicist subagent under user direction (2026-05-27 session).

**Approval gate.** Before hash-lock, the document is reviewed for:
- (a) Are all 25 §4.A basket entries clearly defined and verifiable?
- (b) Are all 10 §7 falsifier rules mechanically checkable?
- (c) Is the three-outcome scheme exhaustive (FOUND ∪ UNDERDETERMINED ∪ CLOSED-NEGATIVE covers all admissible verdicts)?
- (d) Has the FTD-0014 retirement been correctly cited (commit `ca7eb61`)?
- (e) Has CODATA 2022 / PDG 2024 been correctly cited as the constant source (`REF_EXTERNAL_CONSTANTS.md`)?
- (f) Does the LEDGER row reservation FTD-0210 correspond to the next-free ID?
- (g) Does §3 correctly cite the Path A CLOSED-NEGATIVE status (`scripts/exploration/check_zeta_hessian_gstar2.py`)?

**Hash-lock protocol.** Once §§2–9 are approved:

1. Compute SHA256: `sha256sum docs/theory/10_eft_program/PREREG_X_MINUS_PHYSICAL_IDENTIFICATION_v1.md`. Record the SHA in `REF_PREREGISTER_MANIFEST.md` as a new row.
2. Add a LEDGER row (FTD-0210 or next-free) tagged `[PRE-REGISTRATION]`, citing this file.
3. `git commit` the pre-registration. Note the commit SHA in `REF_PREREGISTER_MANIFEST.md`.
4. Create lightweight tag: `git tag preregister-x-minus-physical-identification-v1 -m "Pre-reg for x_- physical identification under harmonic-sum-rule + L/R-asymmetry + FTD-0189-style adversarial uniqueness constraints"`.
5. The search (§6 steps 2–10) runs ONLY against the tagged commit. Its result lands in a SEPARATE document per §6 step 10 — never as edits to this file.
6. If a definition or filter here proves defective once the search starts, the correct response is a **v2 pre-registration**, NOT an edit to v1 (cf. the FTD-0186 v1→v2 precedent and the FTD-0208 v1→v2 precedent).

**Locked by hash:** the question (§2), definitions D1–D6 (§3), the admissible basket and uniqueness criterion (§4), the three outcomes and their tag consequences (§5), the measurement procedure (§6), the falsifier rules F-a through F-j (§7), the banned moves B-1 through B-10 (§8), this sign-off (§9).

**Open** — and only this: the **verdict**. Whether the §4.A basket contains a Q_phys satisfying all three §2 constraints under the §7+§8 discipline is exactly what the search will determine. FOUND, UNDERDETERMINED, and CLOSED-NEGATIVE are all admissible; the prior on CLOSED-NEGATIVE is high, and a clean CLOSED-NEGATIVE is itself a load-bearing project deliverable.

---

*Pre-registration authored 2026-05-27. No result. The search (§6) is the next step, and runs only after hash-lock.*
