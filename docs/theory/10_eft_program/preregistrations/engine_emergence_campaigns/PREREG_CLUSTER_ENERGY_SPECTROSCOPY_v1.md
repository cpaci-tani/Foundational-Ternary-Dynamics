# PREREG — Cluster energy spectroscopy: mass as flux-energy in flip-quanta (FTD-0273)

**Status:** `[PRE-REGISTERED]` — verdict logic frozen before the run of record.
**Date:** 2026-06-11. **LEDGER id:** FTD-0273. **Prior-favoured outcome:** BOUNDARY
(the energy reframe collapses to the existing voxel-count law FTD-0110/0269).

## 1. Question (owner's reframe)

Separate **flux** from **manifestation**: flux `J` is a continuous field that only
carries energy (`½|J|²`); manifestation (a void voxel flipping to `±1`) is a per-voxel
**threshold** event (`|J| > K_GENESIS`). Define the single-voxel **flip quantum**

```
ε ≡ ½·K_GENESIS²,   K_GENESIS = K_MANIFEST·N_C = 0.511·3 = 1.533,   ε ≈ 1.1750
```

and re-express a stable cluster's **mass** as its settled flux energy *above the vacuum
floor*, in ε-units — a dimensionless quanta count **decoupled from `m_e`**. Two questions:

- **Q1 (mass):** does the flux-energy-in-quanta reveal a per-particle energy/threshold
  spectrum, or does it just track the voxel count `N` (i.e. collapse to FTD-0110/0269
  `N(A) ≈ k·A²`)?
- **Q2 (quarks):** quantize a 3-colour "quark" with voxels — what colour phenomena does
  it present (confinement, colour-singlet binding)?

## 2. Hard guardrails (load-bearing)

- **No `m_e` anchor / no tuning to 511.** No parameter (ε, K_GENESIS, A_min, settle,
  window, L, R_local, flood_frac) may be tuned toward `m_e = 0.511` or any target mass.
  The "minimum stable cluster" `A_min` is defined by **stability/geometry only** (smallest
  injection amplitude whose settled cluster is BOUNDED and survives the survival window).
- **Circularity hard-separation.** The frozen O_h geometric seeds (`genesis=false`:
  octahedron 7, cuboctahedron 13, stella 9, moore 27) have read-back energy = injected
  energy ⇒ **circular** ⇒ used ONLY as a ledger-read control, **never** quoted as a mass.
  The only non-circular number is the **emergent attractor** (genesis grows the cluster).
- **First-order failure modes reported, not hidden** (FTD-0272): every emergent row is
  classified BOUNDED / FLOODED / EVAPORATED / UNSTABLE; only BOUNDED rows get a quoted mass.
- **Determinism gate first.** No energy number is trusted until the langevin-OFF harness
  passes the Phase-0 determinism gate (bit-identical per `(A,seed)`, omp1 == pool).

## 3. Frozen artifacts (SHA-256, run-of-record code)

```
campaign_determinism_gate.cpp            4e71012bb6ffb384b4659cd2d6789ab22c64e3694245b7ca597d6b10c9565d6b
campaign_cluster_energy_spectroscopy.cpp 71906c85b7c5ea1c71ab3c7c6500256641aa8707082efe08ed52426778bffe99
campaign_quark_quantization.cpp          1926ebf908d0b9283582203fb2a3b44de48e6d61112d214db9f4b12badaa6826
analyze_determinism_gate.py              a671d3598cb2950b40ea9753f1d74638dfe41268011188bf088224000b6ac924
analyze_cluster_energy_spectroscopy.py   b01b636bcbfeb1d8c3846d634c4c68fd7ba04bcfe0bc0b6aeacb3a20dc2375d2
analyze_quark_quantization.py            620b8bf69c62772ad297764e9aba05190734779a4e9b515ac53fd38a7451bb82
```

The **verdict logic lives in the analyzers** and is frozen by these SHAs *before* the
run of record. (The C++ campaigns were developed iteratively via pilots `pilot/pilot2/
pilot3/qq…`; that is design iteration. The adjudication criteria below were fixed in the
analyzer code at the SHAs above before the `v1` run.)

## 4. Run of record

- **Phase 0 gate:** `campaign_determinism_gate --L=24 --As=2,10 --seeds=2 --repeats=8`
  (both `OMP_NUM_THREADS=1` and full pool). MUST print `DETERMINISM: PASS`.
- **Phase 1:** `campaign_cluster_energy_spectroscopy --Ls=24,32
  --As=2,4,6,8,10,12,14,16,20,28,40 --seeds=5 --settle=300 --survive=150 --tag=v1`.
- **Phase 2:** `campaign_quark_quantization --L=28 --settle=200 --window=50
  --rs=2,3,4,5,6,7,8,9,10,11,12,14 --tag=v1`.

Harness (Phases 0/1): `force_cpu`, `set_sor_iterations(150)`, fresh `RenderBridge` per
measurement, `disable_all()` then only `{wave_propagation, gauss_projection, genesis}`,
`langevin=false`, `dual_substrate=false`. Mass proxy = time-averaged flux energy over the
window (the undamped field SLOSHES flux↔wave, so the cluster identity N is the stable
observable, not the instantaneous field energy). Cluster-local energy = `½Σ|J|²` over the
manifested cluster + Chebyshev-radius-2 shell.

## 5. Three-outcome verdict map (frozen)

**Q1 (mass):**
- **ENERGY-COLLAPSES-TO-N** — cluster-local quanta-per-voxel is ~constant (CV < 0.5, no
  per-particle plateaus/jumps) AND whole-lattice flux CV > 1.5× the voxel-count CV. The
  flux-energy mass carries no information beyond `N`; a clean BOUNDARY confirming
  FTD-0110/0269. `[MEASURED — BOUNDARY]`.
- **ENERGY-STRUCTURE** — quanta-per-voxel shows seed-robust, A-correlated structure
  distinct from `N`. Would justify a follow-up pre-registration. (Not a derivation.)
- **INCONCLUSIVE** — neither cleanly holds (e.g. most rows FLOODED/EVAPORATED, or no
  bounded window at L=32). Reported as an honest boundary.

**Q2 (quarks):**
- **COLOR-PHENOMENA-IMPOSED** — the engine exhibits a 3-regime confinement force and
  geometric triad binding, but as built-in toggle rules; the quarks are seeded fixed
  structures that do not emerge as bound clusters from substrate dynamics. `[MEASURED]`.

## 6. What this is and is not

This re-expresses the FTD-0110/0269 cluster law in flip-quantum energy units. It is **not**
a mass derivation under any outcome. Highest claim ceiling: `[MEASURED — BOUNDARY]` /
`[EMERGENT]`. **Nothing is promoted:** FTD-0013 stays `[STRONGLY MOTIVATED CONJECTURE]`,
MC-T4.3 stays a foundational obstruction, FTD-0110/0261/0269/0272 unchanged. Next free
LEDGER id after this: **FTD-0274**.
