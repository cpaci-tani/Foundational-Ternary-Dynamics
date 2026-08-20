# PRE-REGISTRATION — Live sourced Newton v1

**Tag:** `[PRE-REGISTRATION]` — locks the CPU observer that re-solves `latency_field` Poisson **with the light probe present** and asks whether FTD-1016 still matches Q0 on that live well, and whether the 1017 freeze is a test-body approximation. Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-live-sourced-newton-v1` (pending owner commit; until the tag resolves this lock is `anchored-late` via §12 prefix SHA256).
**LEDGER reservation:** FTD-1021.
**Parent:** FTD-1017 frozen sourced wiring; operator = FTD-1016; SCOPE [`SCOPE_UNIVERSAL_FREEFALL_v1.md`](../../scopes_and_specs/SCOPE_UNIVERSAL_FREEFALL_v1.md) step 11. Strong EP / two comparable masses is **not** this lock.
**Does not move:** FTD-0250, FTD-0349, FTD-0402, FTD-0208, FTD-0131, FTD-0361, FTD-1013–1020, U-8. No golden tick. No production default ON. No GPU campaign. No P6C-G. No graviton. No \(g_{rr}\). No \(n(\mathcal{L})\). No CODATA retune of \(G_N\). No movement. No two-mass Nordtvedt.

> LOCK-STD v1. Sections §1–§11 are frozen before any live-arm acceleration is observed. Post-hoc edits to §1–§11 void v1.

---

## §1 — The question (LOCKED)

**Q-LIVE-SRC-NEWTON-v1.** On the FTD-1017 fixture (heavy locked source, light locked probe, extra forces off), after Step S has written a source-only well: if Poisson is **re-solved with the probe present**, does FTD-1016 still reproduce

\[
g=C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}
\]

from the *live* `voxel.latency` at the probe, and is the live kick a small perturbation of the freeze kick?

Three arms, one fixture:

| Arm | Poisson occupancy | Kick |
|---|---|---|
| **Z** (freeze) | source only (FTD-1017 Step S). Probe injected after; `latency_field=false`. | one `phase_forces` with `geometric_gravity` |
| **L** (live split) | source + probe. Public `solve_latency_poisson()` after probe inject. Then `latency_field=false`. | one `phase_forces` with `geometric_gravity` |
| **S** (self) | probe only. No source cube. Public `solve_latency_poisson()`. | one `phase_forces` with `geometric_gravity` |
| **T** (production tick) | source + probe. One `tick()` with `latency_field` and `geometric_gravity` both on (Poisson then forces, production order). | that tick |

\(g\) is always the tier-2 Q0 stencil on the \(\mathcal{L}\) that the kick actually reads. \(a=v_x/\mathrm{dt}\). \(r=a_x/g_x\).

**Not asked:** two comparable masses; unlocked motion; \(1/r^2\); CODATA \(G\); lensing; GWs; making the toggle a production default; GNC / FTD-0349.

**Prior-favoured.** A1 (live operator reads live \(\mathcal{L}\)) FOUND — the operator is local in \(\mathcal{L}\). A2/A3 (test-body freeze ≈ live; self-force small) unknown at 1/125. Priors are not measurements.

---

## §2 — Fixture (LOCKED)

| Item | Frozen value |
|---|---|
| Backend | CPU (`force_cpu()`). GPU off. |
| Lattice | \(L=32\), periodic. |
| Source | Locked \(+1\) cube, edge 5, \(N_s=125\), \(J=0\). Origin \((6,13,13)\), COM \(=(8,15,15)\). Absent on arm S. |
| Probe | Locked \(+1\) voxel, \(N_p=1\), \(J=0\), at \((18,15,15)\). \(\Delta x=+10\). |
| Step S | Identical to FTD-1017: `disable_all`, `gravity=true`, `latency_field=true`, forces/movement/`geometric_gravity` off. One `tick()`. Arm S skips the cube and runs this tick with the probe already placed. |
| Arm Z kick | After probe inject: `forces=true`, `gravity=true`, `cluster_inertia=true`, `geometric_gravity=true`, `latency_field=false`. One public `phase_forces` split. No `tick()`. |
| Arm L kick | After probe inject: `solve_latency_poisson()`, then the same kick toggles as Z. |
| Arm T | After probe inject: `forces=true`, `gravity=true`, `cluster_inertia=true`, `geometric_gravity=true`, `latency_field=true`, movement/EM off. One `tick()`. |
| Arm S | Probe only; `solve_latency_poisson()`; Z-style kick. |
| Stencil | \(g=C^2\,\mathcal{L}\,\nabla\mathcal{L}\), `GRAD_TIER2_SCALE` on \(\mathcal{L}(\pm 2)\). |

---

## §3 — Measurands (LOCKED)

\[
r_Z=\frac{a_{Z,x}}{g_{Z,x}},\quad
r_L=\frac{a_{L,x}}{g_{L,x}},\quad
r_T=\frac{a_{T,x}}{g_{T,x}},\quad
\delta_a=\frac{\lvert a_{L,x}-a_{Z,x}\rvert}{\lvert a_{Z,x}\rvert},\quad
\delta_g=\frac{\lvert g_{L,x}-g_{Z,x}\rvert}{\lvert g_{Z,x}\rvert},\quad
\rho_S=\frac{\lvert a_{S,x}\rvert}{\lvert a_{Z,x}\rvert}.
\]

Also record \(\mathcal{L}\) at the probe on Z and L, and \(\delta_T=\lvert a_{T,x}-a_{L,x}\rvert/\lvert a_{L,x}\rvert\). No scan over \(N_s\), separation, or \(G_N\). No CODATA target.

---

## §4 — Executable protocol (LOCKED)

Instrument: `engine/tests/test_live_sourced_newton.cpp` (CTest name `live_sourced_newton`).

**Protocol gates:**

| ID | Claim | Pass if |
|---|---|---|
| P1 | Freeze well non-vacuous (1017 inherit) | \(\lvert g_{Z,x}\rvert>10^{-8}\) |
| P2 | Freeze well toward the source | \(g_{Z,x}\cdot\Delta x<0\) |
| P3 | Probe is not a source member | probe \(\notin\) the 125-cube; \(x\in[4,27]\) |
| P4 | Extra forces off on Z and L | max of coulomb/strong/magnetic/exchange \(<10^{-12}\) on the probe |
| P5 | Z gravity diagnostic nonzero | \(\lvert f_{\rm gravity,Z}\rvert>0\) |
| P6 | Freeze replica of 1017 | \(\lvert r_Z-1\rvert<0.05\) |
| P7 | Live well non-vacuous | \(\lvert g_{L,x}\rvert>10^{-8}\) |
| P8 | Live well toward the source | \(g_{L,x}\cdot\Delta x<0\) |
| P9 | Self-force does not dominate | \(\rho_S<0.20\) |
| P10 | Split vs production tick concordance | \(\delta_T<0.05\) or \(\lvert a_{L,x}\rvert<10^{-18}\) |

**Physics / classification (exactly one physics class if protocol passes):**

| ID | Class | Pass if |
|---|---|---|
| A1 | Live operator reads live \(\mathcal{L}\) | \(\lvert r_L-1\rvert<0.05\) |
| A2 | Test-body: kick freeze ≈ live | \(\delta_a<0.05\) |
| A3 | Test-body: well freeze ≈ live | \(\delta_g<0.05\) |

CTest **passes** if protocol holds and A1 holds (live wiring). A2/A3 classify the test-body approximation; they do **not** fail CTest. If protocol holds and A1 fails, CTest fails (CLOSED-NEGATIVE on live wiring).

---

## §5 — Outcome map (LOCKED)

**IMPROPER** (precedes): injecting \(F=mg\); retuning origin/\(N_s\)/separation after seeing \(\delta_a\); widening ε; colour/EM on; movement on; two equal masses; GPU; CODATA; inserting \(n(\mathcal{L})\).

**FOUND — live wiring + test-body.** P1–P10, A1, A2, A3. Tag: `[MEASURED — LIVE SOURCED NEWTON, TEST-BODY]`. Freeze was a good approximation at 1/125.

**FOUND — live wiring, well responds.** P1–P10, A1, and (A2 or A3 fails). Tag: `[MEASURED — LIVE SOURCED NEWTON, PROBE SOURCES]`. Operator tracks live \(\mathcal{L}\); the photograph is not the field. Not Nordtvedt.

**CLOSED-NEGATIVE.** P1–P10, A1 fails. Operator does not match live \(\mathcal{L}\). CTest fails.

**UNDERDETERMINED.** Any of P1–P10 fails. CTest fails on that gate. P9 fail means this mass ratio is not a test-body experiment.

---

## §6 — Tie-breaks (LOCKED)

- \(x_{\rm COM}\) is the mean integer \(x\) of the 125 source sites (unused on arm S).
- \(a_x=v_x/\mathrm{dt}\) after the kick/tick. \(\gamma_{\rm FTD}\) is not divided out.
- Equality at a numeric gate is a pass for that gate.
- Arm T reads \(g_T\) from \(\mathcal{L}\) after the tick (forces do not rewrite latency).
- Arm S \(a_{S,x}\) may be near zero; \(\rho_S\) uses \(\lvert a_Z\rvert\) in the denominator.

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can fail? | Witness |
|---|---|---|
| P6 | Yes | FTD-1017 regression |
| P9 | Yes | Single-voxel self-force comparable to the source kick |
| P10 | Yes | Tick side-effects (proper time, leftover toggles) vs split |
| A1 | Yes | Operator reads a stale buffer, or live Poisson NaNs |
| A2/A3 | Yes | Probe occupancy at 1/125 already rewrites the well |

---

## §8 — Banned moves (LOCKED)

- Golden-tick change; production default ON; GPU in this lock.
- Promote FTD-1013 / 1017; claim physical \(G_N\); claim \(1/r^2\); claim strong EP.
- Coincidence scan; CODATA; graviton; \(g_{rr}\); \(n(\mathcal{L})\).
- Edit this prereg after observing live \(a\).

---

## §9 — Quantifier coverage (LOCKED)

This source, this probe, this \(L=32\) periodic box, this operator, one re-solve. Not \(\forall N_s\), not motion, not two comparable masses, not GPU.

---

## §10 — Window (LOCKED)

2026-08-19 America/Chicago through 23:59, this session, CPU observer. Past window with no verdict books F10. Git tag pending; result cites §12 SHA as `anchored-late`.

---

## §11 — Reconciliation (LOCKED)

FTD-1021 is a new row. FTD-1017 remains the frozen sourced FOUND. This lock only unfreezes Poisson occupancy. FTD-1014 remains CLOSED-NEGATIVE for default \(\nabla|J|\). FTD-0349 / strong EP remain unmoved. Production default remains \(\nabla|J|\).

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive, including the trailing newline after that line.

**Content SHA256 of hashed prefix:** `9D76CCBB63C05BEDE4B07A71DC68CA96A18305888D9071BA093A206B398D7EEF`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19 America/Chicago, CPU observer, CTest `live_sourced_newton` **Passed**. Instrument `engine/tests/test_live_sourced_newton.cpp` SHA256 `CDFA232627E341838C5FA486C2A84167591BF46ABCD8A4B48983C1A7D40FC12E`. Frozen classifier **FOUND — live wiring, well responds**. A1 \(r_L=0.985671\). A2/A3 fail: \(\delta_a=3.707\), \(\delta_g=3.773\). Arm Z matches FTD-1017 (\(r_Z=0.999542\), \(g_Z=-2.917418\times10^{-4}\)). Arm T identical to L (\(\delta_T=0\)). Arm S self-force \(\rho_S=5.503\times10^{-16}\). Live re-solve is a public latency-only `tick()` (`solve_latency_poisson` is private). Result: [`ANALYSIS_LIVE_SOURCED_NEWTON_v1.md`](../../../03_derivations/gravity_and_cosmology/ANALYSIS_LIVE_SOURCED_NEWTON_v1.md). Anchor: **`anchored-late`** until `git rev-parse preregister-live-sourced-newton-v1` succeeds. FTD-1013–1020 / 0131 / 0250 / 0349 / 0402 / 0208 / U-8 unmoved. Production default remains \(\nabla|J|\). Not Nordtvedt.
