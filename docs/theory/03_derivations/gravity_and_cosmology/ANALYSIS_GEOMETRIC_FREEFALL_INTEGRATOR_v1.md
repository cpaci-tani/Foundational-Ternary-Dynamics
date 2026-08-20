# ANALYSIS — Geometric free-fall integrator v1

**Tag:** `[MEASURED — SELECTED INTEGRATOR, FOUND]`
**Date:** 2026-08-19
**LEDGER:** FTD-1016
**Lock:** [`PREREG_GEOMETRIC_FREEFALL_INTEGRATOR_v1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_GEOMETRIC_FREEFALL_INTEGRATOR_v1.md) prefix SHA256 `B825351085CAFBD36831E4A165F6CF22AB97849AB7915B606087031136EC7287` (`anchored-late`).
**Instrument:** `engine/tests/test_geometric_freefall_integrator.cpp` SHA256 `8EDA5AE06CDBFEDF34F9E5653E7B93CA5AE3BA519D8468B2527C19544CD7005B`
**Coverage:** [`CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md`](CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md)
**Does not move:** FTD-1013, FTD-1014, FTD-1015, FTD-0250, FTD-0349, FTD-0402, FTD-0208, U-8. Does not derive \(m_i=m_g\). Does not adopt P6C-G. Golden tick unchanged (toggle default OFF).

---

## Verdict

**FOUND.** Protocol P1–P6 Passed. A1 Passed. CTest `geometric_freefall_integrator` Passed.

The default-off CPU operator

\[
\mathbf F = M_{\rm INERTIAL}\,C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}
\]

reproduces Q0’s weak \(g_{\rm ext}\) on the FTD-1014 prescribed-well fixture. Default gravity remains \(F=G_N\nabla|J|\) (P6: \(r_{F{\rm-off}}=0\)).

---

## Numbers (CPU observer, \(L=32\), \(\mathcal{L}=0.05+10^{-3}x\), origin 14)

| \(N\) | \(g_{\rm ext}\) | \(r_G\) | \(r_{F{\rm-on}}\) | \(r_{F{\rm-off}}\) | \(\|f_{\rm gravity,on}\|\) |
|---|---|---|---|---|---|
| 1 | \(2.133333\times 10^{-5}\) | 1 | 0.997950 | 0 | \(1.090133\times 10^{-5}\) |
| 8 | \(2.150000\times 10^{-5}\) | 1 | 0.997918 | 0 | \(1.107167\times 10^{-5}\) |
| 27 | \(2.166667\times 10^{-5}\) | 1 | 0.997885 | 0 | \(1.124200\times 10^{-5}\) |

The \(\sim 2\times 10^{-3}\) shortfall vs 1 is the locked \(\gamma_{\rm FTD}\) factor at \(\mathcal{L}\approx 0.05\) (\(\sqrt{1-\mathcal{L}^2}-1\)), inside A1’s 0.05 gate. Constructor logs “GPU backend active” then `force_cpu()` — not a GPU campaign, not IMPROPER.

---

## What this is not

- Not a derivation of FC-2, class \(\mathcal{C}\), or \(m_i=m_g\).
- Not production-default gravity. Golden profile still uses \(\nabla|J|\).
- Not lensing, Shapiro, perihelion, frame dragging, GWs, or cosmology. Those stay **OUT** in the coverage catalog.
- Not a GPU kernel (`GpuTermImpl::CpuOnly`).
