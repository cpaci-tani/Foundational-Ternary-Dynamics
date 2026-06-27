# PRE-REGISTRATION (v3, post red-team x2) — The genesis-cokernel grading: does the lossy-merge fiber carry δ, Galois-positioned over the field ℚ(G\*)? (MC-T4.3 cokernel crack)

**Status:** PROTOCOL — to be hash-locked (SHA256 + git tag `preregister-genesis-cokernel-grading-v1`) **before** any construction run. Two adversarial soundness red-teams blocked the earlier drafts (v1: real-only discriminator + unfalsifiable G3 + §2 type-collision; v2: complex-coset blind spot fixed but a **field-vs-polynomial** false-negative remained and §2 well-posedness was overstated). **v3** closes the discriminator (genuine field membership over ℚ(G\*), denominators cleared) and honestly reframes §2 with a new well-posedness gate. Fixes recorded inline.
**Date:** 2026-06-27 · **LEDGER (on verdict):** next-free id, assigned at lock (CLAUDE.md next-free = FTD-0329; confirm at lock) · **Arc:** MC-T4.3 cokernel crack — design `docs/theory/02_foundations/EXPLR_GENESIS_COKERNEL_GRADED_SQRT.md`.
**Frozen instrument (§1):** `scripts/proofs/proof_genesis_cokernel_grading.py` (v3) — SHA256 `63c342fae6c122c20ce5c6a93349e33a6a8710f73a0cf304ab4983a71b585b39`.

---

## 0 · Purpose and honesty ceiling

Every algebraic-side route to the master-quadratic surd `δ = √(G*(4G*−1))` has closed negative: native operators sit in ℚ(G\*) (FTD-0244 K-BIND); all 5 native ℤ/2's are Galois-blind to δ (FTD-0326); the AGM is δ-blind (FTD-0327). One structurally-distinct carrier remains **unexamined**: the **information-loss fiber of the irreversible, many-to-one genesis/manifestation merge** — the one object multivalued *by construction*.

**The genuinely-`[OPEN]` slice (the only question here):** expressed in the discrete Dirac–Kähler complex of FTD-0089, does the **section-invariant arithmetic content** of the genesis disintegration fiber carry the discriminant `±G*(4G*−1)` — i.e. is the fiber's chirality grading `g` an element whose field `ℚ(G*)(g)` **contains δ** (Outcome A), or does it sit in `ℚ(G*,i)` with δ absent (Outcome B)?

**Honesty ceiling (re-asserted in §4/§5/§7):** This object **formalizes and locates** where the missing free orientation must enter (the section-invariant content of the lossy-merge fiber). It does **not** conjure that orientation from the deterministic forward substrate, and **not** eliminate it — the disintegration is multivalued *by construction*, and that non-uniqueness *is* the choice. **Even Outcome A does NOT derive α.** Re-sourcing δ supplies only the **root-distinguishing surd**; it does **not** assemble the master-quadratic operator `(Tr, Det) = (16G*², 16G*³)` (MC-T4.3, untouched) **nor** identify `x₊` with `1/α` (FTD-0013, `[SMC]`). **FC-W-conditional is the ceiling of Outcome A.** Zero promotions under any outcome (§5).

---

## 1 · Frozen artifact (the instrument)

`scripts/proofs/proof_genesis_cokernel_grading.py` (v3), SHA256 `63c342fae6c122c20ce5c6a93349e33a6a8710f73a0cf304ab4983a71b585b39`, default dps 140, degree budget 12.

The instrument is the **discriminator** `classify_grading(g)` → `'A' | 'B' | 'UNDERDETERMINED'`, using the FTD-0244/0326/0327 PSLQ-over-ℚ(G\*) protocol. **v3 criterion — genuine FIELD membership.** `g` is decomposed against the ℚ(G\*)-basis `{1, i, δ, iδ}` of the degree-4 CM field `ℚ(G*,i,δ)`, where ℚ(G\*) is the **field of rational functions** of G\* (not the polynomial ring): denominators are cleared by PSLQ over `{x·G*^j}` jointly with `{G*^k}` (and `{δ·G*^k}`). **Outcome A iff δ is present** (`ℚ(G*)(g)` carries δ); **Outcome B iff `g ∈ ℚ(G*,i)` or `g` is a root of unity** (δ absent — any cyclotomic sign is δ-blind, FTD-0326); **UNDERDETERMINED iff `g` lies genuinely outside `ℚ(G*,i,δ)`** (e.g. `ζ₈·δ`, `√2·δ` — involving `√2`) or is degenerate (G2). This closes both prior blind spots: complex δ-cosets (`i·δ`, `(1+i)·δ`) **and** rational-function-coefficient cosets (`δ/G*`, `δ/(4G*−1)`, `1/δ` — the *generic* output of the §2 Moore–Penrose complement) are now correctly **A**. Self-tests pass **17/17** (5 B / 9 A / 3 UNDERDETERMINED), `reclassify_over_dps_band` unanimous across dps {100,120,140} (recorded at lock). `g` is fed **un-normalized** (the magnitude carries the δ-content; §7).

