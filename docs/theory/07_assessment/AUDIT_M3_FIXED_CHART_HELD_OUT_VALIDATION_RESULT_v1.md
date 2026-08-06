# FTD-0758 — M3 fixed-chart held-out validation result v1

**Status:** `[CONSUMED — INFRASTRUCTURE UNRESOLVED; NO CERTIFIED PHYSICS VERDICT]`  
**Date:** 2026-07-30  
**Protocol:** `PREREG_M3_FIXED_CHART_HELD_OUT_VALIDATION_v1.md`  
**Certificate:** `scripts/proofs/proof_m3_fixed_chart_held_out_validation.py`

## Official verdict

All six registered CUDA modes ran exactly once and produced the complete
locked record:

```text
24 files = 12 CSV + 12 JSON
FTD-0758 artifact: 6239/6257 checks
18 failures: candidate summary parity
```

The frozen certificate does not emit a physics verdict after a nonzero exit.
Under the preregistered first-failed map this is

```text
M3_VALIDATION_INFRASTRUCTURE_UNRESOLVED
```

FTD-0758 is consumed. Its runner, certificate, artifacts, tolerances, and
summaries are not repaired or rerun under this identifier.

## Exact certificate-contract defect

Every failed check has the same cause. The candidate summaries record pass,
while the inherited certificate reconstructs each volume pass using the
unconditional conjunct

```text
site_projection_valid == 1.
```

In this record `site_projection_valid` means that the two constituent records
have distinct integer site anchors. It is false on 2,230 of 2,736 dynamic
candidate rows, beginning at tick 161 in every candidate/volume arm.

That is not the selected action-validity rule. The frozen runner inherits

```text
allow_shared_anchor_chart = true
```

and the engine accepts a transaction according to

```text
allow_shared_anchor_chart || site_projection_valid.
```

The two polarities may therefore share one integer anchor while retaining
distinct continuous remainders. `step.valid` and
`common_action_gates_pass` correctly use the conditional rule. The runner's
history summary uses those accepted transaction bits. The inherited
certificate instead treats the unique-anchor diagnostic as mandatory and so
contradicts the selected shared-anchor sector it is certifying.

The defect is in the pre-execution certificate contract. It is not a failed
continuity, Gauss, energy, recoil, speed, graph-locality, root, observer, or
support-ladder calculation. Because it was discovered only after registered
output existed, changing the certificate now cannot rescue FTD-0758.

## Post-hoc diagnostic facts — not a validation verdict

After the certificate failure was fixed as the official outcome, a read-only
diagnostic isolated the excluded conjunct. The immutable artifacts show:

- all 18 candidate histories remain members of the support-independent core
  predicate through tick 312;
- all graph and internal-energy margins exceed the frozen strict gates;
- all accepted common-action, energy, recoil, causal-speed, root-regularity,
  state-only observer, and `{4,6,8}` ladder gates pass;
- every nested-volume comparison passes with zero class/branch mismatches;
- all six baseline/remote causal-fibre comparisons pass, with nonzero initial
  global environmental-energy difference and zero registered pre-contact
  local/core/constituent/bound differences.

The extrema reconstructed from the candidate CSVs are:

| ray | minimum graph margin | minimum energy margin | maximum common residual | minimum singular value | maximum condition number |
|---|---:|---:|---:|---:|---:|
| face | `0.10257193193124747` | `0.0006705241964946046` | `4.549485788096774e-14` | `0.9564714382959042` | `1.0876072124779779` |
| edge | `0.09559354649937535` | `0.0005786897052038431` | `5.106895809015022e-14` | `0.951750008083017` | `1.0874517550146936` |
| body | `0.10257675777681441` | `0.0007660270835095281` | `5.303049666061099e-14` | `0.9562593109097131` | `1.0875424419776514` |

Replacing the certificate's unconditional diagnostic with the already-frozen
engine rule would make the artifacts satisfy the final selected-family branch.
That statement is explicitly post hoc. It is evidence for successor design,
not `M3_FINITE_TIME_SELECTED_MATTER_FAMILY` under FTD-0758.

## Ontological consequence

This failure clarifies the representation rather than matter dynamics.
Manifested polarity remains site-valued, but a two-constituent object is not
required to project injectively onto two distinct sites at every tick. Its
continuous constituent remainders and reciprocal field/current transaction
can carry relational separation while both records occupy one site chart.

Consequently, `site_projection_valid` is a unique-anchor diagnostic, not a
universal physical-validity predicate. A successor must freeze a
chart-admissibility field such as

```text
chart_admissible = allow_shared_anchor_chart || site_projection_valid
```

and require its runner summary and independent certificate to reconstruct the
same conditional rule. Fresh held-out evidence is still required; the now
inspected FTD-0758 histories cannot be relabelled unseen.

No production default, action, current, field update, predicate, scenario, or
ontology was changed by this audit.
