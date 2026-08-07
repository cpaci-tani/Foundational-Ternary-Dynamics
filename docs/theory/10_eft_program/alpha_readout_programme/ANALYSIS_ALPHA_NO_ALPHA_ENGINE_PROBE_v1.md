# ANALYSIS: Alpha No-Alpha Engine Probe v1

**FTD ID:** FTD-0285  
**Status:** [INVALIDATED PROTOCOL -- NO CLAIM PROMOTED]  
**Pre-reg:** [`PREREG_ALPHA_NO_ALPHA_ENGINE_PROBE_v1.md`](../preregistrations/alpha_readout_programme/PREREG_ALPHA_NO_ALPHA_ENGINE_PROBE_v1.md)  
**Lock tag:** `preregister-alpha-no-alpha-engine-probe-v1`  
**Lock commit:** `cce615b07009a36de4d045d6fb5218b44fbbde6c`  
**Artifact:** `engine/tests/campaign_alpha_no_alpha_probe.cpp`  
**Artifact SHA256:** `883a917358077f626e90f5affacabd8e565f48fd1cc00aa775ac6e4c7ffbdade`

---

## 1. Run of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^alpha_no_alpha_probe$" --output-on-failure
```

Result:

```text
0% tests passed, 1 tests failed out of 1
verdict,INVALIDATED_PROTOCOL_OR_ENGINE_DRIFT
```

This is a pre-registered valid outcome class. It does not promote,
demote, or close the alpha claim.

---

## 2. Recorded Output Summary

Protocol:

```text
L=32, ticks=300, SOR=100, r={5,7,9}, rel_tol=0.1
coupling=false
coulomb_charge_coupling_native=1.0
forces=false
poisson_coulomb=false
lorentz_force=false
```

Positive control:

```text
postulate_w_g    = 0.214127440488941
postulate_w_g^2  = 0.0458505607703448
alpha_tree_input = 0.00729734339013571
```

Native arm:

| r | alpha_r | Phase-G expected | g_dyn^2 | rel_err |
|---:|---:|---:|---:|---:|
| 5 | 0.0392130644178632 | 0.0917826176998266 | 0.427238461928694 | 0.572761538071306 |
| 7 | 0.0333856940608977 | 0.0649993891038937 | 0.513630889784744 | 0.486369110215256 |
| 9 | 0.0155202364541491 | 0.0409872744788163 | 0.378659880450712 | 0.621340119549288 |

Native summary:

```text
mean_g_dyn_sq = 0.43984307738805
mean_rel_err  = 0.56015692261195
max_rel_err   = 0.621340119549288
pass=false
```

Postulate-W control arm:

| r | alpha_r | Phase-G expected | g_dyn^2 | rel_err |
|---:|---:|---:|---:|---:|
| 5 | 0.00179794099308269 | 0.00420828449050722 | 0.0195891230620903 | 0.572761538071304 |
| 7 | 0.00153075279439938 | 0.00298025844014337 | 0.0235502643256024 | 0.486369110215240 |
| 9 | 0.000711611544711212 | 0.00187928951930177 | 0.0173617678599000 | 0.621340119549219 |

Control summary:

```text
mean_g_dyn_sq = 0.0201670517491976
mean_rel_err  = 0.560156922611921
max_rel_err   = 0.621340119549219
pass=false
```

Adjudication:

```text
native_phase_g_unit_match=false
native_postulate_w_match=false
native_alpha_rel=8.59297051111601
control_match=false
verdict=INVALIDATED_PROTOCOL_OR_ENGINE_DRIFT
```

---

## 3. Interpretation

The run did not find a dynamical alpha result.

It also did not validate the FTD-0285 absolute Phase-G gate. Both arms miss the
pre-registered `2 r G_L(r)` absolute expectation by the same relative pattern.
The control arm scales almost exactly with the explicit inserted coupling:

```text
mean_g_dyn_sq(control) / mean_g_dyn_sq(native)
= 0.0201670517491976 / 0.43984307738805
= 0.0458505607703448
= postulate_w_g^2
```

So the engine's `coulomb_charge_coupling` scaling behaves as expected, but the
finite live-engine protocol at `L=32`, `ticks=300`, `SOR=100`, and `r={5,7,9}`
does not realize the absolute analytic Green-function normalization used as
the pass gate.

The honest result is therefore:

```text
FTD-0285 = INVALIDATED PROTOCOL
```

Not:

```text
DYNAMICAL-FOUND
NATIVE-NULL
POSTULATE-W-CONFIRMED-AS-DERIVATION
```

---

## 4. What We Learned

1. The leak guard was necessary. The old Phase-H helper leaves
   `toggles.coupling = true` unless callers override it, which would inject
   `sqrt(alpha)` through `phase_read`.

2. The explicit Gauss-source coupling remains a pure scaling knob in this
   protocol. The Postulate-W arm is exactly a matching insertion, not a
   derivation.

3. The live finite-cell dynamic estimator is not interchangeable with the
   analytic Phase-G Green-function normalization without an additional
   equilibration/estimator validation step.

4. FTD-0284 remains the right discriminator. FTD-0285 failed as its first
   engine instrument.

---

## 5. Next Correct Move

Do not tune FTD-0285.

A v2 must be pre-registered before any run and should separate two tasks:

- First validate the live-engine estimator against a control whose expected
  value is native to the same dynamic protocol.
- Only then ask whether a no-alpha arm lands on the master-quadratic
  normalization.

No claim changes:

| Claim | Status after FTD-0285 |
|---|---|
| `x_+ = 1/alpha` physical identification | unchanged: [STRONGLY MOTIVATED CONJECTURE] |
| FTD-0284 discriminator | unchanged: [PRE-REGISTRATION -- LOCKED] |
| FTD-0285 engine probe | [INVALIDATED PROTOCOL] |
| Postulate-W `g_W^2 = 2*pi/x_+` | explicit matching control only, not a derivation |
