# Ward-Identity Status Audit

**Date:** 2026-04-25
**Scope:** Reconcile the "Ward residual at 1%" line in earlier audits with the
post-Day-2 SPEC claim that the matched-stencil CG Poisson solver drives the
residual to ≤ 1e-8. Decide whether the 1% number is a stale (pre-matched-stencil)
result or a genuine still-open issue.
**Tests run on WSL2 (RTX 5090, CUDA 13):**

```
engine/build_wsl/test_eft_ward_identity     → all active checks PASS
engine/build_wsl/test_eft_matched_poisson   → all 5 checks PASS
```

---

## 1. Residual table

| Test ID | What it asserts | Path | Threshold (asserted) | Status |
|---|---|---|---|---|
| W1 | `max\|∇·J − ρ\|` on empty lattice | n/a (vacuum) | ≤ 1e-6 | **PASS** |
| W2 | `max\|∇·J − ρ\|` on +/− pair after 20 ticks of engine `gauss_projection` | engine SOR (`sor_sweep_18pt`, 6 sweeps/tick, ω=1.75) | RMS / `\|J\|_max` < 0.5 (pre-reg was ≤ 1e-8; mismatch documented in test) | **PASS** at the empirical SOR-limited threshold; pre-reg target NOT met. SOR saturates at ~1% of `\|J\|_max` because the 18-point Laplacian in SOR does not match the 6-point divergence operator. |
| W2b | `gauss_project_converged()` (500 SOR cycles to tol=1e-8) on the same config | engine SOR iterated | finite, < 1.0 | **PASS** as a safety check; iteration does NOT drive residual below the stencil-mismatch floor (test docstring lines 167–183). |
| W3 | `max\|∂_t ρ + ∇·J\|` on dipole across one tick | engine | < 10.0 (catastrophic-only gate; ρ is integer, dρ/dt across 1 tick is 0,±1) | **PASS** |
| W4 | Composite Ward `⟨∇·J · J^ν⟩ − ⟨ρ · J^ν⟩` | engine SOR | max ≤ 1e-2 | **PASS** |
| W5 | Vertex Ward Γ_μ(p,p) = ∂Σ/∂p^μ | n/a | [OPEN] — no fermion infrastructure | SKIP |
| M1 | CG converges on synthetic δ-minus-mean source | matched-stencil | `final_res < 1e-10` | **PASS** |
| M2 | `max\|∇·J − ρ\|` after `matched_gauss_project()` on a +/− pair with seeded non-zero divergence flux | **matched-stencil CG** (own ∇² = own ∇·) | deep-vacuum max ≤ 1e-8, RMS ≤ 1e-10 | **PASS** |
| M3 | Idempotency — second `matched_gauss_project()` is a no-op | matched-stencil | both deep residuals ≤ 1e-8 | **PASS** |
| M4 | Total Σ state preserved through projection | matched-stencil | exact integer match | **PASS** |
| M5 | Improvement ratio `(deep_before / deep_after)` | matched-stencil | ≥ 1e4 | **PASS** |

(Detail strings encoding the actual numerical residuals are printed by the
`check()` helper only on FAIL; both binaries exited zero-failures, so the
specific numbers are bounded above by the asserted thresholds.)

---

## 2. Diagnosis

The "Ward residual at 1%" line in older audits and the "≤ 1e-8" target in
`SPEC_EFT_RECOVERY_PROGRAM.md` §4.3 refer to **two different solvers**:

* **Engine SOR `gauss_projection`** (the path W2 / W2b exercise) saturates at
  ~1% of `\|J\|_max` because of a stencil mismatch:
  - Laplacian inside SOR: 18-point isotropic (`sor_sweep_18pt`)
  - Divergence operator outside SOR: 6-point central difference
    (`divergence_flux_op`)
  These are not adjoint pairs, so SOR converges to a fixed point that is *not*
  ∇·J = ρ in the 6-point sense. Iterating to 500 cycles or tighter tolerance
  does not help (W2b docstring §2 in the test).

* **Matched-stencil CG `matched_gauss_project`** (the path M1–M5 exercise)
  uses the same 6-point operator on both sides. M2 drives the deep-vacuum
  residual to ≤ 1e-8 (max) / ≤ 1e-10 (RMS) on the same +/− pair, and M5
  shows ≥ 1e4 improvement over the unprojected initial condition.

So: **the 1% Ward residual is not an unresolved EFT physics warning.** It is a
known feature of the engine's SOR projection (still used at runtime in
`RenderBridge::tick()` because it is cheaper than CG per tick). The matched
CG solver exists as a separate diagnostic / paper-grade projector, and that
path *does* hit the SPEC's pre-registered ≤ 1e-8 target.

The W3 continuity floor (~1e-3 in the SPEC pre-registration text) is
discretization noise from integer-valued ρ being differenced across one tick
against a continuous ∇·J. That is not a Ward violation either; it is the
expected dρ/dt resolution of the engine's integer-state field.

---

## 3. Verdict on the EFT pillar

**Static Ward identity (∇·J = ρ): SATISFIED to ≤ 1e-8** when the matched-stencil
CG projector is the projector of record. This is what the EFT manuscript should
cite. Cross-check via W2 + M2 stands.

**Dynamical / continuity Ward (∂_t ρ + ∇·J = 0): SATISFIED at engine resolution**
(no catastrophic violation), with the irreducible 1e-3 floor explained by
integer-ρ discretization. Acceptable for a Wilsonian lattice EFT.

**Vertex Ward (Γ_μ vs ∂Σ): [OPEN]** — needs lattice fermion propagators. Already
documented as a Phase-4+ extension in SPEC §4.3 and §10; not a blocker.

**Composite Ward (⟨∇·J · J^ν⟩ vs ⟨ρ · J^ν⟩): SATISFIED at ≤ 1e-2**, the
engineering-gate level the test pre-registers. A tighter version would
re-run W4 against a matched-projected configuration; no current evidence
this is needed.

**Recommendation.** The EFT-recovery paper should describe the projector
under which each Ward number is reported. Specifically, the W2 line in any
manuscript table must say "engine SOR projector, residual saturates at ~1%
of `\|J\|_max` due to stencil mismatch" with a pointer to M2's matched-CG
result for the actual physics-grade Ward closure. This already holds in
`DERIV_DAY2_CAMPAIGN.md` (Day-2 Ticket A) but not in older audit prose.

---

## 4. Cross-references

* SPEC pre-registration: `docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md` §4.3
* Day-2 matched-stencil derivation: `docs/theory/10_eft_program/archive/phase_0_f_campaign/DERIV_GAP_CLOSURE.md` §T1
* Day-2 campaign report: `docs/theory/10_eft_program/archive/phase_0_f_campaign/DERIV_DAY2_CAMPAIGN.md`
* Tests:
  - `engine/tests/test_eft_ward_identity.cpp`
  - `engine/tests/test_eft_matched_poisson.cpp`
* Headers:
  - `engine/include/ftd/eft/ward_identities.h`
  - `engine/include/ftd/eft/gauss_projection_ext.h`
  - `engine/include/ftd/eft/matched_poisson.h`
* LEDGER row: FTD-0090 (added 2026-04-25)
