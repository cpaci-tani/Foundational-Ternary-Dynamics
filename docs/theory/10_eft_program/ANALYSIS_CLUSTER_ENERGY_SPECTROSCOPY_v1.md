# ANALYSIS — Cluster energy spectroscopy: mass as flux-energy in flip-quanta (FTD-0273)

**Status:** `[MEASURED — BOUNDARY]`. **Date:** 2026-06-11. **Pre-registration:**
[`PREREG_CLUSTER_ENERGY_SPECTROSCOPY_v1.md`](preregistrations/PREREG_CLUSTER_ENERGY_SPECTROSCOPY_v1.md)
(verdict logic frozen by analyzer SHAs before the run of record). **Prior-favoured
outcome:** BOUNDARY — confirmed.

## 0. The owner's reframe and the honest answer

The owner proposed separating **flux** (a continuous field that just carries energy
`½|J|²`) from **manifestation** (a per-voxel threshold flip at `|J|>K_GENESIS`), defining
a single-voxel **flip quantum** `ε ≡ ½·K_GENESIS² ≈ 1.1755` (`K_GENESIS = K_MANIFEST·N_C =
0.511·3 = 1.533`), and re-expressing a cluster's **mass** as its flux energy above the
vacuum floor in ε-units — *decoupled from `m_e`*, with the explicit hypothesis "≈511
quanta in a cluster = an electron."

**The engine's answer: that re-expression collapses back to the voxel count `N`, and the
specific "511 quanta = electron" hypothesis is falsified.** Two decoupled results below.

## 1. Phase 0 — determinism gate (PASS; gates everything)

The earlier-observed non-reproducibility (a fixed `(A,seed)` giving N=2/3/10) was traced
to the **langevin thermostat** (`s0-seed-emergent-ic1` force-sets `langevin=true, T=0.005`
— the spread is the Brownian fingerprint), **not** an engine race. The langevin-OFF
spectroscopy harness is bit-deterministic:

- `campaign_determinism_gate --L=24 --As=2,10 --seeds=2 --repeats=8`: every `(A,seed)`
  bit-identical across 8 repeats; `omp1 == pool` bit-for-bit (1 thread vs 32).
- `DETERMINISM: PASS`. Golden gate `0x56fa28acb5b9fe88` unchanged (read-only campaigns).

N still varies with the **seed** (genesis manifestation is probabilistic via the stateless
`voxel_uniform` RNG), so Phase 1 reports per-seed / seed-averaged energies — each `(A,seed)`
is frozen.

## 2. Phase 1 — mass as flux-energy: ENERGY-COLLAPSES-TO-N

Run of record: `campaign_cluster_energy_spectroscopy --Ls=24,32
--As=2,4,6,8,10,12,14,16,20,28,40 --seeds=5 --settle=300 --survive=150 --tag=v1`.

### 2.1 The undamped field SLOSHES — N is the stable observable, not the flux energy

With no dissipation the flux field never settles to a constant energy: it oscillates flux↔
wave_vel every tick (slosh amplitude ~0.3–0.7 of the mean, present even for the FROZEN O_h
controls). The **manifested count `N` is the stable cluster identity**; the instantaneous
flux energy is not. So the mass proxy is the time-averaged flux energy over a window, and
"BOUNDED" is judged by N-stability, not energy drift. (`total_energy` is dominated by
spurious `wave_vel` pumped by the non-variational Gauss projection — e.g. the moore-cell
control has field 4.1 but wave 995 — so it is a diagnostic, never the mass.)

### 2.2 The result — energy carries no information beyond N

`<<PHASE1_V1_NUMBERS>>`

- **Cluster-local flux energy per voxel is ~constant** (`M_local/N ≈ 0.5` quanta, set by the
  genesis flip residual `v.flux *= max(0,1−K_GENESIS/|J|)`), so `M_local ≈ 0.5·N·ε`. The
  flux-energy mass is just `c·N`: it **collapses to the FTD-0110/0269 voxel-count law**.
- **Whole-lattice flux energy is dominated by an un-condensed radiation halo** and is far
  noisier across seeds than `N` (e.g. at A=8, N=1 but the whole-lattice quanta ≈130 — a
  single voxel wrapped in ~130 quanta of free flux that did not condense). It is not a
  localized mass.
