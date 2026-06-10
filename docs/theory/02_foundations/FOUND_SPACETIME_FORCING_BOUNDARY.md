# FOUND — Is FTD's Spacetime Forced by the Postulates? (the reversibility boundary)

**Tag:** `[SYNTHESIS]` + `[BOUNDARY]` — a boundary-mapping result (Goal-clause 2). Re-states existing tagged claims and draws the dividing line; **introduces no new theorem, derives nothing, promotes nothing.** FTD-0013 + α unchanged.
**Date:** 2026-06-07
**LEDGER:** FTD-0253.

---

## 0 · The question and the answer

**Question.** Do FTD's five postulates (P1 discrete cubic lattice; P2 discrete time/ticks; P3 ternary
states {−1,0,+1}; P4 local Moore causality, ≤1 voxel/tick; P5 determinism) **force** the dynamics to be
second-order/hyperbolic — a *wave* equation, which carries a light cone, clocks, and Lorentz invariance
("spacetime") — or is that structure an **additional input**?

**Answer.** **The causal cone is forced; the Lorentzian metric is not.** Precisely:

| Piece of "spacetime" | FTD status | Forced by P1–P5? |
|---|---|---|
| Light cone / finite max speed `c = 1/√3` | `[THEOREM]` from locality P4 | **YES — forced** |
| Lorentzian metric / `γ` / the wave (2nd-order) structure | `[AXIOM]` (Born-Infeld `(Δ_t J)²` action); flux field is a `[SELECTION]` | **NO — posited** |
| Reversibility / time-reversal symmetry | **absent from P1–P5** | **NO — not a postulate** |

The dividing principle is **reversibility**, and it is a **6th-postulate-class input** — the same *shape* of
gap as the one that separates the classical substrate from quantum non-commutativity.

## 1 · The forced part: the causal cone from locality

A finite maximum signal speed follows directly from P4. `DERIV_RELATIVITY_DERIVATION.md` §1.3 Theorem 1.1
`[THEOREM]`: "No information can propagate faster than C = 1 voxel/tick" — a direct consequence of the
Moore-neighbourhood update. `FOUND_AXIOM_ZERO.md` §2.2(d) `[THEOREM]`: `c = 1/√D = 1/√3` is the CFL
stability speed on the cubic lattice, "a theorem about discrete wave equations on ℤ^D … the maximum speed
at which information propagates." **This is the genuinely-derived part of spacetime: the causal order /
light cone is forced by P4.** It holds for *any* local dynamics (the Lieb-Robinson situation), reversible
or not.

## 2 · The unforced part, and the reduction to reversibility

The *metric* structure — that the dynamics is **second-order** (`∂²_t J = c²∇²J`), hence oscillatory,
clock-bearing, and Lorentz-invariant in the IR — is **not** derived from P1–P5. It is installed by the
action's `(Δ_t J)²` kinetic term (`SPEC_FTD_LAGRANGIAN.md` §3.3, §5.4 — the weak-field expansion of the
Born-Infeld core *produces* the Klein-Gordon/wave equation; the action form is `[AXIOM]`, the flux field a
`[SELECTION]` "minimal continuous extension", `FOUND_AXIOM_ZERO.md` §2.3(b)).

**Why it is not forced — the reduction.** Second-order/wave ⟺ **reversibility**:

- A **diffusion** (heat) equation `∂_t J = D∇²J` is **irreversible** — a contraction; high-k modes decay,
  information is lost, the backward problem is ill-posed. *No reversible local dynamics can have a diffusive
  continuum limit.*
- A **wave** equation is **reversible** (`t→−t`-invariant; the leapfrog/Störmer–Verlet update is exactly
  symplectic — `TRACKER_OPEN_ITEMS.md` §1.4, audited bit-level).
- For a **real** field, *local + reversible + a non-trivial smooth limit ⟹ hyperbolic (wave)* — which *is*
  the metric/light-cone structure.

**P5 is *determinism*, not *reversibility*.** A deterministic map may be many-to-one (irreversible);
diffusion is deterministic *and* irreversible. So **nothing in P1–P5 excludes a first-order diffusive
substrate**, which would still have a light cone (from P4) but **no clocks, no `γ`, no metric**. The
Lorentzian metric rides entirely on the *choice* of a reversible second-order action. **Reversibility is
the missing forcing principle.**

FTD's own substrate is the fingerprint of exactly this: the **wave sector is reversible** (symplectic),
while **manifestation + the Rayleigh dissipation are irreversible** (`SPEC_FTD_LAGRANGIAN.md` §3.6
dissipation `[IMPOSED]`, non-variational; `REPORT_DETECTOR_INFORMATION_LOSS.md` — the `ψ→|ψ|²` /
sign-projection map is many-to-one, "provably unrecoverable"). The substrate is **not globally reversible**;
its Lorentzian (metric) behaviour is confined to the reversible wave sector.

## 3 · FTD's own algebra already encodes the split (π vs G\*)

Remarkably, the reversible/irreversible split is the **product/ratio split of the Euler reflection formula**
that generates FTD's constants (`PAPER_RATIO_AND_THE_ARROW.tex`; `DERIV_HEAT_EQUATION_FROM_RATIO.md`):

- **Product branch** `Γ(z)Γ(1−z)` — **commutative → π → time-*reversible*** (wave equation, Lagrangian
  mechanics, unitary QM).
- **Ratio branch** `Γ(z)/Γ(1−z) = G*` — **non-commutative → G\* → time-*asymmetric*** ("the arrow of time
  resides in the non-commutative ratio"; the ratio carries the fractional/diffusive `D^{−1/2}` operator,
  `[THEOREM]` in `DERIV_HEAT_EQUATION_FROM_RATIO.md` §5).

