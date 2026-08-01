# Audit — indexed-local causal excitation separation

**Campaign:** FTD-0694  
**Audit status:** `[PASS — SCOPED MEASUREMENT]`

## Evidence integrity

- Protocol SHA256:
  `29DE3DCA11FEC2C77F5A765F89CCF4FDD06379CD23FE9A9EE73B044025B5025A`.
- JSON SHA256:
  `9D80E03709684A4847DCEC718EE1AB5662DB75F652955439697C4EAF50DD0B96`.
- CSV SHA256:
  `4667648AC9B5552C71B790203A20D3554741A19DB02B22EB2E6ECA4989CDF197`.
- The run contains 300 complete rows: two signs, 25 sampled ticks, and six
  radii. Every row is valid.
- The independent certificate
  `scripts/proofs/proof_causal_excitation_separation_indexed_local.py`
  reproduces arrivals, shell speed, spreading fits, exact gates, and scope.

## Solver scope

FTD-0692 directly compared the full-field and local-residual routes. Forward
states were bit-identical; reverse complete-state difference was
`1.77636e-15`; materialized residual difference was zero; and both routes used
195 forward residual evaluations. The indexed representation in FTD-0694
changes only lookup complexity and retains ordered deposition. Every accepted
root is rebuilt by the established complete evaluator before finalization.

FTD-0693 is retained as execution-invalid: its unindexed local lookup did not
reach tick 1 inside the registered resource window and emitted no result file.
It supplies no physical evidence.

## Claim boundary

The data establish local sourcing, ordered outward transport, linear radial
spreading of a positive field norm, continuing core-to-field transfer, polarity
agreement, and state-only inversion for the locked mode/amplitude/volume.

They do not establish a universal propagation speed, a free photon, stable
mobile matter, a literal flux strand, a wake, a pilot wave, irreversibility,
unitarity, or a continuum pole. The positive radial profile and conserved
modified-energy ledger are different observables and are not combined as one
energy budget.

