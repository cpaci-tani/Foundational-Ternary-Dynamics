# Audit — internal-mode return-time discriminator v1

**Ledger ID:** FTD-0666  
**Status:** `[RETRACTED — MODAL MASS-METRIC ERROR, FTD-0675]`  
**Historical verdict:** `[SELECTED DYNAMICS — MIXED OUT-OF-SAMPLE PREDICTION]`  
**Production status:** unchanged

> FTD-0675 invalidates the modal return observable used by this campaign. The
> historical execution is preserved, but tick 73 is not a canonical mode-
> energy return time.

Protocol SHA-256:
`4AFD79B3207C16A37EBDF96197EFCDA64ADFD5410DB0825D6085280791D8FDEC`.

Execution passes, with recoveries `1.110e-10` and `1.164e-10`. Both signs
return at tick 73, outside the locked `74..78` prediction. No tolerance is
changed and the result remains mixed.

Artifact SHA-256 values:

- runner: `E2B9BF586167F1B161D791C2EAD5DC1AEA922DDDA4744EFC139D27540E255733`;
- JSON: `E89871BA5CE26D098AFB1063BD74084E6971D4E3426CCB4907009565AA9A0749`;
- ticks: `341B20D7D81CF8464AAC945EA8D2379DE8A404ED64096CB9DEEBBDE0FFA4ED32`.

Independent certificate:
`scripts/proofs/proof_volume_scaled_internal_mode_transfer.py`, SHA-256
`7352CF6E5A5AFA5C78487506048F3F0A05CE0C77080841A7C940D39C63C9807A`.

The combined `{73,76,76}` timing rejects simple direct circumference scaling
but does not exclude subtler finite-volume influence or prove an
infinite-volume resonance.
