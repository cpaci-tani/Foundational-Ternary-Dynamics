# PRE-REGISTRATION — Slow-envelope live Newton v1.1

**Tag:** `[PRE-REGISTRATION]` — locks the CPU observer that repeats FTD-1022 v1 with **one** repair: \(g\) is the **member-mean** of the Q0 stencil, matching \(a_{\rm COM}=\mathrm{mean}_i(C^2\mathcal{L}\nabla\mathcal{L})\). Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-slow-envelope-live-newton-v1-1` (pending owner commit; until the tag resolves this lock is `anchored-late` via §12 prefix SHA256).
**LEDGER reservation:** FTD-1022 (same row; v1 booked UNDERDETERMINED on P6).
**Parent:** [`PREREG_SLOW_ENVELOPE_LIVE_NEWTON_v1.md`](PREREG_SLOW_ENVELOPE_LIVE_NEWTON_v1.md) prefix SHA256 `4C82033F93AC8A182E4B7AE7538BD6F3065428823C850853D4C03CC4AEDD9D25`.
**Does not move:** FTD-0250, FTD-0349, FTD-0402, FTD-0131, FTD-0361, FTD-1013–1021, U-8. No golden tick. No production default ON. No GPU. No \(N_p=N_s\). No movement.

> LOCK-STD v1. Sections §1–§11 are frozen before any v1.1 live-arm acceleration is observed. Post-hoc edits to §1–§11 void v1.1.

---

## §1 — The question (LOCKED)

**Q-SLOW-ENV-NEWTON-v1.1.** Same question as v1: on the FTD-1017 source with a locked 3³ probe (COM at the 1021 site), does freeze≈live return, and does FTD-1016 match live Q0?

**The only change from v1:** \(g\) is

\[
\bar g=\frac1{N_p}\sum_{i\in\mathrm{probe}} C_{\rm SPEED}^2\,\mathcal{L}_i\nabla\mathcal{L}_i
\]

with the production tier-2 stencil at each member, not the single COM sample. \(a_{\rm COM}\) is already that mean (cluster inertia). v1 P6 failed because it compared a 27-site mean kick to a 1-site \(g\). Fixture, arms, A2/A3, and ε are unchanged.

**Prior-favoured.** P6 and A1 FOUND (algebra of `cluster_inertia`). A2/A3 unknown (3³ vs one-voxel self-depth). Priors are not measurements.

---

## §2 — Fixture (LOCKED)

Identical to v1 §2 (source origin \((6,13,13)\) edge 5; probe origin \((17,14,14)\) edge 3; arms Z/L/T/S; CPU; no movement), except \(g\) is \(\bar g\) as in §1.

---

## §3 — Measurands (LOCKED)

\(r_Z=a_Z/\bar g_Z\), \(r_L=a_L/\bar g_L\), \(r_T=a_T/\bar g_T\), \(\delta_a,\delta_g,\rho_S,\delta_T\) as in v1, with \(\delta_g\) using \(\bar g\). Also record COM \(\mathcal{L}\) on Z and L (diagnostic, not a gate).

---

## §4 — Executable protocol (LOCKED)

Instrument: `engine/tests/test_slow_envelope_live_newton.cpp` (CTest name `slow_envelope_live_newton`).

**Protocol gates:** v1 P1–P10 with P6 now \(\lvert r_Z-1\rvert<0.05\) under \(\bar g\). P1 still uses \(\lvert\bar g_Z\rvert>10^{-8}\). P2/P8 use \(\bar g_x\cdot\Delta x<0\).

**Physics / classification:** v1 A1/A2/A3 unchanged (ε=0.05). CTest passes if protocol holds and A1 holds. A2/A3 classify.

---

## §5 — Outcome map (LOCKED)

Same four columns as v1 (IMPROPER / FOUND test-body recovered / FOUND envelope still responds / CLOSED-NEGATIVE / UNDERDETERMINED), with tags:

- **FOUND — test-body recovered.** P1–P10, A1, A2, A3. `[MEASURED — SLOW-ENVELOPE TEST-BODY]`
- **FOUND — live wiring, envelope still responds.** P1–P10, A1, A2 or A3 fail. `[MEASURED — SLOW-ENVELOPE STILL SOURCES]`
- **CLOSED-NEGATIVE.** P1–P10, A1 fails.
- **UNDERDETERMINED.** Any P-gate fails.

---

## §6 — Tie-breaks (LOCKED)

- Member-mean \(\bar g\) uses every one of the 27 probe sites, equal weight, the same stencil as FTD-1021.
- \(a_x\) from rigid \(V_{\rm COM}\). Equality at a gate is a pass. \(\gamma_{\rm FTD}\) not divided out.

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can fail? | Witness |
|---|---|---|
| P6 | Yes | γ_FTD or force_diag omission at some members |
| P9 | Yes | 3³ self-force not small |
| A2/A3 | Yes | 3³ still peaked on the \(\pm 2\) stencil |

---

## §8 — Banned moves (LOCKED)

Same as v1. Do not revert to COM-only \(g\). Do not widen ε. Do not set \(N_p=125\).

---

## §9 — Quantifier coverage (LOCKED)

This 3³, this source, \(\bar g\). Not \(\forall\) edge, not Nordtvedt.

---

## §10 — Window (LOCKED)

2026-08-19 America/Chicago through 23:59, this session, CPU observer. Git tag pending; result cites §12 SHA as `anchored-late`.

---

## §11 — Reconciliation (LOCKED)

FTD-1022 remains one row. v1 is UNDERDETERMINED on P6 (COM \(g\) vs cluster-mean \(a\)). v1.1 is the physics execution of the same question. FTD-1021 remains the one-voxel result. Production default remains \(\nabla|J|\).

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive, including the trailing newline after that line.

**Content SHA256 of hashed prefix:** `5D0BB44FFDDEF81C1B3E84DFB46F45A79508DFDEDC9EAE611594776B07843FF9`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19 America/Chicago, CPU observer, CTest `slow_envelope_live_newton` **Passed** (P1–P10 + A1; A2/A3 fail). Instrument `engine/tests/test_slow_envelope_live_newton.cpp` SHA256 `3C4BAE486B4451FE647FD66B4AADD6FA4B3B39C0555198CF727800F1318960C5`. Frozen classifier **FOUND — live wiring, envelope still responds**. Tag of record: `[MEASURED — SLOW-ENVELOPE STILL SOURCES]`.

Member-mean \(\bar g_Z=-2.589412\times10^{-4}\), \(a_Z=-2.588788\times10^{-4}\), \(r_Z=0.999759\) (P6 pass). Live \(r_L=0.954590\) (\(\lvert r_L-1\rvert=0.04541\), A1 pass inside \(\varepsilon=0.05\)). \(\delta_a=3.183\), \(\delta_g=3.380\) (A2/A3 fail). \(\delta_T=0\). \(\rho_S=1.16\times10^{-16}\). COM \(\mathcal{L}_Z=0.030271\), \(\mathcal{L}_L=0.337421\). Rigid \(=1\). Constructor logs “GPU backend active” then `force_cpu()`; not IMPROPER. Anchor: **`anchored-late`** until `git rev-parse preregister-slow-envelope-live-newton-v1-1` succeeds.
