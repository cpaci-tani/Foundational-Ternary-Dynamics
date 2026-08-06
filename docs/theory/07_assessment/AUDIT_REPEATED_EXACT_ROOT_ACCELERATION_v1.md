# Audit — repeated exact-root acceleration v1

**Ledger ID:** FTD-0651  
**Verdict:** `REPEATED_EXACT_ROOT_ACCELERATION_CONSTRUCTIVE`

All 12 preregistered width/direction/speed arms are present. Width-two arms
contain three forward and three reverse solves under both methods; widths
three and four contain paired one-step forward/reverse roots. Complete states,
not reduced observables, agree within `2.16538e-11`. Both methods satisfy the
exact action and invert within the registered limits.

The evaluation-count gate passes narrowly (`884 < 955`) only for the repeated
width-two histories. Summed wall time over all arms is approximately `249.01`
seconds matrix-free versus `3347.87` seconds cached because dense Jacobian
initialization dominates one-step widths three and four. The preregistration
explicitly excluded wall time as a verdict gate. Accordingly, the registered
constructive verdict is valid but must not be paraphrased as general speed
superiority.

Run-of-record hashes:

- protocol: `06371B4E788FBB3E2840875340557F620617C593D686E8C410ECFE341266298A`;
- runner: `88897AE09016A598A34F0D5EA0F3685240808787199832ED18B6220A8D9C847B`;
- JSON: `2B3BE6C836E09FA015A0D9D7333135E19A9ECCB15255C336226573235C0F6BB3`;
- arm CSV: `366A2E2221C3CB83C8343360723B4ED83F4FAD3722DE3F231F5337A4B0AD5156`;
- independent certificate: `143C5FFEA8A8953D40A2F1DCA94AE5A0914D09C79D8DCAE0D69A2110006DC86E`.

The only licensed successor is a checkpointed long-history run that tests
whether cache initialization is actually amortized. No physical matter claim
changes under FTD-0651.
