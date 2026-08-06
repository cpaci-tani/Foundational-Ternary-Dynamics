# Audit — FTD-0716 period-three co-moving-field solvability

**Verdict:** `[AUDIT PASS — REGULAR FINITE-VOLUME SOLUTION]`

The C++ observer reconstructs the FTD-0715 trajectory and emits the affine
three-tick field source from 48 exact face-current segments. The independent
Python certificate hashes all inputs, applies the complete `6x6` Fourier SVD
at every `L=33` momentum, and checks source norm, reality, Parseval equality,
left-null projection, minimum-norm reconstruction, conditioning, and the final
real-space residual.

The source has zero incompatible modes to numerical precision. The status is
still a finite-volume numerical fact: no volume ladder or infinite-lattice
limit has been run. More importantly, source compatibility does not imply
reciprocity. The minimum-norm field has not yet been shown to supply the exact
matter recoil or energy exchange, and the common action has not generated the
registered cycle.

The result closes the field-existence gate for this `L=33`, period-three
candidate. It does not qualify mobile matter.

FTD-0717 subsequently checks the previously unmeasured absolute Gauss
condition and finds maximum residual `1.89e-15`. This upgrades the registered
field to a Gauss-realizable translated solution, while leaving reciprocity
negative for the minimum-norm representative.
