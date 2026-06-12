# Analysis — Order of the FTD Genesis Transition (RG-spectrum probe)

**Tag:** `[MEASURED — BOUNDARY] = GENESIS-FIRST-ORDER`
**Date:** 2026-06-11
**LEDGER row:** FTD-0272
**Pre-registration:** `PREREG_GENESIS_CRITICALITY_v1.md`, tag `preregister-genesis-criticality-v1` (lock `c7371211`; pre-run stats amendment `05caee9a`)
**Artifacts:** `engine/tests/campaign_genesis_criticality.cpp` (FSS), `engine/tests/campaign_genesis_hysteresis.cpp` (first-order confirmation), `scripts/exploration/analyze_genesis_criticality.py`

---

## 0 · Verdict

The FTD genesis/manifestation transition is **strongly FIRST-ORDER** — a
discontinuous jump with a maximal hysteresis loop, not a critical point. There
is **no diverging correlation length, no scaling fixed point, no critical
exponents.** Therefore the genesis nonlinearity is **NOT a relevant operator in
the RG/critical-scaling sense**, and the cluster-mass ladder `N(A) ≈ ¼A²` is
**energy-budget / pattern formation on top of a first-order manifestation
transition — it is NOT an RG-flow-derived spectrum.** This closes the
"lattice spectrum via renormalization" route *through the genesis sector* as a
clean `[BOUNDARY]`.

This is the Number-One-Goal's second clause: a rigorous map of what the discrete
ontology does **not** determine. The free (flux/wave) sector is Gaussian
(spectrum = dispersion + imposed clock scale, anisotropy dying as k⁴, PL-5); the
interacting (genesis) sector is first-order (no scaling spectrum). **Neither
sector derives a mass spectrum via RG.**

## 1 · The question (narrowed)

Is the genesis transition 2nd-order CRITICAL (⇒ genesis is a *relevant* operator
with a scaling spectrum ⇒ the cluster ladder has RG content) or
FIRST-ORDER/trivial (⇒ no RG-derived spectrum)? Genesis is an absorbing-state
transition (void `s=0` = quiescent, manifestation `s=±1` = activity), driven
purely by Langevin temperature `T` (no injection). The **order** of the
transition is the verdict.

## 2 · Evidence (three independent first-order signatures)

**(a) Hysteresis loop — the gold standard.** One persistent bridge, T ramped UP
(heating from void) then DOWN (cooling, state carried). `m = N_manifested/L³`:

| L=24 | T=0.030 | 0.035 | **0.040** | 0.045 | 0.050 | ... | 0.10 → 0.0 (cooling) |
|---|---|---|---|---|---|---|---|
| **heating** (from void) | 0.000 | 0.0001 | **0.92** | 1.000 | 1.000 | 1.000 | — |
| **cooling** (from active) | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | **1.000 at every T down to T=0** |

The heating branch jumps `0.0001 → 0.92` in a single step at **T_up ≈ 0.040**;
the cooling branch stays **pinned at m=1.000 all the way to T=0** (**T_down = 0**).
The loop spans the *entire* transition region. A 2nd-order transition has **no**
loop (branches coincide).

**L=32 confirms — the loop persists and even widens** (not a finite-size
artifact, which would *shrink* with L): cooling stays pinned at **m=1.000 down
to T=0**, while heating is still **~0 up to T≈0.05** (`m=0.014` at T=0.055). The
absorbing→active jump shifts slightly *up* with L (nucleation barrier grows), the
active phase stays latched to T=0 at both sizes — the coexistence region is
robust.

**(b) Discontinuous jump.** `m` jumps from ~0 to ~1 (the WHOLE lattice manifests)
across one temperature step (ΔT = 0.005) — a discontinuity, the defining feature
of a first-order transition.

**(c) Negative Binder cumulant.** The FSS scout (L=24) gave
`U4 = 1 − ⟨m⁴⟩/(3⟨m²⟩²) = −0.54` near T_c, with a huge susceptibility peak
(`χ = N·Var(m) ≈ 844`) and seed bistability (`m = 0.20` vs `0.77`). The Binder
cumulant is bounded in `[0, ⅔]` at a *critical* point and crosses at a *positive*
universal value; it can only go **negative** under **phase coexistence** — i.e.
first-order.

All three pre-registered/auxiliary discriminators agree: **D1 P(m) bimodal /
coexistence, D2 Binder negative, hysteresis loop maximal.**

## 3 · The physics — a self-sustaining (latched) condensate

The manifested phase is **self-sustaining even at T = 0**: once the lattice fully
manifests, the manifested charges' own gauss-projected Coulomb + coupling
self-field keeps every voxel above `K_GENESIS`, and with no damping there is no
dissipation to relax it. So the active phase is an *absolutely metastable
condensate* (the "runaway genesis" regime, FTD-0107 ic2). The void→manifested
direction needs thermal noise to cross `K_GENESIS` (at `T_up ≈ 0.04`); the
reverse never happens within the thermal range. This is why the hysteresis is
maximal (`T_down = 0`). It is a *strongly* first-order transition.

## 4 · Consequence for "spectrum via RG"

A first-order transition has **no RG critical fixed point** and **no diverging
correlation length** — the hallmarks an RG-derived spectrum requires. So:

- **Genesis is RG-irrelevant as a spectrum generator.** The `N(A) ≈ ¼A²` cluster
  law (FTD-0110/0269) is a *dimensional/energy-budget* scaling (cluster volume ∝
  injected energy ∝ A²), **not** a critical-exponent scaling. It is pattern
  formation on a first-order background, not RG flow.
- Combined with the Gaussian free sector (FTD-0050 closed-negative; dispersion +
  imposed clock scale), **the FTD lattice does not derive a mass spectrum via
  renormalization** in either sector. The only RG-clean spectral result is the
  *dispersion* flowing to the Lorentz-invariant continuum (anisotropy ~k⁴, PL-5)
  — which carries no derived mass scale.

## 5 · No promotions

`[MEASURED — BOUNDARY]`. Nothing promoted: FTD-0013 `[SMC]`, MC-T4.3, FTD-0050
`[CLOSED-NEGATIVE]`, FTD-0110 (cluster law unchanged — now understood as
*not* RG-derived), FTD-0270/0271 all unchanged. This avoided the FTD-0050 trap
(it drives by temperature, not by blocking the BCC-orthogonal stencil) and gives
a definite, pre-registered verdict. The ℏ-scale and atomic-spectrum gaps
(FTD-0270) are untouched.
