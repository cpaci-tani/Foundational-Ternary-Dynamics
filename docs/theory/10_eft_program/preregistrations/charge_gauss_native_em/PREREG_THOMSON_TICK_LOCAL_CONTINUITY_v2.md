# PREREG: Source-Free Discrete Tick Local Continuity v2

**FTD ID:** FTD-0295
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED/RUN]
**Parent:** FTD-0294 Source-free discrete tick local continuity v1
**Engine artifact:** `engine/tests/campaign_thomson_tick_local_continuity_v2.cpp`
**Artifact SHA256:** `9b48ca418e784ba98e35708563214b22c78cf2580f880fda9fa923cef4c7a804`
**Lock commit:** `1d4a29a5`
**Lock tag:** `preregister-thomson-tick-local-continuity-v2`
**Run analysis:** `docs/theory/10_eft_program/ANALYSIS_THOMSON_TICK_LOCAL_CONTINUITY_v2.md`

---

## 1. Question

FTD-0294 v1 used the correct source-free tick density and boundary current,
but invalidated its relative gate because the denominator
`abs(Delta H_V) + abs(Phi_out)` becomes degenerate when both terms are near
zero. The absolute balance closed at roundoff scale:

```text
max_abs_balance = 4.66e-16
max_exchange_rel_balance = 1
```

FTD-0295 asks the same local-continuity question with a scale-relative
denominator:

```text
Does the source-free finite-volume identity close when the relative residual
is normalized by finite-volume energy scale rather than quiet exchange scale?
```

This is not a current change. The density, current, update, initial condition,
and absolute gate remain unchanged from v1.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_thomson_tick_local_continuity_v2.cpp
```

Frozen source hash:

```text
9b48ca418e784ba98e35708563214b22c78cf2580f880fda9fa923cef4c7a804
```

Any source edit after this lock requires another version or a new FTD ID.

---

## 3. Fixed Protocol

Shared setup:

| Quantity | Value |
|---|---:|
| Lattice size | `L = 33` |
| Ticks | `200` |
| Plane-wave mode | `mode_n = 4` |
| Plane-wave amplitude | `0.05` |
| Particle | none |
| Wave | y-polarized flux/wave-velocity plane wave along x |
| Ball radii | `{5, 7, 9, 11, 13}` |
| Accumulation | long-double Kahan |
| Balance absolute gate | `1e-10` |
| Balance scale-relative gate | `1e-12` |

Shared toggles are unchanged from v1: source-free single-substrate wave
propagation only; coupling, damping, forces, movement, and phenomenological
extensions are all off.

---

## 4. Frozen Local Density And Current

Unchanged from v1:

```text
h_i = 0.5 |W_i|^2 + 0.5 J_i dot (KJ)_i - 0.5 W_i dot (KJ)_i
Phi_i->j = 0.5 c^2 w_ij [J_i(old) dot W_j(next) - W_i(next) dot J_j(old)]
Delta H_V + Phi_out(boundary V) = 0
```

---

## 5. Frozen Relative Metric

The gated relative residual is:

```text
scale = max(abs(H_V_old), abs(H_V_next),
            abs(Delta H_V) + abs(Phi_out), 1e-300)

scale_relative_balance = abs(Delta H_V + Phi_out) / scale
```

The v1 exchange-relative residual is still reported for provenance:

```text
exchange_relative_balance =
  abs(Delta H_V + Phi_out) / max(abs(Delta H_V) + abs(Phi_out), 1e-300)
```

It is not gated in v2.

---

## 6. Gates And Outcomes

The run is invalid if any metric is non-finite.

The local continuity law is confirmed if:

```text
max_abs_balance <= 1e-10
and
max_scale_rel_balance <= 1e-12
```

Outcome labels:

```text
SOURCE_FREE_LOCAL_TICK_CONTINUITY_CONFIRMED
```

The finite-volume identity closes under the frozen gates.

```text
SOURCE_FREE_LOCAL_TICK_CONTINUITY_INVALIDATED
```

The finite-volume identity does not close under the frozen gates.

---

## 7. Non-Claims

This campaign does not:

- derive `alpha`;
- measure a Thomson cross-section;
- compute a QED scattering amplitude;
- claim radiation;
- include state-coupling source/work terms;
- scan amplitudes, modes, lattice sizes, gates, or initial conditions.

It only checks the source-free local current required before returning to the
charge-plus-beam recoil setup.

---

## 8. Frozen Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_thomson_tick_local_continuity_v2 --parallel 24
```

Run:

```sh
ctest --test-dir engine/build -C Release -R "^thomson_tick_local_continuity_v2$" --output-on-failure
```
