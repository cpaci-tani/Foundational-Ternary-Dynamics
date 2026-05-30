# AUDIT — B-QM-1″: Full Symplectic Budget Symmetry from FTD Geometry — CLOSED-NEGATIVE

**Tag:** `[CLOSED NEGATIVE]` (pre-reg §6: N_c=3 candidate fails the kind-test; no other FTD source). **Adversarial (P4).** **No spine claim moved.**
**Date:** 2026-05-29
**Result of:** `PREREG_SYMPLECTIC_BUDGET_SYMMETRY_v1.md` (B-QM-1″, FTD-0228 provisional), SHA256 `dd8a8fa065ae2800d7554a2c82938137d340e0825e37a3362ffc1f22951a0f20` (commit deferred; SHA recorded in-session before the attempt).
**Verifier:** [`scripts/proofs/proof_symplectic_budget_symmetry.py`](../../../scripts/proofs/proof_symplectic_budget_symmetry.py) (5/5).
**Corrects:** the "J ⋈ N_c=3 convergence" surfaced (and flagged UNVERIFIED) in `AUDIT_SPEKKENS_KNOWLEDGE_BALANCE_PARTIAL.md` §1.

---

## 0 · Executive summary

**Verdict: CLOSED-NEGATIVE.** FTD's geometry does **not** supply the full symplectic `S₃` budget symmetry. The seductive `N_c=3 → ℤ/3` candidate is **apophenia** — a count-match that fails the *kind*-test.

The decisive pre-registered test (§3) was **commuting vs complementary** (verified 5/5):

| object | structure | role |
|---|---|---|
| `J²=−I` | order-4 rotation of *one* conjugate pair | supplies the **ℤ/2** (real) |
| `{J_x, J_y, J_z}` (spatial flux components) | **commute** `[J_i,J_j]=0` — a single **co-measurable** triple | what the body-diagonal C₃ permutes |
| budget bases (Pauli X,Y,Z) | **complementary** `[X,Y]=2iZ≠0` — mutually non-commuting | what the **ℤ/3** must permute |

Same **count** (3), different **kind**. The body-diagonal C₃ / N_c=3 permutes a *co-measurable* triple; the budget needs a 3-cycle on *complementary* bases. They are not the same operation, and no other FTD structure supplies a genuine Bloch ℤ/3: the planar rotations generate `SO(3)` acting **on** the commuting triple, which never makes those components complementary.

---

## 1 · The unification — the epistemic route re-derives the non-commutativity wall

The ℤ/3 the budget needs is the 3-cycle of `SU(2)/SO(3)` mixing complementary observables — a **non-commutative** rotation. So:

> **"Supply the budget symmetry" = "supply a non-commutative SU(2) on complementary observables" = supply the very non-commutativity the commutative substrate lacks.**

The ψ-epistemic reframe **relocated** the gap (from "derive ontic non-commutativity" to "derive the budget symmetry") but did **not dissolve** it. Every route tried this session lands on the same wall:

| route | wall |
|---|---|
| B1 (modular time) | commutative → type I → no canonical flow |
| B-QM-1 (manifestation) | functions of `J` → Boolean event lattice |
| B-QM-1′ (binding) | binding **derived**, but not sharp |
| **B-QM-1″ (budget symmetry)** | **the missing sharpness IS a non-commutative SU(2) the substrate lacks** |

One diagnosis, four faces: **the substrate is commutative; QM's non-commutativity is not in it.**

---

## 2 · What survives (the honest positive)

- **Binding is derived** (B-QM-1′, classical self-reference) — FTD genuinely yields a binding epistemic horizon for internal observers. Unaffected by this closure.
- **The ψ-epistemic framing is correct and valuable** — QM as epistemic ensemble statistics; FTD supplies the ontic discrete events + the binding restriction; and it **explains FTD-0199/0200** (binding-but-not-sharp ⇒ classical-with-noise ⇒ Rice/Gaussian, *not* the Spekkens/Born set).
- **The commutative spine is derived** — G\*, π√2, the master quadratic (the Z/4 quarter-structure). That is what the classical substrate *does* yield.

What FTD does **not** have is the **sharp budget symmetry** (the non-commutative SU(2)). So it gives **binding-but-not-sharp = classical-with-noise**, not the Spekkens-class.

---

## 3 · Falsifier checklist (pre-reg §5) — all clean

| F | Fires? | Why |
|---|---|---|
| F-α (count-match accepted) | **no — rejected by design** | the §3 kind-test (commuting vs complementary) is precisely what defeats the 3=3 match |
| F-β (posited SU(2)) | no | no SU(2) inserted; its *absence* is the finding |
| F-γ (`[q,p]=i`/ℏ) | no | none |
| F-δ (CODATA/fitting) | no | none |

---

## 4 · GTCA discipline note

This attempt was routed **P4 (adversarial)** with the **Aesthetic Filter inverted**: the "J ⋈ N_c convergence" was attractive (elegant, aligned with the session's `J`-thread — an F9 collusion-bias risk and an F1 apophenia risk), so its appeal triggered *extra* scrutiny rather than endorsement. The pre-registered kind-test (commuting vs complementary) was the mechanical guard that converted "two 3's, how beautiful" into "two 3's of different kind, candidate fails." **This is the self-correction working** — B-QM-1′ correctly tagged the convergence UNVERIFIED; B-QM-1″ killed it. Recording it prevents a zombie re-emergence of "N_c=3 supplies the quantum ℤ/3."

---

## 5 · Honest accounting

- **Not over-hardening.** The kind-test rests on a structural fact: independent field components commute (`[J_i,J_j]=0`), Pauli axes do not (`[X,Y]=2iZ`). Verified. Every FTD "three" (spatial components, color charges, conjugate pairs) is either co-measurable or a multi-system structure — none is the three complementary bases of a single cell.
- **Spine untouched.** `x₊=1/α` `[SMC]`; no tag moves.
- **The arc is complete.** The derive-QM / epistemic program is comprehensively mapped: binding derived, sharpness closed against current geometry, the non-commutativity wall unified across all four routes.

**Bottom line:** FTD's classical substrate derives the *binding* epistemic horizon and the *commutative* spine, but **cannot** derive the *sharp* (non-commutative) budget symmetry — so it reproduces classical-with-noise, not the Spekkens-class. The ψ-epistemic reframe is the right *framing* and yields a real positive (binding), but the non-commutativity gap is invariant under it. To get the Spekkens-class FTD must add a non-commutative SU(2) on the budget — an import, the same 6th-postulate-class move as everywhere else.

---

## 6 · Provenance & discipline

Deferred commit; pre-reg SHA `dd8a8fa0…` recorded in-session before the analysis. Verifier 5/5; no posited SU(2), no inserted ℏ, no count-match accepted as evidence, no CODATA. Routed P4 under GTCA with inverted Aesthetic Filter; F1/F9 guards active and load-bearing.
