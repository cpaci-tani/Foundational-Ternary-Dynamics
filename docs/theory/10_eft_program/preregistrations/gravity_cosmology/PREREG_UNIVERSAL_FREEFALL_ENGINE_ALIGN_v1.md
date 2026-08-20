# PRE-REGISTRATION — Universal free-fall engine alignment v1

**Tag:** `[PRE-REGISTRATION]` — locks the dual-path CPU comparison of Q0 geometry vs live `F/M`. Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-universal-freefall-engine-align-v1` (pending owner commit; until the tag resolves this lock is `anchored-late` via §12 prefix SHA256).
**LEDGER reservation:** FTD-1014.
**Parent:** [`SCOPE_UNIVERSAL_FREEFALL_v1.md`](../../scopes_and_specs/SCOPE_UNIVERSAL_FREEFALL_v1.md) step 5; desk Q0 = FTD-1013.
**Does not move:** FTD-0250, FTD-0349, FTD-0402, FTD-0208, FTD-1013, U-8. No golden tick. No production gravity rewrite. No GPU campaign.

> LOCK-STD v1. Sections §1–§11 are frozen before any Path-F acceleration is observed. Post-hoc edits to §1–§11 void v1.

---

## §1 — The question (LOCKED)

**Q-UFF-ALIGN-v1.** On a locked external-well fixture, extra force channels off, several test-body sizes \(N\), does the **live** `phase_forces` gravity update reproduce Q0’s weak geometric acceleration

\[
g_{\rm ext}=C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}
\]

at the cluster COM, as measured by \(a_{\rm COM}(N)/g_{\rm ext}\)?

Two paths share the same initial state and well:

- **Path G (geometric, test-only).** Per-voxel kick \(\Delta\mathbf v=g_{\rm ext}\Delta t\) from rest. No mass, no `F/M`, not a production toggle.
- **Path F (live).** One call to `RenderBridge::phase_forces()` with the production gravity operator `F = G_N \nabla\rho`, \(\rho=|J|\) (`Voxel::density()`), then the live \(\gamma_{\rm FTD}\) / cluster-inertia integrator as configured.

**Not asked:** adopting Path G into production; replacing \(\nabla\rho\) with an \(\mathcal{L}\)-gradient force; strong EP; GNC; self-force as a weak-EP falsifier; composition; graviton; CI-5 injected-force EP.

**Prior-favoured outcome.** CLOSED-NEGATIVE (live gravity is \(\nabla|J|\), Q0 is \(\mathcal{L}\nabla\mathcal{L}\)). Favoured is not predetermined.

---

## §2 — Fixture (LOCKED)

| Item | Frozen value |
|---|---|
| Backend | CPU (`force_cpu()`). GPU off. |
| Lattice | \(L=32\), periodic. |
| Well | \(\mathcal{L}(x,y,z)=\mathcal{L}_0+k x\) with \(\mathcal{L}_0=0.05\), \(k=10^{-3}\), \(x\) the integer lattice coordinate in \(\{0,\ldots,31\}\). Written onto **every** voxel. Poisson is **not** run. |
| \(g_{\rm ext}\) | Analytic: \(g_{\rm ext}=C_{\rm SPEED}^2\,(\mathcal{L}_0+k\,x_{\rm COM})\,k\) in \(+\hat x\). No periodic finite difference (the linear well is discontinuous at the wrap). |
| Test bodies | Locked \(+1\) cubes, flux \(J=0\), colour/spin 0, rest. \(N\in\{1,8,27\}\) as edge \(1,2,3\). Origin \(x=y=z=14\) (bulk; away from the wrap). Separate bridge per \(N\). |
| Toggles Path F | `disable_all`, then `forces=true`, `gravity=true`, `cluster_inertia=true`. `latency_field=false` (well is prescribed). EM modes: default-off `poisson_coulomb`, `emergent_forces`, `lorentz_force`. Colour/Yukawa/exchange off. |
| Integration | One `phase_forces()` call. No `tick()`, no movement. \(a_{\rm COM}=v_x/\mathrm{dt}\) from any member (rigid). |
| Path G | Same well and cubes. Do **not** call `phase_forces`. Write \(v_x=g_{\rm ext}\,\mathrm{dt}\) on every member. |

External: the well is imposed, not sourced by the test body. Self-Poisson is out of scope by construction.

---

## §3 — Measurand (LOCKED)

\[
r(N)=\frac{a_{\rm COM}(N)}{g_{\rm ext}(N)}.
\]

Single registered number per path per \(N\). No scan over couplings. No CODATA target.

---

## §4 — Executable protocol (LOCKED)

Instrument: `engine/tests/test_universal_freefall_engine_align.cpp` (CTest name `universal_freefall_engine_align`). Recomputes \(g_{\rm ext}\) from \(\mathcal{L}_0,k,x_{\rm COM},C_{\rm SPEED}\); does not bookkeep a hard-coded acceleration.

**Protocol gates (must pass before a physics verdict):**

| ID | Claim | Pass if |
|---|---|---|
| P1 | Well non-vacuous | \(\|g_{\rm ext}\|>10^{-8}\) at every \(N\) |
| P2 | Path G is Q0 | \(\|r_G(N)-1\|<0.02\) at every \(N\) |
| P3 | Path G \(N\)-independent | \(\bigl(\max r_G-\min r_G\bigr)/|\mathrm{mean}\,r_G|<0.02\) |
| P4 | Extra forces off on Path F | \(\max\|f_{\rm coulomb}\|,\|f_{\rm strong}\|,\|f_{\rm magnetic}\|,\|f_{\rm exchange}\|<10^{-12}\) on all members after `phase_forces` |
| P5 | Path F ran | at least one member has `force_diag_.f_gravity` written (may be ~0) |

**Physics gate (does not fail CTest; classifies):**

| ID | Claim | FOUND if |
|---|---|---|
| A1 | Live tick is Q0 | \(\|r_F(N)-1\|<0.05\) at every registered \(N\) |

---

## §5 — Outcome map (LOCKED)

**IMPROPER** (precedes): injecting \(F=m g_{\rm ext}\) then dividing by \(m\); retuning \(k\) or \(\mathcal{L}_0\) after seeing \(r_F\); treating a near-miss \(|r_F-1|\) as FOUND by widening ε; enabling colour/EM; calling the full `tick()` Poisson overwrite; GPU run.

**FOUND.** Not IMPROPER. P1–P5 pass. A1 passes. Tag: live `F/M` **reproduces** Q0 weak EOM on this fixture `[MEASURED — alignment]`. Still does **not** derive \(m_i=m_g\).

**CLOSED-NEGATIVE.** Not IMPROPER. P1–P5 pass. A1 fails. Live gravity is **not** the Q0 action on this fixture `[MEASURED]`. Expected mechanism, if it fires: \(F=G_N\nabla|J|\) does not couple to prescribed \(\mathcal{L}\). No production change. FTD-1013 unmoved.

**UNDERDETERMINED.** Not IMPROPER. Any of P1–P5 fails. No physics verdict.

Partition: IMPROPER first; then if protocol fails → UNDERDETERMINED; else A1 true → FOUND else CLOSED-NEGATIVE. One column only.

---

## §6 — Tie-breaks (LOCKED)

- \(x_{\rm COM}\) is the mean integer \(x\) of members (not remainder).
- \(a_{\rm COM}\) from member 0’s \(v_x/\mathrm{dt}\) after asserting rigid \(v\) (all members equal to 1e-15).
- A1 uses 0.05. Equality at exactly 0.05 is FOUND (closed interval).
- P4 uses max over members of each channel’s Euclidean magnitude.

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can fail? | Witness |
|---|---|---|
| P2 | Yes | Wrong Path-G formula (e.g. \(a=k\) not \(C^2\mathcal{L}k\)) |
| A1 | Yes | \(J=0\) ⇒ \(\nabla\rho=0\) ⇒ \(r_F\approx 0\neq 1\) on this well |
| P1 | Yes | \(k=0\) |

CI-5 (injected uniform \(f\), gravity off) is **excluded** from A1. Using it as FOUND is IMPROPER.

---

## §8 — Banned moves (LOCKED)

- Golden-tick change; production default; new live gravity operator.
- Promote FTD-0250 / 0402 / 1013; claim UFF derived from the engine.
- Coincidence scan; CODATA; graviton; occupancy telegraph.
- Self-force / GNC / colour-on as this weak-EP test.
- Edit this prereg after observing Path F.

---

## §9 — Quantifier coverage (LOCKED)

A1 is \(\forall N\in\{1,8,27\}\) on **this** well and **this** operator pair. It is not \(\forall\) wells, not GPU, not unlocked clumps, not \(\mathcal{L}\) from live Poisson.

---

## §10 — Window (LOCKED)

2026-08-19 America/Chicago through 23:59, this session, CPU observer. Past window with no verdict books F10. Census is not a gate on this desk-adjacent engine observer. Git tag pending; result cites §12 SHA as `anchored-late`.

---

## §11 — Reconciliation (LOCKED)

FTD-1014 is a new row. FTD-1013 remains the desk theorem. CI-5 remains a demonstration of imposed cluster inertia, not this campaign.

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive.

**Content SHA256 of hashed prefix:** `5A99CA4FD0EB3EACA1FE9712688E0D8D118F9756CB53EC86DD860C8FCAAA3E9F`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19 America/Chicago, CPU observer, CTest `universal_freefall_engine_align` **Passed** (protocol gates; A1 does not fail CTest). Instrument `engine/tests/test_universal_freefall_engine_align.cpp` SHA256 `BC8554773E71FF27649E7D04D18B8D023E34DF1EAE87D8F2C3CD99CA86D40E90`. Frozen classifier **CLOSED-NEGATIVE**. Path G: \(r_G=1\) at \(N\in\{1,8,27\}\). Path F: \(r_F=0\), \(a_{\rm COM}=0\), \(\|f_{\rm gravity}\|=0\) at every registered \(N\). Mechanism of record: \(J=0\Rightarrow\nabla|J|=0\); prescribed \(\mathcal{L}\) is unread by \(F=G_N\nabla\rho\). Constructor logs “GPU backend active” then `force_cpu()`; the measured update is the CPU free-function split of `phase_forces`, not a GPU campaign (not IMPROPER). Result: [`ANALYSIS_UNIVERSAL_FREEFALL_ENGINE_ALIGN_v1.md`](../../../03_derivations/gravity_and_cosmology/ANALYSIS_UNIVERSAL_FREEFALL_ENGINE_ALIGN_v1.md). Anchor: **`anchored-late`** until `git rev-parse preregister-universal-freefall-engine-align-v1` succeeds. FTD-1013 / 0250 / 0349 / 0402 / 0208 / U-8 unmoved. No production gravity rewrite.
