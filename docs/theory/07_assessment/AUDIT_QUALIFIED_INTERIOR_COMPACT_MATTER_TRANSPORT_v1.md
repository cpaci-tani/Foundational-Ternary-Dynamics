# Audit — FTD-0608 qualified-interior compact matter transport v1

**Status:** `[AUDIT — STRICT SITE CHART BLOCKS COMPLETE TRANSPORT; DYNAMICS
VERDICT UNRESOLVED]`
**Verdict:** `QUALIFIED_INTERIOR_COMPACT_MATTER_NUMERICALLY_UNRESOLVED`

## Reproducibility record

- protocol SHA-256:
  `B64BB90EF082EC8E47BE83BA1F9951D7B30C3C5904AE8E4C639B33543020C5E0`;
- runner: `engine/tests/test_qualified_interior_compact_matter_transport.cpp`;
- certificate:
  `scripts/proofs/proof_qualified_interior_compact_matter_transport.py`;
- JSON/CSV: `engine/results/ftd_0608/`;
- focused CTest and independent certificate: pass.

## Gate disposition

| gate | result |
|---|---|
| phase-15 static fingerprint | exact |
| static stability and field | pass |
| integer translation covariance | exact |
| `v=1/64` clean forward prefix | four ticks |
| `v=1/32` clean forward prefix | two ticks |
| prefix common action / energy | pass near machine precision |
| first rejected candidate | one predicted duplicate anchor in each arm |
| common nominal boundary time | `1/16` cell in both arms |
| complete forward histories | not covered |
| state-only inverse histories | not reached |
| locked complete-motion verdict | unresolved |

## Audit conclusion

The experiment isolates a representational discontinuity. The dynamic
equations remain clean until the next effective-position update maps two
distinct records into one integer anchor. Because strict site validity is
part of candidate construction, Newton receives no admissible starting point.

Calling this a physical failure would conflate the coordinate chart's capacity
rule with the common-action equations. Calling it constructive motion would
ignore the missing histories. The only licensed conclusion is that the strict
one-record anchor chart is insufficient for this trajectory.

