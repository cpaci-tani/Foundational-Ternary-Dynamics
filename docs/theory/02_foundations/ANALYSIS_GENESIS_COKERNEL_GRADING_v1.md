# ANALYSIS — Genesis-cokernel grading, construction v1: verdict **UNDERDETERMINED** (re-scope)

**Status:** **[UNDERDETERMINED — pre-registered construction attempt v1; re-scope per protocol]** (run 2026-07-02; registered 2026-07-03; LEDGER **FTD-0365**). Verdict row 3 of the frozen map (`PREREG_GENESIS_COKERNEL_GRADING_v1.md` §4). **Zero promotions** under this outcome, per the pre-registration's own §5.
**Pre-registration (frozen BEFORE this run):** [`PREREG_GENESIS_COKERNEL_GRADING_v1.md`](PREREG_GENESIS_COKERNEL_GRADING_v1.md), hash-locked at commit `b92455ac` (2026-06-27), git tag `preregister-genesis-cokernel-grading-v1`.
**Frozen instrument:** `scripts/proofs/proof_genesis_cokernel_grading.py` (v3), SHA256 `63c342fae6c122c20ce5c6a93349e33a6a8710f73a0cf304ab4983a71b585b39` — verified unchanged at run time (the construction script re-hashes it before import) and re-verified at registration.
**Construction script:** `scripts/proofs/genesis_cokernel_construction_v1.py` — deterministic (no randomness), mpmath dps 140, banned-move compliance documented in its header (import-separation statically self-checked: 0 references to the δ-carrier accessors).
**Design memo:** [`EXPLR_GENESIS_COKERNEL_GRADED_SQRT.md`](EXPLR_GENESIS_COKERNEL_GRADED_SQRT.md) (FTD-0344 arc). **Ceiling carried over verbatim:** even Outcome A would not have derived α; this run's UNDERDETERMINED moves nothing.

---

## 1 · The frozen question

Expressed in the discrete Dirac–Kähler complex of FTD-0089: does the **section-invariant arithmetic content** of the genesis lossy-merge fiber — the one object multivalued *by construction*, and the last structurally-distinct candidate carrier after FTD-0244/0326/0327/0341 — carry the master-quadratic surd `δ = √(G*(4G*−1))`? Frozen verdict map: **A** (field ℚ(G\*)(g) carries δ), **B** (g ∈ ℚ(G\*, i) *and* a genuine order-2 branch label is exhibited — a 6th forced ℤ/2, hardening the wall), **UNDERDETERMINED** (any gate failure; re-scope, never a default-B).

## 2 · The construction executed

Per the script header (provenance tags per step): the linear DK complex `K = d − δ_codiff` with chirality `γ = (−1)^p` is **reused** from FTD-0089; the genesis merge `M_disc` (threshold `|J| > K_B`, polarity `s = sign(div J)`, radial drain) **factorizes** as `M_disc = m ∘ π` with `π(J) = (ρ, u)` — the drain is radial, maps π-fibers to π-fibers, and adds no fiber structure [DERIVED from the SPEC P3 §4.1 rule, which is itself [IMPOSED]]; the Moore–Penrose/Hodge-orthogonal complement gives the fibration's **single** independent Ehresmann-curvature component `b = P_V([∇ρ, ∇u])` [DERIVED]; the branch involution `ι: J → −J` exchanges the `s = ±1` merge fibers and is represented on the DK grade observables exactly as `γ = (−1)^p`, with `b` γ-odd [DERIVED; verified numerically in-run]. Both divergence conventions were constructed: **engine-canonical central** stencil (`field_operators.h`, `w_c = 0`) and **DK-canonical backward** stencil (FTD-0089 codifferential, `w_c = +1`). Fiber-content invariants at admissible sections of the reference fiber (ρ = 1, u = 1, branch s = +1): the γ-even `|P_V b|²` and the γ-odd (graded) `q = ⟨P_V b, ∇T⟩`, `T` the DK pseudoscalar.

## 3 · Gate results (frozen gates, PREREG §3)

