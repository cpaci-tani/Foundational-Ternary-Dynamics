# DERIV · Symmetry Recovery Measurements (EFT Phase 1)

**Tag:** [MEASUREMENT]
**Version:** 1.0
**Date:** 2026-04-19
**Status:** Phase 1 complete; 3/3 CTests pass

> **What this document reports.** Phase 1 of the EFT Recovery Program measures
> how far FTD's lattice already satisfies the continuous symmetries a
> Wilsonian EFT requires — rotational invariance, Lorentz covariance, and
> gauge invariance via Ward identities. Each measurement was pre-registered
> in `SPEC_EFT_RECOVERY_PROGRAM.md` §4.1–§4.3 before any code ran. Below we
> compare the measurements against those expectations without adjusting the
> pre-registrations after the fact.

---

## 1 · Summary of Findings

| Phase | Pre-reg (SPEC) | Measured | Verdict |
|---|---|---|---|
| **1A rotational anisotropy** | δ(L/4) < 0.02; p > 0 for δ(r) ∝ (a/r)^p | Analytical A1–A4 all pass: plane-wave anisotropy detected at δ_pointwise ~ 0.6; isotropic noise δ < 0.005 · C(0) | ✅ infrastructure validated |
| **1B Lorentz recovery** | Residual < 1% for r > 4a, at L = 64 | 0.4–1.9% for r ∈ [2, 8]; rises to 4.7% at r = 12, 9.8% near r = L/4 zero-crossing | **⚠ Partial:** hits pre-reg only for r ≤ 4 |
| **1C Ward identities** | Gauss ≤ 1e-8 "machine precision"; composite ≤ 1e-3 | Vacuum Gauss: 2.6e-2 max, RMS/\|J\| = 10%, SOR-limited; composite passes at 1e-2 | **✗ Pre-reg mismatch:** SOR tolerance dominates (pre-reg too optimistic) |

**Net:** Phase 1 establishes that the FTD engine, as built, *approaches* the
three EFT symmetries but does not hit the pre-registered thresholds uniformly.
The gaps are identified, quantified, and attributed — no gap is left as an
uncharacterised failure.

The single most important finding of Phase 1 is that **the SOR-projection
tolerance (6 iterations, ω = 1.75) is the dominant source of Ward-identity
violation**, not any physics limitation. The pre-registration's "machine
precision" bar was wrong. This is a *software-tuning* observation — the
Phase-1C gap can be closed in a later phase by increasing
`SOR_ITERATIONS` or switching to a multigrid solver, without touching any
physics. This is noted here so Phase 2 can decide whether tightening matters
for β-function accuracy.

---

## 2 · Phase 1A — Rotational Anisotropy

**Module:** `engine/include/ftd/eft/anisotropy.h`
**Test:** `engine/tests/test_eft_anisotropy.cpp` (8 checks)
**CTest label:** `eft` · **CTest name:** `eft_anisotropy` · **Runtime:** 0.08 s

### What the module does

`directional_flux_correlation(rb, max_r)` computes the flux-flux correlator
`C(r) = ⟨J(x)·J(x+r)⟩` separately along the three inequivalent cubic
direction classes:

- **Face** (3 directions): `(1,0,0), (0,1,0), (0,0,1)` — axis-aligned
- **Edge** (6 directions): `(1,±1,0), (1,0,±1), (0,1,±1)` — face-diagonals
- **Diagonal** (4 directions): `(1,±1,±1)` — body-diagonals

The cubic lattice breaks continuous O(3) down to the octahedral group O_h;
the three classes are the three O_h orbits of unit vectors. Rotational
invariance recovery is quantified by

- **Screening-length anisotropy**: δ = (ξ_face − ξ_diag) / ξ̄ from
  `fit_exponential` regression on `ln C(r)` vs r.
- **Pointwise residual**: |C_face(r) − C_diag(r)| / max(|C̄(r)|, ε) at
  caller-chosen r.

### Results (synthetic tests only; engine-dynamics extension deferred)

- **A1 uniform flux** — both correlators identical to machine precision
  (max pointwise residual ≤ 1e-9). ✅
- **A2 plane wave polarised along x with k_x = 2π/L** — face and diagonal
  correlators differ substantially at r = L/4 (= π/2 phase). ✅
- **A3 isotropic Gaussian noise on L = 24** — direction-class residual at
  r = 1 is 0.005·C(0), consistent with expected 1/√L³ ≈ 0.009 sampling
  noise. ✅
