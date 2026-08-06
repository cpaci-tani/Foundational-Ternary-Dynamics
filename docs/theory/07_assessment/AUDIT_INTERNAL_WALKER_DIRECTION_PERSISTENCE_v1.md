# Audit — FTD-0616 internal-walker direction and persistence

**Status:** `[AUDIT — CURVED TRANSPORT; STRAIGHT DIRECTION CONTROL CLOSED]`
**Verdict:** `INTERNAL_WALKER_TRANSIENT_OR_UNCONTROLLED`

- corrected protocol prefix SHA-256: `E55D5CFA...A730B`;
- locked FTD-0615 parent SHA-256: `8B7DD580...ABA2C`;
- runner: `engine/tests/test_internal_walker_direction_persistence.cpp`;
- certificate: `scripts/proofs/proof_internal_walker_direction_persistence.py`;
- independent checks: 20/20 pass;
- run of record: `engine/results/ftd_0616/`.

The preregistration's first displayed hash was a clerical shell-escaping
error: PowerShell consumed the Markdown backtick used as the prefix delimiter.
The actual unchanged prefix hash is `E55D5CFA...A730B`. No arm, gate,
tolerance, or verdict rule changed, and the complete campaign was rerun with
correct executable metadata before certification.

All 12 arms and 12,288 transactions close. The independent certificate
reconstructs every 128-tick window, the signed vector verdict, tickwise cyclic
covariance, internal `(q,p)` parity, and the result classification from the
CSV/JSON records.

The registered straight persistence and sign-control gates both fail.
However, every window moves at least `0.916` cell and every arm reaches about
`3.311` cells, so the measured mechanism is not a stopped transient. It is
curved transport with a sign-even in-plane displacement and sign-odd axial
displacement. The result remains externally neutralized and misses a closed
pseudomomentum ledger by up to `9.13e-4`; no self-propulsion, particle, pole,
or electromagnetic claim is licensed.

