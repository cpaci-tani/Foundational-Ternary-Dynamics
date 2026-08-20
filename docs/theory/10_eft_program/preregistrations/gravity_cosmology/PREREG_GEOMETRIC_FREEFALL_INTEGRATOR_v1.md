# PRE-REGISTRATION — Geometric free-fall integrator v1

**Tag:** `[PRE-REGISTRATION]` — locks the default-off CPU operator that reads prescribed (or Poisson-written) \(\mathcal{L}\) and applies Q0’s weak kick. Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-geometric-freefall-integrator-v1` (pending owner commit; until the tag resolves this lock is `anchored-late` via §12 prefix SHA256).
**LEDGER reservation:** FTD-1016.
**Parent:** [`SCOPE_UNIVERSAL_FREEFALL_v1.md`](../../scopes_and_specs/SCOPE_UNIVERSAL_FREEFALL_v1.md) step 5 remainder; desk Q0 = FTD-1013; live \(\nabla|J|\) alignment = FTD-1014 CLOSED-NEGATIVE.
**Coverage catalog (not hashed):** [`CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md`](../../../03_derivations/gravity_and_cosmology/CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md) — every measured gravity phenomenon at its real tag and whether this campaign puts it in the engine. This lock does **not** claim full GR.
**Does not move:** FTD-0250, FTD-0349, FTD-0402, FTD-0208, FTD-1013, FTD-1014, FTD-1015, U-8. No golden tick. No production default ON. No GPU kernel. No P6C-G. No graviton. No \(g_{rr}\). No CODATA retune of \(G_N\).

> LOCK-STD v1. Sections §1–§11 are frozen before any Path-F-with-toggle acceleration is observed. Post-hoc edits to §1–§11 void v1.

---

## §1 — The question (LOCKED)

**Q-GEO-FF-v1.** On the FTD-1014 prescribed-well fixture, extra force channels off, several test-body sizes \(N\), does a new default-off production operator

\[
\mathbf F = M_{\rm INERTIAL}\,C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}
\]

(read from `voxel.latency`, same tier-2 stencil as live gravity) reproduce Q0’s weak geometric acceleration

\[
g_{\rm ext}=C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}
\]

at the cluster COM, as measured by \(a_{\rm COM}(N)/g_{\rm ext}\)?

Three paths share the same initial state and well:

- **Path G (geometric, test-only).** Per-voxel kick \(\Delta\mathbf v=g_{\rm ext}\Delta t\) from rest. No mass, no `F/M`. Same as FTD-1014 Path G.
- **Path F-on (live, toggle ON).** One call to the public `phase_forces` split with `geometric_gravity=true`. Gravity force is the \(\mathcal{L}\nabla\mathcal{L}\) operator above, not \(G_N\nabla|J|\). Then the live \(\gamma_{\rm FTD}\) / cluster-inertia integrator.
- **Path F-off (live, toggle OFF).** Same as FTD-1014 Path F: production \(F=G_N\nabla\rho\), \(\rho=|J|\). Residue check: prescribed \(\mathcal{L}\) remains unread.

**Not asked:** deriving \(m_i=m_g\); deriving FC-2; adopting P6C-G; GPU parity; self-force / strong EP; lensing / \(g_{rr}\); GWs; making the toggle a production default; retuning \(G_N\) to CODATA.

**Prior-favoured outcome.** FOUND (the operator is the Q0 force that the FTD-1014 residue showed was missing). Favoured is not predetermined: A1 is a CTest assertion this time.

---

## §2 — Fixture (LOCKED)

| Item | Frozen value |
|---|---|
| Backend | CPU (`force_cpu()`). GPU off. Toggle backends bit is **CPU only**. |
| Lattice | \(L=32\), periodic. |
| Well | \(\mathcal{L}(x,y,z)=\mathcal{L}_0+k x\) with \(\mathcal{L}_0=0.05\), \(k=10^{-3}\), \(x\) the integer lattice coordinate in \(\{0,\ldots,31\}\). Written onto **every** voxel. Poisson is **not** run. |
| \(g_{\rm ext}\) | Analytic: \(g_{\rm ext}=C_{\rm SPEED}^2\,(\mathcal{L}_0+k\,x_{\rm COM})\,k\) in \(+\hat x\). No periodic finite difference of the linear well (discontinuous at the wrap). The production stencil is still tier-2 FD of \(\mathcal{L}\); on this linear well away from the wrap, tier-2 FD equals \(k\) exactly (`GRAD_TIER2_SCALE=1/4` over span 4). |
| Test bodies | Locked \(+1\) cubes, flux \(J=0\), colour/spin 0, rest. \(N\in\{1,8,27\}\) as edge \(1,2,3\). Origin \(x=y=z=14\) (bulk; away from the wrap). Separate bridge per \(N\). |
| Toggles Path F-on | `disable_all`, then `forces=true`, `gravity=true`, `cluster_inertia=true`, `geometric_gravity=true`. `latency_field=false`. EM modes off: `poisson_coulomb`, `emergent_forces`, `lorentz_force`. |
| Toggles Path F-off | Identical except `geometric_gravity=false`. |
| Integration | One public `phase_forces` split. No `tick()`, no movement. \(a_{\rm COM}=v_x/\mathrm{dt}\) from any member (rigid). |
| Path G | Same well and cubes. Do **not** call `phase_forces`. Write \(v_x=g_{\rm ext}\,\mathrm{dt}\) on every member. |

External: the well is imposed, not sourced by the test body. Self-Poisson is out of scope. Sourcing identity of this campaign: when `latency_field` later writes `voxel.latency` from manifestation Poisson, the same operator reads that \(\mathcal{L}\). No new sourced-orbit CTest in v1 (self-force would be IMPROPER as a weak-EP gate).

---

## §3 — Measurand (LOCKED)

\[
r(N)=\frac{a_{\rm COM}(N)}{g_{\rm ext}(N)}.
\]

Single registered number per path per \(N\). No scan over couplings. No CODATA target.

---

## §4 — Executable protocol (LOCKED)

Instrument: `engine/tests/test_geometric_freefall_integrator.cpp` (CTest name `geometric_freefall_integrator`). Recomputes \(g_{\rm ext}\) from \(\mathcal{L}_0,k,x_{\rm COM},C_{\rm SPEED}\); does not bookkeep a hard-coded acceleration.

**Protocol gates (must pass before a physics verdict):**

| ID | Claim | Pass if |
|---|---|---|
| P1 | Well non-vacuous | \(\|g_{\rm ext}\|>10^{-8}\) at every \(N\) |
| P2 | Path G is Q0 | \(\|r_G(N)-1\|<0.02\) at every \(N\) |
| P3 | Path G \(N\)-independent | \(\bigl(\max r_G-\min r_G\bigr)/|\mathrm{mean}\,r_G|<0.02\) |
| P4 | Extra forces off on Path F-on | \(\max\|f_{\rm coulomb}\|,\|f_{\rm strong}\|,\|f_{\rm magnetic}\|,\|f_{\rm exchange}\|<10^{-12}\) on all members after `phase_forces` |
| P5 | Path F-on ran | at least one member has `force_diag_.f_gravity` written with \(\|f_{\rm gravity}\|>0\) |
| P6 | Default gravity unread of \(\mathcal{L}\) | Path F-off: \(\|r_{F{\rm-off}}(N)\|<0.05\) at every \(N\) (FTD-1014 residue) |

**Physics gate (fails CTest if false — this is an adoption test, not a classifier):**

| ID | Claim | FOUND if |
|---|---|---|
| A1 | Toggle-ON live tick is Q0 | \(\|r_{F{\rm-on}}(N)-1\|<0.05\) at every registered \(N\) |

---

## §5 — Outcome map (LOCKED)

**IMPROPER** (precedes): injecting \(F=m g_{\rm ext}\) then dividing by \(m\); retuning \(k\) or \(\mathcal{L}_0\) after seeing \(r_{F{\rm-on}}\); treating a near-miss as FOUND by widening ε; enabling colour/EM; calling the full `tick()` Poisson overwrite; GPU run; turning the toggle ON in the golden profile; claiming lensing/GWs from this operator.

**FOUND.** Not IMPROPER. P1–P6 pass. A1 passes. Tag: selected CPU operator **reproduces** Q0 weak EOM on this fixture `[MEASURED — selected integrator]`. Still does **not** derive \(m_i=m_g\). Default-off gravity remains \(\nabla|J|\) (P6). Golden tick unchanged.

**CLOSED-NEGATIVE.** Not IMPROPER. P1–P6 pass. A1 fails. The new operator does not instantiate Q0 on this fixture. CTest fails. No silent bookkeeping.

**UNDERDETERMINED.** Not IMPROPER. Any of P1–P6 fails. No physics verdict. CTest fails on the failed protocol gate.

Partition: IMPROPER first; then if protocol fails → UNDERDETERMINED; else A1 true → FOUND else CLOSED-NEGATIVE. One column only.

---

## §6 — Tie-breaks (LOCKED)

- \(x_{\rm COM}\) is the mean integer \(x\) of members (not remainder).
- \(a_{\rm COM}\) from member 0’s \(v_x/\mathrm{dt}\) after asserting rigid \(v\) (all members equal to 1e-15).
- A1 uses 0.05. Equality at exactly 0.05 is FOUND (closed interval). \(\gamma_{\rm FTD}\) at \(\mathcal{L}\approx 0.05\) shifts \(r\) by \(\sim\sqrt{1-\mathcal{L}^2}-1\approx -1.25\times 10^{-3}\), inside the gate; do not “correct it out.”
- P4 uses max over members of each channel’s Euclidean magnitude.
- P5 requires \(\|f_{\rm gravity}\|>0\) (stricter than FTD-1014 P5, which allowed a written zero).
- Toggle name frozen: `geometric_gravity`. Default `false`. `bulk_managed=true`. Requires `gravity` and `forces`. Backends: CPU only. `GpuTermImpl::CpuOnly`.

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can fail? | Witness |
|---|---|---|
| P2 | Yes | Wrong Path-G formula |
| P6 | Yes | Accidental rewrite of default \(\nabla\rho\) to read \(\mathcal{L}\) |
| A1 | Yes | Toggle wired but force still \(\nabla|J|\), or wrong stencil, or missing \(M_{\rm INERTIAL}\) so \(F/M\) is not \(g_{\rm ext}\) |
| P1 | Yes | \(k=0\) |
| P5 | Yes | Operator writes `f_gravity=0` |

---

## §8 — Banned moves (LOCKED)

- Golden-tick change; production default ON.
- Promote FTD-0250 / 0402 / 1013; claim UFF derived from the engine; claim full GR.
- Coincidence scan; CODATA retune of \(G_N=0.01\); graviton; occupancy telegraph; TEGR import to salvage FTD-1015.
- Self-force / GNC / colour-on as this weak-EP test.
- Lensing, Shapiro, perihelion, frame dragging, GWs, cosmology as this campaign’s measurands.
- Edit this prereg after observing Path F-on.

---

## §9 — Quantifier coverage (LOCKED)

A1 is \(\forall N\in\{1,8,27\}\) on **this** well and **this** operator. It is not \(\forall\) wells, not GPU, not unlocked clumps, not a sourced Poisson well, not strong EP.

Empirical coverage of this campaign (frozen; the catalog expands the same rows, it does not add measurands):

| Phenomenon | This campaign |
|---|---|
| UFF / weak EP (Eötvös-class, extra forces off) | **IN** — A1 |
| Gravitational redshift / clock dilation | **ALREADY** — latency → \(d\tau\) / \(\gamma_{\rm FTD}\) if \(\mathcal{L}\) is written; this lock adds no new redshift operator |
| Newtonian falling from a sourced well | **IN as selected wiring** — same operator reads Poisson-written `voxel.latency`; \(G_N=0.01\) toy; physical \(G_N=1/100\) falsified (FTD-0131); \(\alpha_G\) stays `[SMC]` |
| Light deflection / Shapiro / lensing | **OUT** — needs \(g_{rr}\) (retracted, FTD-0361) |
| Perihelion / PPN beyond \(g_{00}\) | **OUT** |
| Frame dragging / GPB | **OUT** |
| GWs / Hulse–Taylor / LIGO | **OUT** — FTD-1015 |
| Cosmology / Hubble | **OUT** |

---

## §10 — Window (LOCKED)

2026-08-19 America/Chicago through 23:59, this session, CPU observer. Past window with no verdict books F10. Git tag pending; result cites §12 SHA as `anchored-late`.

---

## §11 — Reconciliation (LOCKED)

FTD-1016 is a new row. FTD-1013 remains the desk theorem. FTD-1014 remains CLOSED-NEGATIVE for the default \(\nabla|J|\) operator (P6 re-asserts that residue). FTD-1015 remains CLOSED-NEGATIVE. This is a **[SELECTION]** engine extension, not a derivation of mass-role equality.

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive.

**Content SHA256 of hashed prefix:** `B825351085CAFBD36831E4A165F6CF22AB97849AB7915B606087031136EC7287`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19 America/Chicago, CPU observer, CTest `geometric_freefall_integrator` **Passed**. Instrument `engine/tests/test_geometric_freefall_integrator.cpp` SHA256 `8EDA5AE06CDBFEDF34F9E5653E7B93CA5AE3BA519D8468B2527C19544CD7005B`. Frozen classifier **FOUND**. Path G: \(r_G=1\) at \(N\in\{1,8,27\}\). Path F-on: \(r_{F{\rm-on}}\approx 0.9979\) (γ_FTD at \(\mathcal{L}\approx 0.05\), inside A1). Path F-off: \(r_{F{\rm-off}}=0\) (FTD-1014 residue). Constructor logs “GPU backend active” then `force_cpu()`; not a GPU campaign (not IMPROPER). Golden battery 7/7 Passed with toggle default OFF. Result: [`ANALYSIS_GEOMETRIC_FREEFALL_INTEGRATOR_v1.md`](../../../03_derivations/gravity_and_cosmology/ANALYSIS_GEOMETRIC_FREEFALL_INTEGRATOR_v1.md). Coverage: [`CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md`](../../../03_derivations/gravity_and_cosmology/CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md). Anchor: **`anchored-late`** until `git rev-parse preregister-geometric-freefall-integrator-v1` succeeds. FTD-1013 / 1014 / 1015 / 0250 / 0349 / 0402 / 0208 / U-8 unmoved. Production default remains \(\nabla|J|\).