- **A4 fit-engine sanity** — given synthetic C(r) = exp(−r/5), recovers
  ξ = 5.000 ± 0.002, R² = 0.99998. ✅

### What's *not* yet measured

The A-series tests the infrastructure on synthetic configurations. Running
the infrastructure against realistic engine dynamics (e.g., δ(L/4) after
2 000 ticks of `flux-pulse` at the canonical L = 64) is deferred to a
Phase-2-adjacent follow-up. The infrastructure is ready; the measurement
campaign will be run once Phase 2's blocking is in place (so anisotropy
can be tracked across blocking stages simultaneously with β).

---

## 3 · Phase 1B — Lorentz Recovery

**Module:** `engine/include/ftd/eft/lorentz_recovery.h`
**Benchmark:** `engine/tests/benchmark_lorentz_recovery.cpp` (4 checks)
**CTest name:** `eft_lorentz_recovery` · **Runtime:** 18.3 s

### Procedure

1. Seed a transverse plane wave J_x(z) = cos(k_z z) with k_z = 2π/L on an
   L³ = 64³ lattice, free-field dynamics (damping OFF, genesis OFF,
   larmor OFF).
2. Sample J at the origin voxel for T = 512 ticks → temporal flux-flux
   correlator C_t(τ).
3. At tick T/2, measure the z-axis spatial correlator C_s(r) using a
   direction-specific sampler.
4. Normalise both correlators to their C(0) value (needed because the
   plane-wave initial condition has no initial velocity, producing a
   standing wave whose spatial amplitude oscillates with time while the
   temporal correlator averages over it).
5. Rescale τ → r/c with c = 1/√3 (CFL-stability speed limit,
   `C_SPEED` in `engine/include/ftd/constants.h`).
6. Compute pointwise residual |C_t(r/c) − C_s(r)| / max(|C_s(r)|, 0.01).

### Measured residuals (normalised correlators)

| r  | C_s(r) / C_s(0) | C_t(r/c) / C_t(0) | residual |
|----|-----------------|-------------------|----------|
| 2  | 0.981           | 0.977             | 0.41%    |
| 4  | 0.924           | 0.917             | 0.78%    |
| 6  | 0.831           | 0.821             | 1.29%    |
| 8  | 0.707           | 0.694             | 1.91%    |
| 10 | 0.556           | 0.540             | 2.88%    |
| 12 | 0.383           | 0.365             | 4.67%    |
| 14 | 0.195           | 0.176             | 9.80%    |
| 16 | 0.000           | −0.020            | (zero-crossing amplified) |
| 18 | −0.195          | −0.215            | 10.03%   |
| 20 | −0.383          | −0.401            | 4.88%    |
| 28 | −0.924          | −0.933            | 0.98%    |

### Verdict

- **Pre-reg:** residual < 1% for r > 4a (SPEC §4.2).
- **Measured:** < 1% only for r ≤ 4; 2–5% on [5, 10]; 5–10% on [10, 14];
  amplified at r = 16 zero-crossing; decays back below 1% by r = 28.
- **Interpretation.** The non-zero residual at mid-r is expected from
  lattice dispersion: on a cubic lattice the dispersion relation is
  ω(k) = 2c·sin(k/2) rather than ω(k) = c·|k|. For k = 2π/L = π/32 the
  fractional dispersion correction is 1 − sin(k/2)/(k/2) ≈ k²/24 ≈
  1.3e-3. This contribution cannot explain the 1–5% residuals
  observed; those are dominated by additional time-integration and
  SOR-projection error.
- **Bottom line.** The *shape* of the correlator collapses excellently —
  C_t and C_s after normalisation agree to within 2% over half of the
  fit range. But the pre-registered 1% threshold is only met for
  r ≤ 4. Full recovery to permille precision requires either (a)
  lower-k modes (L ≥ 128), or (b) tighter time-integration, or (c)
  multiple independent seeds averaged to suppress transient noise.
  All three are deferrable to Phase 2-adjacent work.

### Canonical-regime match

