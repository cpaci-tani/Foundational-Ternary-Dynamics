# AUDIT — Native energy-contract reconciliation

**Date:** 2026-07-24  
**Identifier:** `FTD-0452`  
**Status:** `[THEOREM — FIXED-J HOP ENERGY CLOSURE]` + `[CORRECTION — FTD-0451 ENERGY LABEL]` + `[DIAGNOSTIC DEFECT]`  
**Verdict:** `ENERGY_CONTRACT_RECONCILED_DIAGNOSTICS_INCOMPLETE`  
**Pre-registration:** [`PREREG_NATIVE_ENERGY_CONTRACT_RECONCILIATION_v1.md`](../10_eft_program/preregistrations/PREREG_NATIVE_ENERGY_CONTRACT_RECONCILIATION_v1.md)  
**Run of record:** `engine/results/ftd_0452/windows_msvc_cpu.csv`

## 1. Fixed-field hop energy closes without wave deposition

The declared interaction is

```text
L_int = +G_C sum_x s_x div(J)_x.
```

For a fixed-field hop from `a` to `b`, FTD-0443 proved

```text
W = Delta L_int = G_C q [div(J)_b - div(J)_a].
```

The corresponding interaction Hamiltonian contribution is
`H_int=-L_int`, so

```text
Delta H_int = -W.
```

The FTD-0450 selected particle update is constructed to satisfy
`Delta E_particle=+W`. Therefore

```text
Delta E_particle + Delta H_int = 0.
```

The registered `W=1e-4` fixture closes this identity to
`1.10182e-17`. No independent wave-field energy deposit is required by this
fixed-`J` transaction.

FTD-0451 named an additional `-W` entry `field_energy_exchange`. Adding that
entry after the interaction energy has already changed leaves residual
`-0.00010000000000001102`: the whole exchange is counted twice. The FTD-0451
entry is therefore reclassified as an interaction-potential ledger entry, not
demonstrated energy stored in `J/W`. Its momentum entry remains an open recoil
requirement.

## 2. `total_hamiltonian` omits the field sector

`compute_lagrangian_diagnostics()` separately accumulates field kinetic and
gradient terms, but its `total_hamiltonian` adds only
`hamiltonian_density(v,divJ,rho)`. That density contains Born-Infeld,
coupling, velocity coupling, and Gauss terms; it contains neither field
kinetic nor field gradient energy.

The frozen fixtures make the omission observable:

- empty-vacuum diagnostic baseline: `124.17299999999911`;
- add `7.47509765625` field kinetic energy: diagnostic change exactly `0`;
- add `1.5687772877104338` field gradient energy: diagnostic change exactly
  `0`.

Consequently the former pure-wave “Hamiltonian conservation” section in
`test_action_stationarity.cpp` did not test wave energy. It compared an
unchanged non-field baseline before and after the wave evolution. FTD-0452
replaced that vacuous check with the exact modified tick-energy invariant.

## 3. The amplitude norm is not the wave Hamiltonian

`EnergyAudit::field_energy` reports

```text
0.5 sum_x |J_x|^2.
```

That is an amplitude norm. The source-free production tick is governed by
spatial differences and preserves

```text
E_tick = 0.5 W^T W + 0.5 J^T KJ - 0.5 W^T KJ.
```

A uniform field and a transverse patterned field were normalized to the same
amplitude norm. `EnergyAudit::field_energy` agreed to `8.44e-15`, while their
gradient energies were `0` and `1.5687772877104338`. Thus the amplitude
diagnostic cannot support a field-energy conservation claim for the wave
equation.

The independent FTD-0452 implementation of the exact modified invariant
drifted by at most `8.22e-14` absolute and `7.69e-14` relative over 64 exact
production ticks. FTD-0293's invariant, not the amplitude norm, remains the
valid source-free energy contract.

## 4. Intuitive questions now answered

1. **When a particle rolls downhill in the interaction landscape, where does
   its kinetic energy come from?** From the decrease in `H_int=-L_int`.
2. **Must the wave field also lose the same energy at that instant?** No; doing
   so double-counts the fixed-field exchange.
3. **Can a number called total Hamiltonian ignore a moving field?** The current
   diagnostic does; it is therefore not a total Hamiltonian.
4. **Can two fields have the same amount of `J` but different wave energy?**
   Yes. Uniform and corrugated fields can have equal amplitude norm and
   different gradient energy.
5. **What makes the exact discrete energy unusual?** The time-ordered tick adds
   the cross term `-0.5 W^T KJ`; the continuum-looking kinetic-plus-gradient
   expression alone is not exactly invariant.
6. **What remains unbalanced after energy is fixed?** Momentum. The interaction
   supplies energy bookkeeping but no demonstrated local `J/W` recoil state.
7. **Does this make the half-tick link useless?** No. It still locates the
   transition and its required recoil; only the energy label was wrong.

## 5. Consequence for the next gate

The next campaign must not force `J/W` to absorb `-W`. It must ask the narrower
question: can a local field update carry the equal-and-opposite momentum while
the combined particle-plus-interaction energy remains closed? If nonzero recoil
necessarily creates wave tick energy, a compensating redistribution or a
larger local support is required; assigning a second `-W` ledger entry is not a
solution.

## 6. Reproducibility

- campaign SHA256: `38118dea47e93b2082da0c865990f00dd39644b8fe3fd4bf7d2a93d59cc2983c`
- helper SHA256: `3db8f2dc573e7f4a87e17409878915e7b5a52ce1673713998c544516e0175621`
- record SHA256: `0ecce64c78fb4672f1f0538fe56a72df8898a41c4cb6fee1bbee24de673d58fe`
- compiler: pinned MSVC `14.44.35207`, Release
- production dynamics changed: no