The construction stub `construct_genesis_cokernel_grading()` raises `NotImplementedError`. The **pre-reg MANDATES** import-separation, the no-sign-substitution quarantine, and section-invariance (§3/§7); these are **reviewer-verified construction obligations the frozen instrument cannot self-enforce** — it sees only the final number `g`.

---

## 2 · The construction (math criteria, frozen) — type-collision REFRAMED, not claimed resolved

The v1 draft conflated two incompatible objects. v2/v3 **reframe** it (the residual decidability is itself part of the open attempt, gated by G5):

1. **The linear DK complex** `K = d − δ` of **FTD-0089** (`DERIV_DIRAC_KAHLER_IDENTIFICATION.md`, the lattice `(d−δ)Φ = mΦ` system) — **reuse**. `K` is the legitimate graded chiral square root (`K² = −Δ_Hodge`, the EVEN/self-adjoint side); its ℤ/2 chirality `γ` is well-defined. **We do NOT claim `D∘D =` the nonlinear merge.**
2. **The canonical section.** The genesis merge `M_disc` (nonlinear: threshold + sign + drain) is taken with the **Moore–Penrose / Hodge-orthogonal (minimal-norm) complement** — a *forced* complement (its structure constants are rational functions of G\*, hence the §1 field-membership requirement).
3. **The fiber content.** The genesis **nonlinearity** is **relocated** to the structure constants of the information-loss fiber over that complement, in the DK complex (ℚ(G\*)-valued, K-BIND/FTD-0244).
4. **`g`** = the **section-invariant** chirality grading of the fiber content (≥ 140 digits, un-normalized), fed to `classify_grading(g)`.

**Honest scope (the red-team correction):** whether the genuine nonlinear information-loss content **separates** from the *forced-cyclotomic* linear-harmonic data of `K` (whose branch sign `(1+i)/√2` is a root of unity, ℚ(G\*)-blind, FTD-0323 §5) is **itself part of the open attempt** — it is asserted in design, not exhibited. If the fiber content is not separably well-defined, the verdict is **UNDERDETERMINED** (G5), never a default B.

---

## 3 · Gates (all must pass for the attempt to count)

- **G1 — discriminator validity:** `_selftests()` 17/17 + G4-band PASS at lock (recorded §1); frozen thresholds (`PSLQ_MAXCOEFF=10¹²`, `MAXDEG=12`, `ROU_MAX_ORDER=24`, dps {100,120,140}) unchanged post-lock.
- **G2 — single well-defined, non-degenerate `g`:** the construction yields one algebraic `g` (≥ 140 digits), not a family. Else → **UNDERDETERMINED**. **A null or degenerate grading is NOT Outcome B** — `g = 0`, or any `g` that collapses into ℚ(G\*) **without an exhibited non-trivial ℤ/2 branch structure**, is **UNDERDETERMINED**. An Outcome-B reading (a forced ℤ/2 that hardens the wall) requires the construction to **exhibit a genuine order-2 branch label** of the multivalued inverse; a number in ℚ(G\*) alone does **not** establish that a ℤ/2 branch exists — it may equally mean the fiber had no branch content (a degenerate well-posedness failure that must not masquerade as wall-hardening). Reviewer-verified, alongside G3.
- **G3 — section-invariance (construction obligation, reviewer-verified):** the instrument cannot check this from one number. The post-lock attempt **must exhibit** invariance — compute `g` under **≥ 2 distinct admissible sections** and show machine-zero agreement; the canonical minimal-norm complement (§2) is the reference. If `g` depends on the section, → **UNDERDETERMINED** (not B). The invariance proof is itself red-teamed on verdict.
- **G4 — dps-band reproducibility (instrumented):** `reclassify_over_dps_band(g, {100,120,140})` returns a verdict only if **unanimous** (else UNDERDETERMINED); guards against spurious PSLQ relations (a spurious B would falsely harden the wall). The post-lock attempt also raises `MAXDEG` once and confirms **degree-stability**.
- **G5 — construction well-posedness (NEW):** if the nonlinear information-loss fiber content is **not separably well-defined** from the forced-cyclotomic linear-harmonic data (§2 honest scope), the verdict is **UNDERDETERMINED** — a construction stall is recorded honestly, never leaked into a B reading.

Any gate failure ⇒ the run does not count (re-scope), **not** a positive verdict.

---

## 4 · Frozen verdict map

`classify_grading(g)` returns exactly one of:

