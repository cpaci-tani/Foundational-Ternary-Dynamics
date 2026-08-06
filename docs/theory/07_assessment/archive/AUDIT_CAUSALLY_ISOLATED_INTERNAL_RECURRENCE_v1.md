# Audit — causally isolated internal recurrence v1

**Ledger ID:** FTD-0668  
**Status:** `[RETRACTED — MODAL MASS-METRIC ERROR, FTD-0675]`  
**Historical verdict:** `CAUSALLY_ISOLATED_INTERNAL_RECURRENCE_MIXED`

> FTD-0675 invalidates the modal recurrence/turning observable. Exact causal-
> buffer and field-morphology facts are retained, but no recurrence verdict
> survives.

The protocol was locked before implementation at
`FD959EADB5B50D237D78929295A45BC507DE37843DECA151705856F2359FA70C`.
The run uses `L=97`, horizon 80, a radius-8 source bound, and conservative
self-contact tick 81. Initial excited/control fields are bitwise equal. The
observed maximum deposited-current radius is 4.

All execution gates pass. Dense and sparse current storage agree exactly in
the locked preflight. Both polarity arms contain ticks `0..80`; complete
energy drift is at most `1.07e-14`, common residual at most `5.32e-13`, and
full recoveries are below `1.11e-12`.

The physical verdict is nevertheless mixed. The doublet minima occur at tick
72 but stay above the locked `0.60` prerequisite, at approximately `0.63128`.
Thus neither a registered return nor the registered no-return branch exists.
The subsequent rise to approximately `1.32374` at tick 80 and the simultaneous
outward field morphology are descriptive evidence only. They may motivate a
fresh turning-point test but cannot retroactively replace the threshold.

Run-of-record hashes:

- runner: `92314D0F21BE50365E5BC5198912D1470EC334393651FB889F7CCA6EBF595870`;
- JSON: `D1EF53978C9B04F9EEC2FF34954D7D04CA9163AAE6FAD6833D7CCF352CEAE0D2`;
- tick CSV: `E34AC8AAE7FC703B037D9F1B730A2A97213419A9A5D01996D5C9716999256FDB`;
- independent certificate: `BF3A4B3B037C0E937A430BAD25B818CA3CAB2EA8E9A14158668E0F67422D5F63`.

No production defaults or ontology tags move under this mixed result.
