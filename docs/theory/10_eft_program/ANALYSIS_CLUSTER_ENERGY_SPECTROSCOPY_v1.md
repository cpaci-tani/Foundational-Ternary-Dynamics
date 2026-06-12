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

At fixed L (run of record `--Ls=24,32 --As=2..40 --seeds=3`):

- **The cluster-local flux energy tracks the voxel count N.** At fixed L, the per-A
  cluster-local energy is explained by `N` alone — Pearson `r(N, M_local) = 0.995` (L=24)
  and `0.984` (L=32), mean **0.990**. So `M_local ≈ c(L)·N`: the flux-energy mass carries
  no information beyond `N` and **collapses to the FTD-0110/0269 voxel-count law**. (The
  per-voxel coefficient has ~40–70 % scatter from halo contamination at low N — it is *not*
  a clean constant — but the strong N-correlation is the robust collapse signal, and there
  are no per-particle plateaus or jumps breaking from the N trend.)
- **Whole-lattice flux energy is dominated by an un-condensed radiation halo** and is far
  noisier across seeds than `N` (e.g. at A=8, N=1 but the whole-lattice quanta ≈130 — a
  single voxel wrapped in ~130 quanta of free flux that did not condense). It is not a
  localized mass.
- `N(A)` power-law exponent below the knee = **b ≈ 2.14 ≈ A²**, reproducing FTD-0269.
- **Verdict-logic note (false-positive caught):** a naive collapse test using the M/vox CV
  *pooled across L* flips to "ENERGY-STRUCTURE" — but that is an artifact of pooling, because
  `c(L)` genuinely decays with box size (§2.3). The verdict is computed *per-L* (the
  `r(N, M_local)` correlation above) precisely to avoid conflating the L-trend with a
  per-particle spectrum.
- **The "minimum stable cluster" `A_min` is a single voxel** (A=2, the electron amplitude
  `2√(m_e/m_e)`) — **not 511 quanta.** The owner's "511 eV-quanta = electron, 1 voxel =
  1 eV" hypothesis is **falsified**: the electron-amplitude cluster is ~1 voxel, and the
  mass is the voxel count, not an energy threshold.

### 2.3 Lattice-size convergence (does the local energy stabilize on a bigger box?)

A natural objection: the flux↔wave sloshing and the halo could be a *small-periodic-box
artifact* — on a small lattice the radiated wave wraps around and re-interferes with the
cluster; on a bigger box the radiation escapes and the cluster-local energy should settle
to a clean, box-independent mass. **Directly tested** (`--Ls=24,32,48,64 --As=10,16
--seeds=3`, deterministic), measuring the cluster-local energy AND its slosh vs L:

| A | quantity | L=24 | L=32 | L=48 | L=64 | trend |
|---|---|---|---|---|---|---|
| 10 | ⟨N⟩ | 3.7 | 4.3 | 3.7 | 4.3 | **L-invariant** |
| 10 | ⟨M_local⟩ | 6.19 | 1.10 | 0.31 | 0.068 | **→ 0** (≈ L⁻⁵) |
| 10 | ⟨M/vox⟩ | 1.94 | 0.32 | 0.086 | 0.016 | → 0 |
| 10 | slosh_local | 1.20 | 1.51 | 2.05 | 2.88 | **grows** |
| 16 | ⟨N⟩ | 20.0 | 21.7 | 19.3 | 20.0 | **L-invariant** |
| 16 | ⟨M_local⟩ | 6.96 | 3.10 | 1.05 | 0.196 | **→ 0** |
| 16 | slosh_local | 1.13 | 1.57 | 1.78 | 3.35 | grows |

The objection is **partly right but mass-fatal**: on a bigger box the radiation *does*
escape rather than reflect (the whole-lattice slosh_tot drops, 0.73→0.34 at A=10), so the
effects *are* more local — **but the consequence is that the cluster-local flux energy
decays toward zero (≈ L⁻⁵), not toward a stable mass.** The injected energy radiates into
the larger volume and the cluster retains only its small bound near-field (which itself is
~const-per-voxel ⇒ still ∝ N). The relative local slosh *grows* with L precisely because
the local mean energy → 0. **The cluster COUNT N is the L-invariant; the flux ENERGY is
box-dependent and vanishing.** There is no "511 quanta" at any L — at L=64 the
electron-amplitude cluster retains < 0.1 quanta total.

**Verdict: ENERGY-COLLAPSES-TO-N.** Re-expressing mass as flux energy yields no new
per-particle threshold spectrum at any lattice size; the L-invariant mass is the voxel
count `N` (FTD-0110/0269), and the flux energy decays with box size. `[MEASURED — BOUNDARY]`.

### 2.4 Interpretation — a thermodynamic reading (why N, not energy)

`[INTERPRETATION — grounded in the measured L-trend §2.3]`. The L-convergence has a clean
thermodynamic picture: a hot copper ball in a jar. The supercritical flux injection is the
hot ball; the lattice is the jar of "atmosphere." A fixed energy goes in; give it more
volume (bigger `L³`) and the same energy spreads thinner, so the *local* energy density near
the source falls — exactly the measured `M_local` decay. In the long-time limit on a closed
box the conserved total energy smears toward a uniform low "ambient" density `∝ 1/L³`: a
bigger jar reaches a cooler equilibrium. (The measured local near-field falls *faster* than
`1/L³` because the window catches the transient before equilibrium and because we sample the
cluster's bound near-field, not the uniform ambient.)

**One qualification:** with `langevin=false` (required for determinism) there is no friction
and no heat bath, so this is a **lossless wave dispersing**, not true irreversible
thermalization — on a finite periodic box the energy is conserved and would recur rather
than settle. But over the observation window the effect on the local region is identical:
the energy leaves and does not return. (Turning langevin on would make it a genuine
copper-ball-cools-to-ambient process, at the cost of determinism.)

**The point the analogy sharpens — and the reason `N` is the mass.** A copper ball *cools*:
it loses its energy to the ambient and its own temperature drops. But the cluster **does not
shrink** — `N` stays fixed (≈4 at A=10, ≈20 at A=16) regardless of jar size. The flux energy
and the cluster count therefore behave like two *different* thermodynamic quantities:

| | engine quantity | thermodynamic analogue | behaviour under "more volume" |
|---|---|---|---|
| **heat** | flux energy `½Σ|J|²` | temperature / internal energy | extensive, dilutes, equilibrates away `∝1/L³` |
| **matter** | cluster count `N` | conserved particle number | invariant — set once at the genesis burst |

The cluster's `N` is frozen in at nucleation (the one-shot genesis burst, FTD-0267) and is a
**counting (topological) invariant**, not a field-energy concentration. That is *why* energy
fails as a mass and `N` succeeds: the "511 quanta" hypothesis asked the *heat* to be the
mass, but the heat dissipates into the jar while the conserved thing is the droplet of
condensed matter, which does not care about the jar size. **In this engine, the discrete
ontology's mass is conserved matter-count, not equilibrating field-energy** — the cleanest
statement of the FTD-0273 boundary.

### 2.5 O_h controls (ledger-read validation, NOT a mass)

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