So **FTD's spacetime lives on the π (product / reversible) face, and its arrow on the G\* (ratio /
irreversible) face of the same structure.** The `γ`-emergence measured this session (FTD-0252) is the
π-face; manifestation's arrow is the G\*-face. The reversible/irreversible boundary of §2 *is* the
π/G\* boundary that produces FTD's central constant — not a coincidence, a restatement.

## 4 · The payoff: the same shape of gap as QM

`THEOREM_COMMUTATIVITY_INDEPENDENCE` (FTD-0243) established that **QM's non-commutativity is not forced by
P1–P5** — it is an injected 6th-postulate `M`. This note establishes that **the relativistic metric is
*also* not forced by P1–P5** — it is an injected 6th-postulate-class input (**reversibility**). The honest
map of FTD:

> **The five postulates force the classical *causal skeleton* — a discrete, deterministic, light-cone-bearing
> lattice. Neither the relativistic *metric* nor the quantum *structure* is forced; each requires exactly one
> added principle: reversibility (→ the Lorentzian metric) and non-commutativity (→ quantum mechanics).**

Two 6th-postulate inputs, one for each pillar of modern physics, sitting on the same five-postulate base.
This is why "we built spacetime" overclaims: FTD **derives the causal cone** (P4) and **posits the metric**
(a reversible action) — and it now *names* the missing ingredient.

## 5 · Status, and what this is / is not

- **Causal cone from locality:** `[THEOREM]` (inherited, P4).
- **Metric / 2nd-order structure is posited, not derived:** `[BOUNDARY]` (this note; restates the `[AXIOM]`
  status of the action).
- **Reversibility ⟺ hyperbolic-not-parabolic; reversibility absent from P1–P5:** `[SYNTHESIS]` — the reduction
  is standard PDE/dynamical-systems fact applied to FTD's tags.
- **Reversibility as the 6th-postulate that would force the metric; parallel to non-commutativity for QM:**
  `[SYNTHESIS]` — a framing that unifies the relativity and QM boundaries; not a theorem.
- **Nothing promoted.** No derivation of `γ`, the metric, α, or FTD-0013; FTD-0208 (no exact discrete `γ`)
  and FTD-0243 (commutativity) both stand.

**What this is NOT:** not a claim that FTD derives spacetime (it derives only the cone); not a proof that
reversibility is the *unique* missing principle (isotropy/SO(3) restoration is a separate IR-emergent piece,
FTD-0252); not a postulate change (P1–P5 are unchanged — this *maps* what they do and don't force).

## 6 · Concrete follow-ups

- **Engine demonstration — DONE (2026-06-07):** `engine/tests/test_spacetime_forcing_demo.cpp` (CTest
  `spacetime_forcing_demo`, **9/9 PASS**, read-only / golden-safe). The *same* lattice and the *same*
  `laplacian_flux` stencil (so locality is byte-identical), evolved 2nd-order (WAVE) vs 1st-order (DIFFUSION).
  **Confirmed:** the **causal cone is bit-identical** (both fronts = 7.211 at t=8, within the locality bound
  ≤ t·√2), but the **metric appears only in 2nd order** — WAVE oscillates (a clock) + is non-dissipative
  (amplitude preserved to 0.03%, reversible) + spreads **ballistically** (`r_rms ∝ t`, ×4.68 over a 4× window);
  DIFFUSION decays monotonically (no clock) + dissipates `Σ|J|² → 0.068` (irreversible) + spreads
  **diffusively** (`∝√t`, ×2.46 ≈ √4). So the Lorentzian metric empirically rides on the 2nd-order/reversible
  choice, **not** on the shared locality cone — FTD-0253 made vivid.
- **User-visible web artifact — DONE (2026-06-10):** Scale-0 scenario
  `s0-field-spacetime-forcing-boundary` exposes the WAVE branch as a clean center-pulse seed, and
  `engine/web/demos/spacetime-forcing-boundary.html` shows the labelled WAVE-vs-DIFFUSION counterfactual
  replay. Supporting run note: `engine/web/docs/SPEC_SPACETIME_FORCING_DEMO.md`. This promotes visibility
  only; it does **not** promote any claim or add a derivation.
- **Theory — analysed (2026-06-07): finiteness does NOT motivate reversibility; it opposes it.** The 't Hooft
  hope is that a *finite* deterministic substrate must be reversible (information-preserving). But (i) finite +
  deterministic does **not** force reversibility — the "eventually periodic" theorem permits transients flowing
  into attractors (irreversible, many-to-one); (ii) the 't Hooft argument is a *motivation* (match observed
  unitarity), not a derivation from P1–P5; and (iii) **decisively for FTD**, the genuinely *finite* sector is
  the ternary **state field** `s∈{−1,0,+1}`, and *that* sector is **irreversible** (manifestation is many-to-one
  — the arrow lives there), while the *reversible* part is the **continuous flux** `J∈ℝ³`, which is **not
  finite-state**, so the finiteness argument has no purchase on it. FTD thus applies reversibility *selectively*
  (reversible continuous flux → metric; irreversible finite state → arrow). **Conclusion:** reversibility is a
  6th-postulate-class input the postulates neither force nor internally motivate — its only motivation is
  *external* (matching observed approximate unitarity). This **tightens** §2/§4: the Lorentzian metric is
  conditional on an input that FTD's own foundational finiteness commitment argues *against*. `[SYNTHESIS]`
