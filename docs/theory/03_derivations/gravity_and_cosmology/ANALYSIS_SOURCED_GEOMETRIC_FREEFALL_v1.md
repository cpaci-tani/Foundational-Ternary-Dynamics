# ANALYSIS — Sourced geometric free-fall v1

**Tag:** `[MEASURED — SOURCED WIRING, FOUND]`
**Date:** 2026-08-19
**LEDGER:** FTD-1017
**Lock:** [`PREREG_SOURCED_GEOMETRIC_FREEFALL_v1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_SOURCED_GEOMETRIC_FREEFALL_v1.md) prefix SHA256 `A428956329AFC7DAD006178368FDA19ABE337754F20FD7372EECA376CE240D39` (`anchored-late`).
**Instrument:** `engine/tests/test_sourced_geometric_freefall.cpp` SHA256 `E41E627A2BF9B011E5D56ED31F3D8732B70B8A78E96340951026506982C104A6`
**Does not move:** FTD-1013, FTD-1014, FTD-1015, FTD-1016, FTD-0131, FTD-0250, FTD-0349, FTD-0402, FTD-0208, U-8. Does not derive \(m_i=m_g\) or physical \(G_N\). Does not claim \(1/r^2\). Does not run live mutual gravity.

---

## Verdict

**FOUND.** Protocol P1–P6 Passed. A1 Passed. CTest `sourced_geometric_freefall` Passed.

A heavy locked source (\(N_s=125\)) writes \(\mathcal{L}\) via `latency_field` Poisson. That well is frozen. A light locked probe (\(N_p=1\)) then falls under FTD-1016’s operator \(F=M_{\rm INERTIAL} C^2\,\mathcal{L}\,\nabla\mathcal{L}\). Default gravity remains unread of \(\mathcal{L}\) (P6).

---

## Numbers (CPU observer, \(L=32\), source origin \((6,13,13)\), probe \((18,15,15)\))

| Quantity | Value |
|---|---|
| \(\Delta x\) (probe − source COM) | \(+10\) |
| \(g_{{\rm ext},x}\) | \(-2.917418\times 10^{-4}\) (toward the source) |
| \(a_{{\rm on},x}\) | \(-2.916081\times 10^{-4}\) |
| \(r_{\rm on}\) | \(0.999542\) |
| \(r_{\rm off}\) | \(0\) |
| \(\|f_{\rm gravity,on}\|\) | \(1.490801\times 10^{-4}\) |

The \(\sim 4.6\times 10^{-4}\) shortfall vs 1 is \(\gamma_{\rm FTD}\) on the written well, inside A1’s 0.05 gate. Constructor logs “GPU backend active” then `force_cpu()` — not a GPU campaign, not IMPROPER.

---

## What this is not

- Not a \(1/r^2\) theorem (periodic Poisson Green’s function is already Phase G).
- Not Newton’s \(G\) (FTD-0131 stands; engine \(G_N=0.01\) is the toy coupling).
- Not strong EP: the probe did not source. Poisson was not re-solved with the probe present.
- Not production-default gravity. Golden profile still uses \(\nabla|J|\).
