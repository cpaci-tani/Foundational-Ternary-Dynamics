# AUDIT — Half-tick link exchange

**Date:** 2026-07-24  
**Identifier:** `FTD-0451`  
**Status:** `[CONSTRUCTIVE EXAMPLE — REVERSIBLE SELECTED EXCHANGE LEDGER]` + `[CORRECTED BY FTD-0452 — ENERGY LABEL]` + `[OPEN — FIELD MOMENTUM]`  
**Verdict:** `REVERSIBLE_HALF_TICK_LINK_LEDGER_CONSTRUCTED_NOT_DYNAMICS`  
**Pre-registration:** [`PREREG_HALF_TICK_LINK_EXCHANGE_v1.md`](../10_eft_program/preregistrations/PREREG_HALF_TICK_LINK_EXCHANGE_v1.md)  
**Run of record:** `engine/results/ftd_0451/windows_msvc_cpu.csv`

## 1. Constructed object

FTD-0451 packages the corrected FTD-0450 selected finite-work map into an
oriented exchange record located at half tick `n+1/2`. Each record contains:

- one of 13 unoriented Moore channels and an orientation sign;
- particle momentum before and after;
- equal-and-opposite field momentum exchange;
- particle work `W` and a provisionally named field-energy entry `-W`;
- integer doubled time `2n+1`.

The link representation avoids FTD-0448's source-versus-target placement
ambiguity: the exchange belongs to the transition itself.

All 26 directed Moore displacements reconstruct exactly from channel plus
orientation. Reversal uses the same channel with the opposite sign.

## 2. Closure results

For the registered production momentum family and work `1e-4`:

- local particle-plus-link momentum residual: exactly zero;
- local particle-plus-link energy residual: `1.10182e-17`;
- independently recomputed reverse particle round trip: `2.94392e-17`;
- forward/reverse link-momentum cancellation: `2.94392e-17`;
- forward/reverse link-energy cancellation: exactly zero.

The complete record is cubic covariant. All `26*48=1248` transformed momentum,
recoil, and channel reconstruction tests pass with zero worst covariance
residual.

This proves that a local reversible bookkeeping object can be constructed
without assigning recoil to either integer-time endpoint. FTD-0452 later
showed that the `-W` energy entry is the change in interaction Hamiltonian for
fixed `J`; it is not an additional wave-field deposit.

## 3. What this does not prove

The ledger contains a required field-momentum recoil and a now-corrected
interaction-energy entry. No production field presently owns the recoil. It
does not:

- change or propagate `Voxel::flux` or `wave_vel`;
- supply a Hamiltonian for 13 channel amplitudes;
- show that the three-vector `J/W` field can realize the exchange;
- derive the preserved-transverse or branch selection in the finite map;
- couple successive link records into a persistent wave;
- change the frozen production movement event measured by FTD-0449.

Therefore its momentum closure is not physical conservation. It is a
necessary data contract for testing candidate recoil realizations. Its energy
closure is superseded by FTD-0452's particle-plus-interaction identity.

## 4. Ontological consequence

A clean discrete picture is now available:

1. manifested state occupies a site at integer tick;
2. a transition occupies an oriented Moore link at the half tick;
3. work and recoil belong to that transition;
4. the next manifested state occupies the target site;
5. reversal conjugates the oriented link record.

This makes the temporal object a transaction rather than another spatial
voxel. It is compatible with the user's proposed distinction between physical
site geometry and a temporal/BCC-like transition domain, but it does not derive
that identification. The 4 corner channels are only a geometric BCC direction
class at this stage; all 13 link channels participate in the constructed
ledger.

## 5. Next decisive gate

FTD-0452 removes the spurious demand that `J/W` also absorb `-W`. The next
campaign must attempt to realize only the equal-and-opposite momentum in a
field with a genuine state and momentum generator. Two candidates remain:

1. **Three-vector realization:** find a local update of site-centered `J/W`
   whose measured field momentum equals the ledger and which reverses exactly
   without adding uncompensated tick energy.
2. **Thirteen-channel realization:** give each Moore link channel a conjugate
   pair, define a cubic-invariant Hamiltonian, and show that its first moment
   reduces to `J/W` while retaining the FTD-0446 kernel energy.

Failure of the three-vector realization would be evidence that `J` is only a
coarse resultant. Success of a 13-channel realization would still constitute
an ontology extension, not frozen-native emergence.

## 6. Reproducibility

- campaign SHA256: `efa7f7e897765e039e518da439d0013c85ecce1e7a24c5dcca107f6a904a5d51`
- helper SHA256: `eeebe9622acb6a23a87e78ecf68884a58fc3b655000615da5e531c07fef5baf8`
- record SHA256: `6c980f334b50ad7606470713b34ae434682b56809701ae0d93dffdc19f9ffd00`
- compiler: pinned MSVC `14.44.35207`, Release
- execution: algebraic observer, no production tick
- result: `REVERSIBLE_HALF_TICK_LINK_LEDGER_CONSTRUCTED_NOT_DYNAMICS`

No production dynamics were changed.
