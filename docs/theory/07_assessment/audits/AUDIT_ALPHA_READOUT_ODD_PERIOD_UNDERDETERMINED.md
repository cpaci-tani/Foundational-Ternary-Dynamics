# AUDIT — Odd-Period via J-Twisted Regularized Determinant: UNDERDETERMINED

**Tag:** `[UNDERDETERMINED]` — pre-reg §6 UNDERDETERMINED ("the J-twisted det_ζ ratio is real and FTD-native, but its identification with the operator determinant is natural-yet-unforced — a structural lead, not a closure"). **No spine claim moved.**
**Date:** 2026-05-28
**Result of:** the pre-registered attempt `PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md` (FTD-0234 provisional), SHA256 `a5c97b7363a1e389ea5e2eff0f139a00f0bd04f8b0d21166845fefd38c53faa1` (recorded in-session before the analysis; commit deferred by owner — §6).
**Verifier:** [`scripts/proofs/proof_odd_period_jtwisted.py`](../../../scripts/proofs/proof_odd_period_jtwisted.py) — 6/6 facts verified.
**Chains from:** `AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md` (FTD-0233, the parity no-go this attempt broadens) and the owner's hint *"the lattice is J²."*

---

## 0 · Executive summary

The determinant-grading no-go (FTD-0233) showed: within the frozen ingredients (all **even** G\*-degree), the determinant's **odd** third power of G\* is unreachable, the only even→odd route being `√Watson` (a forbidden prefactor, F4). This attempt broadened the admissible set by exactly one candidate — the **J-twisted ζ-regularized determinant** (FQCR Model I) — following the owner's *"the lattice is J²"* hint.

**Two findings, one verdict (UNDERDETERMINED):**

1. **PROGRESS — a clean forward odd-degree G\* source exists.** FQCR Model I (`[THEOREM]`, Prop 1) gives `det_ζ(D_{3/4})/det_ζ(D_{1/4}) = Γ(1/4)/Γ(3/4) = G*` — a **degree-1 (odd) G\***, on the same `J²=−I` structure as `V_complex`, and **clean**: the `√(2π)` cancels in the ratio, so there is **no √π prefactor** (unlike `ϖ`/`K` or `√Watson`). This **lifts the "no clean odd source" half of the FTD-0233 no-go.** Verified: `16G*³ = (16G*²)·(det_ζ ratio)`. The owner's hint is *partially vindicated* — `J²` does supply a clean odd G\*.

2. **GAP — the link to the operator determinant is unforced (OP3).** The readout operator `T` on `V_complex` is **finite** (2 eigenvalues `x₊ ≈ 137`, `x₋ ≈ 3`); the det_ζ ratio is a ratio of regularized determinants of two **infinite** S¹ progressions `{n+¼}`, `{n+¾}`. Numerically the det_ζ ratio is **G\* ≈ 2.96**, **not** the operator determinant **16G\*³ ≈ 414**, and `T`'s spectrum `{x₊, x₋}` is **not** the J-twisted spectrum `{n+¼}`. Relating them *requires asserting* `Det(T) = Tr(T) · (det_ζ ratio) = 16G*² · G* = 16G*³` — the **same "det = trace × G\*" factorization** the resolution docs assert, now in det_ζ clothing. FQCR Model I §7 *itself* disclaims deriving the master quadratic. No structural compulsion forces the identification → **OP3 fires → UNDERDETERMINED.**

So the determinant grading is **more rescuable than the bare parity no-go suggested** (there is a clean odd source), but it is **not yet rescued** (the forced detdet_ζ identity is missing). The ARC-C1/B2 "FOUND" remains an overclaim; honest status **UNDERDETERMINED**.

---

## 1 · The attempt (pre-reg §7 method, executed)

**Step 1 — Model I read.** `det_ζ{n+a}_{n≥0} = √(2π)/Γ(a)` (Lerch); the J-twisted spectra `D_{1/4}, D_{3/4}` arise from `ψ(φ+2π) = Jψ(φ)`, `J²=−I`. Ratio `= Γ(1/4)/Γ(3/4) = G*` (DERIV_GSTAR_QUARTER_CONJUGACY §4, `[THEOREM]`).

**Step 2–4 — the candidate.** `G*` (degree 1) is genuinely available forward (Model I), clean. Assembling `16G*³ = |μ₄|² · (2π·G_BCC(0)) · (det_ζ ratio) = 16 · G*² · G*` is numerically exact (script §2). **No √π prefactor** is needed (the only prior odd candidates `ϖ = G*√π/2`, `K(1/√2) = G*√(2π)/4` carried √π; the det_ζ ratio does not). So **OP4 does not fire** — the prefactor objection that killed the `√Watson` route is absent here.

**Step 3 (the hinge) — fails.** Is `Det(T)` *structurally* the J-twisted ζ-regularized determinant? No:
- `T` is finite-dimensional (degree-2 char poly, 2 eigenvalues); the det_ζ ratio is built from infinite spectra.
- `det_ζ ratio = G* ≈ 2.96 ≠ Det(T) = 16G*³ ≈ 414`; and `spec(T) = {x₊, x₋} ≠ {n+¼}` (script §3).
- A *transfer-operator  ζ-regularized-determinant* correspondence is **suggestive** (determinants of infinite transfer operators are ζ-regularized in legitimate physics) but is **not exhibited** here for `T`; one must **assert** `Det(T) = Tr(T)·(det_ζ ratio)`. That assertion is the unforced factorization → **OP1/OP3**.