| Gate | Result | Content |
|---|---|---|
| G1 (discriminator validity) | **PASS** | 17/17 self-tests + dps-band check; instrument hash-verified before import |
| G2 (single non-degenerate g) | **FAIL** | Engine-central stencil: the fibration is **flat** (`w_c = 0` ⇒ `b ≡ 0` identically — the polarity signal never reads the center voxel). The unique section-invariant value is the **null grading g₀ = 0**; per the gate's frozen text, a null grading is *NOT Outcome B* — UNDERDETERMINED |
| G3 (section-invariance) | **FAIL** | DK-backward stencil: on one fiber, even content = {6/5, 0, 6/5} and graded content q = {0, 0, 4/15} at sections SB1/SB2/SB3 (exact rationals, confirmed against the closed form `3(3−a²)/(6−a²)`, `a = ĉ·𝟙` free on the fiber). SB1 and SB3 have *equal* even content but *different* graded content — the chirality-graded part is maximally section-dependent. No machine-zero agreement |
| G4 (dps-band reproducibility) | **PASS\*** | The G2/G3 outcomes themselves are unanimous across dps {100, 120, 140} (\*no verdict-bearing g exists to band-classify) |
| G5 (well-posedness of the separation premise) | **FAIL** | The §2.3 premise — fiber structure constants are rational functions of G\* — is **not realized** by the actual SPEC/engine genesis rule: every computed invariant is an **exact rational** in section data; the only non-rational scale in the rule is the import `K_B`. **No G\* enters the genesis rule's fiber content; a fortiori no δ** |

**VERDICT (frozen map, §4 row 3): UNDERDETERMINED.** Any gate failure ⇒ the run does not count as A or B (re-scope), not a positive verdict. UNDERDETERMINED was the registered prior-dominant outcome (~45%).

## 4 · The substantive finding

What the failed gates *say*, beyond the formal verdict: the genesis rule's information-loss fiber is **arithmetically rational**. The rule reads exactly two scalars (ρ, u) built from ±1-weighted stencil sums and a Euclidean norm; no K-BIND ℚ(G\*)-valued operator (Watson scaling `G*²/2π`, det_ζ ratio `G*`) enters it, the FTD-0323 half-derivative branch sign is quarantined by construction (banned move 2), and the rule contains no fractional-order operator. The registered construction imagined ℚ(G\*)-valued structure constants; the actual rule delivers ℚ plus the single imported scale `K_B`.

**Informal reading (explicitly non-verdict):** the last structurally-distinct δ-carrier candidate shows no δ-content under the registered construction — consistent with the one-door corridor (FTD-0353 §8 / FTD-0357 / FTD-0358). **Formally this is NOT Outcome B and the wall is NOT hardened by this run:** no section-invariant g with a genuine exhibited order-2 branch label exists here (G2/G3), which is precisely the degenerate case the pre-registration's gates were written to keep out of a B reading. The FC-W boundary rests exactly where FTD-0244/0314/0326/0327/0341 left it.

## 5 · Quarantined diagnostics (documented for gate semantics; verdict-inert per §7 banned move 5)

1. `classify_grading(0)` returns `('B', 'in Q(G*) (degree 1, field)')` — the naive instrument reading that the G2 override exists to block: the instrument sees one number and cannot see branch content; a null grading masquerading as wall-hardening is the exact failure the gate text pre-empted.
2. Invariantizing by **averaging over the fiber** requires an *imported* regularization of the infinite disintegration measure (the ceiling's named import — a chosen prior/base measure). Under the regularized-uniform marginal the even content averages to `3 − 3·ln(1+√2)/√2 = 1.13032427957930845981…` (closed form; quad cross-check agrees to ~10⁻¹⁰⁰), which the frozen discriminator classifies **UNDERDETERMINED — outside ℚ(G\*, i, δ)** (a log-period, `ln(1+√2) = arcsinh 1`). Even the imported invariantization reaches neither ℚ(G\*) nor δ.

## 6 · Re-scope (what this run settles about any v2)

Per §4 row 3 the re-scope is a **decision point, not a coincidence hunt**. v1 settles: (a) the G5 separation premise *fails for the actual rule* — a v2 would need a well-posed separation of the nonlinear information-loss content from the forced-cyclotomic linear-harmonic data, which v1 shows cannot be had from the rule's own (ρ, u) reading; (b) any invariantization over the fiber imports a measure (§5.2), which the ceiling already prices as a choice; (c) the honest terminus otherwise: the fiber of *this* rule is rational-valued, and the cokernel-crack line closes as re-scoped-out. Minting a v2 pre-registration is an owner decision; nothing in this outcome obligates one.

## 7 · Standing invariants (unchanged, per PREREG §5)

`x₊ = 1/α` **[STRONGLY MOTIVATED CONJECTURE]** (FTD-0013); MC-T4.3 **[FOUNDATIONAL OBSTRUCTION]**; FC-W adopted **[AXIOM]**-class (FTD-0315); FTD-0244/0314/0326/0327 untouched; **no α derived anywhere**; golden gate untouched (pure number theory, no engine state).

## 8 · Reproduction

`python scripts/proofs/genesis_cokernel_construction_v1.py` — aborts unless the frozen instrument's SHA256 matches the lock; deterministic; ~1 min. Re-run at registration (2026-07-03): identical gate outcomes and verdict.
