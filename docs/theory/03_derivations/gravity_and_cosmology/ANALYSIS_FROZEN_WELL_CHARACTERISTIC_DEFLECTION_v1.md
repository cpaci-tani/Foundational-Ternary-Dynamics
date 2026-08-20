# ANALYSIS — Frozen-well characteristic deflection v1

**Tag:** `[MEASURED STRUCTURAL NULL — wave sector unread of frozen \(\mathcal{L}\)]`
**Date:** 2026-08-19
**LEDGER:** FTD-1020
**Lock:** [`PREREG_FROZEN_WELL_CHARACTERISTIC_DEFLECTION_v1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_FROZEN_WELL_CHARACTERISTIC_DEFLECTION_v1.md) prefix SHA256 `B6BF393F332A6CBFD9770ECC6C86CD59092F07C478D7F12DBCA5A986CE02C034` (**FOUND class 0**; `anchored-late`).
**Instrument:** `engine/tests/test_frozen_well_characteristic_deflection.cpp` SHA256 `C46AA05F1AD3DF9E02CB4D1C218949022F04010C4C676757F15065A930F43830`
**Does not move:** FTD-0250, FTD-0349, FTD-0361, FTD-0402, FTD-0189 Gap 10.1, FTD-1013–1019, U-8, FTD-0131. Does not add \(n(\mathcal{L})\) to `phase_read`. Does not predict against Eddington.

---

## Verdict

**FOUND — class 0.** Protocol P1–P7 Passed. A0 Passed. A1/A2 did not match. CTest `frozen_well_characteristic_deflection` Passed.

Live `wave_propagation` characteristics in a **frozen vacuum** Poisson \(\mathcal{L}\) well do not curve at this fixture. \(\theta_{\rm diff}=0\) at the control floor. Lensing stays **OUT**.

This is **not** “nature has no lensing.” It is the engine’s vacuum-wave stencil (\(c^2\nabla^2\), constant \(c\), no latency term) unread of frozen \(\mathcal{L}\). FTD-0361 \(g_{rr}\) remains RETRACTED. Gap 10.1 remains `[GAP]`.

---

## Numbers (CPU observer, \(L=32\), source origin \((6,13,13)\), ray \(y=22,z=15\), \(N_t=40\))

| Quantity | Value |
|---|---|
| \(\theta_{1911}\) | \(-5.840840\times10^{-2}\) |
| \(\theta_{\rm GR}:=2\theta_{1911}\) | \(-1.168168\times10^{-1}\) |
| \(\theta_W\) | \(-1.628974\times10^{-15}\) |
| \(\theta_{C0}\) | \(-1.628974\times10^{-15}\) |
| \(\theta_{\rm diff}\) | \(0\) |
| \(\theta_{z,W}\) | \(-1.628974\times10^{-15}\) |
| floor \(F\) | \(4.886922\times10^{-15}\) |
| mean \(\mathcal{L}\) on ray / toward mass | \(4.813868\times10^{-2}\) / \(8.356927\times10^{-2}\) |
| \(\theta_{\rm diff}/\theta_{1911}\), \(\theta_{\rm diff}/\theta_{\rm GR}\) | \(0\), \(0\) |
| entry / exit \(\sum|J|^2\) (both arms) | \(0.940377\) / \(0.307862\) |

The 1911 yardstick is non-vacuous (\(|\theta_{1911}|\gg F\)) and toward-mass (\(\theta_{1911}<0\), deeper \(\mathcal{L}\) at \(y=20\)). The packet is identical on W and C0 to printer precision. Energy retention \(\approx 0.327>0.25\).

Constructor logs “GPU backend active” then `force_cpu()` — not a GPU campaign, not IMPROPER.

---

## What this is not

- Not Eddington / VLBI / survey lensing. Class 0 does not make a laboratory or solar prediction.
- Not a 1911 clock-medium measurement (that would have been A1).
- Not a GR-full optical response (A2). Do not fake \(g_{rr}\).
- Not a license to insert \(n(\mathcal{L})\) into production `phase_read`.
- Not a reopening of the 2026-07 `PREREG_LIGHT_DEFLECTION_CHANNEL_v1…v4` chain (CLOSED instrument-limited; this is a new name).
- Not production-default gravity. The observer does not touch the golden profile.
