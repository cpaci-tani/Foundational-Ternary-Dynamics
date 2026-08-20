# PRE-REGISTRATION — GPU geometric-gravity parity v1

**Tag:** `[PRE-REGISTRATION]` — locks the CPU/CUDA observer that asks whether native CUDA `phase_forces` reproduces the FTD-1016 operator. Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-gpu-geometric-gravity-parity-v1` (pending owner commit; until the tag resolves this lock is `anchored-late` via §12 prefix SHA256).
**LEDGER reservation:** FTD-1018.
**Parent:** FTD-1016 operator; FTD-1014 default \(\nabla|J|\) residue. No new physics.
**Coverage catalog (not hashed):** [`CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md`](../../../03_derivations/gravity_and_cosmology/CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md) — this lock is the CUDA port of the falling operator, not lensing/GWs.
**Does not move:** FTD-0250, FTD-0349, FTD-0402, FTD-0208, FTD-1013, FTD-1014, FTD-1015, FTD-1016, FTD-1017, U-8, FTD-0131. No golden tick. No production default ON. No P6C-G. No graviton. No \(g_{rr}\). No CODATA retune of \(G_N\).

> LOCK-STD v1. Sections §1–§11 are frozen before any CPU/GPU force or velocity is observed. Post-hoc edits to §1–§11 void v1.

---

## §1 — The question (LOCKED)

**Q-GPU-GEO-PARITY-v1.** On the FTD-1016 prescribed-well fixture, extra forces off, one unlocked rest voxel, does one native-CUDA `tick()` with `geometric_gravity=true` reproduce the CPU `f_gravity` and post-tick velocity of the same operator

\[
\mathbf F = M_{\rm INERTIAL}\,C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}
\]

with the same tier-2 stencil on `voxel.latency`?

Toggle-OFF residue: both backends keep \(F=G_N\nabla|J|\) with \(J=0\).

**Not asked:** weak EP vs \(N\); sourced Poisson (FTD-1017); redshift; making the toggle a production default; bit-identity of unrelated phases; WSL2 vs Windows-native CUDA as a physics split.

**Prior-favoured outcome.** FOUND (the CUDA kernel is a port of the CPU branch). Favoured is not predetermined. A1 fails CTest if false.

---

## §2 — Fixture (LOCKED)

| Item | Frozen value |
|---|---|
| CPU backend | `RenderBridge::force_cpu()` before the CPU tick. |
| GPU backend | `GpuEngine` native CUDA. `graph_capture_enabled=false`. One device tick. Windows-native CUDA is allowed (correctness CTest, not a measurement campaign). |
| Lattice | \(L=32\), periodic. |
| Well | Prescribed \(\mathcal{L}=0.05+10^{-3}x\) on **every** site, including vacuum. Written on the host seed **before** upload / CPU assign. `latency_field=false` (do not re-solve). |
| Probe | Unlocked \(+1\) voxel, \(N=1\), \(J=0\), \(v=0\), `particle_id=1`, at \((14,14,14)\). `cluster_inertia=false`. |
| Toggle-ON | `disable_all`, then `forces=true`, `gravity=true`, `geometric_gravity=true`. `movement=false`, `latency_field=false`, EM/colour/Yukawa/exchange off. One `tick()`. |
| Toggle-OFF | Identical except `geometric_gravity=false`. |
| Integration | Production `tick()` integrate of \(F/M\) (unlocked path). No public `phase_forces` split on GPU. |

---

## §3 — Measurand (LOCKED)

Toggle-ON, at the probe index:

\[
\Delta F=\max_i |F_{{\rm cpu},i}-F_{{\rm gpu},i}|,\qquad
\Delta v=\max_i |v_{{\rm cpu},i}-v_{{\rm gpu},i}|.
\]

\(F\) is `force_diag.f_gravity` / `ForceDiagHost.gravity_*`. Single registered pair. No scan over slope, \(G_N\), or \(N\).

---

## §4 — Executable protocol (LOCKED)

Instrument: `engine/tests/test_gpu_geometric_gravity_parity.cpp` (CTest name `gpu_geometric_gravity_parity`).

**Protocol gates (must pass before a physics verdict):**

| ID | Claim | Pass if |
|---|---|---|
| P1 | CPU ON writes a nonzero geometric kick | \(\|F_{\rm cpu,on}\|>0\) |
| P2 | GPU ON writes a nonzero geometric kick | \(\|F_{\rm gpu,on}\|>0\) |
| P3 | Extra forces off on both ON paths | \(\max\|f_{\rm coulomb}\|,\|f_{\rm strong}\|,\|f_{\rm magnetic}\|,\|f_{\rm exchange}\|<10^{-12}\) on the probe, CPU and GPU |
| P4 | Default gravity unread of \(\mathcal{L}\) | Toggle-OFF: \(\|F_{\rm cpu,off}\|,\|F_{\rm gpu,off}\|,\|v_{\rm cpu,off}\|,\|v_{\rm gpu,off}\|<10^{-12}\) |

**Physics gate (fails CTest if false):**

| ID | Claim | FOUND if |
|---|---|---|
| A1 | CUDA reproduces CPU | \(\Delta F<10^{-10}\) and \(\Delta v<10^{-10}\) on toggle-ON |

---

## §5 — Outcome map (LOCKED)

**IMPROPER** (precedes): injecting \(F=mg_{\rm ext}\) then comparing; retuning the slope or origin after seeing \(\Delta F\); widening ε; enabling `cluster_inertia` or colour/EM; leaving `latency_field` on; treating a \(1/r^2\) residual as this A1; CODATA retune of \(G_N\); requiring WSL2 as a condition of A1.

**FOUND.** Not IMPROPER. P1–P4 pass. A1 passes. Tag: native CUDA implements FTD-1016 `[MEASURED — GPU parity]`. Still does **not** derive \(m_i=m_g\) or physical \(G_N\).

**CLOSED-NEGATIVE.** Not IMPROPER. P1–P4 pass. A1 fails. CTest fails.

**UNDERDETERMINED.** Not IMPROPER. Any of P1–P4 fails. CTest fails on the failed protocol gate.

Partition: IMPROPER first; then if protocol fails → UNDERDETERMINED; else A1 true → FOUND else CLOSED-NEGATIVE. One column only.

---

## §6 — Tie-breaks (LOCKED)

- Euclidean \(\|F\|\) for P1/P2; componentwise max-abs for A1 \(\Delta F,\Delta v\).
- A1 uses \(10^{-10}\). Equality at exactly \(10^{-10}\) is FOUND. \(\gamma_{\rm FTD}\) is not corrected out (both backends keep it).
- P3 uses Euclidean magnitude on the probe’s extra-force channels.
- Probe occupancy is the single site \((14,14,14)\).

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can fail? | Witness |
|---|---|---|
| P1 | Yes | CPU branch not compiled / well not uploaded |
| P2 | Yes | CUDA kernel ignores `geometric_gravity` or `d_latency` |
| P4 | Yes | Accidental rewrite of default \(\nabla\rho\) to read \(\mathcal{L}\) on either backend |
| A1 | Yes | Sign/stencil/SoA mismatch between CPU and CUDA |

---

## §8 — Banned moves (LOCKED)

- Golden-tick change; production default ON.
- Promote FTD-1013 / 1016; claim UFF derived from a CUDA port.
- Coincidence scan; CODATA; graviton; TEGR; \(g_{rr}\).
- Edit this prereg after observing \(\Delta F\) or \(\Delta v\).

---

## §9 — Quantifier coverage (LOCKED)

A1 is this prescribed well, this unlocked \(N=1\), this one-tick pair. It is not \(\forall N\), not sourced Poisson, not graph-capture identity, not a new theorem.

---

## §10 — Window (LOCKED)

2026-08-19 America/Chicago through 23:59, this session, CPU+CUDA observer. Past window with no verdict books F10. Git tag pending; result cites §12 SHA as `anchored-late`.

---

## §11 — Reconciliation (LOCKED)

FTD-1018 is a new row. FTD-1016 remains the prescribed-well CPU operator result; this lock only ports that operator. FTD-1014 remains CLOSED-NEGATIVE for default \(\nabla|J|\) (P4 re-asserts on both backends). FTD-1017 remains the sourced-CPU wiring. Production default remains \(\nabla|J|\).

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive, including the trailing newline after that line.

**Content SHA256 of hashed prefix:** `624969CA01DC55906B55D409A38F56CFD539FFA592DB70E237E881375CF2EE9E`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19 America/Chicago, CPU+CUDA observer, CTest `gpu_geometric_gravity_parity` **Passed**. Instrument `engine/tests/test_gpu_geometric_gravity_parity.cpp` SHA256 `0BDAC00297A5E2B9639B8CC1CDA3302E2AFF80C49CD32DF717175A590E943704`. Frozen classifier **FOUND**. Toggle-ON \(F_x=1.090133\times10^{-5}\) on both backends; \(\Delta F=0\), \(\Delta v=0\). Toggle-OFF \(F=0\), \(v=0\) on both. Golden 7/7 Passed. Constructor GPU banners then `force_cpu()` on the CPU path; GPU path is `GpuEngine` native CUDA (`graph_capture_enabled=false`). Result: [`ANALYSIS_GPU_GEOMETRIC_GRAVITY_PARITY_v1.md`](../../../03_derivations/gravity_and_cosmology/ANALYSIS_GPU_GEOMETRIC_GRAVITY_PARITY_v1.md). Anchor: **`anchored-late`** until `git rev-parse preregister-gpu-geometric-gravity-parity-v1` succeeds. FTD-1013 / 1014 / 1015 / 1016 / 1017 / 0131 unmoved. Production default remains \(\nabla|J|\).
