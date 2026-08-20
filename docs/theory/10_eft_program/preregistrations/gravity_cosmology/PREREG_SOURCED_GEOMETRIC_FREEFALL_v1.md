# PRE-REGISTRATION — Sourced geometric free-fall v1

**Tag:** `[PRE-REGISTRATION]` — locks the CPU observer that writes \(\mathcal{L}\) from a heavy source via `latency_field` Poisson, freezes that well, and asks whether FTD-1016’s operator falls a light probe in it. Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-sourced-geometric-freefall-v1` (pending owner commit; until the tag resolves this lock is `anchored-late` via §12 prefix SHA256).
**LEDGER reservation:** FTD-1017.
**Parent:** [`SCOPE_UNIVERSAL_FREEFALL_v1.md`](../../scopes_and_specs/SCOPE_UNIVERSAL_FREEFALL_v1.md); operator = FTD-1016; default \(\nabla|J|\) residue = FTD-1014.
**Coverage catalog (not hashed):** [`CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md`](../../../03_derivations/gravity_and_cosmology/CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md) — this lock is the sourced-Newton row, not lensing/GWs.
**Does not move:** FTD-0250, FTD-0349, FTD-0402, FTD-0208, FTD-1013, FTD-1014, FTD-1015, FTD-1016, U-8, FTD-0131. No golden tick. No production default ON. No GPU kernel. No P6C-G. No graviton. No \(g_{rr}\). No CODATA retune of \(G_N\). No live mutual gravity / strong EP.

> LOCK-STD v1. Sections §1–§11 are frozen before any probe acceleration is observed. Post-hoc edits to §1–§11 void v1.

---

## §1 — The question (LOCKED)

**Q-SRC-GEO-FF-v1.** After a CPU `tick()` that runs `latency_field` Poisson on a **heavy locked source alone**, with extra force channels off, does one subsequent public `phase_forces` split with `geometric_gravity=true` on a **light locked probe** in that **frozen** well reproduce Q0’s weak kick

\[
g_{\rm ext}=C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}
\]

built from the *written* `voxel.latency` at the probe (same tier-2 stencil as the operator), as measured by \(a_{\rm probe}/g_{\rm ext}\)?

Two steps, one well:

- **Step S (source).** Source present, probe absent. One `tick()`. Poisson writes \(\mathcal{L}\). Forces off, so nobody falls.
- **Step F-on.** Inject probe. `latency_field=false` (do **not** re-solve; the probe must not source). `geometric_gravity=true`. One `phase_forces` split. No `tick()`, no movement.
- **Step F-off.** Same frozen well and probe; `geometric_gravity=false`. Residue: default \(F=G_N\nabla|J|\) with \(J=0\).

**Not asked:** weak EP vs \(N\) (that is FTD-1016); strong EP / self-Poisson of the probe; \(1/r^2\) Coulomb identification of the periodic Green’s function; CODATA \(G\); lensing; GWs; making the toggle a production default.

**Prior-favoured outcome.** FOUND (FTD-1016 already matches a prescribed well; this asks whether Poisson-written \(\mathcal{L}\) is a well the same operator reads). Favoured is not predetermined. A1 fails CTest if false.

---

## §2 — Fixture (LOCKED)

| Item | Frozen value |
|---|---|
| Backend | CPU (`force_cpu()` before any tick). GPU off. `geometric_gravity` remains CPU-only. |
| Lattice | \(L=32\), periodic. |
| Source | Locked \(+1\) cube, edge 5, \(N_s=125\), flux \(J=0\), colour/spin 0. Origin \((6,13,13)\) so COM \(=(8,15,15)\). |
| Probe | Locked \(+1\) voxel, \(N_p=1\), \(J=0\), at \((18,15,15)\). Separation \(\Delta x=+10\). Not overlapping the source cube. Tier-2 neighbours of the probe are not source members. |
| Step S toggles | `disable_all`, then `gravity=true`, `latency_field=true`. `forces=false`, `movement=false`, `geometric_gravity=false`, `field_energy_gravity=false`, `cluster_inertia=false`. EM/colour/Yukawa/exchange off. One `tick()`. |
| Step F-on toggles | After the probe exists: `forces=true`, `gravity=true`, `cluster_inertia=true`, `geometric_gravity=true`, `latency_field=false`. EM modes off. |
| Step F-off | Identical to F-on except `geometric_gravity=false`. |
| \(g_{\rm ext}\) | From the **frozen** well at the probe: \(g_{\rm ext}=C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}\) with \(\nabla\mathcal{L}\) the production tier-2 FD (`GRAD_TIER2_SCALE=1/4`). Not an analytic \(1/r^2\). Not a periodic FD of a linear ramp. |
| Integration | One public `phase_forces` split. \(a=v_x/\mathrm{dt}\). |

The well is sourced by the heavy body, then frozen. Re-solving Poisson with the probe present is IMPROPER for this lock (that is self-force / strong EP, out of v1).

---

## §3 — Measurand (LOCKED)

\[
r=\frac{a_x}{g_{{\rm ext},x}}.
\]

Single registered number per path. No scan over \(G_N\), \(N_s\), or separation. No CODATA target.

---

## §4 — Executable protocol (LOCKED)

Instrument: `engine/tests/test_sourced_geometric_freefall.cpp` (CTest name `sourced_geometric_freefall`). Recomputes \(g_{\rm ext}\) from the written \(\mathcal{L}\) field; does not bookkeep a hard-coded acceleration.

**Protocol gates (must pass before a physics verdict):**

| ID | Claim | Pass if |
|---|---|---|
| P1 | Well non-vacuous at the probe | \(\|g_{\rm ext}\|>10^{-8}\) |
| P2 | Well points toward the source | \(g_{{\rm ext},x}\cdot(x_{\rm probe}-x_{\rm COM})<0\) |
| P3 | Probe is a test body in bulk | probe not a source member; probe \(x\in[4,27]\) |
| P4 | Extra forces off on F-on | \(\max\|f_{\rm coulomb}\|,\|f_{\rm strong}\|,\|f_{\rm magnetic}\|,\|f_{\rm exchange}\|<10^{-12}\) on the probe |
| P5 | F-on gravity diagnostic written and nonzero | \(\|f_{\rm gravity}\|>0\) |
| P6 | Default gravity unread of \(\mathcal{L}\) | F-off: \(\|r_{\rm off}\|<0.05\) |

**Physics gate (fails CTest if false):**

| ID | Claim | FOUND if |
|---|---|---|
| A1 | Toggle-ON live tick is Q0 on the sourced well | \(\|r_{\rm on}-1\|<0.05\) |

---

## §5 — Outcome map (LOCKED)

**IMPROPER** (precedes): injecting \(F=m g_{\rm ext}\) then dividing by \(m\); retuning origin, \(N_s\), or separation after seeing \(r\); widening ε; enabling colour/EM; calling `tick()` on Step F (Poisson overwrite / movement); leaving `latency_field` on for Step F; GPU run; treating a \(1/r^2\) residual as this A1; CODATA retune of \(G_N\).

**FOUND.** Not IMPROPER. P1–P6 pass. A1 passes. Tag: Poisson-written \(\mathcal{L}\) is a well the FTD-1016 operator reads `[MEASURED — sourced wiring]`. Still does **not** derive \(m_i=m_g\), physical \(G_N\), or \(1/r^2\) as a new theorem.

**CLOSED-NEGATIVE.** Not IMPROPER. P1–P6 pass. A1 fails. CTest fails.

**UNDERDETERMINED.** Not IMPROPER. Any of P1–P6 fails. CTest fails on the failed protocol gate.

Partition: IMPROPER first; then if protocol fails → UNDERDETERMINED; else A1 true → FOUND else CLOSED-NEGATIVE. One column only.

---

## §6 — Tie-breaks (LOCKED)

- \(x_{\rm COM}\) is the mean integer \(x\) of source members.
- \(a_x\) from the probe’s \(v_x/\mathrm{dt}\).
- A1 uses 0.05. Equality at exactly 0.05 is FOUND. \(\gamma_{\rm FTD}\) is not corrected out.
- P4 uses Euclidean magnitude on the probe’s `force_diag_`.
- Source cube occupancy is the 125 sites of the edge-5 cube; the probe coordinate is frozen in §2.

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can fail? | Witness |
|---|---|---|
| P1 | Yes | Mean-subtracted Poisson well too shallow at \(\Delta x=10\) |
| P2 | Yes | Sign error in \(\mathcal{L}=\sqrt{\mathrm{clamp}(-\phi)}\) vs \(\nabla\mathcal{L}\) |
| P6 | Yes | Accidental rewrite of default \(\nabla\rho\) to read \(\mathcal{L}\) |
| A1 | Yes | Operator ignores Poisson-written \(\mathcal{L}\), or stencil/wrap artefact |

---

## §8 — Banned moves (LOCKED)

- Golden-tick change; production default ON; GPU port in this lock.
- Promote FTD-1013 / 1016; claim UFF derived from sourced gravity; claim Newton’s \(G\).
- Coincidence scan; CODATA; graviton; occupancy telegraph; TEGR; \(g_{rr}\).
- Self-force / GNC / colour-on as this test.
- Edit this prereg after observing the probe acceleration.

---

## §9 — Quantifier coverage (LOCKED)

A1 is this source, this probe, this frozen well, this operator. It is not \(\forall\) separations, not GPU, not unlocked clumps, not live two-body Poisson, not \(1/r^2\).

---

## §10 — Window (LOCKED)

2026-08-19 America/Chicago through 23:59, this session, CPU observer. Past window with no verdict books F10. Git tag pending; result cites §12 SHA as `anchored-late`.

---

## §11 — Reconciliation (LOCKED)

FTD-1017 is a new row. FTD-1016 remains the prescribed-well operator result. FTD-1014 remains CLOSED-NEGATIVE for default \(\nabla|J|\) (P6 re-asserts). FTD-0131 remains: engine \(G_N=0.01\) is the toy lattice coupling. This is sourced wiring, not a derivation of Newton’s constant.

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive, including the trailing newline after that line.

**Content SHA256 of hashed prefix:** `A428956329AFC7DAD006178368FDA19ABE337754F20FD7372EECA376CE240D39`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19 America/Chicago, CPU observer, CTest `sourced_geometric_freefall` **Passed**. Instrument `engine/tests/test_sourced_geometric_freefall.cpp` SHA256 `E41E627A2BF9B011E5D56ED31F3D8732B70B8A78E96340951026506982C104A6`. Frozen classifier **FOUND**. \(g_{{\rm ext},x}=-2.917418\times 10^{-4}\) (toward the source, \(\Delta x=+10\)). Path F-on \(r_{\rm on}=0.999542\). Path F-off \(r_{\rm off}=0\). Constructor logs “GPU backend active” then `force_cpu()`; not a GPU campaign (not IMPROPER). Result: [`ANALYSIS_SOURCED_GEOMETRIC_FREEFALL_v1.md`](../../../03_derivations/gravity_and_cosmology/ANALYSIS_SOURCED_GEOMETRIC_FREEFALL_v1.md). Anchor: **`anchored-late`** until `git rev-parse preregister-sourced-geometric-freefall-v1` succeeds. FTD-1013 / 1014 / 1015 / 1016 / 0131 / 0250 / 0349 / 0402 / 0208 / U-8 unmoved. Production default remains \(\nabla|J|\). No \(1/r^2\) claim. No live mutual gravity.
