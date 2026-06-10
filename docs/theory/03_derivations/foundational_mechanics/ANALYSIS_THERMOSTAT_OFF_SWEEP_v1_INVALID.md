# ANALYSIS — Thermostat-OFF Sweep v1: run of record INVALID (V-1) — and the diagnosis is the finding

**Tag:** `[INVALID RUN — pre-registered gate V-1 failed; no mechanism outcome claimed or claimable]` + `[OBSERVATION — engine-evolution reproducibility break: the FTD-0110 empirical k(A) baseline is not reproducible on the current engine]`. **Nothing promoted; Mechanisms γ/β remain exactly as FTD-0259 left them (outcome language banned per pre-reg F-a).**
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

## 4 · What this is NOT

Not evidence for or against Mechanism γ (banned, F-a); not a demotion of FTD-0110's linear theorem (math, untouched); not a golden-gate failure (every arc since April passed its gates — the point is that *recaptures permit cumulative behavior drift that no single gate watches*); not a claim that the April data was wrong (it was correct for its engine).
