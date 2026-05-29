# AUDIT — Determinant-Grading Forward Derivation: CLOSED-NEGATIVE (G\*-degree parity no-go)

**Tag:** `[CLOSED NEGATIVE]` — pre-reg §6 CLOSED-NEGATIVE ("a structural argument that the frozen ingredients generate only a restricted set of G\*-degrees that excludes the determinant's"). **No spine claim moved.**
**Date:** 2026-05-28
**Result of:** the pre-registered attempt `PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md` (FTD-0217 provisional), SHA256 `f55c7504401a1e5eb4a61ae18380d10c0ae8a4d407cfb1fc48da45e91918abd7` (recorded in-session before the analysis; git commit deferred by owner instruction — see §8).
**Verifier:** [`scripts/proofs/proof_determinant_grading_parity.py`](../../../scripts/proofs/proof_determinant_grading_parity.py) — 11/11 parity facts verified (sympy + mpmath).
**Confirms + sharpens:** `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` (the ARC-C1/B2 "FOUND" is an overclaim; honest status UNDERDETERMINED).

---

## 0 · Executive summary

The attempt tried to **rescue** the ARC-B2/C1 "FOUND" by forward-deriving the one step the independent review found unjustified: the determinant `Det(T) = 16G*³` (the master-quadratic's product coefficient; with `Tr(T) = 16G*²`, grading `Det/Tr = G*`), using **only** the frozen ingredients `{V_complex ≅ Z[i]², |μ₄|² = 16, Watson G_BCC(0) = G*²/(2π), winding index}` and **without** the forbidden moves (inserting the master quadratic / Theorem 8 / FQCR `M_N`, or a `√Watson`/`G*`-prefactor selection).

**Verdict: CLOSED-NEGATIVE — a structural no-go by G\*-degree parity.**

Every frozen ingredient has an **even** G\*-degree (`|μ₄|²`, winding, `J²=−I`: degree 0; Watson: degree 2). The admissible forward operations (sum, product, Z[i]-norm, trace, determinant) **add or preserve** G\*-degree; for the determinant — a polynomial in the operator's entries — they keep it **even**. But `16G*³` has **odd** degree 3. Since `G* = Γ(1/4)/Γ(3/4)` is transcendental, `{1, G*, G*², G*³, …}` are Q-linearly independent, so `G*³ ∉ Q[G*²]`: **no forward operator over the frozen (even-degree) ring can have determinant `16G*³`.** The unique even→odd route is a square root — `√Watson = G*/√(2π)`, the field **amplitude** rather than the self-**energy** — which is precisely pre-reg falsifier **F4**, and precisely the **"Selection 1"** the resolution docs already admit (FTD-0216 §3, FTD-0215 §4).

**The "FOUND" cannot be rescued within `V_complex + Watson + μ₄`.** The missing third power of G\* is a genuine boundary, not a not-yet-done step (§5 localizes exactly what would be required).

---

## 1 · The attempt (pre-reg §7 method, executed)

**Step 1 — G\*-degree accounting.** Frozen ingredients and their G\*-degree:

| Ingredient | G\*-degree |
|---|---|
| `|μ₄|² = 16` (Gaussian unit-group count, `= |Aut(E)|²`) | 0 |
| winding index `Ind ∈ Z` (charge quantization) | 0 |
| `J² = −I` (complex structure on `V_complex`; `det = 1`, `tr = 0`) | 0 |
| Watson self-energy `G_BCC(0) = G*²/(2π)` | 2 |

Admissible operations — sum, product (degrees add), Z[i]-Hermitian norm `|z|² = z z̄` (lattice configs are degree 0), trace and determinant (polynomials in entries) — generate, from `{0, 2}`, only **even** G\*-degrees `{0, 2, 4, …}`.

**Step 2 — Trace check (reachable).** `Tr = 16G*² = |μ₄|² · 2π · G_BCC(0)` is degree 2 (even) and is plausibly assembled from the ingredients. *Granted.*

**Step 3 — Determinant attempt (the obstruction).** `Det = 16G*³` is degree 3 (**odd**). Verified mechanically (script §3): a general 2×2 operator with entries in `Q[G*²]` has determinant with G\*-powers `{0, 2, 4}` only — coefficient at `G*³` is identically `0`, while the target needs `16`. No assignment of rational entries solves it. The determinant of any forward, even-degree operator is even-degree; `16G*³` is unreachable.

**Step 3′ — the only escape, and it is F4.** The sole even→odd route is `√(G_BCC(0)) = G*/√(2π)` (degree 1) — the field **amplitude** `|J|` rather than the self-**energy** `|J|²`. Selecting the amplitude over the energy, with a prefactor tuned so `R = 16π·√Watson = 8√(2π)·G*`, is an **unforced choice** (the energy is the natural self-interaction quantity) → **F4 fires**. This is identically the resolution docs' admitted "Selection 1."

**Step 4 — falsifier checklist** (§3 below). **Step 5 — numerical comparison: NOT reached** (no forward construction passes; no candidate readout is compared to `1/α`; no CODATA value used).

---

## 2 · The owner's hint, tested fairly

The hint — *"the lattice is J²; we have Aut(E²) where E = 4; they are conjugate fields where each field is 3D"* — was taken as the candidate construction and tested against the locked falsifiers. **It is genuinely illuminating about the even-degree content, and it does not supply the odd power:**

- **`Aut(E²)`, `E = 4`** → the two conjugate Gaussian factors `Z[i] ⊕ Z[i] = V_complex`, each with unit group `μ₄` of order 4, give `|Aut| ⊇ |μ₄|² = 16`. This is a clean, FTD-native origin for the coefficient **16** — degree **0**. ✓ (and it confirms FTD-0212's honest `[DERIVED]` of the 16).
- **`J² = −I`** → the complex structure on `V_complex`. Degree **0** (`det J = 1`, `tr J = 0`; a pure rotation carries no G\*).
- **"conjugate fields, each 3D"** → the parity-conjugate pair embedded across the 3 spatial axes is exactly the **BCC triple-product** structure `σ_BCC = 1 − cos k₁ cos k₂ cos k₃`, whose origin Green's function is the Watson integral. Crucially, the "3D cube" is the **exponent on the 1D lattice sum** (`Σ[C(2m,m)/4ᵐ]³`), and each axis contributes a *rational* (degree 0); the `Γ(1/4)` emerges from the **sum**, giving `Γ(1/4)⁴ = G*²·2π²` — degree **2**, **not** `Γ(1/4)⁶ = G*³·…` (degree 3). Verified (script §5).

So the hint organizes `16` (degree 0) and `G*²` (degree 2) elegantly — but every component is **even-degree**. It supplies **no** odd-degree source. Tested additionally: (i) a "3/2 power from 3D" `(G_BCC(0))^{3/2} = G*³/(2π)^{3/2}` reduces to `energy × √energy` — the `√` is F4; (ii) a "norm over 3 conjugate axes" has no degree-1-per-axis factor (axes are degree 0); (iii) the Z[i]-norm on `V_complex` is integer-valued (degree 0). None reaches `G*³` forward. The hint **reinforces** the parity no-go rather than escaping it.

---

## 3 · Falsifier checklist (pre-reg §5)

| F-rule | Fires? | Why |
|---|---|---|
| **F1** (target insertion) | no | the master quadratic / its coefficients / roots are not used as input |
| **F2** (FQCR `M_N` import) | no | `M_N` is not imported |
| **F3** (root-property circularity) | no | Theorem 8 / root properties not used |
| **F4** (G\*-prefactor / `√Watson` selection) | **YES** | the *only* even→odd route is `√Watson = G*/√(2π)` (amplitude over energy), an unforced choice = the docs' "Selection 1" |
| **F5** (CODATA) | no | no α value anywhere; step 10 not reached |
| **F6** (look-elsewhere) | (subsumed by F4) | the odd power enters only via the unforced amplitude choice |

A single falsifier (F4) firing, together with the structural even-degree argument, gives §6 CLOSED-NEGATIVE.

---

## 4 · Verdict — CLOSED-NEGATIVE (structural parity no-go)

Within the frozen ingredients, the determinant grading `16G*³` is **not forward-derivable**: it is odd-degree in G\*, the ingredients are even-degree, forward operations keep the determinant even-degree, and the unique even→odd route fires F4. This is the strong (§6 CLOSED-NEGATIVE) outcome: *the ingredients generate a restricted set of G\*-degrees (even) that excludes the determinant's (odd 3).*

Consequence: **the ARC-B2/C1 "FOUND" cannot be rescued within `V_complex + Watson + μ₄`.** The independent review's finding stands and is **sharpened**: the ARC-C1/B2 honest status is **UNDERDETERMINED**, and the open gap is now understood as a **structural boundary** (the odd-degree period is simply not in the frozen catalog), not a derivation merely awaiting completion.

---

## 5 · Constructive localization — exactly what would be required

The obstruction names its own resolution. The Watson integral supplies the **even** lemniscatic period `G*²` (the full-period / energy scale). The determinant needs one more, **odd**, power of G\* — i.e. a forward, lattice-native source of an **odd-degree period**, the natural candidate being the **lemniscate constant**

$$\varpi = \frac{\Gamma(1/4)^2}{2\sqrt{2\pi}} = G^*\cdot\frac{\sqrt{\pi}}{2}\quad(\text{degree 1 in }G^*).$$

`ϖ` is the *real period / arc-length* of `y² = x³ − x` — distinct from the Watson integral (the full-period square). **If FTD can forward-derive `ϖ` (or any odd power of G\*) from a lattice-native quantity — distinct from `√Watson`** — then `ϖ · G_BCC(0) ∝ G* · G*² = G*³` supplies the determinant forward and the FOUND is rescued. That derivation is **not** available in the frozen catalog (it would be a `[CONJECTURE — new postulate]`, §6(b) territory), and it is the precise object a future ARC-D / new-postulate pre-reg should target: *a lattice source of the odd lemniscatic period.*

---

## 6 · Falsifiable hinge

This no-go is itself falsifiable. It is **wrong** iff there exists a forward, lattice-native construction yielding an **odd** power of G\* (degree 1 or 3) from the frozen ingredients **without** a `√Watson`/prefactor selection (F4) and without inserting the master quadratic (F1/F3). If the owner's hint encodes such an odd-degree source not captured in §2, exhibiting it overturns this verdict. Absent that, the determinant grading is closed-negative within scope.

---

## 7 · Status ledger (nothing promoted or demoted)

- `Det = 16G*³` from `V_complex + Watson + μ₄`: **`[CLOSED NEGATIVE]`** (this verdict).
- ARC-C1 / ARC-B2 "FOUND-at-ARC-2" (FTD-0215/0216): **honest status UNDERDETERMINED** (per the independent review + this sharpening). *Recommended downgrade pending owner sign-off; not executed here.*
- Genuine kernel preserved: `V_complex ≅ Z[i]²`, charge quantization `{−1,0,+1}`, `16 = |μ₄|²` (FTD-0212 `[DERIVED]`), Watson `G_BCC(0)=G*²/(2π)`, finite-block CLOSED-NEGATIVE — all stand.
- Spine untouched: `x₊ = 1/α` (FTD-0013) `[STRONGLY MOTIVATED CONJECTURE]`; `G*`, master quadratic, coefficient 16 — unchanged. (Contract §7 forbids tag moves before ARC-3 regardless.)
- **MC-T4.3 remains a `[FOUNDATIONAL OBSTRUCTION]`**, now with a sharpened boundary: the EM coupling's algebraic grading needs an odd lemniscatic period the discrete substrate does not (yet) supply forward.

---

## 8 · Provenance & discipline notes

- **Deferred commit (owner-authorized).** The pre-reg was written and SHA-stamped (`f55c7504…`) and its design — including the even/odd G\*-degree framing *as method* — was recorded in-session **before** the analysis ran. The owner directed "skip the commit and just run with it," so the git commit+tag are deferred. For canonization, the pre-reg should be committed first (SHA `f55c7504…`) and this verdict committed separately (B-9 temporal separation), with a separately-dispatched independent review (B-10).
- **Compute, not recall.** All facts verified in `proof_determinant_grading_parity.py` (11/11), cross-checked against `scripts/constants.py` `G_STAR` (FTD-0117 guard: `G* ≈ 2.9587 ≠ ϖ ≈ 2.622` — note `ϖ` is exactly the odd-degree period §5 identifies as missing).
- **GTCA F9 (collusion bias) guarded.** The owner's hint pointed toward a positive outcome; it was tested against the locked falsifiers, not confirmed by default. The verdict is the structural finding, not the prior.

*A clean negative that maps a precise boundary of what the discrete ontology determines (CLAUDE.md goal-clause 2) — and localizes the next step (an odd lemniscatic period) exactly.*
