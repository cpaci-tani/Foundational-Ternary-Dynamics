# ANALYSIS — Radiative shears of transport (GW area-holonomy Q0)

**Tag:** `[CLOSED NEGATIVE]` — kinematic residual of spatial \(\Omega\) is not exactly two TT shears.
**Date:** 2026-08-19
**LEDGER:** FTD-1015
**Lock:** [`PREREG_GW_AREA_HOLONOMY_Q0_v1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_GW_AREA_HOLONOMY_Q0_v1.md) — prefix SHA256 `6381C791B58F3E4259138CCED06A5777DF5175FE015F5BDD670804786DA035C9`. Git tag `preregister-gw-area-holonomy-q0-v1` pending owner commit; this result is **`anchored-late`** until that tag resolves.
**Verifier:** `scripts/proofs/proof_gw_area_holonomy_q0.py` SHA256 `C5FF94BDBD81B1CDAD9F7EA7D117C2F1DF427F7F295A144610D23880EC130A5D` — protocol **11/11**, A1 failed, verdict **CLOSED-NEGATIVE**.
**Parent:** [`SCOPE_GW_AREA_HOLONOMY_v1.md`](../../10_eft_program/scopes_and_specs/SCOPE_GW_AREA_HOLONOMY_v1.md) §4.
**Does not move:** FTD-0189, FTD-0193, FTD-0209, FTD-0213, FTD-0026, U-8, FTD-1013, FTD-1014.

---

## 0 · Verdict

Linearized spatial clock-transport \(\omega_{ij}\) with the SCOPE’s kinematic constraints (fixed solder, local \(\mathrm{SO}(3)\) gauge, no \(\mathrm{SO}(1,3)\), **no action**) has residual dimension 6 at generic \(k\neq 0\). The little-group helicity multiset is

\[
H=\{-2,-1,0,0,+1,+2\}.
\]

The plus/cross pair is present (\(n_{\rm TT}=2\)) and is **not isolated** (leftover \(=4\): helicity \(\pm 1\) and two helicity \(0\)). A1 required \(H=\{+2,-2\}\) and \(n_{\rm res}=2\). Classifier: **CLOSED-NEGATIVE**.

Transport geometry as typed does not isolate LIGO-like radiation. Gravity remains the static well. Do not posit \(h_{\mu\nu}\). Do not retarget \(J\). P6C-G is **not** narrowed from “spin-2 field” to this type.

---

## 1 · What was counted

Nine real components \(\omega_{ij}\). Gauge \(\delta\omega_{ij}=ik_i\theta_j\) has rank 3 at \(k=\hat z\), so \(n_{\rm res}=6\). The \(\mathrm{SO}(2)_k\) generator on the Coulomb slice (longitudinal row set to 0) has spectrum \(iH\) with the multiset above.

Gapless dispersion and cone-compatibility with the flux wave were **not asked**. They need an action; the parent SCOPE forbids selecting one here. Citing them as FOUND would have been IMPROPER.

Hand TT projection of a *symmetric* tensor still has dimension 2 (V8). Using that 2 as A1 would have been IMPROPER. The Q0 residual is the gauge quotient, not the TT projector.

---

## 2 · What this does not license

- A graviton, \(h_{\mu\nu}\), or occupancy telegraph as consolation.
- Promotion or demotion of FTD-0209 (no helicity-±2 *particle* pole in the \(\{J,s,\mathcal{L}\}\) catalog). This census is of a **new** typed field \(\Omega\), and it failed to isolate \(\pm 2\).
- Amendment of `SCOPE_CONSUMPTION_PROGRAM.md`’s “one massless spin-2 field.”
- An engine holonomy correlator, a TEGR action lock, or a Stage-U2 sitting. Parent sequence step 3: if Q0 fails, stop.

Selecting TEGR constraints to kill the leftover four and re-asking A1 is a **different** lock, priced as an action adoption, never as this Q0.

---

*Protocol 11/11. Frozen classifier CLOSED-NEGATIVE. Zero promotions.*
