# AUDIT — Route B / B1: FTD Substrate Algebra Type — CLOSED-NEGATIVE (type I)

**Tag:** `[CLOSED NEGATIVE]` (pre-reg §6: type I → no canonical modular flow). **No spine claim moved.**
**Date:** 2026-05-29
**Result of:** `PREREG_MODULAR_TIME_ALGEBRA_TYPE_v1.md` (B1, FTD-0225 provisional), SHA256 `f8a3e960c400863677e631abba898e13d73ef64023e9da9ea51fe088b63606e5` (commit deferred; SHA recorded in-session before the attempt).
**Verifier:** [`scripts/proofs/proof_modular_time_algebra_type.py`](../../../scripts/proofs/proof_modular_time_algebra_type.py) (4/4).
**Companion:** `SCOPE_ROUTE_B_MODULAR_TIME.md`. **Unifies with:** `AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md` (FTD-0208).

---

## 0 · Executive summary

**Verdict: CLOSED-NEGATIVE — the FTD substrate algebra is type I.** Route B cannot derive objective time *at the substrate level*; it reverts to Route A. But the closure is illuminating: **B1's wall is the *same* wall as FTD-0208**, now diagnosed from the operator-algebra side as **commutativity**.

The argument is short and robust (state-independent):

1. **The substrate algebra is commutative.** The fields `s ∈ {−1,0,+1}`, flux `J`, and wave-velocity `v` are classical real-valued fields advanced by a deterministic leapfrog + classical Langevin noise. Observables are functions of *commuting* fields → an **abelian** von Neumann algebra. (The leapfrog phase space carries a classical *symplectic/Poisson* structure — the geometric `J` — but the observable *product* is commutative: classical mechanics, **not** a CCR algebra.)
2. **Abelian ⟹ trivial modular flow.** A faithful normal state on an abelian von Neumann algebra is automatically **tracial**, and a tracial state has **trivial** modular automorphism group (`σ_t = id`). `[Tomita–Takesaki, textbook]` So there is **no** canonical (type-III₁) modular flow → **no objective emergent time**. The algebra is **type I**.
3. **Masslessness is moot.** The Phase-G gaplessness that the scope flagged as the III₁ driver only matters for a *quantum* field; for the *commutative* substrate it yields a continuous *classical* spectrum but **commutativity caps the type at I**. The Araki–Woods ratio-set computation (pre-reg §7 step 4) never gets to run — the algebra isn't even non-commutative.

**Verified core fact** (script): the modular flow `σ_t(x) = ρ^{it} x ρ^{−it}` is **trivial on the commutative (diagonal) part** of any observable (`‖σ_t(x)−x‖ = 1.2×10⁻¹⁶`) and non-trivial **only** on the non-commutative (off-diagonal) part (it phase-rotates the matrix elements by `(pᵢ/pⱼ)^{it}`). So **"commutative ⟺ trivial modular flow."** FTD's commutative substrate → trivial flow → type I.

---

## 1 · The unification — B1's wall = FTD-0208's wall

The missing ingredient for a non-trivial canonical modular flow is **non-commutativity** — a CCR algebra `[q, p] = i`, i.e. **quantization**. That is *exactly* the inner-product / `L²` structure FTD-0208 proved the discrete substrate lacks (the substrate's `{−1,0,+1}^Λ` has no inner product; its update law is `L¹`/`L∞`, not `L²`).

> **FTD-0208 (can't derive the relativistic `L²` law) and B1 (can't get type III₁ modular time) are one obstruction: the substrate is commutative/classical.** The `L²` law is the signature of a complex/unitary (non-commutative) structure; type III₁ requires non-commutativity; FTD's substrate has neither. Route B does **not** escape FTD-0208 — it re-derives it from the operator-algebra side.

This is the genuine payoff: two scattered obstructions collapse to a single, sharp diagnosis.

---

## 2 · What would be required — and where it survives

To reach type III₁ (canonical modular time) you must **quantize the flux sector** (`[J(x), π(y)] = iδ`). That is:
- **not** in FTD's five postulates (it would be an added axiom), and
- the act of *importing* the non-commutative/quantum structure — i.e. exactly what FTD's "**derive QM from a classical substrate**" program (`DERIV_QM_FROM_LATTICE.md`) is trying to *avoid* by deriving it.

So Route B's viability is **gated on the open derive-QM gap**. The honest map:

- **Substrate level:** `[CLOSED NEGATIVE]` (this verdict). Commutative → type I → no modular time. Reverts to Route A (import the metric/quantum structure).
- **Emergent level (the only survivor):** *if* FTD's derive-QM program succeeds in producing an effective **non-commutative** algebra at the coarse-grained level, *that* emergent algebra could be type III₁ and Route B could live there. But this is **contingent on the open derive-QM gap** and is a **different question** (call it B1′) than the substrate-level B1 closed here.

---

## 3 · Falsifier checklist (pre-reg §5) — all clean

| F | Fires? | Why |
|---|---|---|
| BF1 (assumed answer) | no | derived from the substrate's commutativity, not from QFT analogy |
| BF2 (inserted Hamiltonian/state) | no | **verdict is state-independent** — abelian ⟹ trivial flow for *every* faithful state; no ρ chosen |
| BF3 (continuum smuggling) | no | argument holds at fixed spacing, any extent; no `ε→0` |
| BF4 (Lorentz assumption) | no | none used |
| BF5 (ratio-set fitting) | no | ratio set never computed — commutativity closes it upstream |
| BF6 (CODATA) | no | none |

The verdict is the structural commutativity finding (pre-reg §6 CLOSED-NEGATIVE: "type is I or II … revert to Route A").

---

## 4 · Honest accounting

- **Prior-favoured outcome was III₁** (via masslessness). It is *defeated upstream*: the masslessness→III₁ intuition silently assumed a **quantum** field; the classical substrate is commutative, so it's type I regardless of gaplessness. The scope memo's optimism is corrected here.
- **This is not over-hardening.** "Abelian von Neumann algebra ⟹ trivial modular automorphism group" is a textbook theorem, verified in the script (the commutative observable is fixed by `σ_t` to machine precision). Unlike a structural-resemblance claim, this is dispositive and state-independent.
- **Spine untouched.** `x₊=1/α` (FTD-0013) `[SMC]`; no tag moves. MC-T4.3 / the L²-time problem remain `[OPEN/FOUNDATIONAL OBSTRUCTION]`, now with a unified diagnosis.

**Bottom line:** Route B, at the substrate level, closes negative for the same reason FTD-0208 did — **the substrate is classical/commutative**. Objective time (modular/thermal) and the relativistic `L²` law both require the non-commutativity (quantization) the substrate doesn't have. The boundary is now mapped precisely: *FTD cannot derive relativistic time from the classical substrate; it must either import it (Route A) or first close the derive-QM gap and pursue Route B at the emergent level (B1′).*

---

## 5 · Provenance & discipline

Deferred commit (owner pattern); pre-reg SHA `f8a3e960…` recorded in-session before the analysis. Facts in `proof_modular_time_algebra_type.py` (4/4); no CODATA, no assumed Lorentz, no inserted dynamics, no continuum limit. GTCA note: the verdict is the prior-*dis*favoured one and is delivered without softening — and, having over-hardened a verdict earlier this session, I re-checked: this CLOSED-NEGATIVE rests on a textbook theorem (abelian ⟹ trivial modular flow), not a resemblance argument, so it is robust.
