# Audit — native excited matter clock v1

**Ledger ID:** FTD-0659  
**Verdict:** `[SELECTED DYNAMICS — MIXED]`  
**Production status:** unchanged

The protocol was locked under SHA-256
`FF9566F6D6B7BCAEB7970359043C62F643A6A8315AF43C01EE0C5CFD21ECC342`
before the runner was implemented. The parent FTD-0640 JSON is independently
hash-locked as
`AB43D342CFE48BEF452955E56B1EDC34F9EE51911F7D899932E7E542877E6B9A`.

All `74` arms execute, remain bounded, conserve complete energy, and invert.
Phase, quadrature, amplitude, polarization, cyclic-covariance, and exact-zero
controls pass. The locked `2%` action-stability gate fails decisively: maximum
relative doublet-action drift is `0.898691`.

The independent certificate
`scripts/proofs/proof_native_excited_matter_clock.py` recomputes the action,
doublet support, phase advance, phase RMS, amplitude scaling, quadrature
history, covariance, zero controls, and mixed verdict from the raw CSV records.
Its SHA-256 is
`A4C815AB901A3B8E54F4A7D9115EBC1AC53943F1B0ABF236AFFE5FF85E583A04`.

Artifact hashes:

- runner: `2C041BE29551C863BCB7C667C518E30B34D85A3D3E29F08EA18258569D918DFA`;
- JSON: `DB6CA66770812E4C8FC94411B109F23E424FFF1CE3173A5D16AB43B5949ACEEE`;
- arm CSV: `4F7D2E38B0FE4D6EF33F137E2AA753E4143B3AD541F6934CF39FD11772844941`;
- tick CSV: `4EF51456F161E6CD836518B72EBAACE4A5007F5EF5525E07CD097B343566634A`.

The correct interpretation is narrower than “matter has an internal clock.”
The registered constituent doublet has a robust coherent phase but not a
conserved conjugate action, so it is not an autonomous action--angle degree of
freedom. Exact total-energy conservation makes transfer into other complete-
state degrees of freedom the live explanation; the destination was not a
locked observer and is not promoted to radiation without a successor test.
