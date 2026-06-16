# ANALYSIS — Thermostat-OFF Sweep v1: run of record INVALID (V-1) — and the diagnosis is the finding

> ** RESOLUTION (2026-06-10, owner decision — supersedes §4's open forensics):** *"we've had to fix some mistakes over time so that may be why it's different — just fix it."* The April→today phenomenology change traces to **accumulated deliberate engine corrections**; the **current stack is canonical**; the environment-forensics `[OPEN]` is **CLOSED by decision** (the unreconciled P-α detail is retained below as provenance only). Fixes shipped same day: the canonical test re-baselined to current-stack bands (historical pins preserved in comments), `gpc_03_genesis()` made quantitative (3× cross-backend band), the STACK-PINNED banner added to `FOUND_LATTICE_SPACING_GAUGE_FREEDOM` §12, and FTD-0110's empirical leg re-tagged `[STACK-PINNED — historical]`. Successor `[OPEN]` (TRACKER): re-characterize the current-stack N(A) law + re-assess the SM identification. **The v2 thermostat discriminator is UNBLOCKED against the current-stack control table (this run's arm C).**
>
> ** CORRECTION BANNER (2026-06-10, same day — read §4 before §0–§3.** The §0–§3 conclusion "the engine's tracked behavior changed between April and June" did **not** survive same-day follow-up probes: April source rebuilt today reproduces the *broken* values (tracked code excluded), and CPU and GPU agree with each other today (backend excluded). The honest residual: the historical FTD-0110 baseline is reproduced by **no available combination today**; the change lives in the **runtime environment or an untracked input**. The "ic1 regression bisect" `[OPEN]` announced in §3 is **WITHDRAWN** and replaced by §4's re-scoped item. The V-1 INVALID verdict and the gate record are unaffected.)

**Tag:** `[INVALID RUN — pre-registered gate V-1 failed; no mechanism outcome claimed or claimable]` + `[OBSERVATION — corrected per §4: environment-class reproducibility break; the FTD-0110 empirical k(A) baseline is not reproducible on any available code×backend combination today]`. **Nothing promoted; Mechanisms γ/β remain exactly as FTD-0259 left them (outcome language banned per pre-reg F-a).**
**Date:** 2026-06-09/10
**Pre-registration:** [`PREREG_THERMOSTAT_OFF_AMPLITUDE_SWEEP_v1.md`](PREREG_THERMOSTAT_OFF_AMPLITUDE_SWEEP_v1.md) (tag `preregister-thermostat-off-sweep-v1`, lock commit `4fa056c2`)
**Run of record:** `engine/results/thermostat_off_sweep_2026-06-09/` (28 CSVs + frozen `verdict.txt`; clean worktree at the lock commit; WSL2 build, 0 run failures)
**LEDGER:** FTD-0260.

---

## 0 · One-paragraph result

The discriminator ran exactly as designed and the **validation gate fired**: arm C (thermostat ON at the historical γ = 0.02, T = 0.005) reproduced the historical k(A) table at **0/11 amplitudes** (V-1 requires ≥ 8/11 within 0.025), so per pre-registered rule F-a the run is **INVALID and no mechanism outcome may be claimed**. The diagnosis, executed before any interpretation, eliminated the two mundane explanations and confirmed the interesting one: **(i)** constants are unchanged (April `K_GENESIS = K_B·N_C = 1.533` ≡ today's `K_MANIFEST·N_C = 1.533`, git-verified at the April campaign commit `87158aef`); **(ii)** the rig is faithful (the April runner's toggle block and ic1 injection are **byte-identical** to this campaign's: `wave_propagation + gauss_projection + genesis + langevin(γ=0.02, T=0.005)`, `inject_flux(L/2,L/2,L/2, {A·K_GENESIS,0,0})`, L = 32); **(iii)** therefore **the engine's ic1 phenomenology itself has changed since 2026-04-28**: at A = 10 the steady-state cluster is now N ≈ 3–5 (seed-range 3–5, time-stable: n_min = n_max over the full 210–700 window) versus April's N ≈ 25 — a ~6× regime shift under identical configuration. The FTD-0110 empirical table is an **April-2026-engine artifact**, provenance-pinned, and the thermostat question cannot be adjudicated until the regression is bisected or the baseline re-established.

## 1 · Gate record (mechanical, from the frozen analysis)

- **V-1 rig gate: FAIL 0/11.** Representative: k_C(10) = 0.039 vs historical 0.252; k_C(50) = 0.053 vs 0.222; k_C(117.93) = 1.025 (flooding) vs 0.206.
- **V-2 determinism: FAIL** at 10/11 amplitudes — a **design error in v1, not an engine anomaly**: genesis is itself stochastic (`p = 1 − exp(−excess/K_MANIFEST)` + RNG draw, `phase_write.cpp:219–265`), so the thermostat is not the only noise source and the off-arm is *not* deterministic. v2 must drop or re-scope V-2.
- **F-d fired (off-arm object change):** with the thermostat OFF, A ≥ 20 floods (N̄ up to ~14,000): friction was the only energy exit in the periodic box, so undissipated injection reverberates and re-triggers genesis. Any v2 off-arm needs an energy exit (e.g. the 2026-06-06 absorbing-boundary sponge toggle) — itself a deviation from the historical protocol that must be designed in, not patched in.
- Verdict line (frozen): `VERDICT: INVALID RUN (V-1 failed). Diagnose the rig; no outcome claimed.`

## 2 · Diagnosis chain (what was checked, in order)

| Hypothesis | Check | Result |
|---|---|---|
| Constants drift (K_GENESIS redefined in the unified-mass arc) | `git show 87158aef:…/particle_masses.h` vs today | **Eliminated** — same value 1.533 (`K_B·N_C` → `K_MANIFEST·N_C`, type-rename only) |
| Rig infidelity (wrong toggles/injection/protocol vs April) | April runner source vs this campaign, line-by-line | **Eliminated** — byte-identical toggle block + injection; same L; April measured stride-50 snapshots (timing differences moot given the next row) |
| Transient-vs-steady measurement artifact | per-seed `n_min/n_max` over ticks 210–700 | **Eliminated** — clusters are time-stable at the new small values (e.g. n_min = n_max = 4) |
| **Engine behavior changed since 2026-04-28** | all of the above | **Confirmed by elimination** — identical config, ~6× different steady state |

Six weeks of engine arcs separate the baselines, each individually gated or legitimately golden-recaptured (the golden hash was *recaptured* at the 2026-06-03 odd-lattice change, which permits behavior shifts; candidate windows also include the EWSB/Phase-B physics, the absorbing-sponge addition, and the unified-mass arc). **Which commit moved ic1 is undetermined — a bisect is the queued follow-up** (build + `--A=10 --seeds=1` per probe point; each probe is seconds, so a ~200-commit bisect is ~8 builds ≈ one session).

## 3 · Consequences (tagged)

1. **`[OBSERVATION — load-bearing]`:** the FTD-0110 empirical leg (the 11-point k(A) table, and with it the e/μ/π/K/p/τ cluster-mass matches of `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` §6.5/§12.4) is **provenance-pinned to the April-2026 engine**. The linear theorem (k = ¼, `[DERIVED]`) is pure mathematics and untouched. The *empirical* claims should be read as "engine-at-commit-`87158aef`" measurements until the regression is understood — anyone re-measuring on the current engine gets a different table (today: k(10) ≈ 0.04, flooding at high A even thermostat-on).
2. **`[OPEN — NEW]`: the ic1 regression bisect.** Find the commit(s) between `87158aef` (2026-04-28) and `4fa056c2` (2026-06-09) that changed ic1 steady-state cluster size at A = 10 from ~25 to ~4. Decide whether the change is (a) an unintended physics regression to fix, or (b) an intended consequence of a legitimate arc — in which case the FTD-0110 empirical baseline must be formally re-measured and re-tagged on the current engine.
3. **Thermostat discriminator status:** the question (Mechanism γ vs others) is **unresolved and unprejudiced** — v1 produced no admissible evidence either way. v2 is **blocked on item 2** and must additionally fix: V-2 (genesis RNG), the off-arm energy exit (sponge), and re-baselined gates (validate against a *current-engine* control table, with the April table as context only).
4. **Process note `[SYNTHESIS]`:** the pre-registration machinery converted what would have been a confidently wrong mechanism verdict (the naive reading of the v1 tables "drift ratio 127×" is meaningless across a regime shift) into a reproducibility discovery. Second catch of this shape today (cf. FTD-0252 v1's OTHER).

## 4 · CORRECTION (2026-06-10, same-day follow-up probes) — the §2 diagnosis was wrong in its conclusion; here is what actually stands

The §2 table's three eliminations were correct, but the inference "the engine's tracked behavior changed" did not survive further probing. Two successive hypotheses were tested and **both failed**; the residual facts are stated below at full strength and nothing more.

**Probe record (all same-day, bisect worktree, WSL2):**
- **P-α (April code, today's stack, CPU):** `87158aef` rebuilt today gives N(A=10) ≈ 4 — *the April source reproduces today's broken value, not April's recorded 25.* Long-horizon variant (t = 2500–2900): still 4. With `coupling = true` (the canonical test's extra toggle): still 4. **⇒ Tracked code is EXCLUDED as the cause.** A code bisect has nothing to find; the §3(2) "ic1 regression bisect" item is **WITHDRAWN**.
- **P-β (canonical test, lock commit, CPU):** `test_emergent_ic1_topology` — the project's own FTD-0107 regression test asserting 25 ± 2 — **FAILS on a CPU build today** (T2 0/3 at L=16: 8/6/4 voxels; T1 0/3 at L=32: 4/5/4).
- **P-γ (canonical test, lock commit, CUDA/GPU backend active):** **FAILS identically** (T1/T2 0/3, N = 3–5 at A=10), and its T5b sweep gives k = {0.040, 0.088, 0.068, 0.050, 0.052} at A = {10, 15, 20, 30, 50} — statistically the same family as the CPU control arm (0.039, 0.077, 0.057, 0.047, 0.053). **⇒ Backend divergence is EXCLUDED as the explanation of v1's V-1 failure: both backends agree with each other today, and both disagree with the historical table at every amplitude (≈4–5× low above the knee).**
- **Misread, recorded for provenance (F6-class trap):** an interim reading of the first GPU run's output mistook a *historical-reference column* (entries = k_hist·A²: 50.4, 93.4, 235.8, 554.0) for measured values and briefly concluded "GPU reproduces history." The line-buffered re-run exposed the column structure. No corpus edit was committed under that reading; this paragraph is the only trace, kept deliberately.

**Corrected diagnosis `[OBSERVATION — narrowed]`:** the historical FTD-0110 phenomenology (N(10) = 25; the §6.5 k(A) table; the May-era canonical test passing) is **not reproduced by any combination available today** — {April source, lock-commit source} × {CPU, GPU} all yield the new low family. Since the tracked source and the backend axis are both excluded, the April/May→today change lives in the **runtime environment or an untracked input**: toolchain (gcc/CUDA/driver/glibc/WSL kernel) versions, build-flag resolution, or thread-count-dependent stochastic ordering. The whole k(A) curve sits ≈4–5× low — a systematic shift, not marginal-regime noise.

**Standing coverage finding (unchanged by the correction):** `gpc_03_genesis()` in `test_gpu_parity_complete.cpp` asserts only `manifested_count >= 1` per backend — an existence check that can never detect quantitative genesis drift on either axis. Strengthening it to a quantitative cross-check is required regardless of where the environment forensics land.

**Re-scoped `[OPEN]` (replaces the withdrawn bisect):** (i) environment forensics — recover the April/May build provenance (CMakeCache/compiler/CUDA versions from any preserved build trees, CI logs, or the FTD-0107 result metadata) and identify the changed layer; (ii) determine on which platform/date `test_emergent_ic1_topology` last passed (it is absent from the 2026-06-06 known-failures list — either it passed then, or that list's ctest invocation excluded campaign-labeled tests); (iii) strengthen gpc_03. **v2 of the thermostat discriminator remains blocked** — there is no backend escape hatch; V-1 fails on both backends until the environment question is resolved or the baseline is formally re-measured and re-tagged on the current stack.

## 5 · Scope

Not evidence for or against Mechanism γ (banned, F-a); not a demotion of FTD-0110's linear theorem (math, untouched); not a claim that the April data was wrong (it was correct for its stack — the record that no longer reproduces is the *stack*, not the measurement).
