# PRE-REGISTRATION — Frozen-well characteristic deflection v1

**Tag:** `[PRE-REGISTRATION]` — locks the CPU observer that asks whether live flux characteristics curve in a **frozen vacuum** Poisson \(\mathcal{L}\) well, and classifies the deflection as **0 / 1911-half / GR-full**. Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-frozen-well-characteristic-deflection-v1` (pending owner commit; until the tag resolves this lock is `anchored-late` via §12 prefix SHA256).
**LEDGER reservation:** FTD-1020.
**Parent / successor-of:** [`PREREG_LIGHT_DEFLECTION_CHANNEL_v4.md`](PREREG_LIGHT_DEFLECTION_CHANNEL_v4.md) (2026-07-18 chain **CLOSED [INSTRUMENT-LIMITED — Indeterminate]**; stopping rule forbade a v5). This is a **new name**, not v5. FTD-1017 supplies the missing “well grips matter” gate that v4’s particle arm never achieved. FTD-1019 is the same well law on clocks. FTD-0361 \(g_{rr}\) **RETRACTED**.
**Does not move:** FTD-0250, FTD-0349, FTD-0361, FTD-0402, FTD-0189 Gap 10.1, FTD-1013–1019, U-8, FTD-0131. No golden tick. No production default. No refractive-index operator added to `phase_read`. No P6C-G. No graviton. No Pound–Rebka. No CODATA retune.

> LOCK-STD v1. Sections §1–§11 are frozen before any packet centroid is observed. Post-hoc edits to §1–§11 void v1.

---

## §1 — The question (LOCKED)

**Q-FROZEN-CHAR-DEFLECT-v1.** After a CPU `tick()` that runs `latency_field` Poisson on a heavy locked source alone (FTD-1017 Step S), **freeze** \(\mathcal{L}\), **clear all matter** (true vacuum: no occupancy scatterer), and launch a sub-threshold \(+x\) traveling flux packet at impact parameter \(b\). Do the live vacuum-wave characteristics curve, and which band?

| Class | Meaning |
|---|---|
| **0** | No optical channel. \(\lvert\theta_{\rm diff}\rvert\) is at the control floor. |
| **1911-half** | Clock-medium / \(g_{00}\)-only Fermat: \(\theta_{\rm diff}\) matches \(\theta_{1911}\) from the frozen well. |
| **GR-full** | Twice that (\(\theta_{\rm GR}=2\theta_{1911}\)). |

\(\theta_{1911}\) is **not** a solar 1.75″ target. It is the isotropic optical-metric deflection of *this* frozen field,

\[
n=(1-\mathcal{L}^2)^{-1/2},\qquad
\theta_{1911}=\sum_x (\partial n/\partial y)\,dx
\]

along the unperturbed ray, \(dx=1\), \(\partial\mathcal{L}/\partial y\) the production tier-2 stencil. This is Einstein 1911’s medium for the written \(\mathcal{L}\). \(\theta_{\rm GR}:=2\theta_{1911}\) is the “other half is \(g_{rr}\)” doubling, applied to the same well. Neither formula is inserted into the engine.

**Live operator under test:** `wave_propagation` only. `phase_read` is \(c^2\nabla^2\) with constant \(c\) (no latency term). Code-derived prior: class **0**. Priors are not measurements.

**Not asked:** live Poisson during transit; coupling/EM/genesis; a packet scattering off locked matter; adding \(n(\mathcal{L})\) to the stencil; GPS/Eddington ppm; making lensing IN.

---

## §2 — Fixture (LOCKED)

| Item | Frozen value |
|---|---|
| Backend | CPU (`force_cpu()`). GPU off. |
| Lattice | \(L=32\), periodic. |
| Step S | Identical to FTD-1017: locked \(+1\) cube edge 5, origin \((6,13,13)\), \(J=0\). `gravity=true`, `latency_field=true`, forces/movement/`geometric_gravity` off. One `tick()`. |
| Vacuum | After Step S: every site `state=0`, `locked=false`, `flux=0`, `wave_vel=0`. **Keep** `voxel.latency`. Source occupancy is gone; the well remains. |
| Impact parameter | Unperturbed ray \(y=22\), \(z=15\). COM of the (cleared) source was \(x=8,y=15,z=15\). \(b_y=+7\). |
| Packet | State 0 everywhere. \(z\)-polarized, \(+x\) traveling, photon-pulse idiom: \(J_z=A g\sin(k(x-x_0))\), \(w_z=-\omega A g\cos(k(x-x_0))\), \(\sigma=2.5\), \(\lambda=4\sigma\), \(k=2\pi/\lambda\), \(\omega=2 C_{\rm WAVE}\sin(k/2)\), \(A=0.25 K_B\), \(x_0=4\), center \((4,22,15)\). Cutoff \(3\sigma\). |
| Transit toggles | `disable_all`, then `wave_propagation=true` only. `latency_field=false` (frozen). No coupling, forces, gravity, `geometric_gravity`, genesis, gauss, damping, movement. |
| Arms | **W:** frozen well + packet, \(N_t=40\) ticks. **C0:** no Step S (\(\mathcal{L}=0\)) + same packet, \(N_t=40\). |
| Centroid | \(\lvert J\rvert^2\)-weighted \((x,y,z)\), restricted to \(\lvert y-22\rvert\le 6\), \(\lvert z-15\rvert\le 6\), all \(x\). |
| Entry / exit | Entry ticks \([6,12]\); exit ticks \([28,36]\). \(\theta=(\langle y\rangle_{\rm exit}-\langle y\rangle_{\rm entry})/(\langle x\rangle_{\rm exit}-\langle x\rangle_{\rm entry})\). |
| Differential | \(\theta_{\rm diff}=\theta_W-\theta_{C0}\). Toward-mass sign is \(\theta_{\rm diff}<0\) (ray at \(y=22\), mass COM at \(y=15\)). |

---

## §3 — Measurand (LOCKED)

\(\theta_{\rm diff}\) (radians, small-angle slope). \(\theta_{1911}\) from the frozen \(\mathcal{L}\) **before** the packet exists (W arm, after vacuum-clear). \(\theta_{\rm GR}=2\theta_{1911}\). No solar number. No scan over \(b\), \(\sigma\), or \(A\).

---

## §4 — Executable protocol (LOCKED)

Instrument: `engine/tests/test_frozen_well_characteristic_deflection.cpp` (CTest name `frozen_well_characteristic_deflection`).

**Protocol gates:**

| ID | Claim | Pass if |
|---|---|---|
| P1 | 1911 yardstick is non-vacuous vs the floor | \(\lvert\theta_{1911}\rvert>10^{-4}\) and \(\lvert\theta_{1911}\rvert>10\cdot F\) |
| P2 | Well deeper toward the mass | mean \(\mathcal{L}\) on the ray \(<\) mean \(\mathcal{L}\) at 2 sites toward \(y=15\) (same \(x\) sample) **or** \(\theta_{1911}<0\) (toward-mass Fermat) |
| P3 | Vacuum: no manifested sites at packet inject | zero `state≠0` on W after clear |
| P4 | Packet energy survives | exit \(\sum\lvert J\rvert^2\) in the centroid window \(>0.25\times\) entry (both arms) |
| P5 | Control is a floor | \(\lvert\theta_{C0}\rvert<0.05\,\lvert\theta_{1911}\rvert\) |
| P6 | Well stayed frozen | max \(\lvert\mathcal{L}_{\rm after}-\mathcal{L}_{\rm freeze}\rvert<10^{-15}\) on the ray after transit |
| P7 | FTD-1017 well-grips-matter (inherited, not re-run) | cited FOUND; this lock does not re-open V2 |

Floor \(F=\max(3\lvert\theta_{C0}\rvert,\,3\lvert\theta_{z,W}\rvert)\) with \(\theta_z\) the same slope construction in \(z\).

**Classification (exactly one; CTest fails if none):**

| ID | Class | Pass if |
|---|---|---|
| A0 | **0** | \(\lvert\theta_{\rm diff}\rvert\le\max(F,\,0.20\,\lvert\theta_{1911}\rvert)\) |
| A1 | **1911-half** | \(\theta_{\rm diff}\) same sign as \(\theta_{1911}\) and \(\lvert\theta_{\rm diff}/\theta_{1911}-1\rvert<0.35\) |
| A2 | **GR-full** | \(\theta_{\rm diff}\) same sign as \(\theta_{1911}\) and \(\lvert\theta_{\rm diff}/\theta_{\rm GR}-1\rvert<0.35\) |

If A1 and A2 could both match, A2 wins only if \(\lvert\theta_{\rm diff}/\theta_{\rm GR}-1\rvert<\lvert\theta_{\rm diff}/\theta_{1911}-1\rvert\); else A1. A0 is exclusive of A1/A2: if A1 or A2 matches, do not also book 0.

---

## §5 — Outcome map (LOCKED)

**IMPROPER** (precedes): inserting \(n(\mathcal{L})\) into `phase_read`; leaving `latency_field` on during transit; leaving the source occupancy in the beam; enabling coupling/forces/`geometric_gravity`; retuning \(b,\sigma,A\) after seeing \(\theta_{\rm diff}\); widening ε; a solar/CODATA target; quoting class 0 as “nature has no lensing.”

**FOUND — class 0.** Not IMPROPER. P1–P7 pass. A0. Tag: `[MEASURED STRUCTURAL NULL — wave sector unread of frozen \(\mathcal{L}\)]`. Lensing stays **OUT**. Does **not** move FTD-0361. Does **not** predict against Eddington.

**FOUND — class 1911-half.** Not IMPROPER. P1–P7 pass. A1. Tag: `[MEASURED — clock-medium deflection]`. Unexpected relative to the constant-\(c\) stencil. Mechanism hunt `[OPEN]`. Still not laboratory lensing (half-GR). Do not fake \(g_{rr}\).

**FOUND — class GR-full.** Not IMPROPER. P1–P7 pass. A2. Tag: `[MEASURED — unexplained GR-like optical response]`. Extraordinary; does **not** close Gap 10.1 by itself.

**CLOSED-NEGATIVE / other.** Not IMPROPER. P1–P7 pass. None of A0/A1/A2. CTest fails. Characterize; no class booked.

**UNDERDETERMINED.** Not IMPROPER. Any of P1–P7 fails. CTest fails on the failed gate.

---

## §6 — Tie-breaks (LOCKED)

- \(\partial\mathcal{L}/\partial y\) uses `GRAD_TIER2_SCALE` on \(\mathcal{L}(x,y\pm 2,z)-\mathcal{L}(x,y\mp 2,z)\). Periodic wrap.
- \(\theta_{1911}\) summed over all \(x\) at \((y,z)=(22,15)\) after vacuum-clear, before packet.
- Entry/exit means are energy-weighted centroids averaged over the tick windows (equal tick weight on the already-weighted centroid).
- Equality at a numeric gate is a pass for that gate.

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can fail? | Witness |
|---|---|---|
| P1 | Yes | Periodic well too flat at \(b=7\) |
| P5 | Yes | Packet self-deflects from lattice anisotropy at this \(\sigma\) |
| P6 | Yes | Accidental Poisson overwrite |
| A0 | Yes | Hidden wave–latency channel, or occupancy leftover |
| A1/A2 | Yes | Live stencil is constant \(c\) |

---

## §8 — Banned moves (LOCKED)

- Golden-tick change; production \(n(\mathcal{L})\); v5 of the 2026-07 campaign.
- Promote Gap 10.1; claim Eddington; claim Newton’s \(G\).
- Coincidence scan; CODATA; graviton; TEGR.
- Edit this prereg after observing \(\theta_{\rm diff}\).

---

## §9 — Quantifier coverage (LOCKED)

This \(b\), this frozen well, this packet, this pure-wave toggle set. Not \(\forall b\), not GPU, not live two-body Poisson, not GR in the sky.

---

## §10 — Window (LOCKED)

2026-08-19 America/Chicago through 23:59, this session, CPU observer. Past window with no verdict books F10. Git tag pending; result cites §12 SHA as `anchored-late`.

---

## §11 — Reconciliation (LOCKED)

FTD-1020 is a new row. The 2026-07 optical-channel chain remains CLOSED instrument-limited; this lock does not reopen it. FTD-1017 remains the sourced-falling FOUND (V2 inherited). FTD-0361 remains \(g_{rr}\) RETRACTED. Class 0 does not make “no light bending in nature” a prediction. Class 1911/GR does not license a refractive-index production operator.

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive, including the trailing newline after that line.

**Content SHA256 of hashed prefix:** `B6BF393F332A6CBFD9770ECC6C86CD59092F07C478D7F12DBCA5A986CE02C034`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19 America/Chicago, CPU observer, CTest `frozen_well_characteristic_deflection` **Passed**. Instrument `engine/tests/test_frozen_well_characteristic_deflection.cpp` SHA256 `C46AA05F1AD3DF9E02CB4D1C218949022F04010C4C676757F15065A930F43830`. Frozen classifier **FOUND — class 0**. \(\theta_{1911}=-5.840840\times10^{-2}\), \(\theta_{\rm GR}=-1.168168\times10^{-1}\), \(\theta_W=\theta_{C0}=-1.628974\times10^{-15}\), \(\theta_{\rm diff}=0\), floor \(F=4.886922\times10^{-15}\), mean \(\mathcal{L}\) on ray \(4.813868\times10^{-2}\) vs toward-mass \(8.356927\times10^{-2}\). Constructor logs “GPU backend active” then `force_cpu()`; not a GPU campaign (not IMPROPER). Result: [`ANALYSIS_FROZEN_WELL_CHARACTERISTIC_DEFLECTION_v1.md`](../../../03_derivations/gravity_and_cosmology/ANALYSIS_FROZEN_WELL_CHARACTERISTIC_DEFLECTION_v1.md). Anchor: **`anchored-late`** until `git rev-parse preregister-frozen-well-characteristic-deflection-v1` succeeds. FTD-0250 / 0349 / 0361 / 0402 / 0189 Gap 10.1 / 1013–1019 / U-8 / 0131 unmoved. Lensing stays OUT. Class 0 is not “nature has no lensing.” Production default remains \(\nabla|J|\). No \(n(\mathcal{L})\) operator added.