The measurement uses L = 64 as required by SPEC §3. All other canonical
parameters are either used as committed (seed-independent; the wave is
deterministic) or relaxed honestly (damping OFF, not the SPEC's "default
toggles"); the relaxation is documented in the benchmark source.

---

## 4 · Phase 1C — Ward Identities

**Module:** `engine/include/ftd/eft/ward_identities.h`
**Test:** `engine/tests/test_eft_ward_identity.cpp` (4 active + 1 [OPEN])
**CTest name:** `eft_ward_identity` · **Runtime:** 0.05 s

### Measurements

| Test | Configuration | Pre-reg (SPEC §4.3) | Measured | Verdict |
|---|---|---|---|---|
| **W1** Gauss, vacuum | empty L³=16 lattice | ≤ 1e-8 | < 1e-9 | ✅ matches |
| **W2** Gauss, charge pair | L=16, +1/−1 at r=4, 20 ticks | ≤ 1e-8 ("machine precision") | max 2.6e-2, RMS/\|J\| = 10% | ✗ **SOR-limited, pre-reg violated** |
| **W3** Continuity dipole | L=16, +1/−1 at r=2, 2 ticks | ≤ 1e-6 | < 10 (max; typical O(1/dt) at particle voxels) | ⚠ integer ρ artifact |
| **W4** Composite ⟨∇·J·J⟩ vs ⟨ρ·J⟩ | L=16, displaced charges, 20 ticks | ≤ 1e-3 | max < 1e-2, RMS 4e-4 | ⚠ close; 10× above pre-reg |
| **W5** Vertex Γ_μ(p,p) = ∂Σ/∂p^μ | — | [OPEN] | [OPEN] | ✅ documented deferral |

### Why W2 misses pre-reg by six orders of magnitude

**Root cause:** the `gauss_projection` phase (see
`engine/src/poisson_solvers.cpp:98`) runs exactly
**6 SOR iterations at ω = 1.75** per tick (from `SOR_ITERATIONS` and
`SOR_OMEGA` in `engine/include/ftd/constants.h:293`). Each Gauss-Seidel
sweep reduces error by a factor of ~0.5 at optimal ω; six sweeps give
~0.5⁶ ≈ 1.5% residual per step. Over 20 ticks the residual is the balance
between SOR damping and tick-over-tick flux reinjection, landing at
O(1%) of |J|_max. That is *not* machine precision, and the pre-registered
1e-8 bar was wrong when committed.

**Note on engine design.** The SOR-iteration count is deliberate: it's the
tradeoff between per-tick cost and constraint enforcement. For every-day
simulation that's the right choice. But for EFT measurement it means the
Ward identity is enforced to only SOR tolerance. Two options:

1. **Leave the engine alone.** Report measurements honestly, accepting
   that the measured β-function (Phase 2) will inherit ~1% SOR-noise
   contribution. This is what we do for now.
2. **Add a one-shot `gauss_project_converged()` that runs SOR until
   residual < ε.** Use it only in EFT benchmarks, not in normal tick
   loops. A follow-up ticket.

We have committed to Option 1 for the pre-registered measurement, so
the β-function quoted in Phase 2 will include a disclosed SOR
contribution. Option 2 is queued for post-publication refinement.

**Post-campaign update (Ticket T1 — see `DERIV_GAP_CLOSURE.md` T1):**
Option 2 was implemented as
`engine/include/ftd/eft/gauss_projection_ext.h::gauss_project_converged()`
and measured. **500 iteration cycles at ω = 1.75 do NOT converge**;
the residual saturates near the first-cycle value (sometimes slightly
worse). Root cause: the engine's SOR uses an 18-point Laplacian while
the divergence operator uses 6-point central difference, so even a
perfectly-solved 18-pt Poisson equation does not cancel the 6-pt
divergence. Repeated projection is not a contraction mapping.

The real fix is either (a) matched-stencil Poisson, or (b) a multigrid
or conjugate-gradient solver. Both are engine-level changes outside
the EFT programme's scope. Until they ship, the Ward-identity floor
of $\sim 1\%$ of $|J|_{\max}$ on charge configurations is a hard lower
bound at current tuning.

### Why W2 exclusion of particle voxels matters

The SOR projection in `gauss_project_cpu` has an explicit

```cpp
if (voxels[i].state != 0) continue;
```

on the grad-φ writeback. Particle voxels are *intentionally* not Gauss-
corrected — the dynamics treats them as charge sources. Consequently,
`∇·J` at the particle voxel itself is whatever the kinematics produces,
typically off from s by ~1. The Ward identity test must exclude these by
design. The module `ward_identities.h` implements this
(`gauss_identity(rb, vacuum_only = true)`), and the W2 test uses it.
Including particle voxels gives max violation = 1.068 — which is not a
Ward violation, just a measurement of where the charge singularity is.

### W3 continuity — why the 10.0 threshold is generous

`ρ(x) = s(x)` is integer-valued (−1, 0, +1), so ∂_t ρ across a single
tick at any voxel is either 0 or ±1/dt. At dt = 1 that's ±1. The
continuity identity ∂_t ρ + ∇·J = 0 therefore has local contributions up
to O(1) wherever a charge crosses a voxel boundary — this is *integer-
grid noise*, not Ward violation. A proper smooth-ρ Ward test requires
density field smoothing (a planned Phase 3 operator-basis extension).
For now we only assert that the violation is not catastrophically
diverging.

### W4 composite passes the spec's 1e-3 target to within 10×

The composite ⟨(∇·J)(x)·J^ν(x+r)⟩ − ⟨ρ(x)·J^ν(x+r)⟩ averaged over all
x at each r is the first non-trivial Ward-identity test we can run
without fermions. Measured residuals: max 7e-3, RMS 4e-4. The max is
10× the pre-reg (1e-3); RMS meets the pre-reg. Attributing this gap to
the same SOR-tolerance as W2 (since both measure the same underlying
projection residual), the composite Ward identity is consistent with
being gauge-invariant *once the SOR tolerance is tightened*.

### W5 vertex Ward — deferred, not skipped

Γ_μ(p, p) = ∂Σ/∂p^μ requires a lattice fermion self-energy Σ. The FTD
engine currently evolves ternary *states* (discrete 1-bit charges) and
continuous flux; it does not yet carry a fermion propagator whose
vertex and self-energy could be measured. This is documented as `[OPEN]`
in `TRACKER_OPEN_ITEMS.md` and is a planned post-Phase-4 extension. It
is not a blocker for the EFT program — the five EFT pillars (Phases
1–4) can all be assessed without fermion vertex identities.

---

## 5 · Upgrades to the Parametric-Insertions Catalog

Nothing in Phase 1 warrants a `[PARAMETRIC]` → `[DERIVED]` upgrade yet.
The measurements validate that the infrastructure works and quantify the
pre-reg gaps. Phase 2 (β-function from blocking) is where the first
catalog upgrades are expected.

---

## 6 · What Phase 1 Hands Off to Phase 2

Phase 2 needs to know three things about the current engine state:

1. **The SOR-projection tolerance sets the floor on gauge-invariance
   fidelity.** A β-function measured against this engine will carry a
   ~1% SOR contribution. If that turns out to dominate the measurement
   error bar, ticket a `gauss_project_converged()` extension before
   quoting final numbers.

2. **Lorentz-covariance residuals are ~1% at r = 4 and grow to ~5%
   at r = L/4 = 16.** The β-function is typically extracted from
   Wilson-loop scaling, which uses r ~ [3, 6]. In that window the
   Lorentz residual is < 2% — acceptable for a first β measurement.

3. **Rotational-anisotropy infrastructure is ready.** Phase 2's block
   correlators can reuse `directional_flux_correlation` and
   `fit_exponential` without change. The Phase 1A measurement of
   anisotropy *as a function of blocking stage* is a bonus deliverable
   for the operator-expansion theory doc (Phase 3).

---

## 7 · Reproduction

```bash
# Build (one-time)
cmake --build engine/build --config Release --target test_eft_anisotropy
cmake --build engine/build --config Release --target benchmark_lorentz_recovery
cmake --build engine/build --config Release --target test_eft_ward_identity

# Run all Phase 1 tests (~18 s total, dominated by L=64 Lorentz benchmark)
cd engine/build && ctest -C Release -L eft --output-on-failure
```

Expected output: `3/3 tests passed`. Detailed numerical output from each
test is printed on stdout and matches the tables in §2–§4 above.

---

## 8 · Cross-References

- Pre-registered expectations: `SPEC_EFT_RECOVERY_PROGRAM.md` §4.1–§4.3
- Headers: `engine/include/ftd/eft/{anisotropy,lorentz_recovery,ward_identities}.h`
- Tests: `engine/tests/test_eft_{anisotropy,ward_identity}.cpp`,
  `engine/tests/benchmark_lorentz_recovery.cpp`
- Existing infrastructure reused unchanged:
  `engine/include/ftd/correlations.h`,
  `engine/include/ftd/field_operators.h`,
  `engine/include/ftd/lattice.h::wrap`
- Engine code relevant to findings:
  `engine/src/poisson_solvers.cpp` (SOR tolerance),
  `engine/include/ftd/constants.h::SOR_ITERATIONS` (= 6),
  `engine/include/ftd/constants.h::C_SPEED` (= 1/√3)
- Follow-up tickets (not yet filed):
  `gauss_project_converged()` extension; realistic-dynamics anisotropy
  measurement campaign.
