# PRE-REGISTRATION — Native energy-contract reconciliation v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0452`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0293`, `FTD-0443`, `FTD-0449`, `FTD-0451`  
**Engine artifact:** `engine/tests/campaign_native_energy_contract_reconciliation.cpp`  
**Campaign SHA256:** `38118dea47e93b2082da0c865990f00dd39644b8fe3fd4bf7d2a93d59cc2983c`  
**Helper SHA256:** `3db8f2dc573e7f4a87e17409878915e7b5a52ce1673713998c544516e0175621`

## 1. Questions

1. For fixed `J`, is the finite hop work `W=Delta L_int` already balanced by
   `Delta H_int=-W` when the selected particle update gains `+W`?
2. Does additionally assigning `-W` to the wave field double-count that same
   exchange?
3. Does `LagrangianDiag::total_hamiltonian` respond to nonzero field kinetic
   and gradient energy in particle-free, divergence-free fixtures?
4. Does `EnergyAudit::field_energy = 0.5 sum |J|^2` distinguish a uniform field
   from a spatially varying field with the same amplitude norm?
5. Does an independent implementation of the FTD-0293 modified tick energy
   remain invariant under the exact source-free production update?

## 2. Frozen fixtures

- `L=9` diagnostic fixtures, observer only.
- Empty vacuum baseline.
- Uniform nonzero `wave_vel=(A,-A/2,A/4)`, `A=0.125`, with `J=0`.
- Transverse patterned field `J_y=A cos(2 pi x/L)`, with `wave_vel=0`.
  It is divergence-free because its sole component is independent of `y`.
- Uniform `J_y` normalized to have the same `0.5 sum |J|^2` as the patterned
  fixture.
- One face-link selected particle update with work `W=1e-4`, using the
  production dispersion and FTD-0451 record.
- `L=17`, mode `n=2`, amplitude `0.05`, source-free transverse wave evolved
  for 64 production ticks with only wave propagation enabled.

## 3. Frozen observables and gates

- particle energy change minus `W`: absolute residual `<=1e-12`;
- coupling-Hamiltonian change plus `W`: absolute residual `<=1e-12`;
- particle plus coupling-Hamiltonian closure: `<=1e-12`;
- particle plus coupling plus FTD-0451's named field exchange has magnitude at
  least `0.5|W|`, establishing double counting;
- uniform-`wave_vel` field kinetic energy is positive, but
  `total_hamiltonian` equals the empty baseline to `1e-12`;
- patterned-field gradient energy is positive, but `total_hamiltonian` equals
  the empty baseline to `1e-12`;
- equal-amplitude-norm uniform and patterned `J` fixtures agree in
  `EnergyAudit::field_energy` to `1e-12`, while their exact tick-gradient
  energies differ by more than `1e-6`;
- source-free modified tick-energy maximum absolute drift is `<=1e-10` and
  maximum relative drift is `<=1e-10` over 64 ticks;
- every measured value is finite.

## 4. Locked outcomes

- `ENERGY_CONTRACT_RECONCILED_DIAGNOSTICS_INCOMPLETE`: all gates pass.
- `HOP_REQUIRES_SEPARATE_WAVE_ENERGY_EXCHANGE`: particle plus interaction
  Hamiltonian does not close, while the old two-entry particle/field ledger
  does.
- `DIAGNOSTICS_CAPTURE_CANONICAL_FIELD_ENERGY`: the diagnostic separation
  gates fail because the named diagnostics distinguish the frozen canonical
  fixtures.
- `PROTOCOL_INVALID`: non-finite values or any remaining gate failure.

## 5. Interpretation boundary

Passing the first outcome corrects the semantic label on FTD-0451's `-W`:
for fixed `J` it is interaction-potential energy, not demonstrated wave-field
energy. It also establishes that the current Hamiltonian and energy-audit
diagnostics are incomplete for conservation claims. It does not prove a full
particle-field Hamiltonian, realize recoil momentum in `J/W`, or change the
production tick.

## 6. Banned moves

- No production dynamics, fixtures, formulae, gates, or outcome labels change
  after first execution.
- No replacement of the exact tick invariant by `0.5 sum(|J|^2+|W|^2)`.
- No claim of completed matter mechanics or physical energy conservation.
