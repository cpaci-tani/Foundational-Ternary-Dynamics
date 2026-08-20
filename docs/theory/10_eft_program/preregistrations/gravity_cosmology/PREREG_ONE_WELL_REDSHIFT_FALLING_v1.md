# PRE-REGISTRATION — One-well redshift + falling v1

**Tag:** `[PRE-REGISTRATION]` — locks the CPU observer that asks whether one sourced-then-frozen Poisson \(\mathcal{L}\) drives both rest-clock \(\tau\) (FC-2) and FTD-1016 falling. Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-one-well-redshift-falling-v1` (pending owner commit; until the tag resolves this lock is `anchored-late` via §12 prefix SHA256).
**LEDGER reservation:** FTD-1019.
**Parent:** FTD-1017 sourced well; FTD-1016 operator; FC-2 / FTD-0402 clock law.
**Coverage catalog (not hashed):** [`CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md`](../../../03_derivations/gravity_and_cosmology/CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md) — this lock is the redshift+falling row, not lensing/GWs.
**Does not move:** FTD-0250, FTD-0349, FTD-0402, FTD-0208, FTD-1013, FTD-1014, FTD-1015, FTD-1016, FTD-1017, FTD-1018, U-8, FTD-0131. No golden tick. No production default ON. No GPU campaign. No P6C-G. No graviton. No \(g_{rr}\). No Pound–Rebka ppm target. No CODATA retune of \(G_N\).

> LOCK-STD v1. Sections §1–§11 are frozen before any \(\tau\) ratio or probe acceleration is observed. Post-hoc edits to §1–§11 void v1.

---

## §1 — The question (LOCKED)

**Q-ONE-WELL-RS-FF-v1.** After a CPU `tick()` that runs `latency_field` Poisson on a **heavy locked source alone** (the FTD-1017 Step S well), freeze \(\mathcal{L}\). Do **both** of the following hold on that frozen field?

1. **Clocks.** Two locked rest clocks (\(J=0\), \(v=0\)) at a near site and a far site accumulate \(\tau\) for \(N_\tau=20\) ticks **without re-solving Poisson**, and the measured ratio matches FC-2 rest rates from the frozen \(\mathcal{L}\):

\[
\frac{\tau_{\rm near}}{\tau_{\rm far}}
=\frac{\sqrt{\max(1-\mathcal{L}_{\rm near}^2,0)}}{\sqrt{\max(1-\mathcal{L}_{\rm far}^2,0)}}.
\]

2. **Falling.** A light locked probe at the near site, on a **second** bridge that shares Step S geometry, falls under `geometric_gravity` as in FTD-1017 (\(r_{\rm on}=a_x/g_{{\rm ext},x}\)).

Clock ticks use `de_broglie_clock=true` solely so `accumulate_proper_time()` runs while `latency_field=false`. Clocks have \(J=0\), so the KG term \(-\omega_0^2 J\) is a no-op. The measurand is \(\tau\), not clock phase.

**Not asked:** Pound–Rebka laboratory ppm; live Poisson with clocks as extra mass (EIN-4); strong EP; \(1/r^2\); CODATA \(G\); lensing; GWs; production default ON; GPU.

**Prior-favoured outcome.** FOUND (FC-2 already integrates \(\mathcal{L}\); FTD-1017 already falls in this well). Favoured is not predetermined. A1 or A2 fails CTest if false.

---

## §2 — Fixture (LOCKED)

| Item | Frozen value |
|---|---|
| Backend | CPU (`force_cpu()` before any tick). GPU off. |
| Lattice | \(L=32\), periodic. |
| Source | Locked \(+1\) cube, edge 5, \(N_s=125\), \(J=0\). Origin \((6,13,13)\), COM \(=(8,15,15)\). Identical to FTD-1017. |
| Step S | `disable_all`, then `gravity=true`, `latency_field=true`. `forces=false`, `movement=false`, `geometric_gravity=false`, `field_energy_gravity=false`, `cluster_inertia=false`. One `tick()`. Source alone. |
| Near site | \((18,15,15)\) — FTD-1017 probe coordinate. |
| Far site | \((24,15,15)\) — same ray, larger periodic separation from COM (antipode along \(+x\)). |
| Clocks | After Step S: inject locked \(+1\), \(J=0\), \(v=0\) at near and far. Do **not** re-solve Poisson. |
| Clock ticks | `latency_field=false`, `de_broglie_clock=true`, `forces=false`, `movement=false`, `geometric_gravity=false`. \(N_\tau=20\) `tick()`s. |
| Frozen \(\mathcal{L}\) | Read at the clock voxels **after inject, before clock ticks**. Prediction uses those values. |
| Falling bridge | Independent `RenderBridge`. Same Step S. Then FTD-1017 Step F-on / F-off at the near site (`cluster_inertia=true`, `geometric_gravity` on/off, `latency_field=false`, one public `phase_forces` split). |
| \(g_{\rm ext}\) | FTD-1017 formula at the near site from the frozen well. |

Two bridges, one source law. Re-solving Poisson with clocks or the probe present is IMPROPER.

---

## §3 — Measurand (LOCKED)

Clocks:

\[
\rho_\tau=\frac{(\tau_{\rm near}/\tau_{\rm far})}{(\Gamma_{\rm near}/\Gamma_{\rm far})},\qquad
\Gamma=\sqrt{\max(1-\mathcal{L}^2,0)}\ \text{(rest)}.
\]

Falling:

\[
r=\frac{a_x}{g_{{\rm ext},x}}.
\]

No Pound–Rebka number. No scan over \(N_\tau\), \(N_s\), or separation.

---

## §4 — Executable protocol (LOCKED)

Instrument: `engine/tests/test_one_well_redshift_falling.cpp` (CTest name `one_well_redshift_falling`).

**Protocol gates (must pass before a physics verdict):**

| ID | Claim | Pass if |
|---|---|---|
| P1 | Clock contrast is non-vacuous | \(\Gamma_{\rm near}/\Gamma_{\rm far}<0.999\) and both \(\Gamma>0\) |
| P2 | Near is the deeper well | \(\mathcal{L}_{\rm near}>\mathcal{L}_{\rm far}\) |
| P3 | Clocks are test bodies in bulk | neither site is a source member; both \(x\in[4,27]\) |
| P4 | Well stayed frozen through clock ticks | \(\|\mathcal{L}_{\rm after}-\mathcal{L}_{\rm freeze}\|<10^{-15}\) at both clock sites |
| P5 | Both clocks accumulated \(\tau\) | \(\tau_{\rm near}>0\) and \(\tau_{\rm far}>0\) after 20 ticks |
| P6 | Falling extra forces off on F-on | \(\max\|f_{\rm coulomb}\|,\|f_{\rm strong}\|,\|f_{\rm magnetic}\|,\|f_{\rm exchange}\|<10^{-12}\) on the probe |
| P7 | Falling F-on gravity diagnostic nonzero | \(\|f_{\rm gravity}\|>0\) |
| P8 | Default gravity unread of \(\mathcal{L}\) | F-off: \(\|r_{\rm off}\|<0.05\) |
| P9 | Falling well non-vacuous and toward the source | \(\|g_{\rm ext}\|>10^{-8}\) and \(g_{{\rm ext},x}\cdot(x_{\rm near}-x_{\rm COM})<0\) |

**Physics gates (either fails CTest if false):**

| ID | Claim | FOUND if |
|---|---|---|
| A1 | Rest clocks track FC-2 on the frozen well | \(\|\rho_\tau-1\|<0.05\) |
| A2 | FTD-1016 falls in the same well law | \(\|r_{\rm on}-1\|<0.05\) |

FOUND requires A1 **and** A2.

---

## §5 — Outcome map (LOCKED)

**IMPROPER** (precedes): leaving `latency_field` on during clock ticks (clocks source); using EIN-4 live Poisson as this test; injecting \(F=mg\) then dividing by \(m\); retuning sites after seeing \(\rho_\tau\) or \(r\); widening ε; enabling colour/EM; using de Broglie **phase** as the redshift measurand; a Pound–Rebka ppm target; GPU run; CODATA retune of \(G_N\).

**FOUND.** Not IMPROPER. P1–P9 pass. A1 and A2 pass. Tag: one sourced Poisson \(\mathcal{L}\) drives both FC-2 rest clocks and FTD-1016 falling `[MEASURED — one-well clocks+falling]`. Still does **not** derive \(m_i=m_g\), physical \(G_N\), or Pound–Rebka.

**CLOSED-NEGATIVE.** Not IMPROPER. P1–P9 pass. A1 or A2 fails. CTest fails.

**UNDERDETERMINED.** Not IMPROPER. Any of P1–P9 fails. CTest fails on the failed protocol gate.

Partition: IMPROPER first; then if protocol fails → UNDERDETERMINED; else (A1 and A2) → FOUND else CLOSED-NEGATIVE. One column only.

---

## §6 — Tie-breaks (LOCKED)

- \(\Gamma\) is `proper_time_rate(\(\mathcal{L}\), 0)` (FTD-0402).
- A1/A2 use 0.05. Equality at exactly 0.05 is FOUND. \(\gamma_{\rm FTD}\) is not corrected out of \(r_{\rm on}\).
- \(x_{\rm COM}\) is the mean integer \(x\) of source members.
- \(a_x\) from the probe’s \(v_x/\mathrm{dt}\).
- Clock \(\tau\) is `voxel.tau` after 20 ticks (initial \(\tau=0\) at inject).

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can fail? | Witness |
|---|---|---|
| P1 | Yes | Periodic Poisson too flat between \(x=18\) and \(x=24\) |
| P2 | Yes | Sign error in \(\mathcal{L}=\sqrt{\mathrm{clamp}(-\phi)}\) |
| P4 | Yes | Accidental Poisson overwrite, or inject/tick zeros \(\mathcal{L}\) |
| P5 | Yes | `accumulate_proper_time` not reached (`de_broglie_clock` gating) |
| P8 | Yes | Default \(\nabla\rho\) rewritten to read \(\mathcal{L}\) |
| A1 | Yes | \(\tau\) ignores frozen \(\mathcal{L}\), or clocks source a new well |
| A2 | Yes | Operator unread of Poisson-written \(\mathcal{L}\) (regression of FTD-1017) |

---

## §8 — Banned moves (LOCKED)

- Golden-tick change; production default ON; GPU port in this lock.
- Promote FTD-1013; claim GR redshift; claim Newton’s \(G\).
- Coincidence scan; CODATA; graviton; TEGR; \(g_{rr}\); Pound–Rebka fit.
- Edit this prereg after observing \(\tau\) or \(a\).

---

## §9 — Quantifier coverage (LOCKED)

A1+A2 are this source, these two clock sites, this probe site, this frozen well, \(N_\tau=20\). Not \(\forall\) separations, not GPS, not GPU, not live two-body Poisson.

---

## §10 — Window (LOCKED)

2026-08-19 America/Chicago through 23:59, this session, CPU observer. Past window with no verdict books F10. Git tag pending; result cites §12 SHA as `anchored-late`.

---

## §11 — Reconciliation (LOCKED)

FTD-1019 is a new row. FTD-1017 remains the sourced-falling result; this lock adds rest clocks on the same well law without promoting redshift from ALREADY to a new operator. FTD-1016 remains the operator. FTD-1014 remains CLOSED-NEGATIVE for default \(\nabla|J|\) (P8 re-asserts). FC-2 / FTD-0402 remain the clock law. Production default remains \(\nabla|J|\).

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive, including the trailing newline after that line.

**Content SHA256 of hashed prefix:** `EA5041998D1279DE62FF48D5EC5577451A2949B3DD1256D51AC531B4F1A6B4F6`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19 America/Chicago, CPU observer, CTest `one_well_redshift_falling` **Failed** (protocol). Frozen classifier **UNDERDETERMINED**. P1 failed: \(\Gamma_{\rm near}/\Gamma_{\rm far}=0.9995417\not<0.999\). P2–P9 Passed. A1 \(\rho_\tau=1\) and A2 \(r_{\rm on}=0.999542\) would have been FOUND had P1 passed; they are **not** a v1 verdict. \(L_{\rm near}=3.027058\times10^{-2}\), \(L_{\rm far}=0\), \(\tau_n=19.99083\), \(\tau_f=20\). Constructor logs “GPU backend active” then `force_cpu()`; not a GPU campaign. Repair: v1.1 changes only P1 to \(\Gamma_{\rm near}/\Gamma_{\rm far}<0.9999\); sites, \(N_\tau\), A1/A2 untouched. Anchor: **`anchored-late`**.