- `N(A)` power-law exponent below the knee ≈ **A^1.9 ≈ A²**, reproducing FTD-0269.
- **The "minimum stable cluster" `A_min` is a single voxel** (A=2, the electron amplitude
  `2√(m_e/m_e)`), carrying ~0.5 cluster-local quanta — **not 511.** The owner's "511 eV-
  quanta = electron, 1 voxel = 1 eV" hypothesis is **falsified**: the electron-amplitude
  cluster is ~1 voxel ≈ 0.5 flip-quanta, and "quanta" and "voxel count" are the same
  observable up to the ~0.5 residual constant.

**Verdict: ENERGY-COLLAPSES-TO-N.** Re-expressing mass as flux energy yields no new
per-particle threshold spectrum; mass remains `∝ N` (FTD-0110/0269). `[MEASURED — BOUNDARY]`.

### 2.3 O_h controls (ledger-read validation, NOT a mass)

The frozen geometric seeds pin `N` exactly to their geometric counts (octahedron 7,
cuboctahedron 13, stella 9, moore 27) under `genesis=false`, validating the energy-ledger
read and the N-freezing invariant. Their field energy is the gauss-induced electrostatic
self-energy of an IMPOSED structure — circular, never quoted as a mass.

## 3. Phase 2 — quark quantization: COLOR-PHENOMENA-IMPOSED

Run of record: `campaign_quark_quantization --L=28 --settle=200 --window=50
--rs=2,3,4,5,6,7,8,9,10,11,12,14 --tag=v1`.

- **single** — all 6 seeded quark scenarios (`s0-seed-{up..top}-quark`) stay at **N=1**: the
  seed amplitudes (`K_B·ampBoost`, 0.26–1.28) are sub-K_GENESIS, so they do **not** grow a
  bounded cluster from the substrate. Quarks are seeded fixed structures.
- **confine** — the color force `f_strong` shows a **clean 3-regime profile** (read directly
  from the force diagnostic; movement off):
  | regime | r | F(diff) | law |
  |---|---|---|---|
  | Coulomb | r<3 | 0.133 @ r=2 | F ∝ 1/r² |
  | transition | 3≤r<8 | 0.333/r | F ∝ 1/r |
  | "linear" | r≥8 | r/64 (0.125→0.188) | **F ∝ r** |
  The same-color force is **exactly ½** the diff-color (cf = +0.5 vs −1.0) at every r. The
  large-r force fits **F ~ r^1.00** — i.e. a **harmonic well V∝r², NOT QCD's constant string
  tension (V∝r, F=const)**. The engine comment at `phase_forces.cpp:167` mislabels the r≥8
  regime as "constant string tension"; the code is `F = α_s·cf·r/64` (∝r).
- **triad** — the geometric color-singlet binding fires: a compact near-equilateral triad of
  3 same-state particles (pairwise ≤ TRIAD_RADIUS=3, ratio ≥ 0.8) gets **locked=3 with
  `triad_binding` ON, 0 with it OFF**. The rule is purely geometric (it does not check
  color); the binding does not change the field energy (it pins the particles).

**Verdict: COLOR-PHENOMENA-IMPOSED.** The engine presents color confinement and triad
binding, but as built-in toggle RULES, not emergent substrate dynamics; the quarks are
seeded structures. `[MEASURED]`.

## 4. Epistemic accounting

This work re-expresses the FTD-0110/0269 cluster law in flip-quantum energy units and probes
the color sector. It is **not a mass derivation** and carries no new derived scale. **Nothing
is promoted:** FTD-0013 stays `[STRONGLY MOTIVATED CONJECTURE]`, MC-T4.3 stays a foundational
obstruction, FTD-0110/0261/0269/0272 unchanged. The honest deliverable is a clean BOUNDARY
(the Number-One-Goal's second clause): the discrete ontology's "mass" is the cluster voxel
count, and re-coding it as flux energy adds nothing; its color phenomena are imposed rules,
not derived. Next free LEDGER id: **FTD-0274**.

### Artifacts
- Campaigns: `engine/tests/campaign_{determinism_gate,cluster_energy_spectroscopy,quark_quantization}.cpp`
- Analyzers: `scripts/exploration/analyze_{determinism_gate,cluster_energy_spectroscopy,quark_quantization}.py`
- Runs of record (local, gitignored): `engine/results/{determinism_gate,cluster_energy_spectroscopy,quark_quantization}/*_v1.csv`, `*_pool.csv`
