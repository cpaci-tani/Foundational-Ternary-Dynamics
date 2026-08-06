# FTD-0662 — Internal-mode action-transfer ledger v3

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; observer-only covariance correction  
**Parent v2 JSON:**
`45E0AFAB3E986C72A06252087DDB06F662754964013109E92A205AD37C22C421`

## 1. Single correction

FTD-0661 fixes the redressing-observer floor and adds the correct four-vector
tight frame, but its cyclic sum remains non-covariant because every arm fixes
maximum **constituent displacement**, not generalized modal amplitude. The
maximum component of a unit doublet vector depends on its direction and on the
arbitrary eigenbasis. Consequently the four quadratic histories enter the v2
sum with unequal weights, so the sum is not proportional to `Tr(I_2)`.

V3 changes only the tight-frame covariance observer. Before summing, divide
every quadratic ledger history by its own initial doublet energy

\[
E_2(0)=\frac12\left(|p(0)|^2+\omega^2|q(0)|^2\right).
\]

For the locked momentum quadratures this is proportional to the squared
generalized modal amplitude. The normalized tight-frame sum is therefore
basis invariant.

## 2. Frozen protocol

Retain unchanged from FTD-0661:

- both cyclic orientations;
- the four tight-frame polarizations;
- amplitudes `4e-6,8e-6` and quadratures `pi/2,3pi/2`;
- `32` nonzero plus two zero arms;
- `128` forward and `128` state-only inverse ticks;
- instantaneous dressing, dynamic residual, interference, and shell observers;
- zero gate `1e-14`;
- every exact, transfer, amplitude, sign, morphology, and `5%` covariance
  threshold;
- every verdict name and physical scope boundary.

The tight-frame covariance residual is computed from histories normalized by
`E_2(0)` arm by arm and must be `<=0.05`. V1 and v2 verdicts are immutable.
V3 is a fresh execution.

Even a constructive result means only that a prepared internal constituent
excitation transfers energy into an outward-spreading dynamic residual field
on a finite periodic lattice. It does not establish irreversible decay,
asymptotic radiation, a photon, a lifetime, or an infinite-volume resonance.