**Step 5 — falsifier checklist:**

| F | Fires? | Why |
|---|---|---|
| OP1 (assertion) | **YES** | `Det = Tr·G*` is still posited, not derived from `T`'s structure |
| OP2 (M_N import) | no | `M_N` not imported |
| **OP3 (unforced detdet_ζ link)** | **YES (decisive)** | no structural reason `T`'s finite determinant *is* the infinite J-twisted det_ζ ratio |
| OP4 (prefactor) | **no** ✓ | the det_ζ ratio is clean — *this is the genuine advance* |
| OP5 (insertion) | no | `G*` is *derived* via Model I `[THEOREM]`, not inserted as a value |
| OP6 (CODATA) | no | none |
| OP7 (look-elsewhere) | partial | the det_ζ ratio is principled (not chosen among many), but its *application* to `Det(T)` is unforced |

OP3 firing with OP4 *not* firing is exactly the UNDERDETERMINED signature: a real, clean lead, blocked only by a missing structural identity.

---

## 2 · Verdict — UNDERDETERMINED (a sharp lead, not a closure)

The J-twisted det_ζ ratio is a genuine, clean, forward, FTD-native odd-degree G\* source (real progress over FTD-0233). But the readout operator's determinant is **not compelled** to be that ratio; supplying the odd G\* still rests on the asserted factorization `Det = Tr · G*`. Therefore the ARC-C1/B2 "FOUND" is **not** rescued, and its honest status remains **UNDERDETERMINED** (confirming `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md`), now with the gap **localized to a single structural identity** rather than a flat parity no-go.

---

## 3 · The precise v2 target (what would make it FOUND)

> **Prove (or refute) that the readout operator's determinant is the J-twisted ζ-regularized determinant of its own spectrum** — i.e. exhibit `T` as a (regularized) transfer operator whose ζ-regularized determinant *is* `det_ζ(D_{3/4})/det_ζ(D_{1/4})` content, so that `Det(T) = 16G*³` follows by a forced detdet_ζ identity, not by assertion.

Concretely, a v2 attempt should either (a) construct `T` as a genuine infinite-dimensional transfer operator whose spectrum carries the `{n+¼}`/`{n+¾}` J-twisted structure and whose ζ-regularized determinant therefore equals the Model I ratio; or (b) prove no such `T` exists with the right *finite* eigenvalues `{x₊, x₋}` simultaneously, which would convert this UNDERDETERMINED to CLOSED-NEGATIVE. The hinge is entirely the **detdet_ζ structural identity**; everything else (the clean odd source, the absence of a prefactor, `16 = |μ₄|²`, `G*²` from Watson) is in hand.

---

## 4 · Net status across the three attempts

| Doc | Scope | Verdict |
|---|---|---|
| `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW` | the committed "FOUND" claims | overclaim; honest = **UNDERDETERMINED**; gap = the determinant's odd G\* power |
| `AUDIT_..._DETERMINANT_GRADING_CLOSED_NEGATIVE` (FTD-0233) | frozen ingredients only | **CLOSED-NEGATIVE** — no odd source (parity); only `√Watson` (F4) |
| `AUDIT_..._ODD_PERIOD_UNDERDETERMINED` (FTD-0234, this) | + J-twisted det_ζ | **UNDERDETERMINED** — clean odd source *exists* (det_ζ), but detdet_ζ link unforced |

**Consolidated honest reading:** the ARC-C1/B2 FOUND **overclaims** (the determinant grading is asserted, confirmed twice). But the EM-coupling readout is **closer to derivable than a flat no-go** — there is a clean, FTD-native odd-degree period (the J-twisted det_ζ ratio), and the entire remaining obstruction is one stated structural identity (`Det(T) ` J-twisted det_ζ). MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`, now with a **sharp, single-identity frontier.**

Spine untouched: `x₊ = 1/α` (FTD-0013) `[STRONGLY MOTIVATED CONJECTURE]`; `G*`, master quadratic, coefficient 16 — unchanged. Contract §7 forbids tag moves before ARC-3 regardless.

---

## 5 · Provenance & discipline

- **Deferred commit (owner-authorized).** Pre-reg SHA `a5c97b73…` recorded in-session before the analysis; design (incl. the detdet_ζ hinge as the locked question) frozen pre-attempt. Canonization: commit pre-reg first, verdict separately (B-9), independent review (B-10).
- **Compute, not recall.** All facts in `proof_odd_period_jtwisted.py` (6/6), cross-checked vs `constants.py` `G_STAR`.
- **GTCA F9 (collusion bias) guarded.** The owner's hint pointed at a positive outcome; it was *partially* vindicated (clean odd source found) but **not** rubber-stamped into a FOUND — the unforced detdet_ζ link (OP3) is reported honestly as the blocker. The verdict is the structural finding, not the prior.

*Progress and boundary, both mapped: a clean odd lemniscatic period is forward-available (the J-twisted det_ζ), and the EM-readout's closure now reduces to one provable-or-refutable structural identity.*
