# ANALYSIS — One-well redshift + falling v1 / v1.1

**Tag:** `[MEASURED — ONE-WELL CLOCKS+FALLING, FOUND]`
**Date:** 2026-08-19
**LEDGER:** FTD-1019
**Locks:** v1 [`PREREG_ONE_WELL_REDSHIFT_FALLING_v1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_ONE_WELL_REDSHIFT_FALLING_v1.md) prefix SHA256 `EA5041998D1279DE62FF48D5EC5577451A2949B3DD1256D51AC531B4F1A6B4F6` (**UNDERDETERMINED** on P1). v1.1 [`PREREG_ONE_WELL_REDSHIFT_FALLING_v1_1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_ONE_WELL_REDSHIFT_FALLING_v1_1.md) prefix SHA256 `5DB20B6F59BA192F782772D91AB37894295A86FD15A81096D1A44EE4D8F5D0F5` (**FOUND**; `anchored-late`).
**Instrument:** `engine/tests/test_one_well_redshift_falling.cpp` SHA256 `4164C0DA192C9D24A8B4BB74AF885FFB6BFE59AFC06EE6430186E7500201FFC8`
**Does not move:** FTD-1013, FTD-1014, FTD-1015, FTD-1016, FTD-1017, FTD-1018, FTD-0131, FTD-0250, FTD-0349, FTD-0402, FTD-0208, U-8. Does not derive \(m_i=m_g\), physical \(G_N\), or Pound–Rebka.

---

## Verdict

**v1 UNDERDETERMINED.** P1 required \(\Gamma_{\rm near}/\Gamma_{\rm far}<0.999\); the sourced well gave \(0.9995417\). P2–P9 and A1/A2 would have passed; they are not a v1 verdict.

**v1.1 FOUND.** P1 repaired to \(<0.9999\) only. Sites, \(N_\tau\), A1/A2 unchanged. Protocol P1–P9 Passed. A1 and A2 Passed. CTest `one_well_redshift_falling` Passed.

One FTD-1017 Step S Poisson well, frozen, drives both FC-2 rest clocks and FTD-1016 falling. Clocks do not re-solve Poisson (`de_broglie_clock` only to reach `accumulate_proper_time`; \(J=0\)).

---

## Numbers (CPU observer, \(L=32\), source origin \((6,13,13)\), near \((18,15,15)\), far \((24,15,15)\), \(N_\tau=20\))

| Quantity | Value |
|---|---|
| \(\mathcal{L}_{\rm near}\) | \(3.027058\times10^{-2}\) |
| \(\mathcal{L}_{\rm far}\) | \(0\) |
| \(\Gamma_{\rm near}/\Gamma_{\rm far}\) | \(0.9995417\) |
| \(\tau_{\rm near}\), \(\tau_{\rm far}\) | \(19.99083\), \(20\) |
| \(\rho_\tau\) | \(1\) |
| \(g_{{\rm ext},x}\) | \(-2.917418\times10^{-4}\) (toward the source) |
| \(r_{\rm on}\) | \(0.999542\) |
| \(r_{\rm off}\) | \(0\) |

Falling \(r_{\rm on}\) matches FTD-1017. Clock \(\rho_\tau=1\) is the FC-2 rest formula on the frozen field, not a laboratory ppm.

Constructor logs “GPU backend active” then `force_cpu()` — not a GPU campaign, not IMPROPER.

---

## What this is not

- Not Pound–Rebka, GPS, or a GR redshift derivation. FC-2 / FTD-0402 remain the clock law.
- Not a new falling operator (FTD-1016 / FTD-1017).
- Not strong EP: clocks and the probe did not source.
- Not production-default gravity.
