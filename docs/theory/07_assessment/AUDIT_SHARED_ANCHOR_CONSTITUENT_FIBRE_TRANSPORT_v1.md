# Audit — FTD-0609 shared-anchor constituent-fibre transport v1

**Status:** `[AUDIT — REPRESENTATION REPAIR CONSTRUCTIVE; TWO-VELOCITY
TRANSPORT CONJUNCTION CLOSED NEGATIVE]`
**Verdict:** `SHARED_ANCHOR_FIBRE_TRANSPORT_CLOSED_NEGATIVE`

## Reproducibility record

- protocol SHA-256:
  `8CA3984F9E3FF2B8BE53BBBEA20028618EACFFC54C1B361994D10AD8B95D4D95`;
- observer option: `allow_shared_anchor_chart`, default false;
- runner: `engine/tests/test_shared_anchor_constituent_fibre_transport.cpp`;
- certificate:
  `scripts/proofs/proof_shared_anchor_constituent_fibre_transport.py`;
- JSON/CSV: `engine/results/ftd_0609/`;
- focused CTest and independent certificate: pass.

## Gate disposition

| gate | `v=1/64` | `v=1/32` |
|---|---:|---:|
| full forward/reverse coverage | pass | pass |
| common action | pass | pass |
| energy drift | `1.11e-15` | `4.44e-16` |
| state recovery | `5.11e-13` | `8.88e-15` |
| site hops | 22 | 15 |
| fibre exercised | 22 states | 36 states |
| max anchor multiplicity | 2 | 2 |
| internal trimer geometry | pass | pass |
| longitudinal displacement | fail, `0.2833` | pass, `1.8784` |
| neutral-pair separation | fail, `1.0626` change | pass, `0.1505` change |
| complete arm | fail | pass |

The default-false strict regression reproduces failure ticks `4` and `2`.
All shared-anchor events occur within the second trimer, not between the two
neutralizing partners, and effective constituent positions remain distinct.

## Audit conclusion

The fibre option cleanly removes the site-chart solver obstruction. It does
not rescue the preregistered conjunction because the slower neutral-pair arm
fails macroscopic co-transport and pair-separation gates. These statements
must remain separate: the local representation repair is constructive, while
the two-velocity mobile-object claim is closed at the registered scope.

The failed slow arm does not justify a new intertrimer binding term. The test
system contains two distant charge-conjugate objects to satisfy finite-volume
neutrality, and their relative force was already phase-dependent. The next
experiment must control that neutralizer interaction before changing the
matter core.
