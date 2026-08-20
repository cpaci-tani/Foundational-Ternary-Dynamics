# ANALYSIS — GPU geometric-gravity parity v1

**Tag:** `[MEASURED — GPU PARITY, FOUND]`
**Date:** 2026-08-19
**LEDGER:** FTD-1018
**Lock:** [`PREREG_GPU_GEOMETRIC_GRAVITY_PARITY_v1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_GPU_GEOMETRIC_GRAVITY_PARITY_v1.md) prefix SHA256 `624969CA01DC55906B55D409A38F56CFD539FFA592DB70E237E881375CF2EE9E` (`anchored-late`).
**Instrument:** `engine/tests/test_gpu_geometric_gravity_parity.cpp` SHA256 `0BDAC00297A5E2B9639B8CC1CDA3302E2AFF80C49CD32DF717175A590E943704`
**Does not move:** FTD-1013, FTD-1014, FTD-1015, FTD-1016, FTD-1017, FTD-0131, FTD-0250, FTD-0349, FTD-0402, FTD-0208, U-8. Does not derive \(m_i=m_g\) or physical \(G_N\).

---

## Verdict

**FOUND.** Protocol P1–P4 Passed. A1 Passed. CTest `gpu_geometric_gravity_parity` Passed. Golden 7/7 Passed.

Native CUDA `phase_forces` with `geometric_gravity=true` reproduces the FTD-1016 CPU operator \(F=M_{\rm INERTIAL} C^2\,\mathcal{L}\,\nabla\mathcal{L}\) on the prescribed well. Toggle-OFF residue remains \(F=G_N\nabla|J|\) with \(J=0\) on both backends.

---

## Numbers (prescribed \(\mathcal{L}=0.05+10^{-3}x\), unlocked probe at \((14,14,14)\), one tick)

| Quantity | Value |
|---|---|
| \(F_{{\rm cpu},x}\) | \(1.090133\times10^{-5}\) |
| \(F_{{\rm gpu},x}\) | \(1.090133\times10^{-5}\) |
| \(\Delta F\) | \(0\) |
| \(\Delta v\) | \(0\) |
| Toggle-OFF \(\|F\|\), \(\|v\|\) | \(0\) on both backends |

Bit-identical on this fixture. CPU path uses `force_cpu()` after a GPU-capable constructor; GPU path is `GpuEngine` with `graph_capture_enabled=false`.

---

## What this is not

- Not a new falling law. It is a port of FTD-1016.
- Not a sourced-Poisson GPU campaign (FTD-1017 remains CPU).
- Not production-default gravity. Golden profile still uses \(\nabla|J|\).