| Verdict | Condition (frozen, v3) | Meaning |
|---|---|---|
| **A (re-sources δ)** | `ℚ(G*)(g)` carries δ — `g ∈ ℚ(G*,i,δ)` (field) with a nonzero δ-component | **Does NOT derive α and does NOT move any tag now.** It *would* — only after a **fresh pre-reg + adversarial red-team** — make FC-W a `[CONDITIONAL THEOREM]` given the construction, and reopen the α-readout program. Re-sourcing δ supplies only the root-distinguishing surd, **not** the operator assembly (MC-T4.3) nor `x₊ = 1/α` (FTD-0013). |
| **B (forced ℤ/2)** | `g ∈ ℚ(G*,i)` (real ℚ(G\*) incl. denominators, or Gaussian) **or** `g` a **root of unity** (any cyclotomic sign — δ-blind by the abelian/cyclotomic argument, FTD-0326) — δ absent **and** a genuine non-trivial order-2 branch is exhibited (a null/branch-less collapse is UNDERDETERMINED, G2) | a **6th forced ℤ/2**; **hardens the wall** — extends K-BIND/FTD-0326 from operators + finite symmetries to the lossy-merge fiber, the last structurally-distinct carrier. |
| **UNDERDETERMINED** | no single `g` (G2), section-dependent (G3), non-unanimous over the dps band (G4), construction not well-posed (G5), or `g` genuinely outside `ℚ(G*,i,δ)` (e.g. `ζ₈·δ`, `√2·δ`) | re-scope; **not** a coincidence hunt. |

---

## 5 · Tags and priors (recalibrated — the round-1 correction, confirmed honest)

**The v1 "determinism → B ~75–80%" prior is withdrawn.** The determinism argument (*"a √ of a ℚ(G\*)-valued integral adjoins a sign, not new transcendence"*) **conflates the sign with the grading**: a cyclotomic sign *times* δ-content gives `cyclotomic × δ` = **Outcome A**, not B. It is a **heuristic only**, **quarantined** from the construction (§7).

**Recalibrated priors:** **UNDERDETERMINED ~45 %** (prior-dominant — the disintegration is multivalued; section-invariant single `g` + G5 well-posedness are strong requirements); **B ~35 %**; **A ~20 % — genuinely open**, *not* the near-excluded ~20–25 % of v1. **Note (pre-empting a "priors tuned to taste" objection):** this recalibration moves the registered prior **against** the framework's preferred boundary (B) outcome, and raises A. The headline is that **no outcome is foreclosed by the design**.

**Standing invariants held under every outcome:** `x₊ = 1/α` `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013); MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`; FC-W `[AXIOM]`-class (FTD-0315); FTD-0244/0314/0326/0327 untouched; **no α derived**; golden gate untouched (pure number theory, no engine state). Outcome A does **not** itself promote anything.

---

## 6 · Scope

In scope: whether the section-invariant content of the genesis disintegration fiber, in the DK complex, carries δ (Galois position over the field ℚ(G\*)). Out of scope: the operator-assembly obstruction MC-T4.3 (untouched); the physical identification `x₊ = 1/α` (empirical `[SMC]`); engine measurements (this is number theory).

---

## 7 · Banned moves

1. **No value-planting** of δ (W-CRIT-2): `g` emerges from the FTD-0089 fiber, never inserted. The pre-reg **mandates** import-separation (the construction must not reference `delta_surd()`/`C_disc()`/`G*(4G*−1)` when producing `g`); **reviewer-verified** (the instrument cannot self-enforce).
2. **No sign-substitution (quarantine):** do **not** substitute the known `∂_t^{1/2}` branch sign `(1+i)/√2` (or any cyclotomic sign) for the genesis-cokernel grading; `g` must be computed from the actual fiber. The §5 determinism argument is the prediction's rationale, **not** a construction shortcut.
3. **No section-forcing:** the section is the canonical minimal-norm/Hodge-orthogonal complement (§2); `g` must be **section-invariant** (G3) or the verdict is UNDERDETERMINED.
4. **No modulus-normalization:** feed `g` to `classify_grading` as the **full algebraic number**, never `g/|g|` — normalizing a δ-coset (e.g. `i·δ/(4G*−1)` → `i`) would falsely collapse A→B. The magnitude carries the δ-content.
5. **No near-miss / basket search.** The discriminator classifies a **single defined** `g`.
6. **No reading Outcome A as a derivation of α** or as moving FTD-0013/0315 — A only re-sources δ and requires a fresh pre-reg (§4 row A).
7. **No free-floating new operator:** the linear graded √ is FTD-0089's `K = d − δ` (§2).
8. **No changing** the frozen thresholds (§3 G1) or the verdict map (§4) post-lock.

---

## 8 · Hash-lock

`proof_genesis_cokernel_grading.py` (v3) SHA256 `63c342fae6c122c20ce5c6a93349e33a6a8710f73a0cf304ab4983a71b585b39`; this file + the instrument are committed and git-tagged `preregister-genesis-cokernel-grading-v1` at the lock commit, **before** any construction run (`construct_genesis_cokernel_grading()` is a `NotImplementedError` stub at lock; only `_selftests()` — which exercises the discriminator, not the deferred construction — has been run, 17/17 + G4-band PASS). The construction attempt, analysis, and verdict are recorded in a separate `ANALYSIS_GENESIS_COKERNEL_GRADING_v1.md` (LEDGER id assigned at lock) after the lock.
