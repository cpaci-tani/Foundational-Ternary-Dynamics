# PRE-REGISTRATION — Slow-envelope live Newton v1

**Tag:** `[PRE-REGISTRATION]` — locks the CPU observer that repeats FTD-1021 with a **3³ locked probe** (\(N_p=27\)) and asks whether the freeze≈live test-body limit returns when the occupant is spatially extended. Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-slow-envelope-live-newton-v1` (pending owner commit; until the tag resolves this lock is `anchored-late` via §12 prefix SHA256).
**LEDGER reservation:** FTD-1022.
**Parent:** FTD-1021 (one-voxel live wiring, freeze not test-body); operator = FTD-1016; source fixture = FTD-1017. Two comparable masses / Nordtvedt is **not** this lock.
**Does not move:** FTD-0250, FTD-0349, FTD-0402, FTD-0131, FTD-0361, FTD-1013–1021, U-8. No golden tick. No production default ON. No GPU. No P6C-G. No graviton. No \(g_{rr}\). No \(n(\mathcal{L})\). No CODATA. No movement. No \(N_p=N_s\).

> LOCK-STD v1. Sections §1–§11 are frozen before any live-arm acceleration is observed. Post-hoc edits to §1–§11 void v1.

---

## §1 — The question (LOCKED)

**Q-SLOW-ENV-NEWTON-v1.** FTD-1021 found that a one-voxel occupant’s own \(\mathcal{L}\)-depth multiplies the external tilt (\(\delta_a=3.71\)), with self-**force** null. If the probe is instead a locked 3³ cube (\(N_p=27\ll N_s=125\)) whose COM is the 1021 site, does FTD-1016 still match live Q0, and does freeze≈live return?

Same four arms as FTD-1021 (Z freeze / L live split / T production tick / S self), same source, public latency-only `tick()` for the L re-solve (`solve_latency_poisson` is private).

\(a\) is the rigid cluster \(V_{\rm COM}/{\rm dt}\) (`cluster_inertia` ON). \(g\) is the tier-2 Q0 stencil at the COM voxel. \(r=a_x/g_x\).

**Not asked:** \(N_p=N_s\); unlocked motion; GNC; \(1/r^2\); CODATA \(G\); lensing; GWs.

**Prior-favoured.** A1 FOUND (operator still local in \(\mathcal{L}\)). A2/A3 unknown: a 3³ is broader than a point but COM \(\pm 2\) still sits outside the cube, so the stencil may still see a self-peak. Priors are not measurements.

---

## §2 — Fixture (LOCKED)

| Item | Frozen value |
|---|---|
| Backend | CPU (`force_cpu()`). GPU off. |
| Lattice | \(L=32\), periodic. |
| Source | Locked \(+1\) cube, edge 5, \(N_s=125\), \(J=0\). Origin \((6,13,13)\), COM \(=(8,15,15)\). Absent on arm S. |
| Probe | Locked \(+1\) cube, edge 3, \(N_p=27\), \(J=0\). Origin \((17,14,14)\), COM voxel \((18,15,15)\). No overlap with the source. |
| Step S / kicks | Identical toggle recipe to FTD-1021 (latency-only tick to re-solve; Z-style `phase_forces` with `geometric_gravity` and `cluster_inertia`; arm T one production tick with both on). |
| \(g\) | Q0 at the COM voxel \((18,15,15)\), `GRAD_TIER2_SCALE` on \(\mathcal{L}(\pm 2)\). |
| \(a\) | \(v_x/\mathrm{dt}\) of any probe member after the rigid write-back. |

---

## §3 — Measurands (LOCKED)

Same symbols as FTD-1021: \(r_Z,r_L,r_T,\delta_a,\delta_g,\rho_S,\delta_T\), plus \(\mathcal{L}\) at the COM on Z and L. No scan over edge, origin, or \(G_N\).

---

## §4 — Executable protocol (LOCKED)

Instrument: `engine/tests/test_slow_envelope_live_newton.cpp` (CTest name `slow_envelope_live_newton`).

**Protocol gates:**

| ID | Claim | Pass if |
|---|---|---|
| P1 | Freeze well non-vacuous | \(\lvert g_{Z,x}\rvert>10^{-8}\) |
| P2 | Freeze well toward the source | \(g_{Z,x}\cdot\Delta x<0\) with \(\Delta x=x_{\rm COM}-x_{\rm source\,COM}\) |
| P3 | Probe disjoint from source; COM in bulk | no shared sites; COM \(x\in[4,27]\) |
| P4 | Extra forces off on Z and L | max coulomb/strong/magnetic/exchange \(<10^{-12}\) at COM |
| P5 | Rigid cluster | all 27 members share \(v_x\) to \(10^{-12}\) |
| P6 | Freeze operator on the clump | \(\lvert r_Z-1\rvert<0.05\) |
| P7 | Live well non-vacuous | \(\lvert g_{L,x}\rvert>10^{-8}\) |
| P8 | Live well toward the source | \(g_{L,x}\cdot\Delta x<0\) |
| P9 | Self-force does not dominate | \(\rho_S<0.20\) |
| P10 | Split vs tick concordance | \(\delta_T<0.05\) or \(\lvert a_L\rvert<10^{-18}\) |

**Physics / classification:**

| ID | Class | Pass if |
|---|---|---|
| A1 | Live operator reads live \(\mathcal{L}\) | \(\lvert r_L-1\rvert<0.05\) |
| A2 | Test-body: kick freeze ≈ live | \(\delta_a<0.05\) |
| A3 | Test-body: well freeze ≈ live | \(\delta_g<0.05\) |

CTest **passes** if protocol holds and A1 holds. A2/A3 classify; they do not fail CTest. A1 fail with protocol pass → CTest fails (CLOSED-NEGATIVE).

---

## §5 — Outcome map (LOCKED)

**IMPROPER** (precedes): injecting \(F=mg\); retuning edge/origin after seeing \(\delta_a\); \(N_p=N_s\); movement on; GPU; CODATA; \(n(\mathcal{L})\).

**FOUND — test-body recovered.** P1–P10, A1, A2, A3. Tag: `[MEASURED — SLOW-ENVELOPE TEST-BODY]`. The 1021 jump was a one-voxel self-peak artefact.

**FOUND — live wiring, envelope still responds.** P1–P10, A1, and (A2 or A3 fails). Tag: `[MEASURED — SLOW-ENVELOPE STILL SOURCES]`. Extending to 3³ did not restore freeze. Not Nordtvedt.

**CLOSED-NEGATIVE.** P1–P10, A1 fails. CTest fails.

**UNDERDETERMINED.** Any of P1–P10 fails. CTest fails on that gate.

---

## §6 — Tie-breaks (LOCKED)

- Source \(x_{\rm COM}\) is the mean integer \(x\) of the 125 source sites.
- Probe \(x_{\rm COM}=18\) (locked). \(a_x\) from member 0 after rigid write-back.
- Equality at a numeric gate is a pass. \(\gamma_{\rm FTD}\) is not divided out.
- Arm T reads \(g_T\) after the tick.

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can fail? | Witness |
|---|---|---|
| P5 | Yes | cluster_inertia failed to rigidify |
| P6 | Yes | mean-\(L\nabla L\) over 27 sites ≠ Q0 at COM on the freeze |
| P9 | Yes | 3³ self-force not small |
| A1 | Yes | operator unread of live clump well |
| A2/A3 | Yes | 3³ still peaked on the \(\pm 2\) stencil |

---

## §8 — Banned moves (LOCKED)

- Golden-tick change; production default ON; GPU; \(N_p=125\).
- Promote FTD-1021; claim physical \(G_N\); claim Nordtvedt; claim \(1/r^2\).
- Coincidence scan; CODATA; graviton; \(n(\mathcal{L})\).
- Edit this prereg after observing live \(a\).

---

## §9 — Quantifier coverage (LOCKED)

This 3³, this source, this \(L=32\) box, one re-solve. Not \(\forall\) edge, not motion, not two equal masses.

---

## §10 — Window (LOCKED)

2026-08-19 America/Chicago through 23:59, this session, CPU observer. Past window with no verdict books F10. Git tag pending; result cites §12 SHA as `anchored-late`.

---

## §11 — Reconciliation (LOCKED)

FTD-1022 is a new row. FTD-1021 remains the one-voxel live result. FTD-1017 remains the freeze FOUND for \(N_p=1\). This lock only changes probe extent. Strong EP unmoved. Production default remains \(\nabla|J|\).

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive, including the trailing newline after that line.

**Content SHA256 of hashed prefix:** `4C82033F93AC8A182E4B7AE7538BD6F3065428823C850853D4C03CC4AEDD9D25`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19 America/Chicago, CPU observer, CTest `slow_envelope_live_newton` **Failed** on P6. Instrument at v1 execution: `engine/tests/test_slow_envelope_live_newton.cpp` (then COM-sampled \(g\)). Frozen classifier **UNDERDETERMINED**. \(r_Z=0.887356\) vs P6 \(\lvert r_Z-1\rvert<0.05\). \(g_Z=-2.917418\times10^{-4}\) (same COM field as FTD-1021 freeze); \(a_Z=-2.588788\times10^{-4}\) (cluster mean of local \(L\nabla L\)). A1/A2/A3 are not a v1 verdict. Constructor logs “GPU backend active” then `force_cpu()`; not IMPROPER. v1.1 repairs only the \(g\) sampling (member mean, matching \(a_{\rm COM}\)). Anchor: **`anchored-late`** until `git rev-parse preregister-slow-envelope-live-newton-v1` succeeds.
