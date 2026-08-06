# Audit — cell-measure long-horizon transport v2

**Ledger ID:** FTD-0652  
**Verdict:** `CELL_MEASURE_LONG_HORIZON_MIXED`

The 30-arm matrix and all 2,880 forward/reverse tick records are complete.
Every checkpoint carries the locked protocol hash. Exact action, coherence,
zero, polarity-mirror, cubic, transport-persistence, anisotropy, and
spline-defect gates pass. The minimum-mobility monotonic gate fails, so the
registered mixed verdict is mandatory.

The failure must not be described as loss of motion. All nine high-speed arms
are persistent, and the minimum mobility remains near one. The failure is the
sign of its width trend: `1.03224 > 1.00637 > 0.994804`, whereas the protocol
required nondecrease. The registered anisotropy span and maximum translation
defect both shrink strongly.

The logically natural target-centred mobility error was not preregistered.
Although it decreases strongly in the observed data, it is post-hoc evidence
and cannot upgrade FTD-0652. A successor must use new arms or widths and lock
that criterion before viewing their results.

This campaign establishes a finite-horizon exact/coherent mobile solution of
the selected cell-measure action. It does not establish a continuum limit,
pole, physical particle, native formation mechanism, conserved effective
charge, or production ontology.

Run-of-record hashes:

- protocol: `1F6AB75BC11FD05D93E450029D020CDCA94B76CA7E1186A8197CC110AFFC829D`;
- runner: `9D2711F8F8B63A74B437755FF1CDD3A5DF6C7D46C36C9F602B94820AFA5893D0`;
- JSON: `DF809F90094167AF13F99A1F727869D1A0C123A9CD365AA04508497221217400`;
- arm CSV: `E8828D4E147C20FA31911B90ACF33A357DF31A684A3E87F0A7842422F07E609E`;
- independent certificate: `2CE85557D5287184F151766A00F23C18BEE933F2C15678789DEE684537FA5714`.
